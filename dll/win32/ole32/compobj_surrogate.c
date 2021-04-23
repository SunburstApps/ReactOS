#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define COBJMACROS
#define NONAMELESSUNION

#include "ntstatus.h"
#define WIN32_NO_STATUS
#include "windef.h"
#include "winbase.h"
#include "winerror.h"
#include "winreg.h"
#include "winuser.h"
#define USE_COM_CONTEXT_DEF
#include "objbase.h"
#include "ole2.h"
#include "ole2ver.h"
#include "ctxtcall.h"
#include "dde.h"
#include "servprov.h"

#include "compobj_private.h"
#include "moniker.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(ole);

static DWORD surrogate_rot_key = 0;
static ISurrogate *process_surrogate_instance = NULL;

HRESULT revoke_registered_surrogate(void)
{
    if (process_surrogate_instance == NULL) return S_OK; // nothing to do

    HRESULT hr = S_OK;
    LPRUNNINGOBJECTTABLE rot = NULL;
    hr = GetRunningObjectTable(0, &rot);
    if (FAILED(hr)) goto out;

    hr = IRunningObjectTable_Revoke(rot, surrogate_rot_key);
    ISurrogate_Release(process_surrogate_instance);
    process_surrogate_instance = NULL;

out:
    if (rot != NULL) IRunningObjectTable_Release(rot);

    return hr;
}

static LPWSTR alloc_formatted_string(LPWSTR format, ...)
{
    va_list ap;
    va_start(ap, format);

    LPWSTR buffer = NULL;
    DWORD result = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING, (LPWSTR)&format, 0, 0, buffer, 1, &ap);
    va_end(ap);

    if (result == 0) return NULL;
    return buffer;
}

static HRESULT get_surrogate_identifier_moniker(LPMONIKER *output_moniker, INT pid)
{
    HRESULT hr = S_OK;
    LPWSTR pid_string;
    LPMONIKER base_moniker, pid_moniker;

    if (output_moniker == NULL)
    {
        hr = E_POINTER;
        goto out;
    }

    pid_string = alloc_formatted_string(L"%1!d!", pid);
    if (pid_string == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto out;
    }

    hr = CreateItemMoniker(L"!", L"COM Surrogates", &base_moniker);
    if (FAILED(hr)) goto out;
    hr = CreateItemMoniker(L"!", pid_string, &pid_moniker);
    if (FAILED(hr)) goto out;

    hr = IMoniker_ComposeWith(base_moniker, pid_moniker, FALSE, output_moniker);

out:
    if (pid_string != NULL) LocalFree(pid_string);
    if (base_moniker != NULL) IMoniker_Release(base_moniker);
    if (pid_moniker != NULL) IMoniker_Release(pid_moniker);

    return hr;
}

static HRESULT get_surrogate_signal_event(PHANDLE hEvent, INT pid)
{
    if (hEvent == NULL) return E_POINTER;

    HRESULT hr = S_OK;
    LPWSTR name = alloc_formatted_string(L"COM Surrogate: %1!d!", pid);
    if (name == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto out;
    }

    *hEvent = OpenEventW(SYNCHRONIZE, FALSE, name);
    if (*hEvent == NULL)
        *hEvent = CreateEventW(NULL, FALSE, FALSE, name);

    if (*hEvent == NULL)
        hr = HRESULT_FROM_WIN32(GetLastError());

out:
    if (name != NULL) LocalFree(name);
    return hr;
}

/***********************************************************************
 *           CoRegisterSurrogate [OLE32.@]
 */
HRESULT WINAPI CoRegisterSurrogate(ISurrogate *surrogate)
{
    if (process_surrogate_instance != NULL) return E_FAIL;
    if (surrogate == NULL) return E_POINTER;

    HRESULT hr = S_OK;
    LPMONIKER moniker;
    IRunningObjectTable *rot;
    IUnknown *pUnk;
    HANDLE hEvent;

    hr = get_surrogate_identifier_moniker(&moniker, GetCurrentProcessId());
    if (FAILED(hr)) goto out;
    hr = GetRunningObjectTable(0, &rot);
    if (FAILED(hr)) goto out;

    hr = ISurrogate_QueryInterface(surrogate, &IID_IUnknown, (void **) &pUnk);
    if (FAILED(hr)) goto out;

    hr = IRunningObjectTable_Register(rot, 0, pUnk, moniker, &surrogate_rot_key);
    if (FAILED(hr)) goto out;

    process_surrogate_instance = surrogate;
    ISurrogate_AddRef(surrogate);

    hr = get_surrogate_signal_event(&hEvent, GetCurrentProcessId());
    if (FAILED(hr)) goto out;
    SetEvent(hEvent);

out:
    if (rot != NULL) IRunningObjectTable_Release(rot);
    if (moniker != NULL) IMoniker_Release(moniker);
    if (pUnk != NULL) IUnknown_Release(pUnk);
    if (hEvent != NULL) CloseHandle(hEvent);

    return hr;
}

/***********************************************************************
 *           CoRegisterSurrogateEx [OLE32.@]
 */
HRESULT WINAPI CoRegisterSurrogateEx(REFGUID guid, void *reserved)
{
    FIXME("(%s %p): stub\n", debugstr_guid(guid), reserved);

    return E_NOTIMPL;
}

HRESULT get_surrogate_classobject(REFCLSID clsid, REFIID iid, LPVOID *ppv)
{
    APARTMENT *apt;
    HKEY appIdKey = NULL;
    LPSTR buffer = NULL, expanded = NULL;
    HANDLE hEvent = NULL;
    IMoniker *moniker = NULL;
    IUnknown *pUnk = NULL;
    IRunningObjectTable *rot = NULL;
    ISurrogate *surrogate = NULL;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    LRESULT status;
    DWORD size, type;

    // Don't try to look up a surrogate if one is set for this process, otherwise we'd infinitely recurse.
    if (process_surrogate_instance != NULL) return E_FAIL;

    if (!(apt = apartment_get_current_or_mta()))
    {
        ERR("COM was not initialized\n");
        return CO_E_NOTINITIALIZED;
    }

    HRESULT hr = COM_OpenKeyForAppIdFromCLSID(clsid, GENERIC_READ, &appIdKey);
    if (FAILED(hr)) goto out;

    status = RegGetValueA(appIdKey, NULL, "DllSurrogate", RRF_RT_REG_EXPAND_SZ | RRF_RT_REG_SZ | RRF_NOEXPAND, &type, NULL, &size);
    if (status != STATUS_SUCCESS) { hr = HRESULT_FROM_WIN32(status); goto out; }

    buffer = (LPSTR)calloc(size, sizeof(CHAR));
    status = RegGetValueA(appIdKey, NULL, "DllSurrogate", RRF_RT_REG_EXPAND_SZ | RRF_RT_REG_SZ | RRF_NOEXPAND, &type, buffer, &size);
    if (status != STATUS_SUCCESS) { hr = HRESULT_FROM_WIN32(status); goto out; }

    if (strlen(buffer) == 0)
    {
        static const char system_dllhost[] = "%SystemRoot%\\system32\\dllhost.exe";
        buffer = realloc(buffer, (ARRAY_SIZE(system_dllhost) - 1) * sizeof(CHAR));
        strcpy(buffer, system_dllhost);
    }

    int expanded_size = ExpandEnvironmentStringsA(buffer, NULL, 0);
    if (expanded_size == 0) { hr = HRESULT_FROM_WIN32(GetLastError()); goto out; }
    expanded = (LPSTR)calloc(expanded_size, sizeof(CHAR));
    expanded_size = ExpandEnvironmentStringsA(buffer, expanded, expanded_size);
    if (expanded_size == 0) { hr = HRESULT_FROM_WIN32(GetLastError()); goto out; }

    hr = get_surrogate_signal_event(&hEvent, pi.dwProcessId);
    if (FAILED(hr)) goto out;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    // No other STARTUPINFOW fields need to be set AFAICT.

    if (CreateProcessA(expanded, "", NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto out;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    WaitForSingleObject(hEvent, INFINITE);

    hr = get_surrogate_identifier_moniker(&moniker, pi.dwProcessId);
    if (FAILED(hr)) goto out;
    hr = GetRunningObjectTable(0, &rot);
    if (FAILED(hr)) goto out;

    hr = IRunningObjectTable_GetObject(rot, moniker, &pUnk);
    if (FAILED(hr)) goto out;

    hr = IUnknown_QueryInterface(pUnk, &IID_ISurrogate, (void **)&surrogate);
    if (FAILED(hr)) goto out;

    hr = ISurrogate_LoadDllServer(surrogate, clsid);
    if (FAILED(hr)) goto out;

    hr = COM_GetRegisteredClassObject(apt, clsid, CLSCTX_LOCAL_SERVER, (IUnknown **)ppv);

out:
    if (appIdKey != NULL) RegCloseKey(appIdKey);
    if (buffer != NULL) free(buffer);
    if (expanded != NULL) free(expanded);
    if (hEvent != NULL) CloseHandle(hEvent);
    if (moniker != NULL) IMoniker_Release(moniker);
    if (rot != NULL) IRunningObjectTable_Release(rot);
    if (pUnk != NULL) IUnknown_Release(pUnk);
    if (surrogate != NULL) ISurrogate_Release(surrogate);

    return hr;
}

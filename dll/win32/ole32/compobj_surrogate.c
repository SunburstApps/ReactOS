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

HRESULT revoke_registered_surrogate(void)
{
    if (process_surrogate_instance == NULL) return S_OK; // nothing to do

    HRESULT hr = S_OK;
    LPRUNNINGOBJECTTABLE rot = NULL;
    hr = GetRunningObjectTable(0, &rot);
    if (FAILED(hr)) goto out;

    hr = IRunningObjectTable_Revoke(rot, surrogate_rot_key);

out:
    if (rot != NULL) IRunningObjectTable_Release(rot);

    return hr;
}

static HRESULT get_surrogate_identifier_moniker(LPMONIKER *output_moniker)
{
    HRESULT hr = S_OK;
    LPWSTR pid_string;
    LPMONIKER base_moniker, pid_moniker;

    if (output_moniker == NULL)
    {
        hr = E_POINTER;
        goto out;
    }

    // FIXME: There is no wasprintfW() or wnsprintfW(), so I do not know how to avoid a buffer overrun issue here.
    pid_string = calloc(100, sizeof(WCHAR));
    wsprintfW(pid_string, L"%d", GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", L"COM Surrogates", &base_moniker);
    if (FAILED(hr)) goto out;
    hr = CreateItemMoniker(L"!", pid_string, &pid_moniker);
    if (FAILED(hr)) goto out;

    hr = IMoniker_ComposeWith(base_moniker, pid_moniker, FALSE, output_moniker);

out:
    if (pid_string != NULL) free(pid_string);
    if (base_moniker != NULL) IMoniker_Release(base_moniker);
    if (pid_moniker != NULL) IMoniker_Release(pid_moniker);

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

    hr = get_surrogate_identifier_moniker(&moniker);
    if (FAILED(hr)) goto out;
    hr = GetRunningObjectTable(0, &rot);
    if (FAILED(hr)) goto out;

    hr = ISurrogate_QueryInterface(surrogate, &IID_IUnknown, (void **) &pUnk);
    if (FAILED(hr)) goto out;

    hr = IRunningObjectTable_Register(rot, 0, pUnk, moniker, &surrogate_rot_key);
    if (FAILED(hr)) goto out;

    process_surrogate_instance = surrogate;

out:
    if (rot != NULL) IRunningObjectTable_Release(rot);
    if (moniker != NULL) IMoniker_Release(moniker);
    if (pUnk != NULL) IUnknown_Release(pUnk);

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

ISurrogate *process_surrogate_instance = NULL;

/*
 * PROJECT:         ReactOS api tests
 * LICENSE:         GPLv2+ - See COPYING in the top level directory
 * PURPOSE:         COM interface test for dllhost.exe
 * PROGRAMMER:      William Kent <wjk011@gmail.com>
 */

#include "com_apitest.h"
#include <comtest.h>
#include <windows.h>
#include <shlwapi.h>

typedef HRESULT (WINAPI *DllRegisterServerFunc)(void);

#define myskip(c, ...) ((c) ? 0 : (skip(__VA_ARGS__), 1))

START_TEST(DllSurrogate)
{
    WCHAR modulePath[MAX_PATH + 1];
    ZeroMemory(&modulePath, sizeof(modulePath));
    if (GetModuleFileNameW(NULL, modulePath, MAX_PATH) == 0)
    {
        skip("cannot get path to com_apitest.exe\n");
        return;
    }

    PathRemoveFileSpecW(modulePath);
    if (!PathAppendW(modulePath, L"com_test_library.dll"))
    {
        skip("path manipulation failure\n");
        return;
    }

    HMODULE module = LoadLibraryW(modulePath);
    if (module == INVALID_HANDLE_VALUE)
    {
        skip("could not load com_test_library.dll\n");
        return;
    }

    DllRegisterServerFunc regFunction = (DllRegisterServerFunc)GetProcAddress(module, "DllRegisterServer");
    DllRegisterServerFunc unregFunction = (DllRegisterServerFunc)GetProcAddress(module, "DllUnregisterServer");

    HRESULT hr = regFunction();
    if (FAILED(hr))
    {
        skip("DllRegisterServer failed with hr 0x%lx\n", hr);
        return;
    }

    IComTestInterface *iface;
    hr = CoCreateInstance(&CLSID_ComTestClass, NULL, CLSCTX_INPROC, &IID_IComTestInterface, (void **)iface);
    ok(hr == S_OK, "CoCreateInstance() failed with hr 0x%lx\n", hr);
    if (!myskip(hr == S_OK, "Could not create ComTestClass instance\n"))
    {
        unregFunction();
        return;
    }

    DWORD pid;
    hr = IComTestInterface_GetProcID(iface, &pid);
    ok(hr == S_OK, "ComTestInterface::GetProcID() failed with hr 0x%lx\n", hr);
    ok(pid != GetCurrentProcessId(), "instance was not created in separate process\n");
    IComTestInterface_Release(iface);

    unregFunction();
}

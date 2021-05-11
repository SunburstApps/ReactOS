/*
 * PROJECT:     ReactOS DllHost
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     COM surrogate.
 * COPYRIGHT:   Copyright 2018 Eric Kohl
 */

#include "CSurrogateClassFactory.h"
#include "CStdSurrogate.h"

/* FIELDS *******************************************************************/

static CComModule app_module;

/* FUNCTIONS ****************************************************************/

INT
WINAPI
wWinMain(
    HINSTANCE hInst,
    HINSTANCE hPrevInst,
    LPWSTR lpCmdLine,
    INT nCmdShow)
{
    HRESULT hr;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return hr;

    // TODO: Call CoInitializeSecurity() here

    CComObject<CStdSurrogate> *surrogate;
    hr = CComObject<CStdSurrogate>::CreateInstance(&surrogate);
    if (FAILED(hr)) return hr;
    surrogate->AddRef();

    ISurrogate *surrogate_iface;
    hr = surrogate->QueryInterface(IID_ISurrogate, (void **)&surrogate_iface);
    if (FAILED(hr)) return hr;
    surrogate->Release();

    hr = CoRegisterSurrogate(surrogate_iface);
    if (FAILED(hr)) return hr;

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

/* EOF */

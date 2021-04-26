/*
 * PROJECT:     ReactOS DllHost
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     COM surrogate.
 * COPYRIGHT:   Copyright 2018 Eric Kohl
 */

#include "CSurrogateClassFactory.h"
#include "CStdSurrogate.h"

/* FIELDS *******************************************************************/

HANDLE hStopEvent;
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

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) return hr;

    // TODO: Call CoInitializeSecurity() here

    hStopEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    CComObject<CStdSurrogate> *surrogate;
    hr = CComObject<CStdSurrogate>::CreateInstance(&surrogate);
    if (FAILED(hr)) return hr;

    ISurrogate *surrogate_iface;
    hr = surrogate->QueryInterface(IID_ISurrogate, (void **)&surrogate_iface);
    if (FAILED(hr)) return hr;

    hr = CoRegisterSurrogate(surrogate_iface);
    if (FAILED(hr)) return hr;

    while (true) {
        DWORD result = WaitForMultipleObjects(1, &hStopEvent, false, 120000);
        if (result == WAIT_TIMEOUT)
        {
            CoFreeUnusedLibrariesEx(INFINITE, 0);
        }
        else if (result == WAIT_OBJECT_0)
        {
            break;
        }
    }

    return 0;
}

/* EOF */

/*
 * PROJECT:     ReactOS DllHost
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     COM surrogate.
 * COPYRIGHT:   Copyright 2018 Eric Kohl
 */

#pragma once
#include "CSurrogateClassFactory.h"

extern HANDLE hStopEvent;
class CStdSurrogate :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public ISurrogate {
private:
    CAtlArray<DWORD> class_factory_tokens;

public: // *** ISurrogate methods ***
    virtual HRESULT STDMETHODCALLTYPE LoadDllServer(REFCLSID clsid) {
        CComObject<CSurrogateClassFactory> *factory;
        HRESULT hr = CComObject<CSurrogateClassFactory>::CreateInstance(&factory);
        if (FAILED(hr)) return hr;
        factory->clsid = clsid;

        DWORD cookie;
        CComPtr<IUnknown> pUnk;
        hr = factory->QueryInterface(IID_IUnknown, (void **)&pUnk);
        if (FAILED(hr)) return hr;
        hr = CoRegisterClassObject(clsid, pUnk, CLSCTX_ALL, REGCLS_SURROGATE, &cookie);
        if (FAILED(hr)) return hr;

        class_factory_tokens.Add(cookie);
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE FreeSurrogate() {
        for (size_t i = 0; i < class_factory_tokens.GetCount(); i++)
        {
            CoRevokeClassObject(class_factory_tokens[i]);
        }

        SetEvent(hStopEvent);
        return S_OK;
    }

public:
    DECLARE_NOT_AGGREGATABLE(CStdSurrogate)
    DECLARE_NO_REGISTRY()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CStdSurrogate)
        COM_INTERFACE_ENTRY_IID(IID_ISurrogate, ISurrogate)
    END_COM_MAP()
};
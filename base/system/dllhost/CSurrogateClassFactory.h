/*
 * PROJECT:     ReactOS DllHost
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     COM surrogate.
 * COPYRIGHT:   Copyright 2021 William Kent
 */

#pragma once
#include <windows.h>
#include <unknwn.h>
#include <objidl.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlcoll.h>

class CSurrogateClassFactory :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IClassFactory,
    public IMarshal
{
public:
    CLSID clsid;

public: // *** IClassFactory methods ***
    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv)
    {
        void* pcf;
        HRESULT hr;
 
        hr = CoGetClassObject(clsid, CLSCTX_INPROC_SERVER, NULL, IID_IClassFactory, &pcf);
        if (FAILED(hr)) return hr;
        hr = ((IClassFactory*)pcf)->CreateInstance(pUnkOuter, iid, ppv);

        ((IClassFactory*)pcf)->Release();
        return hr;
    }

    STDMETHODIMP LockServer(BOOL lock) {
        // Don't know if we need to do anything here.
        return S_OK;
    }

public: // *** IMarshal methods ***
    virtual HRESULT STDMETHODCALLTYPE GetUnmarshalClass(REFIID riid, void *pv, DWORD dwDestContext, void *pvDestContext, DWORD mshlflags, CLSID *pCid)
    {
        CComPtr<IMarshal> marshal;
        HRESULT hr = CoGetStandardMarshal(riid, (LPUNKNOWN)pv, dwDestContext, NULL, mshlflags, &marshal);
        if (FAILED(hr)) return hr;

        return marshal->GetUnmarshalClass(riid, (LPUNKNOWN)pv, dwDestContext, pvDestContext, mshlflags, pCid);
    }

    virtual HRESULT STDMETHODCALLTYPE GetMarshalSizeMax(REFIID riid, void *pv, DWORD dwDestContext, void *pvDestContext, DWORD mshlflags, DWORD *pSize)
    {
        CComPtr<IMarshal> marshal;
        HRESULT hr = CoGetStandardMarshal(riid, (LPUNKNOWN)pv, dwDestContext, NULL, mshlflags, &marshal);
        if (FAILED(hr)) return hr;

        return marshal->GetMarshalSizeMax(riid, (LPUNKNOWN)pv, dwDestContext, pvDestContext, mshlflags, pSize);
    }

    STDMETHODIMP MarshalInterface(IStream *pStm, REFIID riid, void *pv, DWORD dwDestContext, void *pvDestContext, DWORD mshlflags)
    {
        CComPtr<IUnknown> pCF;
        HRESULT hr;
 
        hr = CoGetClassObject(clsid, CLSCTX_INPROC_SERVER, NULL, riid, (void **)&pCF);
        if (FAILED(hr)) return hr;   
        hr = CoMarshalInterface(pStm, riid, pCF, dwDestContext, pvDestContext, mshlflags);

        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE UnmarshalInterface(IStream *pStm, REFIID riid, void **ppv)
    {
        CComPtr<IMarshal> marshal;
        HRESULT hr = CoGetStandardMarshal(riid, (LPUNKNOWN)ppv, 0, NULL, 0, &marshal);
        if (FAILED(hr)) return hr;

        hr = marshal->UnmarshalInterface(pStm, riid, ppv);
        if (FAILED(hr)) return hr;

        hr = CoReleaseMarshalData(pStm);
        return hr;
    }

    virtual HRESULT STDMETHODCALLTYPE ReleaseMarshalData(IStream *pStm)
    {
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE DisconnectObject(DWORD dwReserved)
    {
        return E_NOTIMPL;
    }
        
public:
    DECLARE_NOT_AGGREGATABLE(CSurrogateClassFactory)
    DECLARE_NO_REGISTRY()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSurrogateClassFactory)
        COM_INTERFACE_ENTRY_IID(IID_IClassFactory, IClassFactory)
        COM_INTERFACE_ENTRY_IID(IID_IMarshal, IMarshal)
    END_COM_MAP()
};


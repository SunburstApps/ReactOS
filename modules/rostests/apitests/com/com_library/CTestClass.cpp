#include <atlbase.h>
#include <atlcore.h>
#include <atlcom.h>

#include <initguid.h>
#include "comtest.h"
#include "resource.h"

class CComTestClass :
    public CComCoClass<CComTestClass, &CLSID_ComTestClass>,
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IComTestInterface
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetProcID(DWORD *pdword)
    {
        if (pdword == NULL) return E_POINTER;
        *pdword = GetCurrentProcessId();
        return S_OK;
    }

public:
    DECLARE_NOT_AGGREGATABLE(CComTestClass)
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    DECLARE_REGISTRY_RESOURCEID(IDR_COM_TEST_CLASS)

    BEGIN_COM_MAP(CComTestClass)
        COM_INTERFACE_ENTRY_IID(IID_IComTestInterface, IComTestInterface)
    END_COM_MAP()
};

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_ComTestClass, CComTestClass)
END_OBJECT_MAP()

CComModule gModule;

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID fImpLoad)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        gModule.Init(ObjectMap, hInstance, &LIBID_rostests);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        gModule.Term();
    }

    return TRUE;
}

EXTERN_C HRESULT WINAPI DllGetClassObject(REFCLSID rclsid, REFIID iid, LPVOID *ppv)
{
    return gModule.DllGetClassObject(rclsid, iid, ppv);
}

EXTERN_C HRESULT WINAPI DllCanUnloadNow(void)
{
    return S_OK;
}

EXTERN_C HRESULT WINAPI DllRegisterServer(void)
{
    return gModule.UpdateRegistryFromResource(IDR_COM_TEST_CLASS, TRUE, NULL);
}

EXTERN_C HRESULT WINAPI DllUnregisterServer(void)
{
    return gModule.UpdateRegistryFromResource(IDR_COM_TEST_CLASS, FALSE, NULL);
}

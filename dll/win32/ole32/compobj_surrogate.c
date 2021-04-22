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

/***********************************************************************
 *           CoRegisterSurrogate [OLE32.@]
 */
HRESULT WINAPI CoRegisterSurrogate(ISurrogate *surrogate)
{
    FIXME("(%p): stub\n", surrogate);

    return E_NOTIMPL;
}

/***********************************************************************
 *           CoRegisterSurrogateEx [OLE32.@]
 */
HRESULT WINAPI CoRegisterSurrogateEx(REFGUID guid, void *reserved)
{
    FIXME("(%s %p): stub\n", debugstr_guid(guid), reserved);

    return E_NOTIMPL;
}

/*
 * PROJECT:         ReactOS KDNET driver
 * LICENSE:         GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:         Base definitions for the KDNET driver.
 * REFERENCES:      See media/doc/Kdnet_Refs.txt
 */

#ifndef _KDNET_H_
#define _KDNET_H_

#define NOEXTAPI
#include <ntifs.h>
#include <windbgkd.h>

// #define KDDEBUG /* uncomment to enable debugging this dll */

#ifndef KDDEBUG
#define KDDBGPRINT(...)
#else
extern ULONG KdpDbgPrint(const char* Format, ...);
#define KDDBGPRINT KdpDbgPrint
#endif

typedef enum
{
    KDP_PACKET_RECEIVED = 0,
    KDP_PACKET_TIMEOUT  = 1,
    KDP_PACKET_RESEND   = 2
} KDP_STATUS;

#endif /* _KDDLL_H_ */

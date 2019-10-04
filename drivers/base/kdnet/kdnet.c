/*
 * PROJECT:         ReactOS KDNET driver
 * LICENSE:         GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:         Base functions for the KDNET driver.
 * REFERENCES:      See media/doc/Kdnet_Refs.txt
 */

#include "kdnet.h"
#include <arc/arc.h>

/* GLOBALS ********************************************************************/

/* PRIVATE FUNCTIONS **********************************************************/

ULONG
NTAPI
KdpCalculateChecksum(
    IN PVOID Buffer,
    IN ULONG Length)
{
    PUCHAR ByteBuffer = Buffer;
    ULONG Checksum = 0;

    while (Length-- > 0)
    {
        Checksum += (ULONG)*ByteBuffer++;
    }
    return Checksum;
}

/* PUBLIC FUNCTIONS ***********************************************************/

KDP_STATUS
NTAPI
KdReceivePacket(
    IN ULONG PacketType,
    OUT PSTRING MessageHeader,
    OUT PSTRING MessageData,
    OUT PULONG DataLength,
    IN OUT PKD_CONTEXT KdContext)
{
    // Unimplemented!
    return KDP_PACKET_TIMEOUT;
}

VOID
NTAPI
KdSendPacket(
    IN ULONG PacketType,
    IN PSTRING MessageHeader,
    IN PSTRING MessageData,
    IN OUT PKD_CONTEXT KdContext)
{
    // Unimplemented!
}

NTSTATUS
NTAPI
KdD0Transition(VOID)
{
    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
KdD3Transition(VOID)
{
    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
KdSave(IN BOOLEAN SleepTransition)
{
    /* Nothing to do here */
    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
KdRestore(IN BOOLEAN SleepTransition)
{
    /* Nothing to do here */
    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
KdDebuggerInitialize0(IN PLOADER_PARAMETER_BLOCK LoaderBlock OPTIONAL)
{
    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
KdDebuggerInitialize1(IN PLOADER_PARAMETER_BLOCK LoaderBlock OPTIONAL)
{
    return STATUS_SUCCESS;
}

/* EOF */

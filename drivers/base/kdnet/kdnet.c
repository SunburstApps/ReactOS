/*
 * PROJECT:         ReactOS KDNET driver
 * LICENSE:         GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:         Base functions for the KDNET driver.
 * REFERENCES:      See media/doc/Kdnet_Refs.txt
 */

#include "kdnet.h"
#include <stdlib.h>
#include <string.h>
#include <arc/arc.h>

/* GLOBALS ********************************************************************/

#define KDNET_ENCRYPTION_KEY_BYTE_SIZE     32

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

static SHORT
NTAPI
KdpAsciiStringToBase10Integer(
    IN PCHAR String,
    OUT PCHAR* EndOfInteger OPTIONAL)
{
    SHORT Value = 0;

    while (isdigit(*String))
    {
        Value *= 10;
        Value += *String - '0';
    }

    if (EndOfInteger)
        *EndOfInteger = String;

    return Value;
}

static UINT64
NTAPI
KdpAsciiStringToBase36Integer(
    IN PCHAR String,
    OUT PCHAR* EndOfInteger OPTIONAL,
    OUT PBOOLEAN OverflowErrorPtr)
{
    UINT64 Value = 0;
    UINT8 Digit = 0;
    BOOLEAN Overflow = FALSE;

    while (isdigit(*String) || isupper(*String) || islower(*String))
    {
        Digit = 0;
        if (isdigit(*String)) Digit = *String - '0';
        else if (isupper(*String)) Digit = *String - 'A' + 10;
        else if (islower(*String)) Digit = toupper(*String) - 'A' + 10;
        String++;

        if (Value > 0x071C71C71C71C71C)
        {
            Overflow = TRUE;
            break;
        }
        Value *= 36;

        if (Value > 0xFFFFFFFFFFFFFFDB)
        {
            Overflow = TRUE;
            break;
        }
        Value += Digit;
    }

    if (OverflowErrorPtr)
        *OverflowErrorPtr = Overflow;
    if (EndOfInteger)
        *EndOfInteger = String;

    return Value;
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
    PCHAR CommandLine, AddressString, PortString, KeyString, PciIdString, FragmentPtr;
    SHORT PortNumber = 0;
    UINT64 Base36Value = 0;
    BOOLEAN Base36Overflow;
    UINT8 EncryptionKey[KDNET_ENCRYPTION_KEY_BYTE_SIZE];

    if (LoaderBlock)
    {
        // Load and uppercase the command line.
        CommandLine = LoaderBlock->LoadOptions;
        _strupr(CommandLine);

        // Detect and parse the /KDNET_IPADDR parameter.
        // This parameter contains the IP address of the host computer.
        AddressString = strstr(CommandLine, "KDNET_IPADDR");
        if (AddressString)
        {
            // Move past the parameter name, spaces, and equals sign
            AddressString += strlen("KDNET_IPADDR");
            while (*AddressString == ' ') AddressString++;
            AddressString++;

            // TODO: Parse the IP address.
        }
        else
        {
            // The KDNET_IPADDR parameter is required.
            return STATUS_INVALID_PARAMETER;
        }

        // Detect and parse the /KDNET_PORT parameter.
        // This parameter contains the UDP port number to connect to.
        // It is separate from the /KDNET_IPADDR parameter to make IPv6 address specification easier.
        PortString = strstr(CommandLine, "KDNET_PORT");
        if (PortString)
        {
            // Move past the parameter name, spaces, and equals sign
            PortString += strlen("KDNET_PORT");
            while (*PortString == ' ') PortString++;
            PortString++;

            // Now parse the port number.
            PortNumber = KdpAsciiStringToBase10Integer(PortString, NULL);
            if (PortNumber == 0) return STATUS_INVALID_PARAMETER;
            // TODO: Store the port number.
        }
        else
        {
            // The KDNET_PORT parameter is required.
            return STATUS_INVALID_PARAMETER;
        }

        // Detect and parse the /KDNET_KEY parameter.
        // This parameter contains the encryption key, specified as a Base-36 string.
        KeyString = strstr(CommandLine, "KDNET_KEY");
        if (KeyString)
        {
            // Move past the parameter name, spaces, and equals sign
            KeyString += strlen("KDNET_KEY");
            while (*KeyString == ' ') KeyString++;
            KeyString++;

            // Now parse the encryption key.
            RtlZeroMemory(EncryptionKey, sizeof(EncryptionKey));

            Base36Overflow = FALSE;
            Base36Value = KdpAsciiStringToBase36Integer(KeyString, &FragmentPtr, &Base36Overflow);
            if (Base36Overflow)
                // If the base-36 value is too large for a UINT64, then the key is not valid, and we can't continue.
                return STATUS_INVALID_PARAMETER;
            if (FragmentPtr == KeyString)
                // If there were no valid base-36 digits, then the key is not valid, and we can't continue.
                return STATUS_INVALID_PARAMETER;

            RtlCopyMemory(EncryptionKey, Base36Value, sizeof(Base36Value));
            KeyString += (FragmentPtr - KeyString);
            if (*KeyString != '.') return STATUS_INVALID_PARAMETER;
            KeyString++;

            Base36Overflow = FALSE;
            Base36Value = KdpAsciiStringToBase36Integer(KeyString, &FragmentPtr, &Base36Overflow);
            if (Base36Overflow)
                // If the base-36 value is too large for a UINT64, then the key is not valid, and we can't continue.
                return STATUS_INVALID_PARAMETER;
            if (FragmentPtr == KeyString)
                // If there were no valid base-36 digits, then the key is not valid, and we can't continue.
                return STATUS_INVALID_PARAMETER;

            RtlCopyMemory(EncryptionKey + sizeof(Base36Value), Base36Value, sizeof(Base36Value));
            KeyString += (FragmentPtr - KeyString);
            if (*KeyString != '.') return STATUS_INVALID_PARAMETER;
            KeyString++;

            Base36Overflow = FALSE;
            Base36Value = KdpAsciiStringToBase36Integer(KeyString, &FragmentPtr, &Base36Overflow);
            if (Base36Overflow)
                // If the base-36 value is too large for a UINT64, then the key is not valid, and we can't continue.
                return STATUS_INVALID_PARAMETER;
            if (FragmentPtr == KeyString)
                // If there were no valid base-36 digits, then the key is not valid, and we can't continue.
                return STATUS_INVALID_PARAMETER;

            RtlCopyMemory(EncryptionKey + sizeof(Base36Value) * 2, Base36Value, sizeof(Base36Value));
            KeyString += (FragmentPtr - KeyString);
            if (*KeyString != '.') return STATUS_INVALID_PARAMETER;
            KeyString++;

            Base36Overflow = FALSE;
            Base36Value = KdpAsciiStringToBase36Integer(KeyString, &FragmentPtr, &Base36Overflow);
            if (Base36Overflow)
                // If the base-36 value is too large for a UINT64, then the key is not valid, and we can't continue.
                return STATUS_INVALID_PARAMETER;
            if (FragmentPtr == KeyString)
                // If there were no valid base-36 digits, then the key is not valid, and we can't continue.
                return STATUS_INVALID_PARAMETER;

            RtlCopyMemory(EncryptionKey + sizeof(Base36Value) * 3, Base36Value, sizeof(Base36Value));

            // TODO: Store the completed EncryptionKey.
        }
        else
        {
            // The KDNET_KEY parameter is required.
            return STATUS_INVALID_PARAMETER;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
KdDebuggerInitialize1(IN PLOADER_PARAMETER_BLOCK LoaderBlock OPTIONAL)
{
    return STATUS_SUCCESS;
}

/* EOF */

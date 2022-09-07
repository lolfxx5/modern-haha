/*++

Copyright (c) 1996  Intel Corporation
Copyright (c) 1994  Microsoft Corporation

Module Name:

  i64bios.c  copied from hali64\x86bios.c

Abstract:


    This module implements the platform specific interface between a device
    driver and the execution of x86 ROM bios code for the device.

Author:

    William K. Cheung (wcheung) 20-Mar-1996

    based on the version by David N. Cutler (davec) 17-Jun-1994

Environment:

    Kernel mode only.

Revision History:
    Bernard Lint, M.Jayakumar November 1998

--*/

#include "halp.h"
#include "emulate.h"


#define LOW_MEM_SEGMET 0

#define LOW_MEM_OFFSET 0

#define SIZE_OF_VECTOR_TABLE 0x400

#define SIZE_OF_BIOS_DATA_AREA 0x400

extern XM_STATUS x86BiosExecuteInterrupt (
    IN UCHAR Number,
    IN OUT PXM86_CONTEXT Context,
    IN PVOID BiosIoSpace OPTIONAL,
    IN PVOID BiosIoMemory OPTIONAL
    );

extern PVOID x86BiosTranslateAddress (
    IN USHORT Segment,
    IN USHORT Offset
    );

extern BOOLEAN  HalpVideoBiosPresent;
extern ULONG    HalpMaxPciBus;

ULONG
HalpBiosGetPciConfig(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
HalpBiosSetPciConfig(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );


//
// Initialize Default X86 bios spaces
//


#define NUMBER_X86_PAGES (0x100000 / PAGE_SIZE)      // map through 0xfffff

PVOID HalpIoControlBase = NULL;
PVOID HalpIoMemoryBase =  NULL;
PVOID HalpFrameBufferBase = NULL;
PVOID HalpLowMemoryBase = NULL;

#define VGA_FRAMEBUFFER_SIZE  (0xc0000 - 0xa0000)

//
// Define global data.
//

ULONG HalpX86BiosInitialized = FALSE;
ULONG HalpEnableInt10Calls = FALSE;


VOID
HalpInitIoMemoryBase(
    VOID
    )

/*++

Routine Description:
    This routine completes any mappings needed by the bios emulation engine. HalpEfiInitialization maps
    any EFI descriptor that cover the 1st MB of physical memory.  Those mappngs are done according to
    the cachable requirements in the descriptors. Most EFI implementations do not cover the VGA range so
    this function does that if it is not already mapped.

Arguements:

Return Value:

--*/
{
    PHYSICAL_ADDRESS COMPATIBLE_PCI_PHYSICAL_BASE_ADDRESS  = { 0xA0000 };


    if (HalpFrameBufferBase == NULL) {
        HalpFrameBufferBase = MmMapIoSpace (COMPATIBLE_PCI_PHYSICAL_BASE_ADDRESS,
                                            VGA_FRAMEBUFFER_SIZE,
                                            MmNonCached);

        ASSERT(HalpFrameBufferBase != NULL);

        //
        // Adjust to a zero base.
        //

        HalpFrameBufferBase = (PVOID)((ULONG64) HalpFrameBufferBase - 0XA0000);
    }

    ASSERT(HalpIoMemoryBase);

}


ULONG
HalpSetCmosData (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

/*++

Routine Description:

Arguements:

Return Value:

--*/

{
    return 0;
}


ULONG
HalpGetCmosData (
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )

/*++

Routine Description:

Arguements:

Return Value:

--*/


{
    return 0;
}


VOID
HalpAcquireCmosSpinLock (
    VOID
        )

/*++

Routine Description:

Arguements:

Return Value:

--*/


{
    return;
}


VOID
HalpReleaseCmosSpinLock (
    VOID
    )

/*++

Routine Description:

Arguements:

Return Value:

--*/

{
    return ;
}


HAL_DISPLAY_BIOS_INFORMATION
HalpGetDisplayBiosInformation (
    VOID
    )

/*++

Routine Description:


Arguements:


Return Value:

--*/




{
    return HalpVideoBiosPresent ? HalDisplayEmulatedBios : HalDisplayNoBios;
}


VOID
HalpInitializeCmos (
    VOID
    )

/*++

Routine Description:

Arguements:

Return Value:

--*/


{
    return ;
}


VOID
HalpReadCmosTime (
    PTIME_FIELDS TimeFields
    )

/*++

Routine Description:

Arguements:

Return Value:

--*/

{
    return ;
}

VOID
HalpWriteCmosTime (
    PTIME_FIELDS TimeFields
    )

/*++

Routine Description:

Arguements:

Return Value:

--*/


{
    return;
}



BOOLEAN
HalpBiosDisplayReset (
    VOID
    )

/*++

Routine Description:


Arguements:


Return Value:

--*/

{
    //
    // Make an int10 call to set the display into 640x480 16 color mode
    //
    // mov ax, 12h
    // int 10h
    //

    ULONG Eax = 0x12;
    ULONG Exx = 0x00;
    BOOLEAN Success;

    Success = HalCallBios(0x10,
                          &Eax,
                          &Exx,
                          &Exx,
                          &Exx,
                          &Exx,
                          &Exx,
                          &Exx);

    return Success;
}


BOOLEAN
HalCallBios (
    IN ULONG BiosCommand,
    IN OUT PULONG Eax,
    IN OUT PULONG Ebx,
    IN OUT PULONG Ecx,
    IN OUT PULONG Edx,
    IN OUT PULONG Esi,
    IN OUT PULONG Edi,
    IN OUT PULONG Ebp
    )

/*++

Routine Description:

    This function provides the platform specific interface between a device
    driver and the execution of the x86 ROM bios code for the specified ROM
    bios command.

Arguments:

    BiosCommand - Supplies the ROM bios command to be emulated.

    Eax to Ebp - Supplies the x86 emulation context.

Return Value:

    A value of TRUE is returned if the specified function is executed.
    Otherwise, a value of FALSE is returned.

--*/

{

    XM86_CONTEXT Context;

    HalDebugPrint(( HAL_INFO, "HAL: HalCallBios - Cmd = 0x%x, eax = 0x%p\n", BiosCommand, Eax ));
    //
    // If the x86 BIOS Emulator has not been initialized, then return FALSE.
    //

    if (HalpX86BiosInitialized == FALSE) {
        return FALSE;
    }

    //
    // If the Adapter BIOS initialization failed and an Int10 command is
    // specified, then return FALSE.
    //

    if ((BiosCommand == 0x10) && (HalpEnableInt10Calls == FALSE)) {
        return FALSE;
    }

    //
    // Copy the x86 bios context and emulate the specified command.
    //

    Context.Eax = *Eax;
    Context.Ebx = *Ebx;
    Context.Ecx = *Ecx;
    Context.Edx = *Edx;
    Context.Esi = *Esi;
    Context.Edi = *Edi;
    Context.Ebp = *Ebp;


    if (x86BiosExecuteInterruptShadowed((UCHAR)BiosCommand,
        &Context,
        (PVOID)HalpIoControlBase,
        (PVOID)HalpIoMemoryBase,
        (PVOID)HalpFrameBufferBase
        ) != XM_SUCCESS) {

        HalDebugPrint(( HAL_ERROR, "HAL: HalCallBios - ERROR in Cmd = 0x%x\n", BiosCommand ));
        return FALSE;

    }

    //
    // Copy the x86 bios context and return TRUE.
    //

    *Eax = Context.Eax;
    *Ebx = Context.Ebx;
    *Ecx = Context.Ecx;
    *Edx = Context.Edx;
    *Esi = Context.Esi;
    *Edi = Context.Edi;
    *Ebp = Context.Ebp;
    return TRUE;
}

VOID
HalpInitializeX86Int10Call(
    VOID
    )

/*++

Routine Description:

    This function initializes x86 bios emulator, display data area and
    interrupt vector area.


Arguments:

    None.

Return Value:

    None.

--*/

{
    XM86_CONTEXT State;
    PXM86_CONTEXT Context;
    PULONG x86BiosLowMemoryPtr, PhysicalMemoryPtr;

    if (HalpLowMemoryBase == NULL) {

        //
        // There is no memory at zero so there cannot be any bios.
        //

        return;
    }

    //
    // Initialize the x86 bios emulator.
    //


    x86BiosInitializeBiosShadowedPci(HalpIoControlBase,
                                     HalpIoMemoryBase,
                                     HalpFrameBufferBase,
                                     (UCHAR)(HalpMaxPciBus == 255 ? 255 : (HalpMaxPciBus + 1)),
                                     HalpBiosGetPciConfig,
                                     HalpBiosSetPciConfig
                                     );

    x86BiosLowMemoryPtr = (PULONG)(x86BiosTranslateAddress(LOW_MEM_SEGMET, LOW_MEM_OFFSET));
    PhysicalMemoryPtr   = (PULONG) HalpLowMemoryBase;

    //
    // Copy the VECTOR TABLE from 0 to 2k. This is because we are not executing
    // the initialization of Adapter since SAL takes care of it. However, the
    // emulation memory needs to be updated from the interrupt vector and BIOS
    // data area.
    //

    RtlCopyMemory(x86BiosLowMemoryPtr,
                  PhysicalMemoryPtr,
                  (SIZE_OF_VECTOR_TABLE+SIZE_OF_BIOS_DATA_AREA)
                  );

    HalpX86BiosInitialized = TRUE;

    //
    // Check to see if a video bios appears to be present and int10 vector
    // points somewhere inside of the video bios
    //
    {
        PUSHORT pBiosSignature = (PUSHORT)(x86BiosTranslateAddress(0xc000, // VIDEO_BIOS_SEGMENT
                                                                   0x0000));

        ULONG Address = *(PULONG)(x86BiosTranslateAddress(0x0, 0x40));
        Address = ((Address & 0xFFFF0000) >> 12) + (Address & 0xFFFF); // Normalize

        HalpEnableInt10Calls = (*pBiosSignature == 0xAA55) &&
                               (Address >= 0xC0000) && (Address < 0xD0000);
    }

    return;
}


VOID
HalpResetX86DisplayAdapter(
    VOID
    )

/*++

Routine Description:

    This function resets a display adapter using the x86 bios emulator.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ULONG Eax;
    ULONG Ebx;
    ULONG Ecx;
    ULONG Edx;
    ULONG Esi;
    ULONG Edi;
    ULONG Ebp;

    //
    // Initialize the x86 bios context and make the INT 10 call to initialize
    // the display adapter to 80x25 color text mode.
    //

    Eax = 0x0003;  // Function 0, Mode 3
    Ebx = 0;
    Ecx = 0;
    Edx = 0;
    Esi = 0;
    Edi = 0;
    Ebp = 0;

    HalCallBios(0x10,
        &Eax,
        &Ebx,
        &Ecx,
        &Edx,
        &Esi,
        &Edi,
        &Ebp);
}

ULONG
HalpBiosGetPciConfig(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

/*++

Routine Description:

    This function wraps HalGetBusDataByOffset for use by the x86 emulator.

Arguments:

    None.

Return Value:

    None.

--*/

{
    return HalGetBusDataByOffset(PCIConfiguration,
                                 BusNumber,
                                 SlotNumber,
                                 Buffer,
                                 Offset,
                                 Length
                                 );
}

ULONG
HalpBiosSetPciConfig(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

/*++

Routine Description:

    This function wraps HalGetBusDataByOffset for use by the x86 emulator.

Arguments:

    None.

Return Value:

    None.

--*/

{
    return HalSetBusDataByOffset(PCIConfiguration,
                                 BusNumber,
                                 SlotNumber,
                                 Buffer,
                                 Offset,
                                 Length
                                 );
}

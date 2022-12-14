;++
;
;   Copyright (c) 1989  Microsoft Corporation
;
;   Module Name:
;
;   eisa.inc
;
;   Abstract:
;
;   This module contains the assembly structures and definitions
;       for making 16-bit real mode EISA BIOS calls.
;
;   Author:
;
;   Shie-Lin (shielint) 7-June-1991
;
;   Revision History:
;
;--

;
; EISA BIOS call function number.
;

GET_EISA_SLOT_INFORMATION     equ 0D800h
GET_EISA_FUNCTION_INFORMATION equ 0D801h

;
; Length of EISA information block
;

EISA_INFORMATION_BLOCK_LENGTH equ 320

;
; Structure for EISA slot information block
;

EISA_SLOT_INFORMATION   struc

    SlotReturn                  db      0
    SlotFlags                   db      0
    SlotMajorRevision           db      0
    SlotMinorRevision           db      0
    SlotChecksum                dw      0
    SlotNumberFunctions         db      0
    SlotFunctionInformation     db      0
    SlotCompressedId            dd      0

EISA_SLOT_INFORMATION   ends

;
; Structure for EISA function information block
;

EISA_FUNCTION_INFORMATION       struc

    FunctionReturn      db      0
    FunctionInformation db      EISA_INFORMATION_BLOCK_LENGTH dup (0)

EISA_FUNCTION_INFORMATION       ends

;
; Structure definitions and equates for INT 15 function E820
;

E820Frame struc
        ErrorFlag       dd      ?
        Key             dd      ?
        DescSize        dd      ?

        BaseAddrLow     dd      ?
        BaseAddrHigh    dd      ?
        SizeLow         dd      ?
        SizeHigh        dd      ?
        MemoryType      dd      ?
E820Frame ends

MemoryDescriptorFramePointer struc
        E820FramePointer    dd      ?
MemoryDescriptorFramePointer ends

;
; Structure definitions for INT 15 E980
;

E980Info struc
        Signature               dw      ?
        CommandPortAddress      dw      ?
        EventPortAddress        dw      ?
        PollInterval            dw      ?
        CommandDataValue        db      ?
        EventPortBitmask        db      ?
        MaxLevelAc              db      ?
        MaxLevelDc              db      ?
E980Info ends

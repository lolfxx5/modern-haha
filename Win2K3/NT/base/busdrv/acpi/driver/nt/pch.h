/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    pch.h

Abstract:

    This is the precompiled header for the ACPI NT subtree

Author:

    Jason Clark (jasoncl)

Environment:

    Kernel mode only.

Revision History:

--*/

#define _NTDRIVER_
#define _NTSRV_
#define _NTDDK_

#include <stdarg.h>
#include <stdio.h>
#include <ntos.h>
#include <pci.h>
#include <dockintf.h>

#ifndef SPEC_VER
    #define SPEC_VER   100
#endif

#ifndef FAR
    #define FAR
#endif

#include <poclass.h>
#include <wdmguid.h>
#include <zwapi.h>
#include <ntpoapi.h>
#include <arbiter.h>

#include <wmistr.h>
#include <wmiguid.h>
#include <wmilib.h>

//
// This is the header for interfacing with the HAL
//
#include <ntacpi.h>


//
// These are the global include files for this project
//
#include "acpitabl.h"
#include "amli.h"
#include "aml.h"
#include "acpios.h"
#include "ospower.h"
#include "callback.h"
#include "acpi.h"
#include "acpidbg.h"
#include "acpiregs.h"
#include "devioctl.h"
#include "acpipriv.h"
#include "acpiioct.h"
#include "acpictl.h"
#include "acpienbl.h"
#include "acpigpio.h"
#include "acpiinit.h"
#include "acpiio.h"
#include "acpilock.h"
#include "acpintfy.h"
#include "acpioprg.h"
#include "acpiterm.h"

//
// These are the directory dependent files for this project
//
#include "acpiirp.h"
#include "acpilog.h"
#include "acpiosnt.h"
#include "amlisupp.h"
#include "acpidock.h"
#include "buildsrc.h"
#include "bus.h"
#include "button.h"
#include "cmbutton.h"
#include "dat.h"
#include "debug.h"
#include "detect.h"
#include "devpower.h"
#include "dispatch.h"
#include "errlog.h"
#include "extlist.h"
#include "filter.h"
#include "get.h"
#include "gpe.h"
#include "idevice.h"
#include "init.h"
#include "interfaces.h"
#include "internal.h"
#include "interupt.h"
#include "irqarb.h"
#include "msi.h"
#include "match.h"
#include "osnotify.h"
#include "pciopregion.h"
#include "processor.h"
#include "rangesup.h"
#include "res_bios.h"
#include "res_cm.h"
#include "root.h"
#include "rtl.h"
#include "syspower.h"
#include "table.h"
#include "thermal.h"
#include "vector.h"
#include "wake.h"
#include "wmilog.h"
#include "worker.h"


//
// Make sure to have the correct tag
//
#ifdef ExAllocatePool
    #undef ExAllocatePool
#endif
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'ipcA')


#pragma once

#include "master.h"
#include "masternt.h"

#define REGMERGE_TICK_THRESHOLD 125

BOOL
FilterObject (
    IN OUT  PDATAOBJECT SrcObPtr
    );

PBYTE
FilterRegValue (
    IN      PBYTE Data,
    IN      DWORD DataSize,
    IN      DWORD DataType,
    IN      PCTSTR KeyForDbgMsg,        OPTIONAL
    OUT     PDWORD NewDataSize
    );


BOOL
CopyHardwareProfiles (
    IN  HINF InfFile
    );



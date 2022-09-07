/*++


Module Name:

    wmiext.cxx

Abstract:

    This module contains the default ntsd debugger extensions for


Author:

    Ivan Brugiolo 17-05-2000

Revision History:

--*/

#include "wmiexts.h"

# undef DBG_ASSERT



/************************************************************
 *   Debugger Utility Functions
 ************************************************************/



WINDBG_EXTENSION_APIS ExtensionApis;
HANDLE ExtensionCurrentProcess;

USHORT  g_MajorVersion;
USHORT  g_MinorVersion;

BOOL gChkTarget = FALSE;

BOOL g_KD = FALSE;

DWORD g_AddressSize = sizeof(ULONG_PTR);
BOOL g_bUnextend = FALSE;

/************************************************************
 * The WinDBG required Export
 ************************************************************/
 
LPEXT_API_VERSION
ExtensionApiVersion(
    void
    )

/*++

Function Description:

    Windbg calls this function to match between the version of windbg and the
    extension. If the versions doesn't match, windbg will not load the 
extension.

--*/

{
    static EXT_API_VERSION ApiVersion =
#ifdef KDEXT_64BIT
       { 5, 0, EXT_API_VERSION_NUMBER64, 0 };
#else
       { 5, 0, EXT_API_VERSION_NUMBER, 0 };
#endif       
        

    return &ApiVersion;
}


void
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS  lpExtensionApis,
    USHORT                  MajorVersion,
    USHORT                  MinorVersion
    )

/*++

Function Description:

    When windbg loads the extension, it first call this function. You can
    perform various intialization here.

Arguments:

    lpExtensionApis - A structure that contains the callbacks to functions that
        I can use to do standard operation. I must store this in a global
        variable called 'ExtensionApis'.

    MajorVersion - Indicates if target machine is running checked build or 
free.
        0x0C - Checked build.
        0x0F - Free build.

    MinorVersion - The Windows NT build number (for example, 1381 for NT4).

--*/

{
    ExtensionApis = *lpExtensionApis;
    g_MajorVersion = MajorVersion;
    g_MinorVersion = MinorVersion;


    gChkTarget = MajorVersion == 0x0c ? TRUE : FALSE;

#ifdef KDEXT_64BIT
    KDDEBUGGER_DATA64 KdDebuggerData;
#else
    KDDEBUGGER_DATA32 KdDebuggerData;
#endif

    KdDebuggerData.Header.OwnerTag = KDBG_TAG;
    KdDebuggerData.Header.Size = sizeof( KdDebuggerData );

    if (Ioctl( IG_GET_DEBUGGER_DATA, &KdDebuggerData, sizeof( KdDebuggerData ) ))
    {
        g_KD = 1;
    }

    if (sizeof(ULONG64) == sizeof(ULONG_PTR)) //IsPtr64())
    {
        g_AddressSize = 8;
        g_bUnextend = FALSE;
    }
    else
    {
        g_AddressSize = 4;
        g_bUnextend = TRUE;
    }    
}


void
CheckVersion( void )

/*++

Function Description:

    This function is called before every command. It gives the extension
    a chance to compare between the versions of the target and the extension.
    In this demo, I don't do much with that.

--*/

{
    return;
}


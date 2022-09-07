/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 1996-2002 Microsoft Corporation
//
//  Module Name:
//      cluster.cpp
//
//  Description:
//      Cluster.exe main source file.
//      Implements the first level of parsing and hands off execution
//      to appropriate modules.
//
//  Maintained By:
//      David Potter (DavidP                20-ARP-2001
//      Michael Burton (t-mburt)            04-Aug-1997
//      Charles Stacy Harris III (stacyh)   20-March-1997
//
//  Notes:
//
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include <atlimpl.cpp>

#include "cmdline.h"

#include "cluswrap.h"
#include "cluscmd.h"

#include "nodecmd.h"
#include "resgcmd.h"
#include "rescmd.h"

#include "restcmd.h"
#include "netcmd.h"
#include "neticmd.h"

#include "util.h"

CComModule _Module;

#include <atlcom.h>

#include <initguid.h>
#include "Guids.h"      // for minor TASK IDs

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()
    
//  Used by files from Mgmt\ClusCfg project.
HINSTANCE g_hInstance = GetModuleHandle( NULL );

/////////////////////////////////////////////////////////////////////////////
//++
//
//  DispatchCommand
//
//  Routine Description:
//      Identifies the command type and instantiates a class of the
//      specified type to handle remaining options.
//
//  Arguments:
//      IN  CCommandLine & theCmdLine
//          This object contains the parsed command line.
//
//  Return Value:
//      ERROR_SUCCESS               on success
//      Win32 Error code            on failure
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD DispatchCommand( CCommandLine & theCmdLine )
{
    DWORD dwReturnValue = ERROR_SUCCESS;
    CString strClusterName( theCmdLine.GetClusterName() );
    const vector<CString> & strvectorClusterNames( theCmdLine.GetClusterNames() ); 

    if ( strClusterName.GetLength() >= MAX_PATH )
    {
        //  Throw an exception so that wmain returns a nonzero value to the shell.
        CSyntaxException se;
        se.LoadMessage( MSG_NAMED_PARAMETER_TOO_LONG, strClusterName, MAX_PATH - 1 );
        throw se;
    }

    // Special case: If user enters "." as the name of the cluster, pass ""
    // as the name of the cluster.
    if ( strClusterName.CompareNoCase( L"." ) == 0 )
        strClusterName.Empty();

    switch( theCmdLine.GetObjectType() )
    {
        case objCluster:
            // Create new scope for command object...
            {   
                CClusterCmd c( strClusterName, theCmdLine, strvectorClusterNames);
                dwReturnValue = c.Execute();
            }
            break;

        case objNode:
            // Create new scope for command object...
            {
                CNodeCmd c( strClusterName, theCmdLine );
                dwReturnValue = c.Execute();
            }
            break;

        case objGroup:
            // Create new scope for command object...
            {
                CResGroupCmd c( strClusterName, theCmdLine );
                dwReturnValue = c.Execute();
            }
            break;

        case objResource:
            // Create new scope for command object...
            {
                CResourceCmd c( strClusterName, theCmdLine );
                dwReturnValue = c.Execute();
            }
            break;

        case objResourceType:
            // Create new scope for command object...
            {
                CResTypeCmd c( strClusterName, theCmdLine );
                dwReturnValue = c.Execute();
            }
            break;

        case objNetwork:
            // Create new scope for command object...
            {
                CNetworkCmd c( strClusterName, theCmdLine );
                dwReturnValue = c.Execute();
            }
           break;

        case objNetInterface:
            // Create new scope for command object...
            {
                CNetInterfaceCmd c( strClusterName, theCmdLine );
                dwReturnValue = c.Execute();
            }

            break;

        default:
        {
            const CString & strObjectName = theCmdLine.GetObjectName();
            CSyntaxException se;
            
            if ( strObjectName.IsEmpty() )
            {
                se.LoadMessage( IDS_NO_OBJECT_TYPE );
            }
            else
            {
                se.LoadMessage( IDS_INVALID_OBJECT_TYPE, strObjectName );
            }

            throw se;
        }
    }


    if ( dwReturnValue > 0 ) // usage errors are < ERROR_SUCCESS
    {
        if ( HRESULT_FACILITY( dwReturnValue ) == FACILITY_WIN32 )
        {
            dwReturnValue = HRESULT_CODE( dwReturnValue );
        }
        PrintSystemError( dwReturnValue );
    }

    return dwReturnValue;

} //*** DispatchCommand()


/////////////////////////////////////////////////////////////////////////////
//++
//
//  wmain
//
//  Routine Description:
//      Gets the command line, calls functions to parse it and pass the control
//      to the appropriate command handlers.
//
//  Arguments:
//      None.
//
//  Return Value:
//      Same as DispatchCommand
//
//--
/////////////////////////////////////////////////////////////////////////////
extern "C" int __cdecl wmain()
{
    int             nReturnValue;
    CString         cmdLine = GetCommandLine();
    CCommandLine    cmd( cmdLine );
    HRESULT         hr = S_OK;
    BOOL            fComInitialized = FALSE;

    //
    //  Set the process up for tracing.
    //
    TraceInitializeProcess( NULL );

    _Module.Init( ObjectMap, GetModuleHandle( NULL ) );
    hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE );
    if ( FAILED( hr ) )
    {
        nReturnValue = HRESULT_CODE( hr );
        goto Cleanup;
    } // if:
    fComInitialized = TRUE;

    hr = CoInitializeSecurity(
                    NULL,
                    -1,
                    NULL,
                    NULL,
                    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    NULL,
                    EOAC_NONE,
                    0
                    );
    if ( FAILED( hr ) )
    {
        nReturnValue = HRESULT_CODE( hr );
        goto Cleanup;
    } // if:

    //
    // Set the proper thread UI
    //
    hr = RtlSetThreadUILanguage( 0 ); // Pass 0, this is a reserved input parameter
    if ( FAILED( hr ) ) 
    {
        nReturnValue = HRESULT_CODE( hr );
        goto Cleanup;
    }

    //
    // Set the proper codepage to use for CRT routines.
    //
    MatchCRTLocaleToConsole( ); // okay to proceed if this fails?
    
    try
    {
        cmd.ParseStageOne();
        nReturnValue = DispatchCommand( cmd );
    }
    catch( CSyntaxException & se )
    {
        PrintString( se.m_strErrorMsg );
        PrintMessage( se.SeeHelpID() );
        
        //  A script might want to know whether something went wrong;
        //  unfortunately, the exception classes don't carry error codes with them, so use -1.
        nReturnValue = -1;
    }
    catch( CException & e )
    {
        PrintString( e.m_strErrorMsg );
        nReturnValue = -1;
    }

Cleanup:

    if ( fComInitialized )
        CoUninitialize();

    TraceTerminateProcess();

    return nReturnValue;

} //*** wmain()

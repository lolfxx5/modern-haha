/*++

Copyright (c) 1995-1997  Microsoft Corporation

Module Name:

    clusrtl.c

Abstract:

    Provides run-time library support common to any module
    of the NT Cluster.

Author:

    John Vert (jvert) 1-Dec-1995

Revision History:

--*/
#include "clusrtlp.h"
#include "stdarg.h"
#include "stdlib.h"
#include "clusverp.h"
#include "windns.h"
#include "security.h"
#include "secext.h"
#include "sddl.h"

#define WMI_TRACING 1
#define RPC_WMI_TRACING 1

#if defined(WMI_TRACING)

// 789aa2d3-e298-4d8b-a3a3-a8a0ec9c7702 -- Rpc
// b1599392-1a0f-11d3-ba86-00c04f8eed00 -- ClusSvc

#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(ClusRtl,(b1599392,1a0f,11d3,ba86,00c04f8eed00), \
      WPP_DEFINE_BIT(Error)      \
      WPP_DEFINE_BIT(Unusual)    \
      WPP_DEFINE_BIT(Noise)      \
      WPP_DEFINE_BIT(Watchdog)      \
   )        \
   WPP_DEFINE_CONTROL_GUID(ClusRpc,(789aa2d3,e298,4d8b,a3a3,a8a0ec9c7702), \
      WPP_DEFINE_BIT(RpcTrace)   \
   )

//#define WppDebug(x,y) ClRtlPrintf y
#include "clusrtl.tmh"

#define REG_TRACE_CLUSTERING        L"Clustering Service"


#endif // defined(WMI_TRACING)

//
// Local Macros
//

//
// SC Manager failure action parameters. set STARTUP_FAILURE_RESTART to one
// before shipping to get the normal backoff behavior.
//

#if STARTUP_FAILURE_RESTART
#define CLUSTER_FAILURE_RETRY_COUNT             -1  // forever
#else
#define CLUSTER_FAILURE_RETRY_COUNT             0
#endif

#define CLUSTER_FAILURE_MAX_STARTUP_RETRIES             30
#define CLUSTER_FAILURE_INITIAL_RETRY_INTERVAL          60 * 1000           // 60 secs
#define CLUSTER_FAILURE_FINAL_RETRY_INTERVAL            ( 60 * 1000 * 16)   // 16 mins

#define ClRtlAcquirePrintLock() \
        WaitForSingleObject( ClRtlPrintFileMutex, INFINITE );

#define ClRtlReleasePrintLock() \
        ReleaseMutex( ClRtlPrintFileMutex );

#define LOGFILE_NAME L"Cluster.log"

//
// DON'T CHANGE THIS CONSTANT UNLESS YOU REALLY KNOW WHAT YOU'RE
// DOING. ASSUMPTIONS HAVE BEEN MADE ABOUT IT'S SIZE AND THINGS WILL BREAK IF
// YOU MAKE IT TOO SMALL. - charlwi 4/1/02 (and, no, this is not an April
// Fool's joke).
//
#define LOGENTRY_BUFFER_SIZE 512


//
// Private Data
//
BOOL                ClRtlpDbgOutputToConsole = FALSE;
BOOL                ClRtlpInitialized = FALSE;
BOOL                ClRtlPrintToFile = FALSE;
HANDLE              ClRtlPrintFile = NULL;
HANDLE              ClRtlPrintFileMutex = NULL;
DWORD               ClRtlProcessId;
PDWORD              ClRtlDbgLogLevel;
HANDLE				ClRtlWatchdogTimerQueue = NULL;

#define MAX_NUMBER_LENGTH 20

// Specify maximum file size ( DWORD / 1MB )

#define MAX_FILE_SIZE ( 0xFFFFF000 / ( 1024 * 1024 ) )

DWORD               ClRtlPrintFileLimit = ( 8 * 1024 * 1024 ); // 8 MB default
DWORD               ClRtlPrintFileLoWater = 0;

//
// Public Routines
//

// !!!!NOTE!!!!
//
// This initialization routine is call from DllMain(), do not add anyting out
// here that requires synchronization. Do not add any win32 api calls here.
//

DWORD
ClRtlInitialize(
    IN  BOOL    DbgOutputToConsole,
    IN  PDWORD  DbgLogLevel
    )
{
    WCHAR   logFileBuffer[MAX_PATH];
    LPWSTR  logFileName = NULL;
    DWORD   Status = ERROR_SUCCESS;
    DWORD   defaultLogSize = 8;
    HKEY    ClusterKey;
    WCHAR   modulePath[MAX_PATH];
    DWORD   envLength;
    WCHAR   logFileSize[MAX_NUMBER_LENGTH];
    DWORD   logSize;
    LPWSTR  lpszBakFileName = NULL;
    DWORD   fileSizeHigh = 0;
    DWORD   fileSizeLow;

    UNICODE_STRING  logFileString;

    SECURITY_ATTRIBUTES logFileSecurityAttr;

    PSECURITY_DESCRIPTOR    logFileSecurityDesc;

    //
    // init event stuff so we have a means for logging other failures
    //
    ClRtlEventLogInit();

    if (!ClRtlpInitialized) {
        ClRtlpDbgOutputToConsole = DbgOutputToConsole;
        ClRtlpInitialized = TRUE;
        ClRtlDbgLogLevel = DbgLogLevel;

        //
        // GetEnvironmentVariable returns the count minus the null if the
        // buffer is large enough. Otherwise, the return length includes space
        // for the trailing null.
        //
        // the code that deals with the clusterlog and clusterlogsize
        // env. variables is dup'ed in OmpOpenObjectLog
        // (service\om\omlog.c). Any changes made here should be prop'ed to
        // that area if appropriate.
        //
        envLength = GetEnvironmentVariable(L"ClusterLog",
                                            logFileBuffer,
                                            RTL_NUMBER_OF( logFileBuffer ));

        if ( envLength > RTL_NUMBER_OF( logFileBuffer )) {

            logFileName = LocalAlloc( LMEM_FIXED,
                                      envLength * sizeof( WCHAR ) );
            if ( logFileName == NULL ) {
                return GetLastError();
            }

            envLength = GetEnvironmentVariable(L"ClusterLog",
                                                logFileName,
                                                envLength);
            if ( envLength == 0 ) {
                LocalFree( logFileName );
                logFileName = NULL;
            }
        } else if ( envLength != 0 ) {
            logFileName = logFileBuffer;
        }

        //
        // remove any trailing white space. go to the end of the string and
        // scan backwards; stop when we find the first non-white space char or
        // we hit the beginning of the buffer.
        //
        if ( logFileName != NULL ) {
            PWCHAR  p = logFileName + envLength - 1;

            while ( iswspace( *p )) {
                *p = UNICODE_NULL;

                if ( p == logFileName ) {
                    break;
                }

                --p;
            }

            //
            // make sure something useful is left
            //
            if ( wcslen( logFileName ) == 0 ) {
                if ( logFileName != logFileBuffer ) {
                    LocalFree( logFileName );
                }

                logFileName = NULL;
            }
        }

#if CLUSTER_BETA

        //
        // always turn on logging when in beta mode
        //
        if ( ( logFileName != NULL ) && ( *logFileName == UNICODE_NULL ) ) {
            WCHAR *p;

            if ( GetModuleFileName(NULL,
                                   modulePath,
                                   MAX_PATH - sizeof(LOGFILE_NAME)/sizeof(WCHAR) ) ) {
                p = wcsrchr( modulePath, '\\' );
                if ( p != UNICODE_NULL ) {
                    p++;
                    *p = UNICODE_NULL;
                    wcscat( modulePath, LOGFILE_NAME );
                    logFileName = modulePath;
                }
            }
        }
#endif

        if ( logFileName != NULL ) {
            //
            // Try to get a limit on the log file size.
            // This number is the number of MB.
            //
            envLength = GetEnvironmentVariable(L"ClusterLogSize",
                                                logFileSize,
                                                RTL_NUMBER_OF( logFileSize ));
            if ( (envLength != 0) &&
                 (envLength < MAX_NUMBER_LENGTH) ) {
                RtlInitUnicodeString( &logFileString, logFileSize );
                Status = RtlUnicodeStringToInteger( &logFileString,
                                                    10,
                                                    &logSize );
                if ( NT_SUCCESS( Status ) ) {
                    ClRtlPrintFileLimit = logSize;
                }
            } else {
                ClRtlPrintFileLimit = defaultLogSize;
            }

            Status = ERROR_SUCCESS;

            if ( ClRtlPrintFileLimit == 0 ) {
                goto exit;
            }

            if ( ClRtlPrintFileLimit > MAX_FILE_SIZE ) {
                ClRtlPrintFileLimit = MAX_FILE_SIZE;
            }
            ClRtlPrintFileLimit = ClRtlPrintFileLimit * ( 1024 * 1024 );

            ClRtlPrintFileMutex = CreateMutex( NULL,
                                               FALSE,
                                               L"ClusterRtlPrintFileMutex" );
            if ( ClRtlPrintFileMutex != NULL ) {
                BOOL createdDirectory = FALSE;
                //
                //  Chittur Subbaraman (chitturs) - 11/11/98
                //
                //  Check whether the ClusterLogOverwrite environment var is
                //  defined.
                //
                envLength = GetEnvironmentVariable( L"ClusterLogOverwrite",
                                                    NULL,
                                                    0 );
                if ( envLength != 0 )
                {
                    HANDLE  hLogFile = INVALID_HANDLE_VALUE;
                    WCHAR   bakExtension[] = L".bak";

                    //
                    //  Check whether someone else has an open handle to
                    //  the log file.  If so, don't attempt anything.
                    //
                    hLogFile = CreateFile( logFileName,
                                           GENERIC_READ | GENERIC_WRITE,
                                           0, // Exclusive file share mode
                                           NULL,
                                           OPEN_EXISTING,
                                           0,
                                           NULL );
                    if ( hLogFile != INVALID_HANDLE_VALUE )
                    {
                        CloseHandle( hLogFile );

                        lpszBakFileName = LocalAlloc( LMEM_FIXED,
                                                      ( RTL_NUMBER_OF( bakExtension ) + lstrlenW( logFileName ) ) *
                                                      sizeof( WCHAR ) );
                        if ( lpszBakFileName == NULL )
                        {
                            Status = GetLastError();
                            ClRtlDbgPrint(LOG_CRITICAL,
                                          "[ClRtl] Mem alloc for .bak file name failed. Error %1!u!\n",
                                          Status);
                            goto exit;
                        }

                        //
                        //  Append ".bak" to the log file name
                        //
                        lstrcpyW( lpszBakFileName, logFileName );
                        lstrcatW( lpszBakFileName, bakExtension );

                        //
                        //  Move the log file (if it exists) to a bak
                        //  file. Moving preserves the ACL on the file.
                        //
                        if ( !MoveFileExW( logFileName, lpszBakFileName, MOVEFILE_REPLACE_EXISTING )) {
                            //
                            //  There is no reason for this to happen since the
                            //  log file should be deletable.
                            //
                            Status = GetLastError();
                            ClRtlDbgPrint(LOG_CRITICAL,
                                          "[ClRtl] Error %1!u! in renaming cluster log file.\n",
                                          Status);
                            goto exit;
                        }
                    }
                }

                //
                // create a SD giving only local admins and localsystem
                // access.
                //
                if ( !ConvertStringSecurityDescriptorToSecurityDescriptor(
                          L"D:(A;;FA;;;BA)(A;;FA;;;SY)",
                          SDDL_REVISION_1,
                          &logFileSecurityDesc,
                          NULL
                          )
                   )
                {
                    logFileSecurityDesc = NULL;
                }

                logFileSecurityAttr.nLength = sizeof( logFileSecurityAttr );
                logFileSecurityAttr.lpSecurityDescriptor = logFileSecurityDesc;
                logFileSecurityAttr.bInheritHandle = FALSE;

openFileRetry:
                ClRtlPrintFile = CreateFile(logFileName,
                                            GENERIC_READ | GENERIC_WRITE,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                                            &logFileSecurityAttr,
                                            OPEN_ALWAYS,
                                            0,
                                            NULL );

                if ( ClRtlPrintFile == INVALID_HANDLE_VALUE ) {
                    Status = GetLastError();

                    if ( !createdDirectory && Status == ERROR_PATH_NOT_FOUND ) {
                        PWCHAR lastSlash = wcsrchr( logFileName, '\\' );
                        WCHAR  slashChar;

                        if ( lastSlash == NULL ) {
                            lastSlash = wcsrchr( logFileName, '/' );
                        }

                        if ( lastSlash != NULL ) {
                            slashChar = *lastSlash;
                            *lastSlash = UNICODE_NULL;
                            Status = ClRtlCreateDirectory( logFileName );

                            if ( Status == ERROR_SUCCESS ) {
                                createdDirectory = TRUE;
                                *lastSlash = slashChar;
                                goto openFileRetry;
                            }
                        }
                    }

                    ClRtlDbgPrint(LOG_CRITICAL,
                                  "[ClRtl] Open of log file failed. Error %1!u!\n",
                                  Status);
                    goto exit;
                } else {
                    ClRtlPrintToFile = TRUE;
                    ClRtlProcessId = GetCurrentProcessId();

                    //
                    // determine the initial low water mark. We have 3 cases
                    // we need to handle:
                    // 1) log size is less than 1/2 limit
                    // 2) log size is within limit but more than 1/2 limit
                    // 3) log size is greater than limit
                    //
                    // case 1 requires nothing special; the low water mark
                    // will be updated on the next log write.
                    //
                    // for case 2, we need to find the beginning of a line
                    // near 1/2 the current limit. for case 3, the place to
                    // start looking is current log size - 1/2 limit. In this
                    // case, the log will be truncated before the first write
                    // occurs, so we need to take the last 1/2 limit bytes and
                    // copy them down to the front.
                    //
                    //

                    ClRtlAcquirePrintLock();
                    fileSizeLow = GetFileSize( ClRtlPrintFile, &fileSizeHigh );
                    if ( fileSizeLow < ( ClRtlPrintFileLimit / 2 )) {
                        //
                        // case 1: leave low water at zero; it will be updated
                        // with next log write
                        //
                        ;
                    } else {
#define LOGBUF_SIZE 1024
                        CHAR    buffer[LOGBUF_SIZE];
                        LONG    currentPosition;
                        DWORD   bytesRead;

                        if ( fileSizeLow < ClRtlPrintFileLimit ) {
                            //
                            // case 2; start looking at the 1/2 the current
                            // limit to find the starting position
                            //
                            currentPosition = ClRtlPrintFileLimit / 2;
                        } else {
                            //
                            // case 3: start at current size minus 1/2 limit
                            // to find our starting position.
                            //
                            currentPosition  = fileSizeLow - ( ClRtlPrintFileLimit / 2 );
                        }

                        //
                        // read in a block (backwards) from the initial file
                        // position and look for a newline char. When we find
                        // one, the next char is the first char on a new log
                        // line. use that as the initial starting position
                        // when we finally truncate the file.
                        //
                        ClRtlPrintFileLoWater = 0;
                        currentPosition -= LOGBUF_SIZE;

                        SetFilePointer(ClRtlPrintFile,
                                       currentPosition,
                                       &fileSizeHigh,
                                       FILE_BEGIN);
                        if ( ReadFile(ClRtlPrintFile,
                                      buffer,
                                      LOGBUF_SIZE,
                                      &bytesRead,
                                      NULL ) )
                        {
                            PCHAR p = &buffer[ bytesRead - 1 ];

                            while ( *p != '\n' && bytesRead-- != 0 ) {
                                --p;
                            }
                            if ( *p == '\n' ) {
                                ClRtlPrintFileLoWater = (DWORD)(currentPosition + ( p - buffer + 1 ));
                            }
                        }

                        if ( ClRtlPrintFileLoWater == 0 ) {
                            //
                            // couldn't find any reasonable data. just set it to
                            // initial current position.
                            //
                            ClRtlPrintFileLoWater = currentPosition + LOGBUF_SIZE;
                        }
                    }
                    ClRtlReleasePrintLock();
                }

                LocalFree( logFileSecurityDesc );

            } else {
                Status = GetLastError();
                ClRtlDbgPrint(LOG_UNUSUAL,
                              "[ClRtl] Unable to create print file mutex. Error %1!u!.\n",
                              Status);
                Status = ERROR_SUCCESS;
                //goto exit;
            }
        }
    }

exit:
    if ( logFileName != logFileBuffer && logFileName != modulePath ) {
        LocalFree( logFileName );
    }

    //
    //  Chittur Subbaraman (chitturs) - 11/11/98
    //
    if ( lpszBakFileName != NULL )
    {
        LocalFree( lpszBakFileName );
    }

    return Status;

} // ClRtlInitialize

#ifdef RPC_WMI_TRACING

typedef
DWORD (*I_RpcEnableWmiTraceFunc )(
            VOID* fn,               // Rpc now uses TraceMessage, no need to pass trace func
            WPP_WIN2K_CONTROL_BLOCK ** pHandle
      );

HINSTANCE hInstRpcrt4;

#endif


DWORD
ClRtlIsServicesForMacintoshInstalled(
    OUT BOOL * pfInstalled
    )

/*++

Routine Description:

    Determines if SFM is installed on the local system.

Arguments:

    pfInstalled - pointer to a boolean flag to return whether SFM is installed
    returns: TRUE if SFM is installed
             FALSE if SFM is not installed

Return Value:

    Status of request. ERROR_SUCCESS if valid info in pfInstalled.
       Error Code otherwise. On error pfInstalled (if present) is set to FALSE

--*/

{
    HANDLE  scHandle;
    HANDLE  scServiceHandle;

    if ( ARGUMENT_PRESENT( pfInstalled ) ) {
        *pfInstalled = FALSE;
    } else {
        return ERROR_INVALID_PARAMETER;
    }
    scHandle = OpenSCManager(
        NULL,       // Open on local machine
        NULL,       // Open SERVICES_ACTIVE_DATABASE
        GENERIC_READ );
    if ( scHandle == NULL ) {
        return( GetLastError() );
    }

    scServiceHandle = OpenService(
                scHandle,
                L"macfile",
                READ_CONTROL );
    if ( scServiceHandle != NULL ) {
        *pfInstalled = TRUE;
    }
    CloseServiceHandle( scServiceHandle );
    CloseServiceHandle( scHandle );

    return ERROR_SUCCESS;

} // ClRtlIsServicesForMacintoshInstalled


DWORD
ClRtlInitWmi(
    LPCWSTR ComponentName
    )
{
#if defined(RPC_WMI_TRACING)
    {
        DWORD Status = ERROR_SUCCESS;
        PWPP_WIN2K_CONTROL_BLOCK RpcCb;
        I_RpcEnableWmiTraceFunc RpcEnableWmiTrace = 0;

        hInstRpcrt4 = LoadLibrary(L"rpcrt4.dll");
        if (hInstRpcrt4) {
            RpcEnableWmiTrace = (I_RpcEnableWmiTraceFunc)
                GetProcAddress(hInstRpcrt4, "I_RpcEnableWmiTrace");

            if (RpcEnableWmiTrace) {

                Status = (*RpcEnableWmiTrace)(0, &RpcCb);
                if (Status == ERROR_SUCCESS) {
                    WPP_SET_FORWARD_PTR(RpcTrace, WPP_VER_WIN2K_CB_FORWARD_PTR, RpcCb);
                }

            } else {
                ClRtlDbgPrint(LOG_UNUSUAL,
                              "[ClRtl] rpcrt4.dll GetWmiTraceEntryPoint failed, status %1!d!.\n",
                              GetLastError() );
            }
        }
    }
#endif // RPC_WMI_TRACING

    WPP_INIT_TRACING(NULL); // Don't need publishing
    WppAutoStart(ComponentName);
    return ERROR_SUCCESS;
}

VOID
ClRtlCleanup(
    VOID
    )
{
    if (ClRtlpInitialized) {
        ClRtlpInitialized = FALSE;
        ClRtlEventLogCleanup();
        CloseHandle ( ClRtlPrintFileMutex );
        CloseHandle ( ClRtlPrintFile );

        //Cleaning up watchdog stuff
        if(ClRtlWatchdogTimerQueue != NULL) {
        	DeleteTimerQueue(ClRtlWatchdogTimerQueue);
        	ClRtlWatchdogTimerQueue = NULL;
        	}			

        WPP_CLEANUP();
    #if defined(RPC_WMI_TRACING)
        if (hInstRpcrt4) {
            FreeLibrary(hInstRpcrt4);
            hInstRpcrt4 = NULL;
        }
    #endif
    }

    return;
}

VOID
ClRtlpWatchdogCallback(
	PVOID par,
	BOOLEAN timedOut
	)
{

    PWATCHDOGPAR pPar=(PWATCHDOGPAR)par;

	if(!timedOut) {
		// The timer was cancelled, get out.
		ClRtlLogPrint(LOG_NOISE,
		    "[ClRtl] Watchdog Timer Cancelled, ThreadId= 0x%1!x! par= %2!ws!.\n",
		    pPar->threadId,
		    pPar->par
		    );
		return;
	}

	ClRtlLogPrint(LOG_CRITICAL,
		"[ClRtl] Watchdog timer timed out, ThreadId= 0x%1!x! par= %2!ws!.\n",
		pPar->threadId,
		pPar->par
		);

#if CLUSTER_BETA
    if (WPP_LEVEL_ENABLED(Watchdog)) {
	// Breaking into NTSD if available or KD. Do it only for cluster beta builds.
	DebugBreak();
    }
#endif

}

PVOID
ClRtlSetWatchdogTimer(
	DWORD  timeout,
	LPWSTR par
	)
{

	PWATCHDOGPAR pPar;

	// Do the initialization here not in ClRtlInitialize()
	if(ClRtlWatchdogTimerQueue == NULL) {
		if((ClRtlWatchdogTimerQueue = CreateTimerQueue()) == NULL) {
			return NULL;
		}
	}

	if((pPar = LocalAlloc(LMEM_FIXED, sizeof(WATCHDOGPAR))) == NULL) {
	    return NULL;
	    }
	pPar->par = par;
	pPar->threadId = GetCurrentThreadId();

	if(!CreateTimerQueueTimer(
			&pPar->wTimer,
			ClRtlWatchdogTimerQueue,
			ClRtlpWatchdogCallback,
			(PVOID)pPar,
			timeout,
			0,
			0)) {
			LocalFree(pPar);
			return NULL;
		}

#if CLUSTER_BETA
	ClRtlLogPrint(LOG_NOISE,
		"[ClRtl] Setting watchdog timer= 0x%1!x!, Timeout= %2!u!(ms), par= %3!ws!.\n",
		pPar->wTimer,
		timeout,
		par
		);
#endif

	return (PVOID)pPar;		

}	

VOID
ClRtlCancelWatchdogTimer(
	PVOID wTimer
	)
{

    PWATCHDOGPAR pPar=(PWATCHDOGPAR)wTimer;

	if((ClRtlWatchdogTimerQueue == NULL) || (wTimer == NULL)) {
		return;
		}

	if(!DeleteTimerQueueTimer(
		ClRtlWatchdogTimerQueue,
		pPar->wTimer,
		INVALID_HANDLE_VALUE
		)) {
		ClRtlLogPrint(LOG_CRITICAL,
			"[ClRtl] Failed to cancel watchdog timer 0x%1!x!.\n",
			pPar->wTimer
			);
		}
	else {
#if CLUSTER_BETA	
		ClRtlLogPrint(LOG_NOISE,
			"[ClRtl] Cancelled watchdog timer 0x%1!x!.\n",
			pPar->wTimer
			);
#endif
		}
	LocalFree(wTimer);
}	
		
		

BOOL
ClRtlCheckForLogCorruption(
    LPSTR pszOutBuffer
    )
//
// Find the log corrupter. There should never be move than 4
// question marks in a row or character below 32 or above 128
// if English.
//
// Returns:
//      TRUE if it is safe to write
//      FALSE if it is NOT safe to write
//
{
    DWORD count;
    WCHAR  szLocale[ 32 ];
    static BOOL fLocaleFound = FALSE;
    static BOOL fEnglish = FALSE;
    DWORD localeBytes;

    if ( !pszOutBuffer )
        return FALSE;

    if ( !fLocaleFound )
    {
        localeBytes = GetLocaleInfoW(LOCALE_SYSTEM_DEFAULT,
                                     LOCALE_SENGLANGUAGE,
                                     szLocale,
                                     RTL_NUMBER_OF( szLocale ));

        if ( localeBytes != 0 )
        {
            if ( lstrcmpiW( szLocale, L"ENGLISH" ) == 0 )
            {
                fEnglish = TRUE;
            }

            fLocaleFound = TRUE;
        }
    }

    for( count = 0; *pszOutBuffer; pszOutBuffer++ )
    {
        if ( *pszOutBuffer == '?' )
        {
            count++;
            if ( count > 4 )
            {
                return FALSE;
            }
        }
        else if ( fEnglish
               && ( ( *pszOutBuffer < 32
                   && *pszOutBuffer != 0x0A    // linefeed
                   && *pszOutBuffer != 0x0D    // creturn
                   && *pszOutBuffer != 0x09 ) // tab
                 || *pszOutBuffer > 128 ) )
        {
            return FALSE;
        }
    }

    return TRUE;

} // ClRtlCheckForLogCorruption

__inline BOOL
ClRtlpIsOutputDeviceAvailable(
    VOID
    )

/*++

Routine Description:

    Description

Arguments:

    None

Return Value:

    None

--*/

{
    //
    // normally, there is nothing to do
    //
    return ( ClRtlpDbgOutputToConsole || IsDebuggerPresent());
} // ClRtlpIsOutputDeviceAvailable

VOID
ClRtlpOutputString(
    IN PCHAR String
    )

/*++

Routine Description:

    Outputs the specified string based on the current settings

Arguments:

    String - Specifies the string to output.

Return Value:

    None.

--*/

{
    static PCRITICAL_SECTION    dbgPrintLock = NULL;
    PCRITICAL_SECTION           testPrintLock;

    //
    // synchronize threads by interlocking the assignment of the global lock.
    //
    if ( dbgPrintLock == NULL ) {
        testPrintLock = LocalAlloc( LMEM_FIXED, sizeof( CRITICAL_SECTION ));
        if ( testPrintLock == NULL ) {
            return;
        }

        InitializeCriticalSection( testPrintLock );
        InterlockedCompareExchangePointer( &dbgPrintLock, testPrintLock, NULL );

        //
        // only one thread did the exchange; the loser deallocates its
        // allocation and switches over to using the real lock
        //
        if ( dbgPrintLock != testPrintLock ) {
            DeleteCriticalSection( testPrintLock );
            LocalFree( testPrintLock );
        }
    }

    EnterCriticalSection( dbgPrintLock );

    //
    // print to console window has precedence. Besides, if console is the
    // debugger window, you get double output
    //
    if (ClRtlpDbgOutputToConsole) {
        printf( "%hs", String );
    } else if ( IsDebuggerPresent()) {
        OutputDebugStringA(String);
    }

    LeaveCriticalSection( dbgPrintLock );

} // ClRtlpOutputString

VOID
ClRtlMsgPrint(
    IN DWORD MessageId,
    ...
    )

/*++

Routine Description:

    Prints a message to the debugger or console, as appropriate

    Does not alter the formatting of the message as it occurs in the message
    file.

Arguments:

    MessageId - The message id of the string to print

    Any FormatMessage compatible arguments to be inserted in the ErrorMessage
    before it is logged.

Return Value:

    None.

--*/

{
    CHAR szOutBuffer[LOGENTRY_BUFFER_SIZE];
    DWORD Bytes;
    NTSTATUS Status;
    va_list ArgList;

    //
    // don't go any further if nothing to do
    //
    if ( !ClRtlpIsOutputDeviceAvailable()) {
        return;
    }

    va_start(ArgList, MessageId);

    try {
        Bytes = FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE,
                               NULL,
                               MessageId,
                               0,
                               szOutBuffer,
                               RTL_NUMBER_OF( szOutBuffer ),
                               &ArgList);
    }
    except ( EXCEPTION_EXECUTE_HANDLER ) {
        Bytes = FormatMessageA(FORMAT_MESSAGE_FROM_STRING
                               | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                               L"LOGERROR(exception): Could not format message ID #%1!u!\n",
                               0,
                               0,
                               szOutBuffer,
                               RTL_NUMBER_OF( szOutBuffer ),
                               (va_list *) &MessageId );
    }

    va_end(ArgList);

    if (Bytes != 0) {
        if ( !ClRtlCheckForLogCorruption( szOutBuffer ) ) {
            Bytes = FormatMessageA(FORMAT_MESSAGE_FROM_STRING
                                   | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                   "LOGERROR: non-ASCII characters detected after formatting of message ID #%1!u!\n",
                                   0,
                                   0,
                                   szOutBuffer,
                                   RTL_NUMBER_OF( szOutBuffer ),
                                   (va_list *) &MessageId );
        }
        ClRtlpOutputString(szOutBuffer);
    }
} // ClRtlMsgPrint


VOID
ClRtlpDbgPrint(
    DWORD LogLevel,
    PCHAR FormatString,
    va_list ArgList
    )
/*++

 Routine Description:

     Prints a message to the debugger or console, as appropriate.

 Arguments:

    LogLevel - Supplies the logging level, one of
                LOG_CRITICAL 1
                LOG_UNUSUAL  2
                LOG_NOISE    3

     String - The initial message string to print.

     Any FormatMessage-compatible arguments to be inserted in the
     ErrorMessage before it is logged.

 Return Value:
     None.

--*/
{
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    WCHAR wszOutBuffer[LOGENTRY_BUFFER_SIZE];
    WCHAR wszInBuffer[LOGENTRY_BUFFER_SIZE];
    CHAR szOutBuffer[LOGENTRY_BUFFER_SIZE];
    NTSTATUS Status;
    DWORD Bytes;

    //
    // don't go any further if nothing to do
    //
    if ( !ClRtlpIsOutputDeviceAvailable()) {
        return;
    }

    //
    // next check that this message isn't filtered out by the current logging
    // level
    //
    if ( ClRtlDbgLogLevel != NULL ) {
        if ( LogLevel > *ClRtlDbgLogLevel ) {
            return;
        }
    }

    RtlInitAnsiString( &AnsiString, FormatString );
    UnicodeString.MaximumLength = LOGENTRY_BUFFER_SIZE;
    UnicodeString.Buffer = wszInBuffer;
    Status = RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, FALSE );
    if ( !NT_SUCCESS( Status ) ) {
        return;
    }

    try {
        Bytes = FormatMessageW(FORMAT_MESSAGE_FROM_STRING,
                               UnicodeString.Buffer,
                               0,
                               0,
                               wszOutBuffer,
                               RTL_NUMBER_OF( wszOutBuffer ),
                               &ArgList);
    }
    except ( EXCEPTION_EXECUTE_HANDLER ) {
        Bytes = FormatMessageW(FORMAT_MESSAGE_FROM_STRING
                               | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                               L"LOGERROR(exception): Could not print message: %1!hs!.",
                               0,
                               0,
                               wszOutBuffer,
                               RTL_NUMBER_OF( wszOutBuffer ),
                               (va_list *) &FormatString );
    }

    if (Bytes != 0) {
        UnicodeString.Length = (USHORT) Bytes * sizeof(WCHAR);
        UnicodeString.Buffer = wszOutBuffer;
        AnsiString.MaximumLength = LOGENTRY_BUFFER_SIZE;
        AnsiString.Buffer = szOutBuffer;
        Status = RtlUnicodeStringToAnsiString( &AnsiString, &UnicodeString, FALSE );
        if ( NT_SUCCESS( Status ) ) {
            if ( ClRtlCheckForLogCorruption( AnsiString.Buffer ) ) {
                ClRtlpOutputString(szOutBuffer);
            }
            else
            {
                Bytes = FormatMessageA(FORMAT_MESSAGE_FROM_STRING
                                       | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                       "LOGERROR: non-ASCII characters in formatted message: %1!hs!",
                                       0,
                                       0,
                                       szOutBuffer,
                                       RTL_NUMBER_OF( szOutBuffer ),
                                       (va_list *) &FormatString );

                if ( Bytes > 0 ) {
                    ClRtlpOutputString(szOutBuffer);
                    if ( szOutBuffer[ Bytes - 1 ] != '\n' ) {
                        ClRtlpOutputString( "\n" );
                    }
                }
            }
        }
    }

} // ClRtlpDbgPrint


VOID
ClRtlDbgPrint(
    DWORD LogLevel,
    PCHAR FormatString,
    ...
    )
/*++

 Routine Description:

     Prints a message to the debugger or console, as appropriate.

 Arguments:

    LogLevel - Supplies the logging level, one of
                LOG_CRITICAL 1
                LOG_UNUSUAL  2
                LOG_NOISE    3

     String - The initial message string to print.

     Any FormatMessage-compatible arguments to be inserted in the
     ErrorMessage before it is logged.

 Return Value:
     None.

--*/
{
    va_list ArgList;

    va_start(ArgList, FormatString);
    ClRtlpDbgPrint( LogLevel, FormatString, ArgList );
    va_end(ArgList);

} // ClRtlDbgPrint


VOID
ClRtlPrintf(
    PCHAR FormatString,
    ...
    )
/*++

 Routine Description:

     Prints a message to the debugger or console, as appropriate.

 Arguments:

     Just like printf

 Return Value:
     None.

--*/
{
    char buf[128];
    va_list ArgList;

    va_start(ArgList, FormatString);

    _vsnprintf(buf, RTL_NUMBER_OF(buf), FormatString, ArgList);
    buf[ RTL_NUMBER_OF(buf) - 1 ] = 0;

    ClRtlLogPrint( 1, "%1!hs!", buf);

    va_end(ArgList);

} // ClRtlPrintf

DWORD
ClRtlpTruncateFile(
    IN HANDLE FileHandle,
    IN DWORD FileSize,
    IN LPDWORD LastPosition
    )

/*++

Routine Description:

    Truncate a file from the front.

Arguments:

    FileHandle - File handle.

    FileSize - Current End of File.

    LastPosition - Move from this last position to end-of-file to beginning.

Return Value:

    New end of file.

--*/

{
//
// The following buffer size should never be more than 1/4 the size of the
// file.
//
#define BUFFER_SIZE ( 64 * 1024 )
    DWORD   bytesLeft;
    DWORD   endPosition = 0;
    DWORD   bufferSize;
    DWORD   bytesRead;
    DWORD   bytesWritten;
    DWORD   fileSizeHigh = 0;
    DWORD   readPosition;
    DWORD   writePosition;
    PVOID   dataBuffer;


    if ( *LastPosition >= FileSize ) {
        goto error_exit;
    }

    bytesLeft = FileSize - *LastPosition;
    dataBuffer = LocalAlloc( LMEM_FIXED, BUFFER_SIZE );
    if ( !dataBuffer ) {
        goto error_exit;
    }
    endPosition = bytesLeft;

    //
    // Point back to last position for reading.
    //
    readPosition = *LastPosition;
    writePosition = 0;

    while ( bytesLeft ) {
        if ( bytesLeft >= BUFFER_SIZE ) {
            bufferSize = BUFFER_SIZE;
        } else {
            bufferSize = bytesLeft;
        }
        bytesLeft -= bufferSize;
        SetFilePointer( FileHandle,
                        readPosition,
                        &fileSizeHigh,
                        FILE_BEGIN );
        if ( ReadFile( FileHandle,
                       dataBuffer,
                       bufferSize,
                       &bytesRead,
                       NULL ) ) {

            SetFilePointer( FileHandle,
                            writePosition,
                            &fileSizeHigh,
                            FILE_BEGIN );
            WriteFile( FileHandle,
                       dataBuffer,
                       bytesRead,
                       &bytesWritten,
                       NULL );
        } else {
            endPosition = 0;
            break;
        }
        readPosition += bytesRead;
        writePosition += bytesWritten;
    }

    LocalFree( dataBuffer );

error_exit:

    //
    // Force end of file to get set.
    //
    SetFilePointer( FileHandle,
                    endPosition,
                    &fileSizeHigh,
                    FILE_BEGIN );

    SetEndOfFile( FileHandle );

    *LastPosition = endPosition;

    return(endPosition);

} // ClRtlpTruncateFile


VOID
ClRtlLogPrint(
    ULONG   LogLevel,
    PCHAR   FormatString,
    ...
    )
/*++

 Routine Description:

     Prints a message to a log file.

 Arguments:

    LogLevel - Supplies the logging level, one of
                LOG_CRITICAL 1
                LOG_UNUSUAL  2
                LOG_NOISE    3

     String - The initial message string to print.

     Any FormatMessage-compatible arguments to be inserted in the
     ErrorMessage before it is logged.

 Return Value:

     None.

--*/
{
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    WCHAR wszInBuffer[LOGENTRY_BUFFER_SIZE];
    WCHAR wszOutBuffer[LOGENTRY_BUFFER_SIZE];
    CHAR  szOutBuffer[LOGENTRY_BUFFER_SIZE];
    DWORD MsgChars;
    DWORD PrefixChars;
    DWORD BytesWritten;
    DWORD FileSize;
    DWORD FileSizeHigh;
    NTSTATUS Status;
    SYSTEMTIME Time;
    ULONG_PTR ArgArray[9];
    va_list ArgList;
    PWCHAR logLabel;
#define CLRTL_LOG_LABEL_LENGTH  5

    //
    // init the variable arg list
    //
    va_start(ArgList, FormatString);

    ClRtlpDbgPrint( LogLevel, FormatString, ArgList );

    if ( !ClRtlPrintToFile ) {
        va_end(ArgList);
        return;
    }

    // begin_wpp config
    // CUSTOM_TYPE(level, ItemListByte(UNK0, ERR_, WARN, INFO) );
    // end_wpp

    //
    // convert nuemric LogLevel to something readable. If you change the
    // labels, make sure all log labels are the same length and that
    // CLRTL_LOG_LABEL_LENGTH reflects the new length.
    //
    switch ( LogLevel ) {
    case LOG_NOISE:
        logLabel = L"INFO ";
        break;

    case LOG_UNUSUAL:
        logLabel = L"WARN ";
        break;

    case LOG_CRITICAL:
        logLabel = L"ERR  ";
        break;

    default:
        ASSERT( 0 );
        logLabel = L"UNKN ";
        break;
    }

    GetSystemTime(&Time);

    ArgArray[0] = ClRtlProcessId;
    ArgArray[1] = GetCurrentThreadId();
    ArgArray[2] = Time.wYear;
    ArgArray[3] = Time.wMonth;
    ArgArray[4] = Time.wDay;
    ArgArray[5] = Time.wHour;
    ArgArray[6] = Time.wMinute;
    ArgArray[7] = Time.wSecond;
    ArgArray[8] = Time.wMilliseconds;

    //
    // length of this format is 43 chars without trailing null under normal
    // circumstances. FormatMessage will write a larger number if given one
    // that doesn't fit within the field width.
    //
    PrefixChars = FormatMessageW(FORMAT_MESSAGE_FROM_STRING |
                                 FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                 L"%1!08lx!.%2!08lx!::%3!02d!/%4!02d!/%5!02d!-%6!02d!:%7!02d!:%8!02d!.%9!03d! ",
                                 0,
                                 0,
                                 wszOutBuffer,
                                 RTL_NUMBER_OF( wszOutBuffer ),
                                 (va_list*)ArgArray);

    if ( PrefixChars == 0 ) {
        va_end(ArgList);
        WmiTrace("Prefix format failed, %d: %!ARSTR!", GetLastError(), FormatString);
        return;
    }

    //
    // make sure we have some space for the log label and the message
    //
    if (( PrefixChars + CLRTL_LOG_LABEL_LENGTH + 1 ) >= LOGENTRY_BUFFER_SIZE ) {
        va_end(ArgList);
        WmiTrace("Prefix format filled buffer, %!ARSTR!", FormatString);
        return;
    }

    //
    // add on the log label at the end and adjust PrefixChars.
    //
    wcsncat( wszOutBuffer + PrefixChars, logLabel, LOGENTRY_BUFFER_SIZE - PrefixChars );
    PrefixChars += CLRTL_LOG_LABEL_LENGTH;

    // convert the message into unicode
    RtlInitAnsiString( &AnsiString, FormatString );
    UnicodeString.MaximumLength = LOGENTRY_BUFFER_SIZE;
    UnicodeString.Buffer = wszInBuffer;
    Status = RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, FALSE );
    if ( !NT_SUCCESS( Status ) ) {
        va_end(ArgList);
        WmiTrace("AnsiToUni failed, %x: %!ARSTR!", Status, FormatString);
        return;
    }

    try {
        MsgChars = FormatMessageW(FORMAT_MESSAGE_FROM_STRING,
                                  UnicodeString.Buffer,
                                  0,
                                  0,
                                  &wszOutBuffer[PrefixChars],
                                  RTL_NUMBER_OF( wszOutBuffer ) - PrefixChars,
                                  &ArgList);
    }
    except ( EXCEPTION_EXECUTE_HANDLER ) {
        MsgChars = FormatMessageW(FORMAT_MESSAGE_FROM_STRING
                                  | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                  L"LOGERROR(exception): Could not print message: %1!hs!",
                                  0,
                                  0,
                                  &wszOutBuffer[PrefixChars],
                                  RTL_NUMBER_OF( wszOutBuffer ) - PrefixChars,
                                  (va_list *) &FormatString );
    }
    va_end(ArgList);

    if (MsgChars != 0) {

        // convert the out to Ansi
        UnicodeString.Buffer = wszOutBuffer;
        UnicodeString.Length = ((USHORT) MsgChars + (USHORT) PrefixChars) * sizeof(WCHAR);
        AnsiString.Buffer = szOutBuffer;
        AnsiString.MaximumLength = LOGENTRY_BUFFER_SIZE;
        Status = RtlUnicodeStringToAnsiString( &AnsiString, &UnicodeString, FALSE );
        if ( !NT_SUCCESS( Status ) ) {
            WmiTrace("UniToAnsi failed, %x: %!ARWSTR!", Status, wszOutBuffer + PrefixChars);
            return;
        }

        ClRtlAcquirePrintLock();

        FileSize = GetFileSize( ClRtlPrintFile,
                                &FileSizeHigh );
        ASSERT( FileSizeHigh == 0 );        // We're only using DWORDs!
        if ( FileSize > ClRtlPrintFileLimit ) {
            FileSize = ClRtlpTruncateFile( ClRtlPrintFile,
                                           FileSize,
                                           &ClRtlPrintFileLoWater );
        }

        SetFilePointer( ClRtlPrintFile,
                        FileSize,
                        &FileSizeHigh,
                        FILE_BEGIN );
        if ( ClRtlCheckForLogCorruption( AnsiString.Buffer ) )
        {
#if defined(ENCRYPT_TEXT_LOG)
            int i;
            for (i = 0; i < AnsiString.Length; ++i) {
                AnsiString.Buffer[i] ^= 'a';
            }
#endif
            WriteFile(ClRtlPrintFile,
                      AnsiString.Buffer,
                      AnsiString.Length,
                      &BytesWritten,
                      NULL);
#if defined(ENCRYPT_TEXT_LOG)
            for (i = 0; i < AnsiString.Length; ++i) {
                AnsiString.Buffer[i] ^= 'a';
            }
#endif
        }
        else
        {
            MsgChars = FormatMessageA(FORMAT_MESSAGE_FROM_STRING
                                      | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                      "LOGERROR: non-ASCII characters in formatted message: %1!hs!",
                                      0,
                                      0,
                                      &szOutBuffer[PrefixChars],
                                      RTL_NUMBER_OF( szOutBuffer ) - PrefixChars,
                                      (va_list *) &FormatString );

            if ( MsgChars > 0 ) {
                WriteFile(ClRtlPrintFile,
                          szOutBuffer,
                          PrefixChars + MsgChars,
                          &BytesWritten,
                          NULL);

                if ( szOutBuffer[ PrefixChars + MsgChars - 1 ] != '\n' ) {
                    WriteFile(ClRtlPrintFile,
                              "\n",
                              1,
                              &BytesWritten,
                              NULL);
                }

                RtlInitAnsiString( &AnsiString, szOutBuffer );
            }
        }

        if ( (ClRtlPrintFileLoWater == 0) &&
             (FileSize > (ClRtlPrintFileLimit / 2)) ) {
            ClRtlPrintFileLoWater = FileSize + BytesWritten;
        }

        ClRtlReleasePrintLock();

        WmiTrace("%!level! %!ARSTR!", *(UCHAR*)&LogLevel, AnsiString.Buffer + PrefixChars);
/*
#if defined(WMI_TRACING)
        if (ClRtlWml.Trace && ClRtlWmiReg.EnableFlags) {
            ClRtlWml.Trace(10, &ClRtlTraceGuid, ClRtlWmiReg.LoggerHandle,
                LOG(UINT, ClRtlProcessId)
                LOGASTR(AnsiString.Buffer + PrefixChars)
                0);
        }
#endif // defined(WMI_TRACING)
 */
    } else {
        WmiTrace("Format returned 0 bytes: %!ARSTR!", FormatString);
    }
    return;
} // ClRtlLogPrint


VOID
ClRtlpFlushLogBuffers(
    VOID
    )

/*++

Routine Description:

    Flush the cluster log file

Arguments:

    none

Return Value:

    none

--*/

{
    FlushFileBuffers( ClRtlPrintFile );
}

DWORD
ClRtlCreateDirectory(
    IN LPCWSTR lpszPath
    )
/*++

Routine Description:

    Creates a directory creating any subdirectories as required. Allows the
    following forms:

    [drive:][\]dir[\dir...]
    \\?\drive:\dir[\dir...]
    \\?\unc\server\share\dir[\dir...]
    \\server\share\dir[\dir...]

Arguments:

    lpszPath - Supplies the path to the directory.  It may or
               may not be terminated by a back slash.

Return Value:

    ERROR_SUCCESS if successful, else the error code.

--*/
{
    WCHAR   backSlash = L'\\';
    WCHAR   fwdSlash = L'/';
    DWORD   dwLen;
    LPWSTR  pszNext = NULL;
    DWORD   dwError = ERROR_SUCCESS;
    WCHAR   expandedPathPrefix[] = L"\\\\?\\";
    WCHAR   uncPrefix[] = L"\\\\?\\UNC\\";
    DWORD   slashSkipCount = 0;
    PWCHAR  p;
    LPWSTR  dirPath = (LPWSTR)lpszPath;
    BOOL    charsAfterSlash = FALSE;

    //
    // get input string length and validate that we have a string worth using
    //
    dwLen = lstrlenW( lpszPath );
    if ( !lpszPath || dwLen == 0 )
    {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // find the first slash after the first dir spec past the "root". The
    // input string can have a number of different forms at the beginning, so
    // we look for them first.
    //
    // check for a double backslash sequence at the beginning of the
    // string. This could be an SMB path (\\server\share), a expanded storage
    // path (\\?\c:\...) or a UNC path (\\?\UNC\server\share\...)
    //
    if ( dwLen > 2 && dirPath[0]== L'\\' && dirPath[1] == L'\\')
    {
        //
        // is it expanded storage or UNC?
        //
        if (dwLen >= RTL_NUMBER_OF( expandedPathPrefix )
            &&
            wcsncmp( dirPath, expandedPathPrefix, RTL_NUMBER_OF( expandedPathPrefix ) - 1 ) == 0 )
        {
            //
            // now see if it is a UNC name
            //
            if (dwLen >= RTL_NUMBER_OF( uncPrefix )
                &&
                ClRtlStrNICmp( dirPath, uncPrefix, RTL_NUMBER_OF( uncPrefix ) - 1 ) == 0 )
            {
                //
                // skip to end of "\\?\UNC\server\share\xxx\"
                //
                slashSkipCount = 7;
            }
            else
            {
                //
                // it's a expanded storage path. skip to end of \\?\c:\xxx\
                //
                slashSkipCount = 5;
            }
        }
        else
        {
            //
            // it looks like an SMB path; skip to end of \\server\share\xxx\
            //
            slashSkipCount = 5;
        }
    }
    else
    {
        //
        // it should be a normal file path but it could relative or fully
        // qualified, with or without a drive letter. All that really matters
        // is whether there is a slash before the first dir spec. If there is,
        // then skip to the 2nd slash otherwise skip to the first slash.
        //
        if ( dirPath[0] == backSlash || dirPath[0] == fwdSlash )
        {
            //
            // skip to end of \xxx\
            //
            slashSkipCount = 2;
        }
        else if ( dwLen > 1 && dirPath[1] == L':' )
        {
            //
            // might be fully qualified path; check for slash in slot 2
            //
            if ( dwLen > 2 && ( dirPath[2] == L'\\' || dirPath[2] == L'/' )) {
                //
                // skip to end of c:\xxx\
                //
                slashSkipCount = 2;
            } else {
                //
                // skip to the end of c:xxx\
                //
                slashSkipCount = 1;
            }
        }
        else
        {
            //
            // no colon in slot 1 so must be a relative path with no drive
            // letter
            //
            slashSkipCount = 1;
        }
    }

    //
    // get pszNext to point to the appropriate slash; this allows for mixed
    // fwd and bwd slashes which is pretty sick but...
    //
    if ( slashSkipCount > 0 )
    {
        pszNext = dirPath;

        while ( *pszNext != UNICODE_NULL ) {
            if ( *pszNext == backSlash || *pszNext == fwdSlash ) {
                charsAfterSlash = FALSE;
                if ( --slashSkipCount == 0 ) {
                    break;
                }
            } else {
                charsAfterSlash = TRUE;
            }

            ++pszNext;
        }
    } else {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // check that we found all the slashes. If we're missing one slash but
    // we're sitting at the end of the string, then that is the same as
    // finding a slash
    //
    if ( !( slashSkipCount == 0 || ( charsAfterSlash && slashSkipCount == 1 ))) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // run down the directory path, inserting a NULL at every slash and call
    // CreateDirectory to create that portion of the path
    //
    while ( pszNext)
    {
        DWORD_PTR   dwptrLen;
        WCHAR       oldSlash;

        dwptrLen = pszNext - dirPath;

        dwLen = (DWORD)dwptrLen;
        oldSlash = dirPath[ dwLen ];
        dirPath[ dwLen ] = UNICODE_NULL;

        if (!CreateDirectory(dirPath, NULL))
        {
            dwError = GetLastError();
            if (dwError == ERROR_ALREADY_EXISTS)
            {
                DWORD fileAttrs;

                //
                // make sure it is a directory
                //
                fileAttrs = GetFileAttributes( dirPath );
                if ( fileAttrs != INVALID_FILE_ATTRIBUTES )
                {
                    if ( fileAttrs & FILE_ATTRIBUTE_DIRECTORY )
                    {
                        dwError = ERROR_SUCCESS;
                    }
                    else
                    {
                        dwError = ERROR_FILE_EXISTS;
                    }
                }
                else
                {
                    dwError = ERROR_FILE_EXISTS;
                }
            }

            if ( dwError != ERROR_SUCCESS )
            {
                ClRtlDbgPrint(LOG_CRITICAL,
                              "[ClRtl] CreateDirectory Failed on %1!ws!. Error %2!u!\n",
                              dirPath, dwError);
            }
        }

        dirPath[ dwLen ] = oldSlash;

        //
        // bail out if we hit an error or we're at the end of the input string
        //
        if ( dwError != ERROR_SUCCESS || *pszNext == UNICODE_NULL ) {
            break;
        }

        p = pszNext + 1;
        while ( *p != backSlash && *p != fwdSlash && *p != UNICODE_NULL )
            ++p;

        pszNext = p;
    }

    return(dwError);
}





BOOL
WINAPI
ClRtlIsPathValid(
    LPCWSTR  Path
    )

/*++

Routine Description:

    Returns true if the given path looks syntactically valid.

    This call is NOT network-aware.

Arguments:

    Path - String containing a path.

Return Value:

    TRUE if the path looks valid, otherwise FALSE.

--*/

{
    WCHAR   chPrev;
    WCHAR   chCur;
    DWORD   charCount = 0;
#ifdef    DBCS
    BOOL    fPrevLead = FALSE;
#endif

    CL_ASSERT(Path);
    CL_ASSERT(!*Path || !iswspace(*Path));        // no leading whitespace

    if ( iswalpha(*Path) && *(Path+1) == L':' ) {
        Path += 2;
    }

    chCur = *Path;
    chPrev = 0;

    while (chCur) {
        charCount++;
        if ( charCount > MAX_PATH ) {
            return(FALSE);
        }
#ifdef    DBCS
        if (fPrevLead) {
            fPrevLead = FALSE;
            chPrev = 0;
        } else {
            fPrevLead = IsDBCSLeadByte(chCur);
#endif    // DBCS

            switch ( chCur ) {

            // Explicit invalid characters
            case L'*' :
            case L';' :
            case L',' :
            case L'=' :
            case L'?' :
            case L'<' :
            case L'>' :
            case L'|' :
            case L':' :             // no ":" except as second char
                return(FALSE);      // no ":" allowed other than second char */

#if 0   // The following should be okay
            case L'\\' :
                if ( chPrev == chDirSep ) {
                    return(FALSE);  // no double "\\" in middle - but legal
                }
                break;
#endif

            default:
#if 0   // accept anything else for now
                if ( !iswalnum( chCur ) ) {
                    return(FALSE);
                }
#endif
                break;
            }

            chPrev = chCur;

#ifdef    DBCS
        }
#endif

        chCur = *(++Path);
    }

#ifdef    DBCS
    if (fPrevLead)
        return(FALSE);    // ends w/ lead byte
#endif

    return(TRUE);

} // ClRtlIsPathValid


/****
@func       DWORD | ClRtlGetClusterDirectory | Get the directory in which
            the cluster service is installed

@parm       IN LPWSTR | lpBuffer | Supplies the buffer in which the
            directory path is to be copied.

@parm       IN DWORD | dwBufSize | Supplies the size of the buffer.

@rdesc      Returns a Win32 error code if the operation is
            unsuccessful. ERROR_SUCCESS on success.
****/
DWORD
ClRtlGetClusterDirectory(
    IN LPWSTR   lpBuffer,
    IN DWORD    dwBufSize
    )
{
    DWORD           dwLen;
    DWORD           dwStatus;
    LPWSTR          szRegKeyName = NULL;
    HKEY            hClusSvcKey = NULL;
    LPWSTR          lpImagePath = NULL;
    WCHAR           *pTemp = NULL;

    //
    //  Chittur Subbaraman (chitturs) - 10/29/98
    //
    if ( lpBuffer == NULL )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }
    //
    // Open key to SYSTEM\CurrentControlSet\Services\ClusSvc
    //
    dwLen = lstrlenW( CLUSREG_KEYNAME_CLUSSVC_PARAMETERS );
    szRegKeyName = (LPWSTR) LocalAlloc ( LMEM_FIXED,
                                    ( dwLen + 1 ) *
                                    sizeof ( WCHAR ) );
    if ( szRegKeyName == NULL )
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

    dwLen -= lstrlenW( CLUSREG_KEYNAME_PARAMETERS );

    lstrcpyW( szRegKeyName, CLUSREG_KEYNAME_CLUSSVC_PARAMETERS );
    szRegKeyName [dwLen-1] = L'\0';

    if ( ( dwStatus = RegOpenKeyW( HKEY_LOCAL_MACHINE,
                                 szRegKeyName,
                                 &hClusSvcKey ) ) != ERROR_SUCCESS )
    {
        goto FnExit;
    }

    lstrcpyW ( szRegKeyName, L"ImagePath" );
    //
    //  Try to query the clussvc key. If the ImagePath
    //  value is present, then get the length of the image
    //  path
    //
    dwLen = 0;
    if ( ( dwStatus = ClRtlRegQueryString( hClusSvcKey,
                                           szRegKeyName,
                                           REG_EXPAND_SZ,
                                           &lpImagePath,
                                           &dwLen,
                                           &dwLen ) ) != ERROR_SUCCESS )
    {
        goto FnExit;
    }

    //
    //  Now expand any environment strings present in the
    //  ImagePath
    //
    if ( ( dwLen = ExpandEnvironmentStringsW( lpImagePath,
                                              lpBuffer,
                                              dwBufSize ) ) == 0 )
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

    //
    //  If the caller-supplied buffer is not big enough to hold the
    //  path value, then return an error
    //
    if ( dwLen > dwBufSize )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

    //
    //  Replace the last '\\' character in the image path with
    //  a NULL character
    //
    pTemp = wcsrchr( lpBuffer, L'\\' );
    if ( pTemp != NULL )
    {
        *pTemp = L'\0';
    } else
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

FnExit:
    LocalFree( szRegKeyName );
    if( hClusSvcKey != NULL )
    {
        RegCloseKey( hClusSvcKey );
    }
    LocalFree( lpImagePath );

    return( dwStatus );
} // ClRtlGetClusterDirectory

BOOL
ClRtlGetDriveLayoutTable(
    IN  HANDLE hDisk,
    OUT PDRIVE_LAYOUT_INFORMATION * DriveLayout,
    OUT PDWORD InfoSize OPTIONAL
    )

/*++

Routine Description:

    Get the partition table for a drive. If the buffer is not large enough,
    then realloc until we get the right sized buffer. This routine is not in
    disk.cpp since that causes additional symbols to be defined.

Arguments:

    hDisk - handle to a file on the partition

    DriveLayout - address of pointer that points to

    InfoSize - address of dword that receives size of partition table

Return Value:

    TRUE if everything went ok

--*/

{
    DWORD dwSize = 0;
    PDRIVE_LAYOUT_INFORMATION driveLayout = NULL;
    DWORD status = ERROR_INSUFFICIENT_BUFFER;
    DWORD partitionCount = 4;
    DWORD layoutSize;

    while ( status == ERROR_INSUFFICIENT_BUFFER
         || status == ERROR_BAD_LENGTH
          )
    {

        layoutSize = sizeof(DRIVE_LAYOUT_INFORMATION) +
                     (sizeof(PARTITION_INFORMATION) * partitionCount);
        if ( layoutSize > 2<<16 ) {
            break;
        }

        driveLayout = (PDRIVE_LAYOUT_INFORMATION)LocalAlloc( LMEM_FIXED, layoutSize );
        if ( driveLayout == NULL ) {
            break;
        }

        if (DeviceIoControl(hDisk,
                            IOCTL_DISK_GET_DRIVE_LAYOUT,
                            NULL,
                            0,
                            driveLayout,
                            layoutSize,
                            &dwSize,
                            NULL))
        {
            status = ERROR_SUCCESS;
            break;
        } else {
            status = GetLastError();
            LocalFree( driveLayout );
            driveLayout = NULL;
            partitionCount *= 2;
        }
    }

    *DriveLayout = driveLayout;
    if ( ARGUMENT_PRESENT( InfoSize )) {
        *InfoSize = dwSize;
    }

    return status == ERROR_SUCCESS ? TRUE : FALSE;

} // ClRtlGetDriveLayoutTable


BOOL
ClRtlPathFileExists(
    LPWSTR pwszPath
    )
/*++

Routine Description:

    Determines if a file/directory exists.  This is fast.

Arguments:

    pwszPath - Path to validate.

Return Value:

    TRUE if it exists, otherwise FALSE.

    NOTE: This was borrowed from SHLWAPI.

--*/
{
    DWORD dwErrMode;
    BOOL fResult;

    dwErrMode = SetErrorMode( SEM_FAILCRITICALERRORS );

    fResult = ( (UINT) GetFileAttributes( pwszPath ) != (UINT) -1 );

    SetErrorMode( dwErrMode );

    return fResult;

}


DWORD
SetClusterFailureInformation(
    LPWSTR  NodeName OPTIONAL,
    DWORD   ResetPeriod,
    LONG    RetryCount,
    DWORD   RetryInterval
    )

/*++

Routine Description:

    Set the SC failure parameters for the cluster service.

Arguments:

    The args are loosely similar to the members of the SERVICE_FAILURE_ACTIONS
    structure. If RetryCount equals -1, then we set up a series of actions
    where the SC will exponentially back off restarting the service until it
    reaches 5 minutes, where it will continue to retry forever (well, until
    something good or bad happens). Otherwise, if RetryCount is positive, then
    we'll retry that many times (and zero is a valid number of retries) still
    using the same back off technique.

Return Value:

    ERROR_SUCCESS if everything worked ok

--*/

{
    DWORD status;
    BOOL success;
    HANDLE schSCManager;
    HANDLE serviceHandle;
    SERVICE_FAILURE_ACTIONS failureData;
    LPSC_ACTION failureActions;
    LONG i;
    BOOL tryForever = FALSE;

    CL_ASSERT( RetryCount >= -1 && RetryCount <= CLUSTER_FAILURE_MAX_STARTUP_RETRIES );

    ++RetryCount;   // add one more for the final action
    if ( RetryCount == 0 ) {
        DWORD tempInterval = RetryInterval;

        //
        // count the entries we need to go from our initial retry interval to
        // the final (longest) retry interval.
        //
        while ( tempInterval < CLUSTER_FAILURE_FINAL_RETRY_INTERVAL ) {
            tempInterval *= 2;
            ++RetryCount;
        }

        ++RetryCount;
        tryForever = TRUE;
    }
    CL_ASSERT( RetryCount > 0 );

    //
    // open the SC mgr and the service
    //

    schSCManager = OpenSCManager(NodeName,
                                 NULL,                   // database (NULL == default)
                                 SC_MANAGER_ALL_ACCESS); // access required

    if ( schSCManager ) {
        serviceHandle = OpenService(schSCManager,
                                    CLUSTER_SERVICE_NAME,
                                    SERVICE_ALL_ACCESS);

        if ( serviceHandle ) {

            failureActions = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                         RetryCount * sizeof( SC_ACTION ));
            if ( failureActions != NULL ) {

                //
                // build a list that exponentially backs off but does
                // exactly the number of retries that were specified.
                //

                for ( i = 0; i < RetryCount-1; ++i ) {
                    failureActions[i].Type = SC_ACTION_RESTART;
                    failureActions[i].Delay = RetryInterval;

                    RetryInterval = RetryInterval * 2;
                    if ( RetryInterval > CLUSTER_FAILURE_FINAL_RETRY_INTERVAL ) {
                        RetryInterval = CLUSTER_FAILURE_FINAL_RETRY_INTERVAL;
                    }
                }

                if ( tryForever ) {
                    failureActions[i].Type = SC_ACTION_RESTART;
                    failureActions[i].Delay = RetryInterval;
                } else {
                    failureActions[i].Type = SC_ACTION_NONE;
                    failureActions[i].Delay = 0;
                }

                failureData.dwResetPeriod = ResetPeriod;
                failureData.lpRebootMsg = NULL;
                failureData.lpCommand = NULL;
                failureData.cActions = RetryCount;
                failureData.lpsaActions = failureActions;

                success = ChangeServiceConfig2(serviceHandle,
                                               SERVICE_CONFIG_FAILURE_ACTIONS,
                                               &failureData);
                LocalFree( failureActions );

                if ( success ) {
                    status = ERROR_SUCCESS;
                } else {
                    status = GetLastError();
                    ClRtlDbgPrint(LOG_CRITICAL,"[ClRtl] Couldn't set SC failure info %1!u!\n", status);
                }
            } else {
                status = ERROR_OUTOFMEMORY;
                ClRtlDbgPrint(LOG_CRITICAL,"[ClRtl] Couldn't allocate memory to set SM Failure actions\n");
            }

            CloseServiceHandle( serviceHandle );
        } else {
            status = GetLastError();
            ClRtlDbgPrint(LOG_CRITICAL,"[ClRtl] Couldn't get SC handle to Cluster Service %1!u!\n", status);
        }

        CloseServiceHandle( schSCManager );
    } else {
        status = GetLastError();
        ClRtlDbgPrint(LOG_CRITICAL,"[ClRtl] Couldn't get a handle to the SC Manager %1!u!\n", status);
    }

    return status;
} // SetClusterFailureInformation

DWORD
ClRtlSetSCMFailureActions(
    LPWSTR NodeName OPTIONAL
    )

/*++

Routine Description:

    Set the service controller failure parameters for the cluster service.

Arguments:

    NodeName - pointer to string that identifies on which node to modify the
    settings. NULL indicates the local node.

Return Value:

    ERROR_SUCCESS if everything worked ok

--*/

{
    DWORD Status;

    //
    // during startup, we start with a short retry period and then
    // exponentially back off. Set the reset period to 30 minutes.
    //
    Status = SetClusterFailureInformation(NodeName,
                                          30 * 60,
                                          CLUSTER_FAILURE_RETRY_COUNT,
                                          CLUSTER_FAILURE_INITIAL_RETRY_INTERVAL);

    if ( Status != ERROR_SUCCESS ) {
        ClRtlDbgPrint(LOG_CRITICAL,
                      "[ClRtl] Couldn't set SC startup failure info %1!u!\n",
                      Status);
    }

    return Status;
} // ClRtlSetSCMFailureActions


DWORD
ClRtlGetRunningAccountInfo(
    LPWSTR *    AccountBuffer
    )

/*++

Routine Description:

    Get the calling thread's token to obtain account info. It is returned in
    an allocated buffer in the form of "domain\user". Caller is responsible
    for freeing the buffer.

Arguments:

    AccountBuffer - address of pointer to receive allocated buffer

Return Value:

    ERROR_SUCCESS if everything worked ok

--*/

{
    HANDLE          currentToken;
    PTOKEN_USER     tokenUserData;
    DWORD           sizeRequired;
    BOOL            success;
    DWORD           status = ERROR_SUCCESS;
    DWORD           accountNameSize = 128;
    LPWSTR          accountName;
    DWORD           domainNameSize = DNS_MAX_NAME_BUFFER_LENGTH;
    LPWSTR          domainName;
    SID_NAME_USE    sidType;
    DWORD           nameSize = 0;
    HMODULE         secur32Handle;
    FARPROC         getUserNameEx;
    INT_PTR         returnValue;

    //
    // initialize in case the caller doesn't check the return status (tsk, tsk!)
    //
    *AccountBuffer = NULL;

    //
    // rather than link in yet another DLL, we'll load secur32 dynamically and
    // get a pointer to GetUserNameEx.
    //
    secur32Handle = LoadLibraryW( L"secur32.dll" );
    if ( secur32Handle ) {
        getUserNameEx = GetProcAddress( secur32Handle, "GetUserNameExW" );
        if ( getUserNameEx ) {
            //
            // get the length the first time, allocate a buffer and then get the data
            //
            returnValue = (*getUserNameEx)( NameSamCompatible, NULL, &nameSize );
            success = (BOOL)returnValue;

            *AccountBuffer = LocalAlloc( LMEM_FIXED, nameSize * sizeof( WCHAR ));
            if ( *AccountBuffer != NULL ) {

                returnValue = (*getUserNameEx)( NameSamCompatible, *AccountBuffer, &nameSize );
                success = (BOOL)returnValue;
                if ( !success ) {
                    status = GetLastError();
                }
            }
            else {
                status = GetLastError();
            }
        } else {
            status = GetLastError();
        }

        FreeLibrary( secur32Handle );
    }
    else {
        status = GetLastError();
    }

    return status;

#if 0
    //
    // check if there is a thread token
    //
    if (!OpenThreadToken(GetCurrentThread(),
                         MAXIMUM_ALLOWED,
                         TRUE,
                         &currentToken))
    {
        // get the process token
        if (!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &currentToken )) {
            return GetLastError();
        }
    }

    //
    // get the size needed
    //
    success = GetTokenInformation(currentToken,
                                  TokenUser,
                                  NULL,
                                  0,
                                  &sizeRequired);

    tokenUserData = LocalAlloc( LMEM_FIXED, sizeRequired );
    if ( tokenUserData == NULL ) {
        CloseHandle( currentToken );
        return GetLastError();
    }

    success = GetTokenInformation(currentToken,
                                  TokenUser,
                                  tokenUserData,
                                  sizeRequired,
                                  &sizeRequired);

    if ( !success ) {
        CloseHandle( currentToken );
        return GetLastError();
    }

    do {
        //
        // make initial allocs for account and domain name; 1 more byte to
        // hold slash separator. domain buffer holds the complete
        // 'domain\user' entry so it gets more space
        //
        domainName = LocalAlloc( LMEM_FIXED,
                                     (accountNameSize + domainNameSize + 1) * sizeof(WCHAR) );
        accountName = (LPWSTR) LocalAlloc( LMEM_FIXED, accountNameSize * sizeof(WCHAR) );

        if ( accountName == NULL || domainName == NULL ) {
            if ( accountName != NULL ) {
                LocalFree( accountName );
            }

            if ( domainName != NULL ) {
                LocalFree( domainName );
            }

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        //
        // Attempt to Retrieve the account and domain name. If
        // LookupAccountName fails because of insufficient buffer size(s)
        // accountNameSize and domainNameSize will be correctly set for the
        // next attempt.
        //
        if ( LookupAccountSid(NULL,
                              tokenUserData->User.Sid,
                              accountName,
                              &accountNameSize,
                              domainName,
                              &domainNameSize,
                              &sidType ))
        {
            wcscat( domainName, L"\\" );
            wcscat( domainName, accountName );
            *AccountBuffer = domainName;
        }
        else {
            // free the account name buffer and find out why we failed
            LocalFree( domainName );

            status = GetLastError();
        }

        //
        // domain buffer holds complete string so we can lose the account name
        // at this point
        //
        LocalFree( accountName );
        accountName = NULL;

    } while ( status == ERROR_INSUFFICIENT_BUFFER );

    return status;
#endif

} // ClRtlGetRunningAccountInfo

#if 0
//
// no longer needed. Keep it around just in case
//
DWORD
ClRtlGetServiceAccountInfo(
    LPWSTR *    AccountBuffer
    )

/*++

Routine Description:

    Query SCM for the cluster service account info. It is returned in an
    allocated buffer in the form of "domain\user". Caller is responsible for
    freeing the buffer.

Arguments:

    AccountBuffer - address of pointer to receive allocated buffer

Return Value:

    ERROR_SUCCESS if everything worked ok

--*/

{
    DWORD status = ERROR_SUCCESS;
    HANDLE schSCManager;
    HANDLE serviceHandle = NULL;
    LPQUERY_SERVICE_CONFIG scConfigData = NULL;
    ULONG bytesNeeded;
    BOOL success;

    //
    // open a handle to the service controller manager to query the account
    // under which the cluster service was started
    //

    schSCManager = OpenSCManager(NULL,                   // machine (NULL == local)
                                 NULL,                   // database (NULL == default)
                                 SC_MANAGER_ALL_ACCESS); // access required

    if ( schSCManager == NULL ) {
        status = GetLastError();
        goto error_exit;
    }

    serviceHandle = OpenService(schSCManager,
                                CLUSTER_SERVICE_NAME,
                                SERVICE_ALL_ACCESS);

    if ( serviceHandle == NULL ) {
        status = GetLastError();
        goto error_exit;
    }

    success = QueryServiceConfig(serviceHandle, NULL, 0, &bytesNeeded);
    if ( !success ) {
        status = GetLastError();
        if ( status != ERROR_INSUFFICIENT_BUFFER ) {
            goto error_exit;
        } else {
            status = ERROR_SUCCESS;
        }
    }

    scConfigData = LocalAlloc( LMEM_FIXED, bytesNeeded );
    if ( scConfigData == NULL ) {
        status = GetLastError();
        goto error_exit;
    }

    success = QueryServiceConfig(serviceHandle,
                                 scConfigData,
                                 bytesNeeded,
                                 &bytesNeeded);
    if ( !success ) {
        status = GetLastError();
        goto error_exit;
    }

    *AccountBuffer = LocalAlloc(LMEM_FIXED,
                                (wcslen( scConfigData->lpServiceStartName ) + 1 ) * sizeof(WCHAR));

    if ( *AccountBuffer == NULL ) {
        status = GetLastError();
        goto error_exit;
    }

    wcscpy( *AccountBuffer, scConfigData->lpServiceStartName );

error_exit:
    if ( serviceHandle != NULL ) {
        CloseServiceHandle( serviceHandle );
    }

    if ( schSCManager != NULL ) {
        CloseServiceHandle( schSCManager );
    }

    if ( scConfigData != NULL ) {
        LocalFree( scConfigData );
    }

    return status;
} // ClRtlGetServiceAccountInfo
#endif

BOOL
ClRtlCopyFileAndFlushBuffers(
    IN LPCWSTR lpszSourceFile,
    IN LPCWSTR lpszDestinationFile
    )
/*++

Routine Description:

    Copy a source file to a destination file and flush the destination file buffers.

Arguments:

    lpszSourceFile - The source file name.

    lpszDestinationFile - The destination file name.

Return Value:

    TRUE if successful, FALSE otherwise.

--*/
{
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    DWORD       dwStatus = ERROR_SUCCESS;

    //
    //  Copy the file, block until completed. Note that CopyFile family of APIs do not flush the metadata or
    //  the data. So, we need to do that explicitly here.
    //
    if ( !CopyFileEx( lpszSourceFile,       // Source file
                      lpszDestinationFile,  // Destination file
                      NULL,                 // Progress routine
                      NULL,                 // Callback parameter
                      NULL,                 // Cancel status
                      0 ) )                 // Copy flags
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[CLRTL] ClRtlCopyFileAndFlushBuffers:: Failed to copy %1!ws! to %2!ws!, Status=%3!u!\n",
                      lpszSourceFile,
                      lpszDestinationFile,
                      dwStatus);
        goto FnExit;
    }

    //
    //  Open the newly created destination file
    //
    hFile =  CreateFile( lpszDestinationFile,                   // File name
                         GENERIC_READ | GENERIC_WRITE,          // Access mode
                         0,                                     // Share mode
                         NULL,                                  // Security attribs
                         OPEN_EXISTING,                         // Creation disposition
                         FILE_ATTRIBUTE_NORMAL,                 // Flags and attributes
                         NULL );                                // Template file

    if ( hFile == INVALID_HANDLE_VALUE )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[CLRTL] ClRtlCopyFileAndFlushBuffers: Failed to open file %1!ws!, Status=%2!u!...\n",
                      lpszDestinationFile,
                      dwStatus);
        goto FnExit;
    }

    //
    //  Flush the file buffers to avoid corrupting the file on a power failure.
    //
    if ( !FlushFileBuffers( hFile ) )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[CLRTL] ClRtlCopyFileAndFlushBuffers: Failed to flush buffers for %1!ws!, Status=%2!u!...\n",
                      lpszDestinationFile,
                      dwStatus);
        goto FnExit;
    }

FnExit:
    if ( hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle ( hFile );
    }

    if ( dwStatus != ERROR_SUCCESS )
    {
        SetLastError( dwStatus );
        return ( FALSE );
    } else
    {
        return ( TRUE );
    }
}// ClRtlCopyFileAndFlushBuffers


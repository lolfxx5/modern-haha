/*++

Copyright (C) 1996-1999 Microsoft Corporation

Module Name:

    logthred.c

Abstract:

    Performance Logs and Alerts log/scan thread functions.

--*/

#ifndef UNICODE
#define UNICODE     1
#endif
#ifndef _UNICODE
#define _UNICODE    1
#endif

#ifndef _IMPLEMENT_WMI 
#define _IMPLEMENT_WMI 1
#endif

//
//  Windows Include files
//
#pragma warning ( disable : 4201)

#include <assert.h>

// For Trace *** - these are only necessary because of union query data struct.
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <wtypes.h>
#include <float.h>
#include <limits.h>

#if _IMPLEMENT_WMI
#include <wmistr.h>
#include <evntrace.h>
#endif

#include <lmcons.h>
#include <lmmsg.h>  // for net message function

#include <pdh.h>
#include <pdhp.h>

#include <pdhmsg.h>
#include <strsafe.h>
#include "smlogsvc.h"
#include "smlogmsg.h"

#define SECONDS_IN_DAY      ((LONGLONG)(86400))

#define LOG_EVENT_ON_ERROR  ((BOOL)(1))

// A collection thread can get backed up behind some long-running PDH
// operations causing it to timeout;
// In the case of opening a query and adding a counter, retry
// a number of times before giving up
#define NUM_PDH_RETRIES 20



DWORD
ProcessLogFileFolder (     
    IN PLOG_QUERY_DATA pQuery )
{
    DWORD       dwStatus = ERROR_SUCCESS;
    LPWSTR      szLocalPath = NULL;
    LPWSTR      szEnd = NULL;
    DWORD       dwBufferLength = 0;
    LPSECURITY_ATTRIBUTES   lpSA = NULL;
    WCHAR       cBackslash = L'\\';
    LONG        lErrorMode;
    BOOL        fDirectoryCreated;

    //
    // Environment strings are already expanded.
    //
    dwBufferLength = GetFullPathName ( pQuery->szLogFileFolder, 0, NULL, NULL);

    szLocalPath = (LPWSTR) G_ALLOC ( (dwBufferLength + 1) * sizeof(WCHAR) );

    if ( szLocalPath ) {

        if ( GetFullPathName (
                pQuery->szLogFileFolder,
                dwBufferLength,
                szLocalPath,
                NULL ) > 0 ) 
        {
            //
            // Check for prefix
            //
            // Go one past the first backslash after the drive or remote machine name
            // N.B. We are assuming the full path name looks like either "\\machine\share\..."
            //      or "C:\xxx". How about "\\?\xxx" style names
            //
            if ( cBackslash == szLocalPath[0] && cBackslash == szLocalPath[1] ) {
                szEnd = &szLocalPath[2];
                while ((*szEnd != cBackslash) && (*szEnd != L'\0') ) szEnd++;

                if ( cBackslash == *szEnd ) {
                    szEnd++;
                }         
            } else {
                szEnd = &szLocalPath[3];
            }

            if (*szEnd != L'\0') {
                int iPathLen;

                //
                // Remove the trailing back slash character if it is there.
                //
                iPathLen = lstrlen(szEnd) - 1;
                while ( iPathLen >= 0 && cBackslash == szEnd[ iPathLen ]) {
                    szEnd[ iPathLen ] = L'\0';
                    iPathLen -= 1;
                }
            
                lErrorMode = SetErrorMode ( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );
                //
                // There are sub dirs to create. 
                //
                while (*szEnd != L'\0') {
                    //
                    // Go to next backslash.
                    //
                    while ((*szEnd != cBackslash) && (*szEnd != L'\0')) szEnd++;
                    if (*szEnd == cBackslash) {
                        //
                        // Terminate path here and create directory.
                        //
                        *szEnd = L'\0';
                        if (!CreateDirectory (szLocalPath, NULL)) {
                            //
                            // See what the error was and "adjust" it if necessary
                            //
                            dwStatus = GetLastError();
                            if ( ERROR_ALREADY_EXISTS == dwStatus ) {
                                //
                                // This is OK
                                //
                                dwStatus = ERROR_SUCCESS;
                            }
                            else {
                                //
                                // Return error code here and don't continue?
                                //
                            }
                        }
                        //
                        // Replace backslash and go to next dir
                        //
                        *szEnd++ = cBackslash;
                    }
                }

                //
                // If the log folder is the default one, put ACLs on it
                // N.B. The gszDefaultLogFileFolder does not contain a back slash char
                //      at the end
                //
                if (lstrcmpi(szLocalPath, gszDefaultLogFileFolder) == 0) {
                    fDirectoryCreated = PerfCreateDirectory (szLocalPath);
                } else {
                    fDirectoryCreated = CreateDirectory (szLocalPath, NULL);
                }

                if ( !fDirectoryCreated ) {
                    //
                    // See what the error was and "adjust" it if necessary
                    //
                    dwStatus = GetLastError();
                    if ( ERROR_ALREADY_EXISTS == dwStatus ) {
                        //
                        // This is OK
                        //
                        dwStatus = ERROR_SUCCESS;
                    }
                }

                SetErrorMode ( lErrorMode );
            } else {
                //
                // Root directory is OK.
                //
                dwStatus = ERROR_SUCCESS;
            }
        } else {
            dwStatus = GetLastError();
        }
        G_FREE ( szLocalPath );
    } else {
        dwStatus = ERROR_OUTOFMEMORY;
    }
    
    // Report event on error
    if ( ERROR_SUCCESS != dwStatus ) {
        DWORD   dwMessageId; 
        LPWSTR szStringArray[3];
        
        szStringArray[0] = pQuery->szLogFileFolder;
        szStringArray[1] = pQuery->szQueryName;
        szStringArray[2] = FormatEventLogMessage(dwStatus);

        if ( pQuery->bReconfiguration ) {
            dwMessageId = SMLOG_INVALID_LOG_FOLDER_STOP;
        } else {
            dwMessageId = SMLOG_INVALID_LOG_FOLDER_START;
        }

        ReportEvent (hEventLog,
            EVENTLOG_WARNING_TYPE,
            0,
            dwMessageId,
            NULL,
            3,
            sizeof(DWORD),
            szStringArray,      
            (LPVOID)&dwStatus);

        LocalFree( szStringArray[2] );
    }

    return dwStatus;
}

DWORD
ValidateCommandFilePath ( 
    IN    PLOG_QUERY_DATA pArg )
{
    DWORD dwStatus = ERROR_SUCCESS;

    if ( 0 != lstrlen ( pArg->szCmdFileName ) ) {
    
        HANDLE hOpenFile;
        LONG   lErrorMode;

        lErrorMode = SetErrorMode ( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );

        hOpenFile =  CreateFile (
                        pArg->szCmdFileName,
                        GENERIC_READ,
                        0,              // Not shared
                        NULL,           // Security attributes
                        OPEN_EXISTING,  
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

        if ( ( NULL == hOpenFile ) 
                || INVALID_HANDLE_VALUE == hOpenFile ) {

            LPWSTR  szStringArray[3];

            dwStatus = GetLastError();

            szStringArray[0] = pArg->szCmdFileName;
            szStringArray[1] = pArg->szQueryName;
            szStringArray[2] = FormatEventLogMessage(dwStatus);

            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                SMLOG_CMD_FILE_INVALID,
                NULL,
                3,
                sizeof(DWORD),
                szStringArray,
                (LPVOID)&dwStatus );

            LocalFree( szStringArray[2] );

            pArg->dwCmdFileFailure = dwStatus;

        } else {
            CloseHandle(hOpenFile);
        }

        SetErrorMode ( lErrorMode );
    }
    return dwStatus;
}


DWORD 
AddCounterToCounterLog (                        
    IN      PLOG_QUERY_DATA pArg, 
    IN      LPWSTR  pszThisPath,
    IN      HANDLE  hQuery,
    IN      BOOL    bLogErrorEvent,
    IN OUT  DWORD*  pdwCounterCount )
{
    LPWSTR              szStringArray[3];
    LONG                lWaitStatus;
    INT                 iRetries;
    BOOL                bFirstTimeout;
    DWORD               dwStatus = ERROR_SUCCESS;
    HCOUNTER            hThisCounter = NULL;
    PDH_STATUS          pdhStatus;                
    PLOG_COUNTER_INFO   pCtrInfo = NULL;
    HANDLE              arrEventHandle[2];
    WCHAR               szRetryCount[SLQ_MAX_VALUE_LEN];

    arrEventHandle[0] = pArg->hExitEvent;           // WAIT_OBJECT_0
    arrEventHandle[1] = pArg->hReconfigEvent;
                
    iRetries = NUM_PDH_RETRIES;
    bFirstTimeout = TRUE;
    do {                
        dwStatus = pdhStatus = PdhAdd009Counter (
                                    hQuery,
                                    pszThisPath, 
                                    (*pdwCounterCount), 
                                    &hThisCounter);

        if ( bFirstTimeout && WAIT_TIMEOUT == pdhStatus ) {
            //
            // Write event log warning message
            //
            StringCchPrintf (
                szRetryCount,
                SLQ_MAX_VALUE_LEN,
                L"%d", 
                iRetries );

            szStringArray[0] = pszThisPath;
            szStringArray[1] = pArg->szQueryName;
            szStringArray[2] = szRetryCount;
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                SMLOG_ADD_COUNTER_TIMEOUT,
                NULL,
                3,
                sizeof(DWORD),
                szStringArray,
                (LPVOID)&pdhStatus);

            bFirstTimeout = FALSE;
        }

    } while ( WAIT_TIMEOUT == (lWaitStatus = WaitForMultipleObjects (2, arrEventHandle, FALSE, 0))
              && WAIT_TIMEOUT == pdhStatus
              && iRetries-- > 0 );
    if ( WAIT_TIMEOUT != lWaitStatus ) {
        // the loop was terminated by the Exit/Reconfigure event
        return ERROR_CANCELLED;
    }

    if (dwStatus != ERROR_SUCCESS) {

        iRetries = NUM_PDH_RETRIES;
        do {                
            dwStatus = pdhStatus = PdhAddCounter (
                                        hQuery,
                                        pszThisPath, 
                                        (*pdwCounterCount), 
                                        &hThisCounter);
        } while ( WAIT_TIMEOUT == (lWaitStatus = WaitForMultipleObjects (2, arrEventHandle, FALSE, 0))
                  && WAIT_TIMEOUT == pdhStatus
                  && iRetries-- > 0 );
        if ( WAIT_TIMEOUT != lWaitStatus ) {
            // the loop was terminated by the Exit/Reconfigure event
            return ERROR_CANCELLED;
        }
    }

    if ( !IsErrorSeverity(pdhStatus) ) {
        if ( IsWarningSeverity(pdhStatus) ) {
            //
            // Write event log warning message
            //
            szStringArray[0] = pszThisPath;
            szStringArray[1] = pArg->szQueryName;
            szStringArray[2] = FormatEventLogMessage(pdhStatus);
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                SMLOG_ADD_COUNTER_WARNING,
                NULL,
                3,
                sizeof(DWORD),
                szStringArray,
                (LPVOID)&pdhStatus);
            LocalFree( szStringArray[2] );
        }

        pCtrInfo = G_ALLOC (sizeof (LOG_COUNTER_INFO));
        if (pCtrInfo != NULL) {
            //
            // Add this handle to the list
            //
            // Insert at front of list since the order isn't
            // important and this is simpler than walking the
            // list each time.
            //
            pCtrInfo->hCounter = hThisCounter;
            pCtrInfo->next = pArg->pFirstCounter;
            pArg->pFirstCounter = pCtrInfo;
            pCtrInfo = NULL;
            
            (*pdwCounterCount)++; 
       } else {
            dwStatus = ERROR_OUTOFMEMORY;
        }
    } else {
        //
        // For LogByObject, the call is retried with expanded counter if
        // the first try fails, so don't log error event the first time.
        //
        if ( bLogErrorEvent ) {
            // unable to add the current counter so write event log message
            szStringArray[0] = pszThisPath;
            szStringArray[1] = pArg->szQueryName;
            szStringArray[2] = FormatEventLogMessage(pdhStatus);

            if ( PDH_ACCESS_DENIED == pdhStatus ) {
                ReportEvent (
                    hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_UNABLE_ACCESS_COUNTER,
                    NULL,
                    2,
                    0,
                    szStringArray,
                    NULL);
            } else {

                ReportEvent (
                    hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_UNABLE_ADD_COUNTER,
                    NULL,
                    3,
                    sizeof(DWORD),
                    szStringArray,
                    (LPVOID)&pdhStatus);
            }
            LocalFree( szStringArray[2] );
        }
    }
    return dwStatus;
}
    

BOOL
IsPdhDataCollectSuccess ( PDH_STATUS pdhStatus ) 
{
    BOOL bSuccess = FALSE;

    if ( ERROR_SUCCESS == pdhStatus 
            || PDH_INVALID_DATA == pdhStatus ) {
        bSuccess = TRUE;
    } else if ( 0 < iPdhDataCollectSuccessCount ) {
        INT iIndex;

        for ( iIndex = 0; iIndex < iPdhDataCollectSuccessCount; iIndex++ ) {
            if ( pdhStatus == (PDH_STATUS)arrPdhDataCollectSuccess[iIndex] ) {
                bSuccess = TRUE;
                break;
            }
        }
    }

    return bSuccess;
}

void
ComputeSessionTics(
    IN      PLOG_QUERY_DATA pArg,
    IN OUT  LONGLONG*   pllWaitTics
)
{
    LONGLONG    llLocalTime;

    //
    // Compute total session time based on Stop modes
    // and values.  

    // -1 (NULL_INTERVAL_TICS) signals no session time limit.  This is true for
    // Stop mode SLQ_AUTO_MODE_NONE and SLQ_AUTO_MODE_SIZE.
    //
    // 0 signals that the Stop time is past, so exit immediately.
    //
    // Assume that session is starting, so Start mode isn't relevant.
    //

    //
    // Pointer check is a sanity check.  The calling code is trusted.
    //
    if ( NULL != pArg && NULL != pllWaitTics ) {

        *pllWaitTics = NULL_INTERVAL_TICS;

        if ( SLQ_AUTO_MODE_AFTER == pArg->stiCurrentStop.dwAutoMode 
                || SLQ_AUTO_MODE_AT == pArg->stiCurrentStop.dwAutoMode ) {       

            llLocalTime = (LONGLONG)0;
            GetLocalFileTime (&llLocalTime);

            if ( SLQ_AUTO_MODE_AT == pArg->stiCurrentStop.dwAutoMode ) {
        
                if ( pArg->stiCurrentStop.llDateTime > llLocalTime ) {

                    *pllWaitTics = pArg->stiCurrentStop.llDateTime - llLocalTime;

                } else {
                    //
                    // Session length = 0.  Exit immediately.
                    //
                    *pllWaitTics = ((LONGLONG)(0)); 
                }

            } else if ( SLQ_AUTO_MODE_AFTER == pArg->stiCurrentStop.dwAutoMode ) {
            
                TimeInfoToTics( &pArg->stiCurrentStop, pllWaitTics );
            }
        }
    }

    return;
}

void
ComputeNewFileTics(
    IN      PLOG_QUERY_DATA pArg,
    IN OUT  LONGLONG*   pllWaitTics
)
{

    LONGLONG    llLocalTime;  
    //
    // Compute time until next file creation based on Create New File modes
    // and values.  

    // -1 (NULL_INTERVAL_TICS) signals no time limit.  This is true for
    // mode SLQ_AUTO_MODE_NONE and SLQ_AUTO_MODE_SIZE.
    //
    // 0 signals that the time is past, so exit immediately.
    //
    // Assume that session is starting, so Start mode isn't relevant.
    //

    //
    // Pointer check is a sanity check.  The calling code is trusted.
    //
    if ( NULL != pArg && NULL != pllWaitTics ) {

        *pllWaitTics = NULL_INTERVAL_TICS;

        if ( SLQ_AUTO_MODE_AFTER == pArg->stiCreateNewFile.dwAutoMode ) {       

            GetLocalFileTime (&llLocalTime);

            if ( SLQ_AUTO_MODE_AFTER == pArg->stiCreateNewFile.dwAutoMode ) {
                TimeInfoToTics( &pArg->stiCreateNewFile, pllWaitTics );
                assert ( (LONGLONG)(0) != *pllWaitTics );
            } else if ( SLQ_AUTO_MODE_AT == pArg->stiCreateNewFile.dwAutoMode ) {
                assert ( FALSE );
                *pllWaitTics = (LONGLONG)(0);
            }
        }
    }
    return;
}


void 
ComputeSampleCount(
    IN  PLOG_QUERY_DATA pArg,
    IN  BOOL    bSessionCount,
    OUT LONGLONG*   pllSampleCount
)
{
    LONGLONG    llLocalSampleCount = NULL_INTERVAL_TICS;
    //
    // Compute sample count based on Stop or CreateNewFile modes
    // and values.  Account for the first sample in the log.
    //
    // 0 signals no sample limit in the file.  This is true for
    // Stop modes SLQ_AUTO_MODE_NONE and SLQ_AUTO_MODE_SIZE.
    //
    // -1 signals that the Stop time is past.
    //
    // Sampling is starting now, so Start mode isn't relevant.
    //
    
    //
    // Pointer check is a sanity check.  The calling code is trusted.
    //
    assert ( NULL != pllSampleCount );
    if ( NULL != pllSampleCount ) {

        *pllSampleCount = (LONGLONG)(-1);
    
        if ( bSessionCount ) {
            ComputeSessionTics ( pArg, &llLocalSampleCount );
        } else {
            ComputeNewFileTics ( pArg, &llLocalSampleCount );
        }

        if ( NULL_INTERVAL_TICS == llLocalSampleCount ) {
            //
            // No session/sample limit
            //
            *pllSampleCount = (LONGLONG)(0);
        } else if ( (LONGLONG)(0) == llLocalSampleCount ){
            //
            // Stop time is past
            //
            *pllSampleCount = INFINITE_TICS;
        } else {
            *pllSampleCount = llLocalSampleCount 
                                / (pArg->dwMillisecondSampleInterval * FILETIME_TICS_PER_MILLISECOND);
            //
            // Add in the "zero-th" sample.
            //
            *pllSampleCount += 1;  
        }
    }
    
    return;
}


BOOL
ProcessRepeatOption ( 
    IN OUT PLOG_QUERY_DATA pArg,
    OUT LARGE_INTEGER* pliStartDelayTics )
{
    BOOL            bRepeat = TRUE;

    //
    // Pointer check is a sanity check.  The calling code is trusted.
    //
    assert ( NULL != pliStartDelayTics );
    if ( NULL != pliStartDelayTics ) {
        //
        // If restart not enabled, then exit.
        //
        if ( SLQ_AUTO_MODE_NONE == pArg->stiRepeat.dwAutoMode ) {
            pliStartDelayTics->QuadPart = NULL_INTERVAL_TICS;
            bRepeat = FALSE;
        } else {
            //
            // For SLQ_AUTO_MODE_AFTER, the only value currently supported is 0.
            //
            pliStartDelayTics->QuadPart = (LONGLONG)0;
            //
            // For SLQ_AUTO_MODE_CALENDAR, add n*24 hours to the original start time.
            //    If Stop mode is SLQ_AUTO_MODE_AT, add n*24 hours to stop time.
            //
            if ( SLQ_AUTO_MODE_CALENDAR == pArg->stiRepeat.dwAutoMode ) {
                //
                // Delay of NULL_INTERVAL signals exit immediately.
                //
                pliStartDelayTics->QuadPart = ComputeStartWaitTics ( pArg, TRUE );

                if ( NULL_INTERVAL_TICS == pliStartDelayTics->QuadPart ) {
                    //
                    // This should not occur.
                    //
                    assert ( FALSE );
                    bRepeat = FALSE;
                } else {
                    pArg->dwCurrentState = SLQ_QUERY_START_PENDING;
                    WriteRegistryDwordValue (
                        pArg->hKeyQuery, 
                        (LPCWSTR)L"Current State",
                        &pArg->dwCurrentState,
                        REG_DWORD );
                } 
            } // else for SLQ_AUTO_MODE_AFTER, repeat immediately
        }
    }

    return bRepeat;
}

void
SetPdhOpenOptions ( 
    IN  PLOG_QUERY_DATA   pArg,
    OUT DWORD*  pdwAccess,
    OUT DWORD*  pdwLogFileType )
{

    //
    // Get file type
    //
    switch ( pArg->dwLogFileType ) {
        case SLF_TSV_FILE:
            *pdwLogFileType = PDH_LOG_TYPE_TSV;
            break;

        case SLF_BIN_FILE:
        case SLF_BIN_CIRC_FILE:
            *pdwLogFileType = PDH_LOG_TYPE_BINARY;
            break;

        case SLF_SQL_LOG:
            *pdwLogFileType = PDH_LOG_TYPE_SQL;
            break;

        case SLF_CSV_FILE:
        default:
            *pdwLogFileType = PDH_LOG_TYPE_CSV;
            break;
    }

    *pdwAccess = PDH_LOG_WRITE_ACCESS |
                    PDH_LOG_CREATE_ALWAYS;

    if (SLF_BIN_CIRC_FILE == pArg->dwLogFileType)
        *pdwAccess |= PDH_LOG_OPT_CIRCULAR;

    if ( ( PDH_LOG_TYPE_BINARY != *pdwLogFileType ) 
         && (NULL != pArg->szLogFileComment ) )
        *pdwAccess |= PDH_LOG_OPT_USER_STRING;

    // NOTE:  For all types except sequential binary,
    // the append mode is determined by the file type.
    // All Sql logs are APPEND
    // All text logs are OVERWRITE
    if (   (pArg->dwAppendMode)
        && (*pdwLogFileType == PDH_LOG_TYPE_BINARY) ) {
        *pdwAccess |= PDH_LOG_OPT_APPEND;
    }

}


DWORD
StartLogQuery (
    IN  PLOG_QUERY_DATA pArg
)
{
    DWORD           dwStatus = ERROR_SUCCESS;
    HRESULT         hr = S_OK;
    HKEY            hKeyLogQuery = NULL;
    SLQ_TIME_INFO   slqTime;
    SC_HANDLE       hSC = NULL;
    SC_HANDLE       hService = NULL;
    SERVICE_STATUS  ssData;
    WCHAR           szQueryKeyNameBuf[MAX_PATH + 1];
    WCHAR           szLogPath[2*MAX_PATH];  
    DWORD           dwCurrentState;
    DWORD           dwValue;
    DWORD           dwDefault;
    LONGLONG        llTime;
    LONGLONG        llModifiedTime;

    //
    // Open registry key to the desired service
    //
    dwStatus = GetQueryKeyName ( 
                pArg->szPerfLogName,
                szQueryKeyNameBuf,
                MAX_PATH + 1 );

    if ( ERROR_SUCCESS == dwStatus && 0 < lstrlen (szQueryKeyNameBuf) ) {

        hr = StringCchCopy ( szLogPath, 2*MAX_PATH, L"SYSTEM\\CurrentControlSet\\Services\\SysmonLog\\Log Queries\\" );
        if ( SUCCEEDED ( hr ) ) {
            hr = StringCchCat ( szLogPath, 2*MAX_PATH, szQueryKeyNameBuf );
        }

        if ( SUCCEEDED ( hr ) ) {

            dwStatus = RegOpenKeyEx (
                (HKEY)HKEY_LOCAL_MACHINE,
                szLogPath,
                0L,
                KEY_READ | KEY_WRITE,
                (PHKEY)&hKeyLogQuery);

            if (dwStatus == ERROR_SUCCESS) {
                //
                // If current state is running, then skip the rest.
                //
                dwDefault = SLQ_QUERY_STOPPED;
                dwStatus = ReadRegistryDwordValue (
                    hKeyLogQuery,
                    pArg->szPerfLogName,
                    (LPCWSTR)L"Current State",
                    &dwDefault,
                    &dwCurrentState);

                if (dwCurrentState == SLQ_QUERY_STOPPED) {
                    //
                    // Update the start time to MIN_TIME_VALUE.
                    //
                    GetLocalFileTime ( &llTime );

                    memset (&slqTime, 0, sizeof(slqTime));
                    slqTime.wTimeType = SLQ_TT_TTYPE_START;
                    slqTime.wDataType = SLQ_TT_DTYPE_DATETIME;
                    slqTime.dwAutoMode = SLQ_AUTO_MODE_NONE;
                    slqTime.llDateTime = MIN_TIME_VALUE;

                    dwStatus = WriteRegistrySlqTime (
                        hKeyLogQuery,
                        (LPCWSTR)L"Start",
                        &slqTime);
                    //    
                    // If stop time mode set to manual, or StopAt with time before Now,
                    // set the mode to Manual, value to MAX_TIME_VALUE.
                    //
                    memset (&slqTime, 0, sizeof(slqTime));
                    slqTime.wTimeType = SLQ_TT_TTYPE_STOP;
                    slqTime.wDataType = SLQ_TT_DTYPE_DATETIME;
                    slqTime.dwAutoMode = SLQ_AUTO_MODE_NONE;
                    slqTime.llDateTime = MAX_TIME_VALUE;

                    dwStatus = ReadRegistrySlqTime (
                                hKeyLogQuery, 
                                pArg->szPerfLogName,
                                (LPCWSTR)L"Stop",
                                &slqTime,
                                &slqTime);

                    if ( SLQ_AUTO_MODE_NONE == slqTime.dwAutoMode 
                        || ( SLQ_AUTO_MODE_AT == slqTime.dwAutoMode 
                            && llTime >= slqTime.llDateTime ) ) {

                        slqTime.wTimeType = SLQ_TT_TTYPE_STOP;
                        slqTime.wDataType = SLQ_TT_DTYPE_DATETIME;
                        slqTime.dwAutoMode = SLQ_AUTO_MODE_NONE;
                        slqTime.llDateTime = MAX_TIME_VALUE;
                
                        dwStatus = WriteRegistrySlqTime (
                                        hKeyLogQuery, 
                                        (LPCWSTR)L"Stop",
                                        &slqTime);
                    }

                    //
                    // Set state to start pending.
                    //
                    if (dwStatus == ERROR_SUCCESS) {
                        dwValue = SLQ_QUERY_START_PENDING;
                        dwStatus = WriteRegistryDwordValue (
                            hKeyLogQuery,
                            (LPCWSTR)L"Current State",
                            &dwValue,
                            REG_DWORD);
                    }

                    //
                    // Update the modified time to indicate a change has occurred.
                    //
                    memset (&slqTime, 0, sizeof(slqTime));
                    //
                    // LastModified and LastConfigured values are stored as GMT.
                    //
                    GetSystemTimeAsFileTime ( (LPFILETIME)(&llModifiedTime) );

                    slqTime.wTimeType = SLQ_TT_TTYPE_LAST_MODIFIED;
                    slqTime.wDataType = SLQ_TT_DTYPE_DATETIME;
                    slqTime.dwAutoMode = SLQ_AUTO_MODE_NONE;
                    slqTime.llDateTime = llModifiedTime;

                    dwStatus = WriteRegistrySlqTime (
                        hKeyLogQuery,
                        (LPCWSTR)L"Last Modified",
                        &slqTime);


                    if (dwStatus == ERROR_SUCCESS) {
                        hSC = OpenSCManager ( NULL, NULL, SC_MANAGER_ALL_ACCESS);

                        if (hSC != NULL) {
                            //
                            // Ping the service controller to rescan the entries.
                            //
                            hService = OpenServiceW (
                                            hSC, 
                                            (LPCWSTR)L"SysmonLog",
                                            SERVICE_USER_DEFINED_CONTROL | SERVICE_START );

                            if (hService != NULL) {
                                ControlService ( 
                                    hService, 
                                    SERVICE_CONTROL_SYNCHRONIZE, 
                                    &ssData);
                                CloseServiceHandle (hService);
                            } else {
                                // unable to open log service
                                dwStatus = GetLastError();
                            }
                            CloseServiceHandle (hSC);
                        } else {                
                            // unable to open service controller
                            dwStatus = GetLastError();
                        }
                    } // else unable to set the time

                    if ( ( ERROR_SUCCESS != dwStatus )
                            && ( 1 != pArg->dwAlertLogFailureReported ) ) {
                        LPWSTR  szStringArray[2];

                        szStringArray[0] = pArg->szPerfLogName;
                        szStringArray[1] = pArg->szQueryName;

                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_UNABLE_START_ALERT_LOG,
                            NULL,
                            2,
                            sizeof(DWORD),
                            szStringArray,
                            (LPVOID)&dwStatus );
                
                        pArg->dwAlertLogFailureReported = 1;
                    }
                } else {
                    //
                    // The query is either pending or running already.
                    //
                    dwStatus = ERROR_SUCCESS;
                }
            } else { 
                dwStatus = SMLOG_UNABLE_READ_ALERT_LOG;
        
                if ( 1 != pArg->dwAlertLogFailureReported ) {
                    LPWSTR  szStringArray[2];

                    szStringArray[0] = pArg->szPerfLogName;
                    szStringArray[1] = pArg->szQueryName;

                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_UNABLE_READ_ALERT_LOG,
                        NULL,
                        2,
                        0,
                        szStringArray,
                        NULL );
        
                    pArg->dwAlertLogFailureReported = 1;
                }
            }
        } // ToDo: else report StringCchxxx failure.
    } else {

        dwStatus = SMLOG_UNABLE_READ_ALERT_LOG;
        
        if ( 1 != pArg->dwAlertLogFailureReported ) {
            LPWSTR  szStringArray[2];

            szStringArray[0] = pArg->szPerfLogName;
            szStringArray[1] = pArg->szQueryName;

            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                SMLOG_UNABLE_READ_ALERT_LOG,
                NULL,
                2,
                0,
                szStringArray,
                NULL );
        
            pArg->dwAlertLogFailureReported = 1;
        }
    }

    if ( NULL != hKeyLogQuery ) {
        RegCloseKey ( hKeyLogQuery );
    }

    return dwStatus;
}

DWORD
DoAlertCommandFile (
    IN  PLOG_QUERY_DATA     pArg,
    IN  PALERT_COUNTER_INFO pAlertCI,
    IN  LPCWSTR             szTimeStamp,
    IN  LPCWSTR             szMeasuredValue,
    IN  LPCWSTR             szOverUnder,
    IN  LPCWSTR             szLimitValue
)
{
    const   INT ciMaxDelimPerArg = 3;
            DWORD   dwStatus = ERROR_SUCCESS;
            BOOL    bStatus = FALSE;
            LPWSTR  szCommandString = NULL;
            INT     iBufLen = 0;
            LPWSTR  szTempBuffer = NULL;
            LONG    lErrorMode;
            size_t  sizeStrLen;
            DWORD   dwCmdFlags;
            BOOL    bSingleArg = FALSE;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            DWORD   dwCreationFlags = NORMAL_PRIORITY_CLASS;
            LPWSTR  szDelim1;
            LPWSTR  szDelim2;
            BOOL    bFirstArgDone = FALSE;

    if ( NULL != pArg 
            && NULL != pAlertCI ) {

        if ( NULL != pArg->szCmdFileName ) {

            dwStatus = pArg->dwCmdFileFailure;

            if ( ERROR_SUCCESS == dwStatus ) { 

                // See if any of the argument flags are set.
                dwCmdFlags = pArg->dwAlertActionFlags & ALRT_CMD_LINE_MASK;

                if ( 0 != dwCmdFlags ) {
                    // Allocate space for all arguments

                    if ( NULL != pArg->szQueryName ) {
                        iBufLen += lstrlen ( pArg->szQueryName ) + ciMaxDelimPerArg;
                    }
                    if ( NULL != szTimeStamp ) {
                        iBufLen += lstrlen ( szTimeStamp ) + ciMaxDelimPerArg;
                    }
                    if ( NULL != pAlertCI->pAlertInfo->szCounterPath) {
                        iBufLen += lstrlen ( pAlertCI->pAlertInfo->szCounterPath ) + ciMaxDelimPerArg;
                    }
                    if ( NULL != szMeasuredValue ) {
                        iBufLen += lstrlen ( szMeasuredValue ) + ciMaxDelimPerArg;
                    }
                    if ( NULL != szOverUnder ) {
                        iBufLen += lstrlen ( szOverUnder ) + ciMaxDelimPerArg;
                    }
                    if ( NULL != szLimitValue ) {
                        iBufLen += lstrlen ( szLimitValue ) + ciMaxDelimPerArg;
                    }
                    if ( NULL != pArg->szUserText ) {
                        iBufLen += lstrlen ( pArg->szUserText ) + ciMaxDelimPerArg;
                    }
                    iBufLen+= 2;    // 1 for possible leading ", 1 for NULL.

                    szCommandString = (LPWSTR)G_ALLOC(iBufLen * sizeof(WCHAR));

                    if ( NULL != szCommandString ) { 

                        szCommandString[0] = L'\0';

                        // build command line arguments
                        if ((pArg->dwAlertActionFlags  & ALRT_CMD_LINE_SINGLE) != 0) {
                            bSingleArg = TRUE;
                            szDelim1 = L",";
                            szDelim2 = L"\0";
                        } else {
                            // multiple arguments enclosed by double quotes and 
                            // separated by a space
                            szDelim1 = L" \"";
                            szDelim2 = L"\"";
                        }

                        if (pArg->dwAlertActionFlags & ALRT_CMD_LINE_A_NAME ) {
                            if ( NULL != pArg->szQueryName ) {
                                if (bFirstArgDone) {
                                    //
                                    // Add leading delimiter
                                    //
                                    StringCchCat ( szCommandString, iBufLen, szDelim1 );
                                } else {
                                    //
                                    // Add leading quote
                                    //
                                    StringCchCat ( szCommandString, iBufLen, L"\"" );
                                    bFirstArgDone = TRUE;
                                }
                                StringCchCat ( szCommandString, iBufLen, pArg->szQueryName );
                                StringCchCat ( szCommandString, iBufLen, szDelim2 );
                            } else {
                                dwStatus = ERROR_INVALID_PARAMETER;
                            }
                        }

                        if ( ERROR_SUCCESS == dwStatus
                                && ( pArg->dwAlertActionFlags  & ALRT_CMD_LINE_D_TIME ) ) 
                        {
                            if ( NULL != szTimeStamp ) {
                                if (bFirstArgDone) {
                                    StringCchCat ( szCommandString, iBufLen, szDelim1 );
                                } else {
                                    StringCchCat ( szCommandString, iBufLen, L"\"" );
                                    bFirstArgDone = TRUE;
                                }

                                StringCchCat ( szCommandString, iBufLen, szTimeStamp );
                                StringCchCat ( szCommandString, iBufLen, szDelim2 );
                            } else {
                                dwStatus = ERROR_INVALID_PARAMETER;
                            }
                        }

                        if ( ERROR_SUCCESS == dwStatus
                                && ( pArg->dwAlertActionFlags  & ALRT_CMD_LINE_C_NAME ) ) 
                        {
                            if ( NULL != pAlertCI->pAlertInfo->szCounterPath ) {
                                if (bFirstArgDone) {
                                    StringCchCat ( szCommandString, iBufLen, szDelim1 );
                                } else {
                                    StringCchCat ( szCommandString, iBufLen, L"\"" );
                                    bFirstArgDone = TRUE;
                                }
                                StringCchCat ( szCommandString, iBufLen, pAlertCI->pAlertInfo->szCounterPath );
                                StringCchCat ( szCommandString, iBufLen, szDelim2 );
                            } else {
                                dwStatus = ERROR_INVALID_PARAMETER;
                            }
                        }

                        if ( ERROR_SUCCESS == dwStatus
                                && ( pArg->dwAlertActionFlags  & ALRT_CMD_LINE_M_VAL ) ) 
                        {
                            if ( NULL != szMeasuredValue ) {
                                if (bFirstArgDone) {
                                    StringCchCat ( szCommandString, iBufLen, szDelim1 );
                                } else {
                                    StringCchCat ( szCommandString, iBufLen, L"\"" );
                                    bFirstArgDone = TRUE;
                                }
                                StringCchCat ( szCommandString, iBufLen, szMeasuredValue );
                                StringCchCat ( szCommandString, iBufLen, szDelim2 );
                            } else {
                                dwStatus = ERROR_INVALID_PARAMETER;
                            }
                        }

                        if ( ERROR_SUCCESS == dwStatus
                                && ( pArg->dwAlertActionFlags  & ALRT_CMD_LINE_L_VAL ) ) 
                        {
                            if ( NULL != szOverUnder && NULL != szLimitValue ) {
                                if (bFirstArgDone) {
                                    StringCchCat ( szCommandString, iBufLen, szDelim1 );
                                } else {
                                    StringCchCat ( szCommandString, iBufLen, L"\"" );
                                    bFirstArgDone = TRUE;
                                }
                                StringCchCat ( szCommandString, iBufLen, szOverUnder );
                                StringCchCat ( szCommandString, iBufLen, L" ");
                                StringCchCat ( szCommandString, iBufLen, szLimitValue );
                                StringCchCat ( szCommandString, iBufLen, szDelim2 );
                            } else {
                                dwStatus = ERROR_INVALID_PARAMETER;
                            }
                        }

                        if ( ERROR_SUCCESS == dwStatus
                                && ( pArg->dwAlertActionFlags  & ALRT_CMD_LINE_U_TEXT ) ) 
                        {
                            if ( NULL != pArg->szUserText ) {
                                if (bFirstArgDone) {
                                    StringCchCat ( szCommandString, iBufLen, szDelim1 );
                                } else {
                                    StringCchCat ( szCommandString, iBufLen, L"\"" );
                                    bFirstArgDone = TRUE;
                                }
                                StringCchCat ( szCommandString, iBufLen, pArg->szUserText );
                                StringCchCat ( szCommandString, iBufLen, szDelim2 );
                            } else {
                                dwStatus = ERROR_INVALID_PARAMETER;
                            }
                        }

                        if (bFirstArgDone && bSingleArg) {
                            // add closing quote if there's at least 1 arg in the command line
                            StringCchCat ( szCommandString, iBufLen, L"\"" );
                        }
                    } else {
                        dwStatus = ERROR_OUTOFMEMORY;
                    }

                    if ( ERROR_SUCCESS == dwStatus )
                    {

                        iBufLen = lstrlen( pArg->szCmdFileName ) + 1;  // 1 for NULL
                        if ( NULL != szCommandString ) {
                            iBufLen += lstrlen ( szCommandString ) + 1;  // 1 for space char
                        }
                        iBufLen += 2;  // 2 for quote characters
                        szTempBuffer = (LPWSTR)G_ALLOC(iBufLen * sizeof(WCHAR));
                    }

                    if ( NULL != szTempBuffer ) {

                        // build command line arguments
                        StringCchCopy ( szTempBuffer, iBufLen, pArg->szCmdFileName );

                        // see if this is a CMD or a BAT file
                        // if it is then create a process with a console window, otherwise
                        // assume it's an executable file that will create it's own window
                        // or console if necessary
                        //
                        _wcslwr (szTempBuffer);
                        if ((wcsstr(szTempBuffer, L".bat") != NULL) ||
                            (wcsstr(szTempBuffer, L".cmd") != NULL)){
                                dwCreationFlags |= CREATE_NEW_CONSOLE;
                        } else {
                                dwCreationFlags |= DETACHED_PROCESS;
                        }
                        // recopy the image name to the temp buffer since it was modified
                        // (i.e. lowercased) for the previous comparison.

                        szTempBuffer[0] = L'\"';

                        StringCchCopy ( (szTempBuffer+1), iBufLen - 1, pArg->szCmdFileName );
                        StringCchLength (szTempBuffer, iBufLen - 1, &sizeStrLen) ;
                        szTempBuffer[sizeStrLen] = L'\"';
                        sizeStrLen++;                        
                        szTempBuffer[sizeStrLen] = L'\0';

                        if ( NULL != szCommandString ) {
                            // now add on the alert text preceded with a space char
                            szTempBuffer [sizeStrLen] = L' ' ;
                            sizeStrLen++ ;

                            StringCchCopy ( &szTempBuffer[sizeStrLen], iBufLen - sizeStrLen, szCommandString );
                        }
                    
                        // initialize Startup Info block
                        memset (&si, 0, sizeof(si));
                        si.cb = sizeof(si);
                        si.dwFlags = STARTF_USESHOWWINDOW ;
                        si.wShowWindow = SW_SHOWNOACTIVATE ;
                        //si.lpDesktop = L"WinSta0\\Default";

                        memset (&pi, 0, sizeof(pi));

                        // supress pop-ups inf the detached process
                        lErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

                        //
                        // The lpApplication name is NULL for CreateProcess 
                        // because the normal use of this function is to launch
                        // batch files which must be the first part of the lpCommandLine.
                        // The quotes around the szCommandFileName prevents the wrong
                        // file from being executed.
                        //

                        if( pArg->hUserToken != NULL ){
                            bStatus = CreateProcessAsUser (
                                        pArg->hUserToken,
                                        NULL,
                                        szTempBuffer,
                                        NULL, NULL, FALSE,
                                        dwCreationFlags,
                                        NULL,
                                        NULL,
                                        &si,
                                        &pi);
                        } else {
                            bStatus = CreateProcess (
                                        NULL,
                                        szTempBuffer,
                                        NULL, NULL, FALSE,
                                        dwCreationFlags,
                                        NULL,
                                        NULL,
                                        &si,
                                        &pi);
                        }

                        SetErrorMode(lErrorMode);

                        if (bStatus) {
                            dwStatus = ERROR_SUCCESS;
                            if ( NULL != pi.hThread && INVALID_HANDLE_VALUE != pi.hThread ) {
                                CloseHandle(pi.hThread);
                                pi.hThread = NULL;
                            }
                            if ( NULL != pi.hProcess && INVALID_HANDLE_VALUE != pi.hProcess ) {
                                CloseHandle(pi.hProcess);
                                pi.hProcess = NULL;
                            }
                        } else {
                            dwStatus = GetLastError();
                        }
                    } else {
                        dwStatus = ERROR_OUTOFMEMORY;
                    }
                    if (szCommandString != NULL) G_FREE(szCommandString);
                    if (szTempBuffer != NULL) G_FREE(szTempBuffer);
                }
            }

            if ( ERROR_SUCCESS != dwStatus ) { 

                LPWSTR  szStringArray[2];

                szStringArray[0] = szTempBuffer;
                szStringArray[1] = pArg->szQueryName;

                ReportEvent (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_ALERT_CMD_FAIL,
                    NULL,
                    2,
                    sizeof(DWORD),
                    szStringArray,
                    (LPVOID)&dwStatus );

                pArg->dwCmdFileFailure = dwStatus;
            }
        } else {
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
    }
    return dwStatus;
}

BOOL
ExamineAlertValues (
    IN    PLOG_QUERY_DATA pArg
)
{
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    DWORD       dwStatus = ERROR_SUCCESS;
    PALERT_COUNTER_INFO     pAlertCI;
    DWORD                   dwType;
    PDH_FMT_COUNTERVALUE    pdhCurrentValue;
    BOOL                    bDoAlertAction;

    //
    // For each counter in query, compare it's formatted
    // value against the alert value and do the desired operation
    // if the alert condition is exceeded.
    //
    for (pAlertCI = (PALERT_COUNTER_INFO)pArg->pFirstCounter;
         pAlertCI != NULL;
         pAlertCI = pAlertCI->next) {

        bDoAlertAction = FALSE;
        
        //
        // get formatted counter value
        //
        pdhStatus = PdhGetFormattedCounterValue (
                        pAlertCI->hCounter,
                        PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
                        &dwType,
                        &pdhCurrentValue);

        if ((pdhStatus == ERROR_SUCCESS) && 
            ((pdhCurrentValue.CStatus == PDH_CSTATUS_VALID_DATA) || 
             (pdhCurrentValue.CStatus == PDH_CSTATUS_NEW_DATA))) {
            //
            // The value is good so compare it
            //
            if ((pAlertCI->pAlertInfo->dwFlags & AIBF_OVER) == AIBF_OVER) {
                //
                // Test for value > limit.
                //
                if (pdhCurrentValue.doubleValue > pAlertCI->pAlertInfo->dLimit) {
                    bDoAlertAction = TRUE;
                }
            } else {
                //
                // Test for value < limit.
                //
                if (pdhCurrentValue.doubleValue < pAlertCI->pAlertInfo->dLimit) {
                    bDoAlertAction = TRUE;
                }
            }
        }

        if (bDoAlertAction) {
            WCHAR   szValueString[SLQ_MAX_VALUE_LEN];
            WCHAR   szLimitString[SLQ_MAX_VALUE_LEN];
            WCHAR   szOverUnderString[64];
            WCHAR   szTimeStampFmt[2*SLQ_MAX_VALUE_LEN];
            WCHAR   szTimeStamp[2*SLQ_MAX_VALUE_LEN];
            DWORD   dwFmtStringFlags;
            size_t  cchBufLen;
            SYSTEMTIME  st;

            //
            // Build arguments used by event log and net messsage if either 
            // option is enabled
            //
            dwFmtStringFlags = ALRT_ACTION_LOG_EVENT | ALRT_ACTION_SEND_MSG | ALRT_ACTION_EXEC_CMD;

            if ((pArg->dwAlertActionFlags & dwFmtStringFlags) != 0) {
                INT     nResId;
 
                //
                // Report event to event log
                //

                //
                // Format message string elements
                // The following methods truncate and null terminate the string if it is
                // too long for the buffer.  Continue in this case because strings are only
                // used to report data to the user.
                //
                StringCchPrintf (
                        szValueString,
                        SLQ_MAX_VALUE_LEN,
                        L"%.*g", 
                        DBL_DIG, 
                        pdhCurrentValue.doubleValue );

                StringCchPrintf (
                    szLimitString,
                    SLQ_MAX_VALUE_LEN,
                    L"%.*g", 
                    DBL_DIG, 
                    pAlertCI->pAlertInfo->dLimit );

                nResId = pAlertCI->pAlertInfo->dwFlags & AIBF_OVER ? IDS_OVER : IDS_UNDER;
                LoadString (
                    hModule,
                    nResId,
                    szOverUnderString, 
                    (sizeof(szOverUnderString) / sizeof(szOverUnderString[0])));
                
                //
                // Get timestamp format string
                //
                LoadString (
                    hModule,
                    IDS_ALERT_TIMESTAMP_FMT,
                    szTimeStampFmt, 
                    (sizeof(szTimeStampFmt) / sizeof(szTimeStampFmt[0])));

                //
                // Message format string expects the following args:
                //  Timestamp
                //  Counter path string
                //  measured value
                //  over/under
                //  limit value
                GetLocalTime (&st);

                StringCchPrintf (
                    szTimeStamp,
                    2*SLQ_MAX_VALUE_LEN,
                    szTimeStampFmt, 
                    st.wYear, 
                    st.wMonth, 
                    st.wDay,
                    st.wHour, 
                    st.wMinute, 
                    st.wSecond );
            }

            //
            // Do action(s) as defined in flags
            //
            if ((pArg->dwAlertActionFlags & ALRT_ACTION_LOG_EVENT) == ALRT_ACTION_LOG_EVENT) {
                LPWSTR  szStringArray[4];

                szStringArray[0] = pAlertCI->pAlertInfo->szCounterPath;
                szStringArray[1] = szValueString;
                szStringArray[2] = szOverUnderString;
                szStringArray[3] = szLimitString;

                ReportEvent (hEventLog,
                    EVENTLOG_INFORMATION_TYPE,
                    0,
                    SMLOG_ALERT_LIMIT_CROSSED,
                    NULL,
                    4,
                    0,
                    szStringArray,
                    NULL);
            }
            

            if ((pArg->dwAlertActionFlags & ALRT_ACTION_SEND_MSG) == ALRT_ACTION_SEND_MSG) {
               if (pArg->szNetName != NULL) {
                    size_t  sizeCchMessageTextLen = 0;
                    size_t  sizeCchComponentLen = 0;
                    WCHAR   szMessageFormat[MAX_PATH+1];
                    LPWSTR  szMessageText = NULL;
                    //
                    // Get message format string
                    //
                    LoadString (hModule,
                        IDS_ALERT_MSG_FMT,
                        szMessageFormat, 
                        (sizeof(szMessageFormat) / sizeof(szMessageFormat[0])));
                    //
                    // MAX_PATH + 1 - 1
                    //
                    szMessageFormat [MAX_PATH] = L'\0';

                    // message format string expects the following args:
                    //  Timestamp
                    //  Counter path string
                    //  measured value
                    //  over/under
                    //  limit value

                    StringCchLength ( szMessageFormat, STRSAFE_MAX_CCH, &sizeCchComponentLen );
                    sizeCchMessageTextLen += sizeCchComponentLen;
                    
                    StringCchLength ( szTimeStamp, STRSAFE_MAX_CCH, &sizeCchComponentLen );
                    sizeCchMessageTextLen += sizeCchComponentLen;

                    StringCchLength ( pAlertCI->pAlertInfo->szCounterPath, STRSAFE_MAX_CCH, &sizeCchComponentLen );
                    sizeCchMessageTextLen += sizeCchComponentLen;

                    StringCchLength ( szValueString, STRSAFE_MAX_CCH, &sizeCchComponentLen );
                    sizeCchMessageTextLen += sizeCchComponentLen;

                    StringCchLength ( szOverUnderString, STRSAFE_MAX_CCH, &sizeCchComponentLen );
                    sizeCchMessageTextLen += sizeCchComponentLen;
    
                    StringCchLength ( szLimitString, STRSAFE_MAX_CCH, &sizeCchComponentLen );
                    sizeCchMessageTextLen += sizeCchComponentLen;
                    //
                    // Add one for Null.
                    //
                    sizeCchMessageTextLen++;

                    szMessageText = G_ALLOC ( sizeCchMessageTextLen * sizeof(WCHAR) );

                    if ( NULL != szMessageText ) {

                        //
                        // Truncation is okay.
                        //
                        StringCchPrintf (
                                szMessageText,
                                sizeCchMessageTextLen,
                                szMessageFormat, 
                                szTimeStamp,
                                pAlertCI->pAlertInfo->szCounterPath,
                                szValueString,
                                szOverUnderString,
                                szLimitString);

                        //
                        // Send network message to specified computer
                        //
                        dwStatus = NetMessageBufferSend(  
                                        NULL,
                                        pArg->szNetName,    
                                        NULL,      
                                        (LPBYTE)szMessageText,           
                                        (DWORD)(sizeCchMessageTextLen * sizeof(WCHAR)) );

                        G_FREE (szMessageText);
                    } else {
                        dwStatus = ERROR_OUTOFMEMORY;
                    }

                } else {
                   dwStatus = ERROR_OUTOFMEMORY;
                }
                if ( ( ERROR_SUCCESS != dwStatus )
                        && ( 1 != pArg->dwNetMsgFailureReported ) ) {
                    LPWSTR  szStringArray[3];
                    //
                    // Write event log warning message for net message
                    // only one time per session.
                    //
                    szStringArray[0] = pArg->szQueryName;
                    szStringArray[1] = pArg->szNetName;
                    szStringArray[2] = FormatEventLogMessage(dwStatus);
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_NET_MESSAGE_WARNING,
                        NULL,
                        3,
                        sizeof(DWORD),
                        szStringArray,
                        (LPVOID)&dwStatus);
                    
                    LocalFree( szStringArray[2] );
                    pArg->dwNetMsgFailureReported = 1;
                }
            }

            if ((pArg->dwAlertActionFlags & ALRT_ACTION_EXEC_CMD) == ALRT_ACTION_EXEC_CMD) {
                //
                // Errors logged in DoAlertCommandFile.
                //
                dwStatus = DoAlertCommandFile (
                                pArg,
                                pAlertCI,
                                szTimeStamp,
                                szValueString,
                                szOverUnderString,
                                szLimitString);
            }

            if ((pArg->dwAlertActionFlags & ALRT_ACTION_START_LOG) == ALRT_ACTION_START_LOG) {
                //
                // Start specified perf data log. 
                // Errors logged in StartLogQuery.
                //
                dwStatus = StartLogQuery ( pArg );
                
            }
        }
    }  // end of for each counter in alert loop
    return TRUE;
}

BOOL
AlertProc (
    IN    PLOG_QUERY_DATA pArg
)
{
    DWORD           dwStatus = ERROR_SUCCESS;
    LARGE_INTEGER   liStartDelayTics;
    LARGE_INTEGER   liSampleDelayTics;
    LONGLONG        llSampleCollectionTics;
    LONGLONG        llSampleIntervalTics;
    PDH_STATUS      pdhStatus = ERROR_SUCCESS;
    DWORD           dwCounterCount;
    INT             iRetries;
    LONG            lWaitStatus;

    LPWSTR          szThisPath;
    BOOL            bRun = FALSE;
    LPWSTR          szStringArray[4];
    WCHAR           szRetryCount[SLQ_MAX_VALUE_LEN];
    
    LONGLONG        llSessionSampleCount=(LONGLONG)-1;
    PALERT_COUNTER_INFO   pCtrInfo = NULL;
    PALERT_INFO_BLOCK   pAlertInfo = NULL;
    DWORD           dwBufSize;

    LONGLONG        llStartTime = 0;
    LONGLONG        llFinishTime = 0;
    HANDLE          arrEventHandle[2];
    BOOL            bFirstTimeout;

    arrEventHandle[0] = pArg->hExitEvent;           // WAIT_OBJECT_0
    arrEventHandle[1] = pArg->hReconfigEvent;

    __try {

        liStartDelayTics.QuadPart = ((LONGLONG)(0));
        liSampleDelayTics.QuadPart = ((LONGLONG)(0));
        llSampleCollectionTics = ((LONGLONG)(0));

        //
        // Read registry values.
        //
        if ( ERROR_SUCCESS == LoadQueryConfig ( pArg ) ) {
            bRun = TRUE;
        }
     
        if ( TRUE == bRun ) {
            // Delay of -1 signals exit immediately.
            liStartDelayTics.QuadPart = ComputeStartWaitTics ( pArg, TRUE );

            if ( NULL_INTERVAL_TICS == liStartDelayTics.QuadPart ) {
                bRun = FALSE;
            }
        }

        if ( TRUE == bRun ) {
        
            ValidateCommandFilePath ( pArg );

            // open query and add counters from info file

            iRetries = NUM_PDH_RETRIES;
            bFirstTimeout = TRUE;
            do {
                if (pArg->dwRealTimeQuery == DATA_SOURCE_WBEM) {
                    pdhStatus = PdhOpenQueryH(
                            H_WBEM_DATASOURCE, 0, & pArg->hQuery); // from current activity
                } else {
                    pdhStatus = PdhOpenQueryH(
                            H_REALTIME_DATASOURCE, 0, & pArg->hQuery);
                }

                if ( bFirstTimeout && WAIT_TIMEOUT == pdhStatus ) {
                    //
                    // Write event log warning message
                    //
                    StringCchPrintf (
                        szRetryCount,
                        SLQ_MAX_VALUE_LEN,
                        L"%d", 
                        iRetries );

                    szStringArray[0] = pArg->szQueryName;
                    szStringArray[1] = szRetryCount;
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_OPEN_QUERY_TIMEOUT,
                        NULL,
                        2,
                        sizeof(DWORD),
                        szStringArray,
                        (LPVOID)&pdhStatus);

                    bFirstTimeout = FALSE;
                }
            } while ( WAIT_TIMEOUT == (lWaitStatus = WaitForMultipleObjects (2, arrEventHandle, FALSE, 0)) &&
                      WAIT_TIMEOUT == pdhStatus &&
                      iRetries-- > 0);
            if ( WAIT_TIMEOUT != lWaitStatus ) {
                // the loop was terminated by the Exit/Reconfigure event
                if ( ERROR_SUCCESS == pdhStatus ) {
                    PdhCloseQuery(pArg->hQuery);
                }
                bRun = FALSE;
            } else if (pdhStatus != ERROR_SUCCESS) {
                // unable to open query so write event log message and exit
                szStringArray[0] = pArg->szQueryName;
                szStringArray[1] = FormatEventLogMessage(pdhStatus);
                ReportEvent (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_UNABLE_OPEN_PDH_QUERY,
                    NULL,
                    2,
                    sizeof(DWORD),
                    szStringArray,
                    (LPVOID)&pdhStatus);

                LocalFree( szStringArray[1] );

                bRun = FALSE;
            } 
        }

        //
        // Add each counter and associated alert limits.
        //
        if ( TRUE == bRun ) {
            dwCounterCount = 0;
            for (szThisPath = pArg->mszCounterList;
                    *szThisPath != 0;
                    szThisPath += lstrlen(szThisPath) + 1) {
            
                HCOUNTER        hThisCounter;

                //
                // Allocate information block
                //
                dwBufSize = (lstrlenW(szThisPath) + 1) * sizeof(WCHAR);
                dwBufSize += sizeof(ALERT_INFO_BLOCK);
                pAlertInfo = (PALERT_INFO_BLOCK)G_ALLOC(dwBufSize);

                if (pAlertInfo == NULL) {
                    dwStatus = SMLOG_UNABLE_ALLOC_ALERT_MEMORY;
                    break;
                } else {                
                    //
                    // Get alert info from string
                    //
                    if (MakeInfoFromString (szThisPath, pAlertInfo, &dwBufSize)) {

                        iRetries = NUM_PDH_RETRIES;
                        bFirstTimeout = TRUE;
                        do {
                        
                            pdhStatus = PdhAdd009Counter (pArg->hQuery,
                                                   pAlertInfo->szCounterPath, 
                                                   dwCounterCount, 
                                                   &hThisCounter);
                            
                            if ( bFirstTimeout && WAIT_TIMEOUT == pdhStatus ) {
                                //
                                // Write event log warning message
                                //
                                StringCchPrintf (
                                    szRetryCount,
                                    SLQ_MAX_VALUE_LEN,
                                    L"%d", 
                                    iRetries );

                                szStringArray[0] = pAlertInfo->szCounterPath;
                                szStringArray[1] = pArg->szQueryName;
                                szStringArray[2] = szRetryCount;
                                ReportEvent (hEventLog,
                                    EVENTLOG_WARNING_TYPE,
                                    0,
                                    SMLOG_ADD_COUNTER_TIMEOUT,
                                    NULL,
                                    3,
                                    sizeof(DWORD),
                                    szStringArray,
                                    (LPVOID)&pdhStatus);

                                bFirstTimeout = FALSE;
                            }


                        } while ( WAIT_TIMEOUT == (lWaitStatus = WaitForMultipleObjects (2, arrEventHandle, FALSE, 0)) &&
                                  WAIT_TIMEOUT == pdhStatus &&
                                  iRetries-- > 0);

                        if ( WAIT_TIMEOUT != lWaitStatus ) {
                            if ( NULL != pAlertInfo ) {
                                G_FREE (pAlertInfo); // toss unused alert buffer
                                pAlertInfo = NULL;
                            }
                            bRun = FALSE;
                            dwStatus = ERROR_CANCELLED; // don't report an error
                            break;
                        } else {

                            if (pdhStatus != ERROR_SUCCESS) {
                                iRetries = NUM_PDH_RETRIES;
                                do {
                                    pdhStatus = PdhAddCounter (pArg->hQuery,
                                                       pAlertInfo->szCounterPath, 
                                                       dwCounterCount, 
                                                       &hThisCounter);

                                } while ( WAIT_TIMEOUT == (lWaitStatus = WaitForMultipleObjects (2, arrEventHandle, FALSE, 0)) &&
                                          WAIT_TIMEOUT == pdhStatus &&
                                          iRetries-- > 0);

                                if ( WAIT_TIMEOUT != lWaitStatus ) {
                                    if ( NULL != pAlertInfo ) {
                                        G_FREE (pAlertInfo); // toss unused alert buffer
                                        pAlertInfo = NULL;
                                    }
                                    bRun = FALSE;
                                    dwStatus = ERROR_CANCELLED; // don't report an error
                                    break;
                                }
                            }
                        }

                        if ( !IsErrorSeverity(pdhStatus) ) {

                            dwCounterCount++;

                            if ( ERROR_SUCCESS != pdhStatus ) {
                                szStringArray[0] = szThisPath;
                                szStringArray[1] = pArg->szQueryName;
                                szStringArray[2] = FormatEventLogMessage(pdhStatus);
                                ReportEvent (hEventLog,
                                    EVENTLOG_WARNING_TYPE,
                                    0,
                                    SMLOG_ADD_COUNTER_WARNING,
                                    NULL,
                                    3,
                                    sizeof(DWORD),
                                    szStringArray,
                                    (LPVOID)&pdhStatus);
                                LocalFree( szStringArray[2] );
                            }

                            //
                            //  Add this handle to the list
                            //
                            pCtrInfo = G_ALLOC (sizeof (ALERT_COUNTER_INFO));
                    
                            if (pCtrInfo != NULL) {
                                //
                                // Insert at front of list since the order isn't
                                // important.
                                //
                                pCtrInfo->hCounter = hThisCounter;
                                pCtrInfo->pAlertInfo = pAlertInfo;
                                pCtrInfo->next = (PALERT_COUNTER_INFO)pArg->pFirstCounter;
                                pArg->pFirstCounter = (PLOG_COUNTER_INFO)pCtrInfo;
                                pAlertInfo = NULL;
                                pCtrInfo = NULL;
                            } else {
                                dwStatus = SMLOG_UNABLE_ALLOC_ALERT_MEMORY;
                                //
                                // Delete unused alert info structure.
                                //
                                G_FREE (pAlertInfo); 
                                pAlertInfo = NULL;
                                break;
                            }
                        } else {
                            //
                            // Unable to add the current counter.
                            //
                            szStringArray[0] = pAlertInfo->szCounterPath;
                            szStringArray[1] = pArg->szQueryName;
                            szStringArray[2] = FormatEventLogMessage(pdhStatus);

                            if ( PDH_ACCESS_DENIED == pdhStatus ) {
                                ReportEvent (
                                    hEventLog,
                                    EVENTLOG_WARNING_TYPE,
                                    0,
                                    SMLOG_UNABLE_ACCESS_COUNTER,
                                    NULL,
                                    2,
                                    0,
                                    szStringArray,
                                    NULL);
                            } else {

                                ReportEvent (
                                    hEventLog,
                                    EVENTLOG_WARNING_TYPE,
                                    0,
                                    SMLOG_UNABLE_ADD_COUNTER,
                                    NULL,
                                    3,
                                    sizeof(DWORD),
                                    szStringArray,
                                    (LPVOID)&pdhStatus);
                            }
                            LocalFree( szStringArray[2] );

                            //
                            // Delete unused alert info structure.
                            //
                            if ( NULL != pAlertInfo ) {
                                G_FREE (pAlertInfo); 
                                pAlertInfo = NULL;
                            }
                        }
                    } else {
                        //
                        // Unable to parse alert info, or 
                        // unable to add the current counter.
                        //
                        szStringArray[0] = szThisPath;
                        szStringArray[1] = pArg->szQueryName;
                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_UNABLE_PARSE_ALERT_INFO,
                            NULL,
                            2,
                            0,
                            szStringArray,
                            NULL);

                        //
                        // Delete unused alert info structure.
                        //
                        if ( NULL != pAlertInfo ) {
                            G_FREE (pAlertInfo); 
                            pAlertInfo = NULL;
                        }
                    }
                }
            }

            if ( ERROR_SUCCESS == dwStatus ) {
            
                if ( 0 < dwCounterCount ) {
                    //
                    // Raise priority to ensure that data is logged.
                    //
                    SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
                } else {
                    bRun = FALSE;
                    //
                    // Unable to add any counters.
                    //
                    szStringArray[0] = pArg->szQueryName;
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_UNABLE_ADD_ANY_COUNTERS,
                        NULL,
                        1,
                        0,
                        szStringArray,
                        NULL);
                }
            } else {

                assert ( ERROR_OUTOFMEMORY == dwStatus );
                //
                // Memory allocation error.
                //
                szStringArray[0] = pArg->szQueryName;
                ReportEvent (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_UNABLE_ALLOC_ALERT_MEMORY,
                    NULL,
                    1,
                    0,
                    szStringArray,
                    NULL);

                bRun = FALSE;
            }

            while (bRun) {

                if ( 0 < liStartDelayTics.QuadPart ) {
                    //
                    // NtWaitForMultipleObjects requires negative Tic value
                    //
                    liStartDelayTics.QuadPart = ((LONGLONG)(0)) - liStartDelayTics.QuadPart;
                    //
                    // Wait until specified start time, or until exit or reconfigure event.
                    //
                    if ( STATUS_TIMEOUT != NtWaitForMultipleObjects ( 
                                                2, 
                                                &arrEventHandle[0], 
                                                WaitAny,
                                                FALSE, 
                                                &liStartDelayTics )) {
                        //
                        // Exit if not running.
                        //
                        bRun = FALSE;
                        break;
                    }
                }

                pArg->dwCurrentState = SLQ_QUERY_RUNNING;
                dwStatus = WriteRegistryDwordValue (
                            pArg->hKeyQuery, 
                            (LPCWSTR)L"Current State",
                            &pArg->dwCurrentState,
                            REG_DWORD );
                assert (dwStatus == ERROR_SUCCESS);
                    
                szStringArray[0] = pArg->szQueryName;
                ReportEvent (hEventLog,
                    EVENTLOG_INFORMATION_TYPE,
                    0,
                    SMLOG_ALERT_SCANNING,
                    NULL,
                    1,
                    0,
                    szStringArray,
                    NULL);
                //
                // Compute session sample count.
                // 0 samples signals no limit.
                // -1 samples signals exit immediately
                //
                ComputeSampleCount( pArg, TRUE, &llSessionSampleCount );
            
                if ( -1 == llSessionSampleCount ) {
                    goto ProcessAlertRepeat;
                }

                //
                // Start sampling immediately. liSampleDelayTics is initialized to 0.
                // Wait until specified sample time, or until exit or reconfigure event.
                //
                while ( STATUS_TIMEOUT == NtWaitForMultipleObjects ( 
                                            2, 
                                            &arrEventHandle[0], 
                                            WaitAny, 
                                            FALSE, 
                                            &liSampleDelayTics)) {
                    //
                    // An event flag will be set when the sampling should exit or reconfigure. if
                    // the wait times out, then that means it's time to collect and
                    // log another sample of data.
                    //
                
                    GetLocalFileTime (&llStartTime);
                    //
                    // Check for reconfig event.
                    //
                    if ( pArg->bLoadNewConfig ) {
                        bRun = FALSE;
                        break;
                    }

                    pdhStatus = PdhCollectQueryData (pArg->hQuery);

                    if ( IsPdhDataCollectSuccess ( pdhStatus )
                            || IsWarningSeverity ( pdhStatus ) ) {
                    
                        if (pdhStatus == ERROR_SUCCESS) {
                            //
                            // Process alert counters.
                            //
                            ExamineAlertValues (pArg);
                        }
                        //
                        // See if it's time to restart or end the alert scan.
                        // 0 samples signals no sample limit.
                        //
                        if ( 0 != llSessionSampleCount ) {
                            if ( !--llSessionSampleCount ) 
                                break;
                        }
                    } else {
                        //
                        // Unable to collect the query data.
                        //
                        szStringArray[0] = pArg->szQueryName;
                        szStringArray[1] = FormatEventLogMessage(pdhStatus);

                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_UNABLE_COLLECT_DATA,
                            NULL,
                            2,
                            sizeof(DWORD),
                            szStringArray,
                            (LPVOID)&pdhStatus);

                        LocalFree( szStringArray[1] );

                        bRun = FALSE;
                        break;
                    }

                    //
                    // Compute new timeout value
                    //
                    GetLocalFileTime (&llFinishTime);

                    llSampleCollectionTics = llFinishTime - llStartTime;

                    llSampleIntervalTics = 
                        (LONGLONG)pArg->dwMillisecondSampleInterval*FILETIME_TICS_PER_MILLISECOND;
                    if ( llSampleCollectionTics < llSampleIntervalTics ) {
                        liSampleDelayTics.QuadPart = llSampleIntervalTics - llSampleCollectionTics;
                    } else {
                        liSampleDelayTics.QuadPart = ((LONGLONG)(0));                       
                    }
                    //
                    // NtWaitForMultipleObjects requires negative Tic value
                    //
                    liSampleDelayTics.QuadPart = ((LONGLONG)(0)) - liSampleDelayTics.QuadPart;

                } // end while wait keeps timing out
                
                //
                // Use 0 SampleDelayTics value to check for ExitEvent.
                //
                liSampleDelayTics.QuadPart = ((LONGLONG)(0));

                if ( pArg->bLoadNewConfig ) {
                    bRun = FALSE;
                } else if ( STATUS_TIMEOUT != NtWaitForSingleObject (
                                                pArg->hExitEvent, 
                                                FALSE, 
                                                &liSampleDelayTics ) ) {
                    //
                    // The loop was terminated by the Exit event
                    // so clear the "run" flag to exit the loop & thread.
                    //
                    bRun = FALSE;
                }
                //
                // Exit if restart not enabled.
                //
ProcessAlertRepeat:
                if ( bRun ) {
                    bRun = ProcessRepeatOption ( pArg, &liStartDelayTics );
                }

            } // end while (bRun)
            
            PdhCloseQuery (pArg->hQuery);
            pArg->hQuery = NULL;        
        }

        SetLastError ( ERROR_SUCCESS );
    } __except ( EXCEPTION_EXECUTE_HANDLER ) {

        bRun = FALSE;
        
        if ( NULL != pArg->hQuery ) {
            PdhCloseQuery ( pArg->hQuery );
            pArg->hQuery = NULL;
        }

        SetLastError ( SMLOG_THREAD_FAILED );  
    }
        
    DeallocateQueryBuffers ( pArg );

    while ( NULL != pArg->pFirstCounter ) {
        PALERT_COUNTER_INFO pDelCI = (PALERT_COUNTER_INFO)pArg->pFirstCounter;
        if (pDelCI->pAlertInfo != NULL) G_FREE(pDelCI->pAlertInfo);

        pArg->pFirstCounter = (PLOG_COUNTER_INFO)pDelCI->next;

        G_FREE( pDelCI );
    }

    return bRun;
}

BOOL
CounterLogProc (
    IN    PLOG_QUERY_DATA pArg )
{
#define INSTBUFLEN  (4096)

    DWORD           dwStatus = ERROR_SUCCESS;
    LARGE_INTEGER   liStartDelayTics;
    LARGE_INTEGER   liSampleDelayTics;
    LONGLONG        llSampleCollectionTics;
    LONGLONG        llSampleIntervalTics;
    PDH_STATUS      pdhStatus = ERROR_SUCCESS;
    DWORD           dwCounterCount;
    INT             iRetries;
    INT             iCnfSerial;
    DWORD           dwSessionSerial;

    LPWSTR          szThisPath;
    DWORD           dwPdhLogFileType;
    DWORD           dwPdhAccessFlags;
    BOOL            bRun = FALSE;
    LONGLONG        llSessionSampleCount=(LONGLONG)-1;
    LONGLONG        llCnfSampleCount=(LONGLONG)-1;
    LONGLONG        llLoopSampleCount=(LONGLONG)-1;
    // Todo:  Enforce log file name length
    WCHAR           szCurrentLogFile[MAX_PATH+1];
    WCHAR           szRetryCount[SLQ_MAX_VALUE_LEN];
    BOOL            bFirstTimeout;
    LPWSTR          szStringArray[4];
    DWORD           dwFileSizeLimit;
    ULONGLONG       ullFileSizeLimit;
    LONGLONG        llFileSize;

    LONGLONG        llStartTime = 0;
    LONGLONG        llFinishTime = 0;
    PLOG_COUNTER_INFO pDelCI;

    // Wildcard processing
    ULONG   ulBufLen = 0;
    ULONG   ulBufSize = 0;
    ULONG   ulLocaleBufLen = 0;
    LPWSTR  szLocaleBuf = NULL;
    LPWSTR  pLocalePath = NULL;
    INT     nCounterBufRetry;
    LPWSTR  pszCounterBuf = NULL;
    LPWSTR  pszCounter;
    DWORD   dwPdhExpandFlags;
    PPDH_COUNTER_PATH_ELEMENTS pPathInfo = NULL;

    LONG            lWaitStatus;
    HANDLE arrEventHandle[2];

    arrEventHandle[0] = pArg->hExitEvent;           // WAIT_OBJECT_0
    arrEventHandle[1] = pArg->hReconfigEvent;

    __try {

        liStartDelayTics.QuadPart = ((LONGLONG)(0));
        liSampleDelayTics.QuadPart = ((LONGLONG)(0));
        llSampleCollectionTics = ((LONGLONG)(0));

        //
        // Read registry values.
        //
        if ( ERROR_SUCCESS == LoadQueryConfig ( pArg ) ) {
            bRun = TRUE;
        }
    
        if ( TRUE == bRun ) {
            //
            // Delay of -1 signals exit immediately.
            //
            liStartDelayTics.QuadPart = ComputeStartWaitTics ( pArg, TRUE );

            if ( NULL_INTERVAL_TICS == liStartDelayTics.QuadPart ) {
                bRun = FALSE;
            }
        }

        if ( TRUE == bRun ) {
            //
            // Stop the query if new log file folder is not valid.
            // ProcessLogFileFolder reports an error event on failure.  Event message content
            // depends on whether this is a reconfiguration or the original configuration.
            //
            bRun = ( ERROR_SUCCESS == ProcessLogFileFolder( pArg ) );
        }

        if ( TRUE == bRun ) {
       
            ValidateCommandFilePath ( pArg );

            //
            // Open query and add counters from info file.
            //

            iRetries = NUM_PDH_RETRIES;
            bFirstTimeout = TRUE;
            do {
                if (pArg->dwRealTimeQuery == DATA_SOURCE_WBEM) {
                    pdhStatus = PdhOpenQueryH(
                            H_WBEM_DATASOURCE, 0, & pArg->hQuery); 
                } else {
                    pdhStatus = PdhOpenQueryH(
                            H_REALTIME_DATASOURCE, 0, & pArg->hQuery);
                }

                if ( bFirstTimeout && WAIT_TIMEOUT == pdhStatus ) {
                    //
                    // Write event log warning message
                    //
                    StringCchPrintf (
                        szRetryCount,
                        SLQ_MAX_VALUE_LEN,
                        L"%d", 
                        iRetries );

                    szStringArray[0] = pArg->szQueryName;
                    szStringArray[1] = szRetryCount;
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_OPEN_QUERY_TIMEOUT,
                        NULL,
                        2,
                        sizeof(DWORD),
                        szStringArray,
                        (LPVOID)&pdhStatus);

                    bFirstTimeout = FALSE;
                }
            } while ( WAIT_TIMEOUT == (lWaitStatus = WaitForMultipleObjects (2, arrEventHandle, FALSE, 0)) &&
                      WAIT_TIMEOUT == pdhStatus &&
                      iRetries-- > 0 );

            if ( WAIT_TIMEOUT != lWaitStatus ) {
                // the loop was terminated by the Exit/Reconfigure event
                if ( ERROR_SUCCESS == pdhStatus ) {
                    PdhCloseQuery (pArg->hQuery);
                }
                bRun = FALSE;
            } else if (pdhStatus != ERROR_SUCCESS) {
                 //
                // Unable to open query.
                //
                szStringArray[0] = pArg->szQueryName;
                szStringArray[1] = FormatEventLogMessage(pdhStatus);
                ReportEvent (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_UNABLE_OPEN_PDH_QUERY,
                    NULL,
                    2,
                    sizeof(DWORD),
                    szStringArray,
                    (LPVOID)&pdhStatus);
                LocalFree( szStringArray[1] );

                bRun = FALSE;
            }
        }
        //
        // Add each counter to the open query.
        //
        if ( TRUE == bRun ) {
    
            dwStatus = ERROR_SUCCESS;
            dwCounterCount = 0;
            for (szThisPath = pArg->mszCounterList;
                *szThisPath != 0;
                szThisPath += lstrlen(szThisPath) + 1) {

                if (wcschr(szThisPath, L'*') == NULL) {
                    //
                    // No wildcards
                    //
                    dwStatus = AddCounterToCounterLog( pArg, szThisPath, pArg->hQuery, LOG_EVENT_ON_ERROR, &dwCounterCount );
                } else {
                    //
                    // At least one wildcard
                    //
                    dwPdhExpandFlags = 0;
                    pszCounterBuf = NULL;

                    //
                    // Only expand wildcard instances for text log files.
                    //
                    if (pArg->dwLogFileType == SLF_SQL_LOG) {
                        //
                        // No need to expand wildcard instances for SQL log.
                        // SQL log now has the capability to catch dynamic
                        // instances, so we can pass in wildcard-instance
                        // counter names here.
                        //
                        dwPdhExpandFlags |= PDH_NOEXPANDINSTANCES;
                    } else if (   SLF_CSV_FILE != pArg->dwLogFileType
                             && SLF_TSV_FILE != pArg->dwLogFileType) {
                        //
                        // This is the binary counter logfile case.
                        // No need to expand wildcard instances. Also, if
                        // default real-time datasource is from registry (not
                        // WMI), we can handle add-by-object.
                        //
                        dwPdhExpandFlags |= PDH_NOEXPANDINSTANCES;

                        if ( DATA_SOURCE_REGISTRY == pArg->dwRealTimeQuery) {
                            //
                            // If both instance and counter are wildcards, then log by object
                            // rather than expanding the counter path.
                            // This is only true when the actual data source is the registry.
                            //
                            // Parse pathname.
                            //
                            do {
                                if (pPathInfo) {
                                    G_FREE(pPathInfo);
                                    pPathInfo = NULL;
                                }
                                else {
                                    ulBufSize = sizeof(PDH_COUNTER_PATH_ELEMENTS) + (PDH_MAX_COUNTER_PATH + 1) * sizeof(WCHAR);
                                }
    
                                pPathInfo = (PPDH_COUNTER_PATH_ELEMENTS) G_ALLOC(ulBufSize);
    
                                if (pPathInfo == NULL) {
                                    pdhStatus = ERROR_OUTOFMEMORY;
                                    break;
                                }
    
                                pdhStatus = PdhParseCounterPath( szThisPath, pPathInfo, & ulBufSize, 0);
    
                            } while (pdhStatus == PDH_INSUFFICIENT_BUFFER || pdhStatus == PDH_MORE_DATA);
    
                            if (pdhStatus == ERROR_SUCCESS) {
                                if ( 0 == lstrcmpi ( pPathInfo->szCounterName, L"*" ) ) {
                                    if ( NULL != pPathInfo->szInstanceName ) {
                                        if ( 0 == lstrcmpi ( pPathInfo->szInstanceName, L"*" ) ) {
                                            //
                                            // If PdhAddCounter failed,the realtime data source is actually WBEM.
                                            // In this case, expand the counter paths.
                                            //
                                            dwStatus = AddCounterToCounterLog( pArg, szThisPath, pArg->hQuery, !LOG_EVENT_ON_ERROR, &dwCounterCount );
                                            if ( ERROR_SUCCESS == dwStatus ) {
                                                continue;
                                            } else {
                                                //
                                                // Enumerate counter paths below and retry.
                                                //
                                                dwStatus = ERROR_SUCCESS;
                                            }
                                        }
                                    } else {
                                        dwStatus = AddCounterToCounterLog( pArg, szThisPath, pArg->hQuery, !LOG_EVENT_ON_ERROR, &dwCounterCount );
                                        //
                                        // If PdhAddCounter failed,the realtime data source is actually WBEM.
                                        // In this case, expand the counter paths.
                                        //
                                        if ( ERROR_SUCCESS == dwStatus ) {
                                            continue;
                                        } else {
                                            //
                                            // Enumerate counter paths below and retry.
                                            //
                                            dwStatus = ERROR_SUCCESS;
                                        }
                                    }
                                }
                            } else {
                                //
                                // Report event and continue to next counter.
                                //
                                szStringArray[0] = szThisPath;
                                szStringArray[1] = pArg->szQueryName;
                                szStringArray[2] = FormatEventLogMessage(pdhStatus);
                                ReportEvent (
                                    hEventLog,
                                    EVENTLOG_WARNING_TYPE,
                                    0,
                                    SMLOG_UNABLE_PARSE_COUNTER,
                                    NULL,
                                    3,
                                    sizeof(DWORD),
                                    szStringArray,
                                    (LPVOID)&pdhStatus);
                                LocalFree( szStringArray[2] );

                                continue;
                            }
                        }
                    }
                    //
                    // Log by object paths are already processed.  For other paths with at least 
                    // one wildcard, expand the path before adding counters.
                    //

                    //
                    // Initialize the locale path buffer
                    //
                    pLocalePath = NULL;
                    if (ulLocaleBufLen == 0) {
                        ulLocaleBufLen = PDH_MAX_COUNTER_PATH + 1;

                        szLocaleBuf = (LPWSTR) G_ALLOC(ulLocaleBufLen * sizeof(WCHAR));
                        if (szLocaleBuf == NULL) {
                            dwStatus = ERROR_OUTOFMEMORY;
                            ulLocaleBufLen = 0;
                        }
                    }

                    if ( szLocaleBuf != NULL ) {
                        //
                        // Translate counter name from English to Localized.
                        //
                        ulBufSize = ulLocaleBufLen;
        
                        pdhStatus = PdhTranslateLocaleCounter(
                                        szThisPath,
                                        szLocaleBuf,
                                        &ulBufSize);
        
                        if (pdhStatus == PDH_MORE_DATA) {
                            if (szLocaleBuf) {
                                G_FREE(szLocaleBuf);
                                szLocaleBuf = NULL;
                                ulLocaleBufLen = 0;
                            }

                            szLocaleBuf = (LPWSTR) G_ALLOC(ulBufSize * sizeof(WCHAR));
                            if (szLocaleBuf != NULL) {
                                ulLocaleBufLen = ulBufSize;

                                pdhStatus = PdhTranslateLocaleCounter(
                                                szThisPath,
                                                szLocaleBuf,
                                                &ulBufSize);
                            }
                            else {
                                dwStatus = ERROR_OUTOFMEMORY;
                            }
                        }

                        if (pdhStatus == ERROR_SUCCESS) {
                            pLocalePath = szLocaleBuf;
                        }
                    }

                    if (pLocalePath) {
                        ulBufLen          = INSTBUFLEN;
                        nCounterBufRetry  = 10;   // the retry counter

                        do {
                            //
                            // pszCounterBuf is always NULL the first time through.
                            //
                            if ( NULL != pszCounterBuf ) {
                                G_FREE(pszCounterBuf);
                                pszCounterBuf = NULL;
                                ulBufLen *= 2;
                            }

                            pszCounterBuf = (WCHAR*) G_ALLOC(ulBufLen * sizeof(WCHAR));
                            if (pszCounterBuf == NULL) {
                                dwStatus = ERROR_OUTOFMEMORY;
                                break;
                            }
                
                            pdhStatus = PdhExpandWildCardPath (
                                NULL,
                                pLocalePath,
                                pszCounterBuf,
                                &ulBufLen,
                                dwPdhExpandFlags);
                            nCounterBufRetry--;
                        } while ((pdhStatus == PDH_MORE_DATA) && (nCounterBufRetry));
    

                        if (ERROR_SUCCESS == pdhStatus && ERROR_SUCCESS == dwStatus ) {
                            //
                            // Add path. 
                            //
                            for (pszCounter = pszCounterBuf;
                                *pszCounter != 0;
                                pszCounter += lstrlen(pszCounter) + 1) {

                                dwStatus = AddCounterToCounterLog ( pArg, pszCounter, pArg->hQuery, LOG_EVENT_ON_ERROR, &dwCounterCount );
                                if ( ERROR_OUTOFMEMORY == dwStatus ) {
                                    break;
                                }
                            }
                        }
                        if ( NULL != pszCounterBuf ) {
                            G_FREE(pszCounterBuf);
                            pszCounterBuf = NULL;
                        }
                    }
                }

                if ( ERROR_OUTOFMEMORY == dwStatus ) {

                    szStringArray[0] = pArg->szQueryName;
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_UNABLE_ALLOC_LOG_MEMORY,
                        NULL,
                        1,
                        0,
                        szStringArray,
                        NULL);
                    bRun = FALSE;
                } // Other errors reported within the loop
            }

            if (szLocaleBuf) {
                G_FREE(szLocaleBuf);
                szLocaleBuf = NULL;
            }
            if ( bRun ) {

                if ( 0 < dwCounterCount ) {
                    //
                    // Raise priority to make sure we get to log the data.
                    //
                    SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
                } else {
                    //
                    // Unable to add any counters.
                    //
                    bRun = FALSE;

                    szStringArray[0] = pArg->szQueryName;
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_UNABLE_ADD_ANY_COUNTERS,
                        NULL,
                        1,
                        0,
                        szStringArray,
                        NULL);

                }
            }

            while (bRun) {
                
                //
                // Wait until specified start time, or until exit or reconfig event.
                //
                if ( 0 < liStartDelayTics.QuadPart ) {
                    //
                    // NtWaitForMultipleObjects requires negative Tic value
                    //
                    liStartDelayTics.QuadPart = ((LONGLONG)(0)) - liStartDelayTics.QuadPart;

                    if ( STATUS_TIMEOUT != NtWaitForMultipleObjects ( 
                                                2, 
                                                &arrEventHandle[0], 
                                                WaitAny,
                                                FALSE, 
                                                &liStartDelayTics)) {
                        bRun = FALSE;
                        break;  // if we're not supposed to be running then bail out
                    }
                }
                //
                // Compute session sample count.
                // 0 samples signals no limit.
                // -1 samples signals exit immediately, because stop time is past.
                //
                ComputeSampleCount( pArg, TRUE, &llSessionSampleCount );

                if ( (LONGLONG)(-1) == llSessionSampleCount ) {
                    goto ProcessCounterRepeat;
                }

                //
                // Set session or cnf file size limit.
                //
                if ( SLQ_DISK_MAX_SIZE != pArg->dwMaxFileSize ) {
                    if (pArg->dwLogFileType == SLF_SQL_LOG) {
                        dwFileSizeLimit = pArg->dwMaxFileSize;
                    }
                    else {
                        dwFileSizeLimit = pArg->dwMaxFileSize * pArg->dwLogFileSizeUnit;    
                    }
                }
                else {
                    dwFileSizeLimit = 0;
                }

                //
                // 0 file size signals no limit.
                // Translate from DWORD to ULONGLONG instead of LONGLONG to preserve 
                // positive value, even if high bit of dword is used.
                //
                ullFileSizeLimit = ((ULONGLONG)(dwFileSizeLimit));

                ComputeSampleCount( pArg, FALSE, &llCnfSampleCount );
                if ( (LONGLONG)(-1) == llCnfSampleCount ) {
                    // Todo cnf:  Internal program error, report error and exit.
                    bRun = FALSE;
                    break;
                }

                if ( SLQ_AUTO_MODE_AFTER == pArg->stiCreateNewFile.dwAutoMode 
                    || SLQ_AUTO_MODE_SIZE == pArg->stiCreateNewFile.dwAutoMode ) {
                    iCnfSerial = 1;
                } else {
                    assert ( SLQ_AUTO_MODE_NONE == pArg->stiCreateNewFile.dwAutoMode );
                    iCnfSerial = 0;
                }

                dwSessionSerial = pArg->dwCurrentSerialNumber;

                BuildCurrentLogFileName (
                    pArg->szQueryName,
                    pArg->szBaseFileName,
                    pArg->szLogFileFolder,
                    pArg->szSqlLogName,
                    szCurrentLogFile,
                    &dwSessionSerial,
                    pArg->dwAutoNameFormat,
                    pArg->dwLogFileType,
                    iCnfSerial++ );

                //
                // Update log serial number if modified.
                //
                if (pArg->dwAutoNameFormat == SLF_NAME_NNNNNN) {
                
                    pArg->dwCurrentSerialNumber++;
                    // Todo:  Info event on number wrap.
                    if ( MAXIMUM_SERIAL_NUMBER < pArg->dwCurrentSerialNumber ) {
                        pArg->dwCurrentSerialNumber = MINIMUM_SERIAL_NUMBER;
                    }

                    dwStatus = RegSetValueEx (
                        pArg->hKeyQuery,
                        L"Log File Serial Number",
                        0L,
                        REG_DWORD,
                        (LPBYTE)&pArg->dwCurrentSerialNumber,
                        sizeof(DWORD));

                    assert ( ERROR_SUCCESS == dwStatus );
                }

                SetPdhOpenOptions ( pArg, &dwPdhAccessFlags, &dwPdhLogFileType );

                //
                // Create new file loop.
                //
                while ( bRun && (LONGLONG)(-1) != llSessionSampleCount ) {
                    assert ( (LONGLONG)(-1) != llCnfSampleCount );
                    //
                    // Compute cnf or session loop interval.
                    //
                    if ( (LONGLONG)(0) == llCnfSampleCount 
                            || ( (LONGLONG)(0) != llSessionSampleCount
                                    && llCnfSampleCount > llSessionSampleCount ) ) 
                    {   
                        //
                        // No need to create new file within session.
                        //
                        llLoopSampleCount = llSessionSampleCount;
                        //
                        // Specify exit after first loop if not cnf by size.
                        //
                        if ( SLQ_AUTO_MODE_SIZE != pArg->stiCreateNewFile.dwAutoMode ) {
                            llSessionSampleCount = (LONGLONG)(-1);
                        }
                    } else {
                        //
                        // Create new file by time before session ends.
                        //
                        llLoopSampleCount = llCnfSampleCount;
                        if ( (LONGLONG)(0) != llSessionSampleCount ) {
                            llSessionSampleCount -= llCnfSampleCount;
                            // todo cnf:  The following should be logically impossible,
                            // because session > newfile wait.
                            if ( llSessionSampleCount <= (LONGLONG)(0) ) {
                                llSessionSampleCount = (LONGLONG)(-1);
                            }
                        }
                    }

                    __try {
                        //
                        // Open log file using this query.
                        // For text files, max size is checked after each data collection
                        //
                        pdhStatus = PdhOpenLog (
                            szCurrentLogFile,
                            dwPdhAccessFlags,
                            &dwPdhLogFileType,
                            pArg->hQuery,
                            (   SLF_BIN_CIRC_FILE == pArg->dwLogFileType
                             || SLF_BIN_FILE == pArg->dwLogFileType
                             || SLF_SQL_LOG == pArg->dwLogFileType )
                                    ? dwFileSizeLimit
                                    : 0,                                  
                            ( ( PDH_LOG_TYPE_BINARY != dwPdhLogFileType ) ? pArg->szLogFileComment : NULL ),
                            &pArg->hLog);
                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }

                    if ( ERROR_SUCCESS != pdhStatus ) { 
                        //
                        // Unable to open log file.
                        //
                        dwStatus = GetLastError();
                        szStringArray[0] = szCurrentLogFile;
                        szStringArray[1] = pArg->szQueryName;
                        szStringArray[2] = FormatEventLogMessage(dwStatus);

                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_UNABLE_OPEN_LOG_FILE,
                            NULL,
                            3,
                            sizeof(DWORD),
                            szStringArray,
                            (LPVOID)&dwStatus);

                        LocalFree( szStringArray[2] );

                        bRun = FALSE; // exit now
                        break;
                    } else {

                        RegisterCurrentFile( pArg->hKeyQuery, szCurrentLogFile, 0 );

                        pArg->dwCurrentState = SLQ_QUERY_RUNNING;
                        dwStatus = WriteRegistryDwordValue (
                                    pArg->hKeyQuery, 
                                    (LPCWSTR)L"Current State",
                                    &pArg->dwCurrentState,
                                    REG_DWORD );
                        assert (dwStatus == ERROR_SUCCESS);
                
                        szStringArray[0] = pArg->szQueryName;
                        szStringArray[1] = szCurrentLogFile;
                        ReportEvent (hEventLog,
                            EVENTLOG_INFORMATION_TYPE,
                            0,
                            SMLOG_LOGGING_QUERY,
                            NULL,
                            2,
                            0,
                            szStringArray,
                            NULL);
                    } 

                    //
                    // Start sampling immediately. liSampleDelayTics is initialized to 0.
                    //
                    while ( STATUS_TIMEOUT == NtWaitForMultipleObjects ( 
                                                2, 
                                                &arrEventHandle[0], 
                                                WaitAny,
                                                FALSE, 
                                                &liSampleDelayTics)) {
                        //    
                        // An event flag will be set when the sampling should exit or reconfigure. if
                        // the wait times out, then that means it's time to collect and
                        // log another sample of data.
                        //
            
                        GetLocalFileTime (&llStartTime);
                        //
                        // Check for reconfig event.
                        if ( pArg->bLoadNewConfig ) {
                            bRun = FALSE;
                            break;
                        }

                        pdhStatus = PdhUpdateLog (pArg->hLog, pArg->szLogFileComment );

                        if ( IsPdhDataCollectSuccess ( pdhStatus ) 
                            || IsWarningSeverity ( pdhStatus ) ) {
                            //
                            // See if it's time to restart or end the log.
                            // 0 samples signals no sample limit.
                            //
                            if ( ((LONGLONG)0) != llLoopSampleCount ) {
                                if ( !--llLoopSampleCount ) 
                                    break;
                            }

                            if ( ( ((ULONGLONG)0) != ullFileSizeLimit ) 
                                && ( SLF_BIN_CIRC_FILE != pArg->dwLogFileType ) ) {
                                //
                                // See if the file is too big.
                                //
                                pdhStatus = PdhGetLogFileSize (pArg->hLog, &llFileSize);
                                if (pdhStatus == ERROR_SUCCESS) {
                                    if (ullFileSizeLimit <= (ULONGLONG)llFileSize) 
                                        break;
                                }
                            }
            
                        
                        } else {
                            //
                            // Unable to update the log.
                            //
                            szStringArray[0] = pArg->szQueryName;
                            szStringArray[1] = FormatEventLogMessage(pdhStatus);
                            ReportEvent (hEventLog,
                                EVENTLOG_WARNING_TYPE,
                                0,
                                SMLOG_UNABLE_UPDATE_LOG,
                                NULL,
                                2,
                                sizeof(DWORD),
                                szStringArray,
                                (LPVOID)&pdhStatus);

                            LocalFree( szStringArray[1] );

                            bRun = FALSE;
                            break;
                        }

                        //
                        // Compute new timeout value.
                        //
                        GetLocalFileTime (&llFinishTime);
                        //
                        // Compute difference in tics
                        //
                        llSampleCollectionTics = llFinishTime - llStartTime;

                        llSampleIntervalTics = 
                            (LONGLONG)pArg->dwMillisecondSampleInterval*FILETIME_TICS_PER_MILLISECOND;

                        if ( llSampleCollectionTics < llSampleIntervalTics ) {
                            liSampleDelayTics.QuadPart = llSampleIntervalTics - llSampleCollectionTics;
                        } else {
                            liSampleDelayTics.QuadPart = ((LONGLONG)(0));                       
                        }
                        //
                        // NtWaitForMultipleObjects requires negative Tic value.
                        //
                        liSampleDelayTics.QuadPart = ((LONGLONG)(0)) - liSampleDelayTics.QuadPart;
                    } // end while wait keeps timing out
                
                    //
                    // Use 0 SampleDelayTics value to check for ExitEvent.
                    //
                    liSampleDelayTics.QuadPart = ((LONGLONG)(0));

                    if ( pArg->bLoadNewConfig ) {
                        bRun = FALSE;
                    } else if ( STATUS_TIMEOUT != NtWaitForSingleObject (
                                                    pArg->hExitEvent, 
                                                    FALSE, 
                                                    &liSampleDelayTics ) ) {
                        //
                        // The loop was terminated by the Exit event
                        // so clear the "run" flag to exit the loop & thread.
                        //
                        bRun = FALSE;
                    }
                    //
                    // Close log file, but keep query open.
                    //
                    PdhCloseLog (pArg->hLog, 0);
                    pArg->hLog = NULL;
                
                    if ( pArg->bLoadNewConfig )
                        break;

                    if ( pArg->szCmdFileName != NULL )
                        DoLogCommandFile (pArg, szCurrentLogFile, bRun);
            
                    if ( (LONGLONG)(-1) != llSessionSampleCount ) {
                        //
                        // Create new log name
                        //
                        BuildCurrentLogFileName (
                            pArg->szQueryName,
                            pArg->szBaseFileName,
                            pArg->szLogFileFolder,
                            pArg->szSqlLogName,
                            szCurrentLogFile,
                            &dwSessionSerial,
                            pArg->dwAutoNameFormat,
                            pArg->dwLogFileType,
                            iCnfSerial++ );

                        // Todo cnf:  report event on error;
                    }

                } // End of log file creation while loop

                // cnf Todo:  Handle break from sample loop. ?

                //
                // Exit if restart not enabled.
                //
ProcessCounterRepeat:
                if ( bRun ) {
                    bRun = ProcessRepeatOption ( pArg, &liStartDelayTics );
                }

            } // end while (bRun)

            PdhCloseQuery (pArg->hQuery);
            pArg->hQuery = NULL;
        }
        SetLastError ( ERROR_SUCCESS );

    } __except ( EXCEPTION_EXECUTE_HANDLER ) {

        bRun = FALSE;
        
        if ( NULL != pszCounterBuf ) {
            G_FREE(pszCounterBuf);
            pszCounterBuf = NULL;
        }

        if ( NULL != pArg->hLog ) {
            PdhCloseLog ( pArg->hLog, 0 );
            pArg->hLog = NULL;
        }

        if ( NULL != pArg->hQuery ) {
            PdhCloseQuery ( pArg->hQuery );
            pArg->hQuery = NULL;
        }

        SetLastError ( SMLOG_THREAD_FAILED );        
    }

    DeallocateQueryBuffers ( pArg );

    while ( NULL != pArg->pFirstCounter ) {
        pDelCI = pArg->pFirstCounter;
        pArg->pFirstCounter = pDelCI->next;
        G_FREE( pDelCI );
    }

    return bRun;
}

BOOL
TraceLogProc (
    IN    PLOG_QUERY_DATA pArg
)
{
    LARGE_INTEGER   liStartDelayTics;
    LARGE_INTEGER   liWaitTics;
    LONGLONG        llSessionWaitTics = 0;
    LONGLONG        llNewFileWaitTics = INFINITE_TICS;
    DWORD           dwStatus = ERROR_SUCCESS;
    BOOL            bRun = FALSE;
    BOOL            bStarted = FALSE;
    LPWSTR          szStringArray[4];
    INT             iCnfSerial = 0;
    ULONG           ulIndex;
    int             iEnableCount = 0;
    DWORD           dwSessionSerial;
    HANDLE          arrEventHandle[2];

    __try {

        liStartDelayTics.QuadPart = NULL_INTERVAL_TICS;
        liWaitTics.QuadPart = ((LONGLONG)(0));
        
        //
        // Read registry values.
        //
        if ( ERROR_SUCCESS == LoadQueryConfig ( pArg ) ) {
            bRun = TRUE;
        }
     
        if ( TRUE == bRun ) {
            //
            // Delay of -1 signals exit immediately.
            //
            liStartDelayTics.QuadPart = ComputeStartWaitTics ( pArg, TRUE );

            if ( NULL_INTERVAL_TICS == liStartDelayTics.QuadPart ) {
                bRun = FALSE;
            }
        }

        if ( TRUE == bRun ) {
            //
            // Stop the query if new log file folder is not valid.
            //
            bRun = ( ERROR_SUCCESS == ProcessLogFileFolder( pArg ) );
        }

        if ( bRun ) {

            ValidateCommandFilePath ( pArg );
            //
            // Raise priority to ensure that data is logged.
            //
            SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
        }


        pArg->bCallCloseTraceLogger = TRUE;
        pArg->bExitOnTermination = TRUE;
        while (bRun) {
            arrEventHandle[0] = pArg->hExitEvent;           // WAIT_OBJECT_0
            arrEventHandle[1] = pArg->hReconfigEvent;

            if ( 0 < liStartDelayTics.QuadPart ) {
                //
                // NtWaitForMultipleObjects requires negative Tic value.
                //
                liStartDelayTics.QuadPart = ((LONGLONG)(0)) - liStartDelayTics.QuadPart;
                //
                // Wait until specified start time, or until exit or reconfig event.
                //
                if ( STATUS_TIMEOUT != NtWaitForMultipleObjects ( 
                                            2, 
                                            &arrEventHandle[0],
                                            WaitAny,
                                            FALSE, 
                                            &liStartDelayTics)) {
                    //
                    // The loop was terminated by the Exit event
                    // so clear the "run" flag to exit the loop & thread.
                    //
                    bRun = FALSE;   
                    break;
                }
            }

            ComputeSessionTics( pArg, &llSessionWaitTics );
            //
            // 0 signals no session time, so exit.
            //
            if ( ((LONGLONG)(0)) == llSessionWaitTics ) {
                goto ProcessTraceRepeat;
            }

            //
            // llNewFileWaitTics defaults to -1 if no time limit.
            //
            ComputeNewFileTics( pArg, &llNewFileWaitTics );
            
            //
            // InitTraceProperties creates the current file name.
            //
            dwSessionSerial = pArg->dwCurrentSerialNumber;

            InitTraceProperties ( pArg, TRUE, &dwSessionSerial, &iCnfSerial );

            dwStatus = GetTraceQueryStatus ( pArg, NULL );

            //
            // If trace session with this name already started and successful,
            // don't create another session.
            //
        
            if ( ERROR_SUCCESS != dwStatus ) {

                dwStatus = StartTrace(
                            &pArg->LoggerHandle, 
                            pArg->szLoggerName, 
                            &pArg->Properties );
                if (dwStatus == ERROR_SUCCESS) {
                    bStarted = TRUE;
                }
                pArg->bExitOnTermination = TRUE;

                if ( ( ERROR_SUCCESS == dwStatus ) 
                     && !( pArg->Properties.EnableFlags & EVENT_TRACE_FLAG_PROCESS
                            || pArg->Properties.EnableFlags & EVENT_TRACE_FLAG_THREAD
                            || pArg->Properties.EnableFlags & EVENT_TRACE_FLAG_DISK_IO
                            || pArg->Properties.EnableFlags & EVENT_TRACE_FLAG_NETWORK_TCPIP ) ) {
            
                    for ( ulIndex = 0; ulIndex < pArg->ulGuidCount; ulIndex++ ) {
                        //
                        // Enable user mode and special kernel tracing.
                        //
                        dwStatus = EnableTrace (
                                    TRUE,
                                    pArg->arrpGuid[ulIndex].dwFlag,
                                    pArg->arrpGuid[ulIndex].dwLevel,
                                    &pArg->arrpGuid[ulIndex].Guid, 
                                    pArg->LoggerHandle);
                        if ( ERROR_SUCCESS == dwStatus ) {
                            iEnableCount++;
                        } else {
                            szStringArray[0] = pArg->arrpGuid[ulIndex].pszProviderName;
                            szStringArray[1] = pArg->szQueryName;
                    
                            ReportEvent (hEventLog,
                                EVENTLOG_WARNING_TYPE,
                                0,
                                SMLOG_UNABLE_ENABLE_TRACE_PROV,
                                NULL,
                                2,
                                sizeof(DWORD),
                                szStringArray,      
                                (LPVOID)&dwStatus);
                        }
                    }
            
                    if ( 0 < iEnableCount ) {
                        dwStatus = ERROR_SUCCESS;
                    } else {
                        szStringArray[0] = pArg->szQueryName;
                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_TRACE_NO_PROVIDERS,
                            NULL,
                            1,
                            0,
                            szStringArray,      
                            NULL);
                        bRun = FALSE;
                    }
                }
            
                if ( bRun && ERROR_SUCCESS == dwStatus ) {

                    pArg->dwCurrentState = SLQ_QUERY_RUNNING;
                    dwStatus = WriteRegistryDwordValue (
                                pArg->hKeyQuery, 
                                (LPCWSTR)L"Current State",
                                &pArg->dwCurrentState,
                                REG_DWORD );
                    

                    szStringArray[0] = pArg->szQueryName;
                    szStringArray[1] = pArg->szLogFileName;
                    ReportEvent (hEventLog,
                        EVENTLOG_INFORMATION_TYPE,
                        0,
                        SMLOG_LOGGING_QUERY,
                        NULL,
                        2,
                        0,
                        szStringArray,
                        NULL);
                } else {
                    //
                    // StartTraceFailed 
                    // dwStatus should be ERROR_ALREADY_EXISTS if logger already started or anything else.
                    //
                    if ( ERROR_ALREADY_EXISTS == dwStatus ) {
                        szStringArray[0] = pArg->szQueryName;
                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_TRACE_ALREADY_RUNNING,
                            NULL,
                            1,
                            0,
                            szStringArray,      
                            NULL);
                    } else {
                        szStringArray[0] = pArg->szQueryName;
                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_UNABLE_START_TRACE,
                            NULL,
                            1,
                            sizeof(DWORD),
                            szStringArray,      
                            (LPVOID)&dwStatus );
                    }
            
                    bRun = FALSE;
                }
            } else {
                //
                // This means that QueryTrace returned Error Success.
                // The specified logger is already running. 
                //
                szStringArray[0] = pArg->szQueryName;
            
                ReportEvent (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_TRACE_ALREADY_RUNNING,
                    NULL,
                    1,
                    0,
                    szStringArray,      
                    NULL);

                bRun = FALSE;
            }

            if ( TRUE == bRun ) {
                //    
                // Trace logger is now running.
                //
                // Exit when:  
                //  Wait times out,
                //  Exit event signaled, or
                //  Reconfig event signaled.                
                //
                // -1 wait time signals no limit.
                //
                // Loop wait intervals, calculating interval before each wait.
                //
                while ( ((LONGLONG)(0)) != llSessionWaitTics ) {

                    //
                    // Calculate wait interval.
                    //
                    if ( INFINITE_TICS == llNewFileWaitTics 
                            || ( INFINITE_TICS != llSessionWaitTics
                                    && llNewFileWaitTics > llSessionWaitTics ) ) {
                        //
                        // No need to create new file within session.
                        //
                        if ( INFINITE_TICS == llSessionWaitTics ) {
                            liWaitTics.QuadPart = llSessionWaitTics;
                            //
                            // Exit after first loop.
                            //
                            llSessionWaitTics = 0;
                        } else {
                            liWaitTics.QuadPart = llSessionWaitTics;
                            //
                            // Exit after first loop
                            //
                            llSessionWaitTics = 0;
                        }
                    } else {
                        //
                        // Create new file before session ends.
                        //
                        liWaitTics.QuadPart = llNewFileWaitTics;

                        if ( INFINITE_TICS != llSessionWaitTics ) {
                            llSessionWaitTics -= llNewFileWaitTics;
                            
                            // todo cnf:  The following should be logically impossible,
                            // because session > newfile wait.
                            if ( 0 > llSessionWaitTics ) {
                                llSessionWaitTics = 0;
                            }
                        }
                    }
                    //
                    // NtWaitForMultipleObjects requires negative Tic value.
                    //
                    if ( INFINITE_TICS != liWaitTics.QuadPart ) {
                        liWaitTics.QuadPart = ((LONGLONG)(0)) - liWaitTics.QuadPart;
                    }
            
                    if ( STATUS_TIMEOUT != NtWaitForMultipleObjects ( 
                                            2, 
                                            arrEventHandle,
                                            WaitAny,
                                            FALSE, 
                                            ( INFINITE_TICS != liWaitTics.QuadPart ) ? &liWaitTics : NULL )) 
                    {
                        bRun = FALSE;
                        break;
                    } else {
                        //
                        // If cnf by time, llNewFileWaitTics will not be infinite.
                        //
                        if ( INFINITE_TICS != llNewFileWaitTics 
                            && ((LONGLONG)(0)) != llSessionWaitTics ) {
                            //
                            // Time to create a new file. Don't update the autoformat
                            // serial number.  Use the initial autoformat serial number
                            //
                            InitTraceProperties ( pArg, FALSE, &dwSessionSerial, &iCnfSerial );
                            dwStatus = UpdateTrace(
                                        pArg->LoggerHandle, 
                                        pArg->szLoggerName, 
                                        &pArg->Properties );
                            // Todo cnf report event on bad status.
                        }
                    }
                }
            }

            if (bStarted == TRUE) {
                if (bRun) {
                    pArg->bExitOnTermination = FALSE;
                }
                    
                if (pArg->bCallCloseTraceLogger) {
                    //
                    // Stop the query.
                    //
                    CloseTraceLogger ( pArg );
                }
                else {
                    pArg->bCallCloseTraceLogger = TRUE;
                }
            }
                
            if ( pArg->bLoadNewConfig )
                break;

            if ( pArg->szCmdFileName != NULL )
                DoLogCommandFile (pArg, pArg->szLogFileName, bRun);

            //
            // If restart not enabled, then exit.
            //
ProcessTraceRepeat:
            if ( bRun ) {
                bRun = ProcessRepeatOption ( pArg, &liStartDelayTics );
            }

        } // end while (bRun)

        SetLastError ( ERROR_SUCCESS );
   
    } __except ( EXCEPTION_EXECUTE_HANDLER ) {

        CloseTraceLogger ( pArg );
        bRun = FALSE;
        SetLastError ( SMLOG_THREAD_FAILED );
    }

    return bRun;
}


DWORD
LoggingThreadProc (
    IN    LPVOID    lpThreadArg
)
{
    PLOG_QUERY_DATA     pThreadData = (PLOG_QUERY_DATA)lpThreadArg;
    DWORD               dwStatus = ERROR_SUCCESS;
    HRESULT             hr = NOERROR;
    BOOL                bContinue = TRUE;
    LPWSTR              szStringArray[2];

    if (pThreadData != NULL) {

        __try {

            hr = PdhiPlaRunAs( pThreadData->szQueryName, NULL, &pThreadData->hUserToken );

            if( ERROR_SUCCESS != hr ){
                szStringArray[0] = pThreadData->szQueryName;
                ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_INVALID_CREDENTIALS,
                        NULL,
                        1,
                        sizeof(HRESULT),
                        szStringArray,
                        (LPVOID)&hr
                    );

                return hr;
            }

            do {
                if (pThreadData->dwLogType == SLQ_ALERT) {
                    bContinue = AlertProc (pThreadData);
                } else if (pThreadData->dwLogType == SLQ_COUNTER_LOG) {
                    bContinue = CounterLogProc (pThreadData);
                } else if (pThreadData->dwLogType == SLQ_TRACE_LOG) {
                    bContinue = TraceLogProc (pThreadData);
                } else {
                    //
                    // Incorrect log type for this function.
                    //
                    assert (FALSE); 
                }
                //
                // Determine if this thread was paused for reloading
                // or stopped to terminate
                //
                if (pThreadData->bLoadNewConfig) {
                    //
                    // Reset the reconfig flag and event.
                    //
                    bContinue = TRUE;
                    pThreadData->bLoadNewConfig = FALSE;
					ResetEvent ( pThreadData->hReconfigEvent );
                } // else  bContinue is always returned as FALSE
                  // so that will terminate this loop
            } while (bContinue);
            
            dwStatus = GetLastError();

        } __except ( EXCEPTION_EXECUTE_HANDLER ) {
            dwStatus = SMLOG_THREAD_FAILED;
        }

    } else {
        //
        // Unable to find data block so return.
        //
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    if ( ERROR_SUCCESS != dwStatus ) {       
        szStringArray[0] = pThreadData->szQueryName;
        ReportEvent (
            hEventLog,
            EVENTLOG_WARNING_TYPE,
            0,
            dwStatus,
            NULL,
            1,
            0,
            szStringArray,
            NULL
        );
    }
        
    return dwStatus;
}

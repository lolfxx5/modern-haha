/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    logsup.c

Abstract:

    WMI logger api set. The routines here will need to appear like they
    are system calls. They are necessary to do the necessary error checking
    and do most of the legwork that can be done outside the kernel. The
    kernel portion will subsequently only deal with the actual logging
    and tracing.

Author:

    28-May-1997 JeePang

Revision History:

--*/

#ifndef MEMPHIS
#include <nt.h>
#include <ntrtl.h>          // for ntutrl.h
#include <nturtl.h>         // for RTL_CRITICAL_SECTION in winbase.h/wtypes.h
#include <wtypes.h>         // for LPGUID in wmium.h
#include "wmiump.h"
#include "evntrace.h"
#include "traceump.h"
#include "tracelib.h"
#include <math.h>
#include "trcapi.h"
#include "NtdllTrc.h"
#include <strsafe.h>
#include <ntperf.h>

ULONG KernelWow64 = FALSE;

#define CPU_ROOT \
    L"\\Registry\\Machine\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor"

#define MHZ_VALUE_NAME \
    L"~MHz"

typedef ULONG (WMIAPI HWCONFIG)(PWMI_LOGGER_CONTEXT);
typedef HWCONFIG * PHWCONFIG ;

HWCONFIG  EtwpDumpHWConfig;

//
// Initially in ntdll EtwpDumpHardwareConfig points
// to Dummy function EtwpDumpHWConfig
//

PHWCONFIG EtwpDumpHardwareConfig = EtwpDumpHWConfig; 

NTSTATUS
EtwpRegQueryValueKey(
    IN HANDLE KeyHandle,
    IN LPWSTR lpValueName,
    IN ULONG  Length,
    OUT PVOID KeyValue,
    OUT PULONG ResultLength
    );

NTSTATUS
EtwpProcessRunDown(
    IN PWMI_LOGGER_CONTEXT Logger,
    IN ULONG StartFlag,
    IN ULONG fEnableFlags
    );

NTSTATUS
EtwpThreadRunDown(
    IN PWMI_LOGGER_CONTEXT Logger,
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo,
    IN ULONG StartFlag,
    IN BOOLEAN bExtended
    );

extern
NTSTATUS
DumpHeapSnapShot(
        IN PWMI_LOGGER_CONTEXT Logger
        );

ULONG
EtwpDumpHWConfig(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    return STATUS_SUCCESS;
}

//
// This function is called when advapi32.dll loads
// into process memory and gives ntdll the pointer 
// of function in advapi32 which traces the infor-
// mation of System configuration
//
 
void EtwpSetHWConfigFunction(PHWCONFIG DumpHardwareConfig, ULONG Reason)
{
    if (Reason == DLL_PROCESS_ATTACH)       
    {
        //
        // On DLL Laod Get the pointer in Advapi32.dll
        //

        EtwpDumpHardwareConfig = DumpHardwareConfig;

    } else {

        //
        // On DLL unload point it back to Dummy function.
        //

        EtwpDumpHardwareConfig = EtwpDumpHWConfig;
    }
}

__inline __int64 EtwpGetSystemTime()
{
    LARGE_INTEGER SystemTime;

    //
    // Read system time from shared region.
    //

    do {
        SystemTime.HighPart = USER_SHARED_DATA->SystemTime.High1Time;
        SystemTime.LowPart = USER_SHARED_DATA->SystemTime.LowPart;
    } while (SystemTime.HighPart != USER_SHARED_DATA->SystemTime.High2Time);

    return SystemTime.QuadPart;
}

ULONG WmiTraceAlignment = DEFAULT_TRACE_ALIGNMENT;

ULONG
EtwpStartLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
/*++

Routine Description:

    This is the actual routine to communicate with the kernel to start
    the logger. All the required parameters must be in LoggerInfo.

Arguments:

    LoggerInfo      The actual parameters to be passed to and return from
                    kernel.

Return Value:

    The status of performing the action requested.

--*/
{
    ULONG Status;
    ULONG BufferSize;
    LPGUID Guid;
    PVOID SavedChecksum;
    ULONG SavedLogFileMode;
    BOOLEAN IsKernelTrace = FALSE;
    BOOLEAN bLogFile = FALSE;
    BOOLEAN bRealTime = FALSE;
    WMI_REF_CLOCK RefClock;
    LARGE_INTEGER RefClockSys, RefClockPerf, RefClockCycle;
    LARGE_INTEGER Frequency;

    Guid = &LoggerInfo->Wnode.Guid;

    if( IsEqualGUID(&HeapGuid,Guid) 
        || IsEqualGUID(&CritSecGuid,Guid)
        ){

        WMINTDLLLOGGERINFO NtdllLoggerInfo;

        NtdllLoggerInfo.LoggerInfo = LoggerInfo;
        RtlCopyMemory(&LoggerInfo->Wnode.Guid, &NtdllTraceGuid, sizeof(GUID));
        NtdllLoggerInfo.IsGet = FALSE;


        Status =  EtwpSendWmiKMRequest(
                        NULL,
                        IOCTL_WMI_NTDLL_LOGGERINFO,
                        &NtdllLoggerInfo,
                        sizeof(WMINTDLLLOGGERINFO),
                        &NtdllLoggerInfo,
                        sizeof(WMINTDLLLOGGERINFO),
                        &BufferSize,
                        NULL
                        );

        return EtwpSetDosError(Status);
    }

    if (IsEqualGUID(Guid, &SystemTraceControlGuid) ||
        IsEqualGUID(Guid, &WmiEventLoggerGuid)) {
        IsKernelTrace = TRUE;
    }
    if ((LoggerInfo->LogFileName.Length > 0) &&
        (LoggerInfo->LogFileName.Buffer != NULL)) {
        bLogFile = TRUE;
    }
    SavedLogFileMode = LoggerInfo->LogFileMode;
    if (SavedLogFileMode & EVENT_TRACE_REAL_TIME_MODE) {
        bRealTime = TRUE;
    }

    //
    // If the user didn't specify the clock type, set the default clock type
    // system time.
    //

    if (LoggerInfo->Wnode.ClientContext != EVENT_TRACE_CLOCK_PERFCOUNTER &&
        LoggerInfo->Wnode.ClientContext != EVENT_TRACE_CLOCK_SYSTEMTIME &&
        LoggerInfo->Wnode.ClientContext != EVENT_TRACE_CLOCK_CPUCYCLE) {
        LoggerInfo->Wnode.ClientContext = EVENT_TRACE_CLOCK_SYSTEMTIME;
    }

    //
    // Take a reference timestamp before actually starting the logger. This is
    // due to the fact that the Kernel logger can pump events with timestamps
    // earier than the LogFileHeader Timestamp. As a result we take the 
    // reference timestamps prior to starting anything. 
    //
    RefClockSys.QuadPart = EtwpGetSystemTime();
    RefClockCycle.QuadPart = EtwpGetCycleCount();
    Status = NtQueryPerformanceCounter(&RefClockPerf, &Frequency);

    if (SavedLogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
        Status = EtwpSendUmLogRequest(
                    WmiStartLoggerCode,
                    LoggerInfo
                    );
    }
    else if (IsKernelTrace) {
        //
        // In order to capture the process/thread rundown accurately, we need to
        // start kernel logger in two steps. Start logger with delay write,
        // do rundown from user mode and then updatelogger with filename.
        //
        WMI_LOGGER_INFORMATION DelayLoggerInfo;
        ULONG EnableFlags = LoggerInfo->EnableFlags;
        //
        // If it's only realtime start logger in one step
        //

        if (bRealTime && !bLogFile) {

            Status =  EtwpSendWmiKMRequest(
                        NULL,
                        IOCTL_WMI_START_LOGGER,
                        LoggerInfo,
                        LoggerInfo->Wnode.BufferSize,
                        LoggerInfo,
                        LoggerInfo->Wnode.BufferSize,
                        &BufferSize,
                        NULL
                        );
            return EtwpSetDosError(Status);
        }

        if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
            PTRACE_ENABLE_FLAG_EXTENSION tFlagExt;

            tFlagExt = (PTRACE_ENABLE_FLAG_EXTENSION)
                       &LoggerInfo->EnableFlags;
            EnableFlags = *(PULONG)((PCHAR)LoggerInfo + tFlagExt->Offset);
        }


        RtlCopyMemory(&DelayLoggerInfo, 
                       LoggerInfo, 
                       sizeof(WMI_LOGGER_INFORMATION));

        RtlZeroMemory(&DelayLoggerInfo.LogFileName, sizeof(UNICODE_STRING) );

        DelayLoggerInfo.Wnode.BufferSize = sizeof(WMI_LOGGER_INFORMATION);

        DelayLoggerInfo.LogFileMode |= EVENT_TRACE_DELAY_OPEN_FILE_MODE;

        //
        // Since there's no filename in step 1 of StartLogger we need to mask
        // the NEWFILE mode to prevent kernel trying to generate a file
        //
        DelayLoggerInfo.LogFileMode &= ~EVENT_TRACE_FILE_MODE_NEWFILE;

        DelayLoggerInfo.EnableFlags = (EVENT_TRACE_FLAG_PROCESS & EnableFlags);
        DelayLoggerInfo.EnableFlags |= (EVENT_TRACE_FLAG_THREAD & EnableFlags);
        DelayLoggerInfo.EnableFlags |= 
                                    (EVENT_TRACE_FLAG_IMAGE_LOAD & EnableFlags);

        Status = EtwpSendWmiKMRequest(
                    NULL,
                    IOCTL_WMI_START_LOGGER,
                    &DelayLoggerInfo,
                    DelayLoggerInfo.Wnode.BufferSize,
                    &DelayLoggerInfo,
                    DelayLoggerInfo.Wnode.BufferSize,
                    &BufferSize,
                    NULL
                    );
        if (Status != ERROR_SUCCESS) {
            return Status;
        }

        LoggerInfo->Wnode.ClientContext = DelayLoggerInfo.Wnode.ClientContext;

        //
        // We need to pick up any parameter adjustment done by the kernel
        // here so UpdateTrace does not fail.
        //
        LoggerInfo->Wnode.HistoricalContext = 
                                        DelayLoggerInfo.Wnode.HistoricalContext;
        LoggerInfo->MinimumBuffers          = DelayLoggerInfo.MinimumBuffers;
        LoggerInfo->MaximumBuffers          = DelayLoggerInfo.MaximumBuffers;
        LoggerInfo->NumberOfBuffers         = DelayLoggerInfo.NumberOfBuffers;
        LoggerInfo->BufferSize              = DelayLoggerInfo.BufferSize;
        LoggerInfo->AgeLimit                = DelayLoggerInfo.AgeLimit;

        BufferSize = LoggerInfo->BufferSize * 1024;

        //
        //  Add the LogHeader
        //
        LoggerInfo->Checksum = NULL;
        if (LoggerInfo->Wnode.ClientContext == EVENT_TRACE_CLOCK_PERFCOUNTER) {
            RefClock.StartPerfClock = RefClockPerf;
        } else if (LoggerInfo->Wnode.ClientContext ==
                   EVENT_TRACE_CLOCK_CPUCYCLE) {
            RefClock.StartPerfClock= RefClockCycle;
        } else {
            RefClock.StartPerfClock = RefClockSys;
        }
        RefClock.StartTime = RefClockSys;

        Status = EtwpAddLogHeaderToLogFile(LoggerInfo, &RefClock, FALSE);


        if (Status == ERROR_SUCCESS) {
            SavedChecksum = LoggerInfo->Checksum;
            //
            // Update the logger with the filename
            //
            Status = EtwpSendWmiKMRequest(
                        NULL,
                        IOCTL_WMI_UPDATE_LOGGER,
                        LoggerInfo,
                        LoggerInfo->Wnode.BufferSize,
                        LoggerInfo,
                        LoggerInfo->Wnode.BufferSize,
                        &BufferSize,
                        NULL
                        );

            if (SavedChecksum != NULL) {
                EtwpFree(SavedChecksum);
                SavedChecksum = NULL;
            }
        }


        if (Status != ERROR_SUCCESS) {
            ULONG lStatus;

            //
            // Logger must be stopped now
            //
            lStatus = EtwpSendWmiKMRequest(
                    NULL,
                    IOCTL_WMI_STOP_LOGGER,
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    &BufferSize,
                    NULL
                    );

            LoggerInfo->LogFileMode = SavedLogFileMode;
            return EtwpSetDosError(Status);
        }
    }
    else {

        LoggerInfo->Checksum = NULL;
        // 
        // Query for supported clock types.  If an unsupported clock type
        // is specified this LoggerInfo will contain the kernel's default
        //
        Status = EtwpSendWmiKMRequest(NULL,
                                      IOCTL_WMI_CLOCK_TYPE,
                                      LoggerInfo,
                                      LoggerInfo->Wnode.BufferSize,
                                      LoggerInfo,
                                      LoggerInfo->Wnode.BufferSize,
                                      &BufferSize,
                                      NULL
                                    );

        if (Status != ERROR_SUCCESS) {
            return EtwpSetDosError(Status);
        }
        if (LoggerInfo->Wnode.ClientContext == EVENT_TRACE_CLOCK_PERFCOUNTER) {
            RefClock.StartPerfClock = RefClockPerf;
        } else if (LoggerInfo->Wnode.ClientContext == EVENT_TRACE_CLOCK_CPUCYCLE) {
            RefClock.StartPerfClock= RefClockCycle;
        } else {
            RefClock.StartPerfClock = RefClockSys;
        }
        RefClock.StartTime = RefClockSys;

        Status = EtwpAddLogHeaderToLogFile(LoggerInfo, &RefClock, FALSE);
        if (Status != ERROR_SUCCESS) {
            return EtwpSetDosError(Status);
        }

        //
        // At this point we have an open handle to the logfile and memory 
        // allocation? 
        //

        BufferSize = LoggerInfo->BufferSize * 1024;
        SavedChecksum = LoggerInfo->Checksum;

       // actually start the logger here
        Status = EtwpSendWmiKMRequest(
                            NULL,
                    IOCTL_WMI_START_LOGGER,
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    &BufferSize,
                            NULL
                    );

        // Close the handle if it's not NULL
        if (LoggerInfo->LogFileHandle != NULL) {
            NtClose(LoggerInfo->LogFileHandle);
            LoggerInfo->LogFileHandle = NULL;
        }
        //
        // If the Start call failed, we will delete the logfile except 
        // when we are appending to an older file. However, we do not 
        // fixup the header!
        //

        if ( (Status != ERROR_MORE_DATA) &&
                  !(LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_APPEND)) {
            if (LoggerInfo->LogFileName.Buffer != NULL) {
                EtwpDeleteFileW(LoggerInfo->LogFileName.Buffer);
            }
        }
        if (SavedChecksum != NULL) {
            EtwpFree(SavedChecksum);
        }
    }
    //
    // Restore the LogFileMode
    //
    LoggerInfo->LogFileMode = SavedLogFileMode;

    return EtwpSetDosError(Status);
}


ULONG
EtwpFinalizeLogFileHeader(
    IN PWMI_LOGGER_INFORMATION LoggerInfo
    )
{
    ULONG                     Status    = ERROR_SUCCESS;
    ULONG                     ErrorCode = ERROR_SUCCESS;
    HANDLE                    LogFile   = INVALID_HANDLE_VALUE;
    LARGE_INTEGER             CurrentTime;
    WMI_LOGGER_CONTEXT        Logger;
    IO_STATUS_BLOCK           IoStatus;
    FILE_POSITION_INFORMATION FileInfo;
    FILE_STANDARD_INFORMATION FileSize;
    PWMI_BUFFER_HEADER        Buffer;  // need to initialize buffer first
    SYSTEM_BASIC_INFORMATION  SystemInfo;
    ULONG                     EnableFlags;
    ULONG                     IsKernelTrace = FALSE;
    ULONG                     IsGlobalForKernel = FALSE;
    USHORT                    LoggerId = 0;

    RtlZeroMemory(&Logger, sizeof(WMI_LOGGER_CONTEXT));
    Logger.BufferSpace = NULL;

    IsKernelTrace = IsEqualGUID(&LoggerInfo->Wnode.Guid, 
                                &SystemTraceControlGuid);


    if (LoggerInfo->LogFileName.Length > 0 ) {
        // open the file for writing synchronously for the logger
        //    others may want to read it as well.
        //
        LogFile = EtwpCreateFileW(
                   (LPWSTR)LoggerInfo->LogFileName.Buffer,
                   GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_READ,
                   NULL,
                   OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL,
                   NULL
                   );
        if (LogFile == INVALID_HANDLE_VALUE) {
            ErrorCode = EtwpGetLastError();
            goto cleanup;
        }

        // Truncate the file size if in PREALLOCATE mode
        if (LoggerInfo->MaximumFileSize && 
            (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_PREALLOCATE)) {
            IO_STATUS_BLOCK IoStatusBlock;
            FILE_END_OF_FILE_INFORMATION EOFInfo;
            // Do this only when we haven't reach the max file size
            if (!(LoggerInfo->LogFileMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE)) {

                if (LoggerInfo->MaximumFileSize > 
                              (((ULONGLONG)LoggerInfo->BuffersWritten * 
                              (ULONGLONG)LoggerInfo->BufferSize) / 
                              1024)) {

                    EOFInfo.EndOfFile.QuadPart = 
                                    (ULONGLONG)LoggerInfo->BuffersWritten * 
                                    (ULONGLONG)LoggerInfo->BufferSize * 
                                    1024;


                    Status = NtSetInformationFile(LogFile,
                                          &IoStatusBlock,
                                          &EOFInfo,
                                          sizeof(FILE_END_OF_FILE_INFORMATION),
                                          FileEndOfFileInformation
                                        );
                    if (!NT_SUCCESS(Status)) {
                        NtClose(LogFile);
                        ErrorCode = EtwpNtStatusToDosError(Status);
                        goto cleanup;
                    }
                }
            }
            else { // using KBytes for file size unit
                if (LoggerInfo->MaximumFileSize > 
                              ((ULONGLONG)LoggerInfo->BuffersWritten * 
                              (ULONGLONG)LoggerInfo->BufferSize)) { // 

                    EOFInfo.EndOfFile.QuadPart = 
                                    (ULONGLONG)LoggerInfo->BuffersWritten * 
                                    (ULONGLONG)LoggerInfo->BufferSize * 
                                    1024;

                    Status = NtSetInformationFile(
                                    LogFile,
                                    &IoStatusBlock,
                                    &EOFInfo,
                                    sizeof(FILE_END_OF_FILE_INFORMATION),
                                    FileEndOfFileInformation
                                   );
                    if (!NT_SUCCESS(Status)) {
                        NtClose(LogFile);
                        ErrorCode = EtwpNtStatusToDosError(Status);
                        goto cleanup;
                    }
                }
            }
        }

        Logger.BuffersWritten = LoggerInfo->BuffersWritten;

        Logger.BufferSpace = EtwpAlloc(LoggerInfo->BufferSize * 1024);
        if (Logger.BufferSpace == NULL) {
            ErrorCode = EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
            goto cleanup;
        }
        Buffer = (PWMI_BUFFER_HEADER) Logger.BufferSpace;
        RtlZeroMemory(Buffer, LoggerInfo->BufferSize * 1024);
        Buffer->Wnode.BufferSize = LoggerInfo->BufferSize * 1024;
        Buffer->ClientContext.Alignment = (UCHAR)WmiTraceAlignment;
        Buffer->Offset = sizeof(WMI_BUFFER_HEADER);
        Buffer->Wnode.Guid = LoggerInfo->Wnode.Guid;
        Status = NtQuerySystemInformation(
                    SystemBasicInformation,
                    &SystemInfo, sizeof (SystemInfo), NULL);

        if (!NT_SUCCESS(Status)) {
            ErrorCode = EtwpNtStatusToDosError(Status);
            goto cleanup;
        }
        Logger.TimerResolution = SystemInfo.TimerResolution;
        Logger.LogFileHandle = LogFile;
        Logger.BufferSize = LoggerInfo->BufferSize * 1024;

        // For Circular LogFile the process rundown data is appended at the
        // last buffer written and not to the end of file.
        //
        Status = NtQueryInformationFile(
                    LogFile,
                    &IoStatus,
                    &FileSize,
                    sizeof(FILE_STANDARD_INFORMATION),
                    FileStandardInformation
                        );
        if (!NT_SUCCESS(Status)) {
            ErrorCode = EtwpNtStatusToDosError(Status);
            goto cleanup;
        }

        //
        // For Kernel Boot Traces, we need to do the Rundown. 
        // configuration at this time. 
        // 1. The Logger ID is GLOBAL_LOGGER_ID
        // 2. The LoggerName is NT_KERNEL_LOGGER
        //
        // The First condition is true for any GlobalLogger but 
        // condition 2 is TRUE only when it is collecting kernel traces. 
        //

        LoggerId = WmiGetLoggerId (LoggerInfo->Wnode.HistoricalContext);

        if ( (LoggerId == WMI_GLOBAL_LOGGER_ID)      &&
             (LoggerInfo->LoggerName.Length > 0)     && 
             (LoggerInfo->LoggerName.Buffer != NULL) &&
             (!wcscmp(LoggerInfo->LoggerName.Buffer, KERNEL_LOGGER_NAMEW))
           ) {
            IsGlobalForKernel = TRUE;
        }

        if (  IsKernelTrace || IsGlobalForKernel )  {
            if (IsGlobalForKernel) {
                ULONG      CpuSpeed;
                ULONG      CpuNum = 0;
                
                //
                // For boot traces we need to re-set the CPU Speed in the
                // log file header as it is not available in the registry 
                // when the log file header is first created.
                //
                if (NT_SUCCESS(EtwpGetCpuSpeed(&CpuNum, &CpuSpeed))) {          
                    FileInfo.CurrentByteOffset.QuadPart =
                        LOGFILE_FIELD_OFFSET(CpuSpeedInMHz);
                    
                    Status = NtSetInformationFile(
                        LogFile,
                        &IoStatus,
                        &FileInfo,
                        sizeof(FILE_POSITION_INFORMATION),
                        FilePositionInformation
                        );
                    if (!NT_SUCCESS(Status)) {
                        ErrorCode = EtwpNtStatusToDosError(Status);
                        goto cleanup;
                    }
                    
                    Status = NtWriteFile(
                        LogFile,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        &CpuSpeed,
                        sizeof(CpuSpeed),
                        NULL,
                        NULL
                        );
                    
                   if (NT_SUCCESS(Status)) {
                        NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);
                    }                
                }
            }

            if (sizeof(PVOID) != 8) {
                // For kernel trace, the pointer size is always 64 on ia64, 
                // whether or not under Wow64. Get Wow64 information and set 
                // the flag so that ProcessRunDown can dajust pointer size.
                ULONG_PTR ulp;
                Status = NtQueryInformationProcess(
                            NtCurrentProcess(),
                            ProcessWow64Information,
                            &ulp,
                            sizeof(ULONG_PTR),
                            NULL);
                if (NT_SUCCESS(Status) && (ulp != 0)) {
                    KernelWow64 = TRUE;
                }
            }

            if (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) {

                ULONG BufferSize = LoggerInfo->BufferSize;  // in KB
                ULONG BuffersWritten = LoggerInfo->BuffersWritten;
                ULONG maxBuffers = (LoggerInfo->MaximumFileSize * 
                                    1024) / 
                                    BufferSize;
                ULONG LastBuffer;
                ULONG StartBuffers;

                FileInfo.CurrentByteOffset.QuadPart =
                                         LOGFILE_FIELD_OFFSET(StartBuffers);
                Status = NtSetInformationFile(
                                     LogFile,
                                     &IoStatus,
                                     &FileInfo,
                                     sizeof(FILE_POSITION_INFORMATION),
                                     FilePositionInformation
                                     );
                if (!NT_SUCCESS(Status)) {
                    ErrorCode = EtwpNtStatusToDosError(Status);
                    goto cleanup;
                }

                Status = NtReadFile(
                            LogFile,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatus,
                            &StartBuffers,
                            sizeof(ULONG),
                            NULL,
                            NULL
                            );
                if (!NT_SUCCESS(Status)) {
                    ErrorCode = EtwpNtStatusToDosError(Status);
                    goto cleanup;
                }

                LastBuffer = (maxBuffers > StartBuffers) ?
                             (StartBuffers + (BuffersWritten - StartBuffers)
                             % (maxBuffers - StartBuffers))
                             : 0;
                FileInfo.CurrentByteOffset.QuadPart =  LastBuffer *
                                                       BufferSize * 1024;
            }
            else {
                FileInfo.CurrentByteOffset = FileSize.EndOfFile;
            }


            Status = NtSetInformationFile(
                         LogFile,
                         &IoStatus,
                         &FileInfo,
                         sizeof(FILE_POSITION_INFORMATION),
                         FilePositionInformation
                         );
            if (!NT_SUCCESS(Status)) {
                ErrorCode = EtwpNtStatusToDosError(Status);
                goto cleanup;
            }

            EnableFlags = LoggerInfo->EnableFlags;

            if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
                PTRACE_ENABLE_FLAG_EXTENSION tFlagExt;

                tFlagExt = (PTRACE_ENABLE_FLAG_EXTENSION)
                           &LoggerInfo->EnableFlags;

                if (LoggerInfo->Wnode.BufferSize >= (tFlagExt->Offset + sizeof(ULONG)) )  {
                    EnableFlags = *(PULONG)((PCHAR)LoggerInfo + tFlagExt->Offset);
                }
                else {
                    EnableFlags = 0;    // Should not happen.
                }
            }

            Logger.UsePerfClock = LoggerInfo->Wnode.ClientContext;

            EtwpProcessRunDown(&Logger, FALSE, EnableFlags);

            if (IsGlobalForKernel) {
                EtwpDumpHardwareConfig(&Logger);
            }

            {
                PWMI_BUFFER_HEADER Buffer1 =
                                (PWMI_BUFFER_HEADER) Logger.BufferSpace;
                    if (Buffer1->Offset < Logger.BufferSize) {
                        RtlFillMemory(
                                (char *) Logger.BufferSpace + Buffer1->Offset,
                                Logger.BufferSize - Buffer1->Offset,
                                0xFF);
                    }
            }
            Status = NtWriteFile(
                        LogFile,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        Logger.BufferSpace,
                        Logger.BufferSize,
                        NULL,
                        NULL);
            if (NT_SUCCESS(Status)) {
                NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);
                Logger.BuffersWritten++;
            }
        }


        // Update the EndTime stamp field in LogFile. No Need to 
        // to do it if it's Relogged File. The old logfile
        // header already has the correct value. 
        //
        if ( !(LoggerInfo->LogFileMode & EVENT_TRACE_RELOG_MODE) ) {
            FileInfo.CurrentByteOffset.QuadPart =
                                    LOGFILE_FIELD_OFFSET(EndTime);
            Status = NtSetInformationFile(
                         LogFile,
                         &IoStatus,
                         &FileInfo,
                         sizeof(FILE_POSITION_INFORMATION),
                         FilePositionInformation
                         );
            if (!NT_SUCCESS(Status)) {
                ErrorCode = EtwpNtStatusToDosError(Status);
                goto cleanup;
            }

            // End Time is always wallclock time.
            //
            CurrentTime.QuadPart = EtwpGetSystemTime();
            Status = NtWriteFile(
                        LogFile,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        &CurrentTime,
                        sizeof(ULONGLONG),
                        NULL,
                        NULL
                        );
            if (NT_SUCCESS(Status)) {
                NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);
            }
        }

        //
        // Update the Number of Buffers Written field in the header
        //
        FileInfo.CurrentByteOffset.QuadPart =
                            LOGFILE_FIELD_OFFSET(BuffersWritten);
        Status = NtSetInformationFile(
                     LogFile,
                     &IoStatus,
                     &FileInfo,
                     sizeof(FILE_POSITION_INFORMATION),
                     FilePositionInformation
                     );
        if (!NT_SUCCESS(Status)) {
            ErrorCode = EtwpNtStatusToDosError(Status);
            goto cleanup;
        }

        Status = NtWriteFile(
                    LogFile,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    &Logger.BuffersWritten,
                    sizeof(ULONG),
                    NULL,
                    NULL
                    );
        if (NT_SUCCESS(Status)) {
            NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);
        }

        ErrorCode = RtlNtStatusToDosError(Status);
        LoggerInfo->BuffersWritten = Logger.BuffersWritten;

        if ( !(LoggerInfo->LogFileMode & EVENT_TRACE_RELOG_MODE) ) {
            //
            // Write the BuffersLost information into the logfile
            // We need to be careful for WOW cases because BuffersLost
            // come after pointers in log file header.
            //

            if (KernelWow64) { // KernelWow64
                FileInfo.CurrentByteOffset.QuadPart =
                                    LOGFILE_FIELD_OFFSET(BuffersLost) + 8;
            }
            else if ( LoggerInfo->Wow && 8 == sizeof(PVOID) &&
                    !(IsKernelTrace ||  IsGlobalForKernel) ) { 
                // We're stopping a non-kernel 32 bit logger in 64 bit mode.
                // The log file header in the file is 32 bit so we need to
                // adjust the field offset.
                FileInfo.CurrentByteOffset.QuadPart =
                                    LOGFILE_FIELD_OFFSET(BuffersLost) - 8;
            }
            else if ( !(LoggerInfo->Wow) && 4 == sizeof(PVOID) &&
                    !(IsKernelTrace || IsGlobalForKernel) ) { 
                // We're stopping a non-kernel logger in 32 bit mode.
                // If this is running on IA64, the log file header in the file is 
                // 64 bit so we need to adjust the field offset.
                ULONG_PTR ulp;
                Status = NtQueryInformationProcess(
                            NtCurrentProcess(),
                            ProcessWow64Information,
                            &ulp,
                            sizeof(ULONG_PTR),
                            NULL);
                if (NT_SUCCESS(Status) && (ulp != 0)) { // Current process is WOW (on IA64)
                    FileInfo.CurrentByteOffset.QuadPart =
                                        LOGFILE_FIELD_OFFSET(BuffersLost) + 8;
                }
                else { // normal x86 case
                    FileInfo.CurrentByteOffset.QuadPart =
                                        LOGFILE_FIELD_OFFSET(BuffersLost);
                }
            }
            else {
                FileInfo.CurrentByteOffset.QuadPart =
                                    LOGFILE_FIELD_OFFSET(BuffersLost);
            }
            Status = NtSetInformationFile(
                         LogFile,
                         &IoStatus,
                         &FileInfo,
                         sizeof(FILE_POSITION_INFORMATION),
                         FilePositionInformation
                         );
            if (!NT_SUCCESS(Status)) {
                ErrorCode = EtwpNtStatusToDosError(Status);
                goto cleanup;
            }

            Status = NtWriteFile(
                        LogFile,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        &LoggerInfo->LogBuffersLost,
                        sizeof(ULONG),
                        NULL,
                        NULL
                        );
            if (NT_SUCCESS(Status)) {
                NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);
            }

            //
            // Write the EventsLost information into the logfile
            //
            FileInfo.CurrentByteOffset.QuadPart =
                                LOGFILE_FIELD_OFFSET(EventsLost);
            Status = NtSetInformationFile(
                         LogFile,
                         &IoStatus,
                         &FileInfo,
                         sizeof(FILE_POSITION_INFORMATION),
                         FilePositionInformation
                         );
            if (!NT_SUCCESS(Status)) {
                ErrorCode = EtwpNtStatusToDosError(Status);
                goto cleanup;
            }

            Status = NtWriteFile(
                        LogFile,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        &LoggerInfo->EventsLost,
                        sizeof(ULONG),
                        NULL,
                        NULL
                        );
            if (NT_SUCCESS(Status)) {
                NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);
            }
        }

    }

cleanup:
    if (LogFile != INVALID_HANDLE_VALUE) {
        NtClose(LogFile);
    }
    if (Logger.BufferSpace != NULL) {
        EtwpFree(Logger.BufferSpace);
    }
    return EtwpSetDosError(ErrorCode);
}

ULONG
EtwpStopLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
/*++

Routine Description:

    This is the actual routine to communicate with the kernel to stop
    the logger. All the properties of the logger will be returned in LoggerInfo.

Arguments:

    LoggerInfo      The actual parameters to be passed to and return from
                    kernel.

Return Value:

    The status of performing the action requested.

--*/
{
    ULONG ErrorCode, ReturnSize;
    PTRACE_ENABLE_CONTEXT pContext;

    //
    //Check For Heap and Crit Sec Guid.
    //

    if( IsEqualGUID(&HeapGuid,&LoggerInfo->Wnode.Guid) 
        || IsEqualGUID(&CritSecGuid,&LoggerInfo->Wnode.Guid)
        ){

        WMINTDLLLOGGERINFO NtdllLoggerInfo;
        ULONG BufferSize;
        
        LoggerInfo->Wnode.BufferSize = 0;
        RtlCopyMemory(&LoggerInfo->Wnode.Guid, &NtdllTraceGuid, sizeof(GUID));

        NtdllLoggerInfo.LoggerInfo = LoggerInfo;
        NtdllLoggerInfo.IsGet = FALSE;


        ErrorCode =  EtwpSendWmiKMRequest(
                        NULL,
                        IOCTL_WMI_NTDLL_LOGGERINFO,
                        &NtdllLoggerInfo,
                        sizeof(WMINTDLLLOGGERINFO),
                        &NtdllLoggerInfo,
                        sizeof(WMINTDLLLOGGERINFO),
                        &BufferSize,
                        NULL
                        );

        return EtwpSetDosError(ErrorCode);
    }

//    pContext = (PTRACE_ENABLE_CONTEXT) & LoggerInfo->Wnode.HistoricalContext;
//    if (   (pContext->InternalFlag != 0)
//        && (pContext->InternalFlag != EVENT_TRACE_INTERNAL_FLAG_PRIVATE)) {
//        // Currently only one possible InternalFlag value. This will filter
//        // out some bogus LoggerHandle
//        //
//        return EtwpSetDosError(ERROR_INVALID_HANDLE);
//    }

    if (LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
        pContext = (PTRACE_ENABLE_CONTEXT) &LoggerInfo->Wnode.HistoricalContext;
        pContext->InternalFlag |= EVENT_TRACE_INTERNAL_FLAG_PRIVATE;
        pContext->LoggerId     = 1;
        ErrorCode = EtwpSendUmLogRequest(WmiStopLoggerCode, LoggerInfo);
    }
    else {


        ErrorCode = EtwpSendWmiKMRequest(
                        NULL,
                        IOCTL_WMI_STOP_LOGGER,
                        LoggerInfo,
                        LoggerInfo->Wnode.BufferSize,
                        LoggerInfo,
                        LoggerInfo->Wnode.BufferSize,
                        &ReturnSize,
                        NULL
                        );
//
// if logging to a file, then update the EndTime, BuffersWritten and do
// process rundown for kernel trace.
//
        if (ErrorCode == ERROR_SUCCESS) {
            ErrorCode = EtwpFinalizeLogFileHeader(LoggerInfo);
        }
    }

    return EtwpSetDosError(ErrorCode);
}


ULONG
EtwpQueryLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo,
    IN ULONG Update
    )
/*++

Routine Description:

    This is the actual routine to communicate with the kernel to query
    the logger. All the properties of the logger will be returned in LoggerInfo.

Arguments:

    LoggerInfo      The actual parameters to be passed to and return from
                    kernel.

Return Value:

    The status of performing the action requested.

--*/
{
    ULONG Status, ReturnSize;
    HANDLE LogFileHandle = NULL;
    PTRACE_ENABLE_CONTEXT pContext;
    BOOLEAN bAddAppendFlag = FALSE;
    ULONG SavedLogFileMode;
    ULONG IsPrivate;

    LoggerInfo->Checksum      = NULL;
    LoggerInfo->LogFileHandle = NULL;
    pContext = (PTRACE_ENABLE_CONTEXT) &LoggerInfo->Wnode.HistoricalContext;

    IsPrivate = (LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE)
                || (pContext->InternalFlag & EVENT_TRACE_INTERNAL_FLAG_PRIVATE);

    //
    // If UPDATE and a new logfile is given throw in the LogFileHeader
    //

    if ( Update && LoggerInfo->LogFileName.Length > 0) {

        if ( ! IsPrivate) {
            Status = EtwpAddLogHeaderToLogFile(LoggerInfo, NULL, Update);
            if (Status  != ERROR_SUCCESS) {
                return EtwpSetDosError(Status);
            }

            LogFileHandle = LoggerInfo->LogFileHandle;
            bAddAppendFlag = TRUE;
            //
            // If we are switching to a new file, make sure it is append mode
            //
            SavedLogFileMode = LoggerInfo->LogFileMode;
        }
    }


    if (IsPrivate) {

        Status = EtwpSendUmLogRequest(
                    (Update) ? (WmiUpdateLoggerCode) : (WmiQueryLoggerCode),
                    LoggerInfo
                    );
    }
    else {
        Status = EtwpSendWmiKMRequest(
                    NULL,
                    (Update ? IOCTL_WMI_UPDATE_LOGGER : IOCTL_WMI_QUERY_LOGGER),
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    &ReturnSize,
                    NULL
                    );

        // Close the handle if it's not NULL
        if (LoggerInfo->LogFileHandle != NULL) {
            NtClose(LoggerInfo->LogFileHandle);
            LoggerInfo->LogFileHandle = NULL;
        }

        if (Update && Status != ERROR_SUCCESS) {
            if (LoggerInfo->LogFileName.Buffer != NULL) {
                EtwpDeleteFileW(LoggerInfo->LogFileName.Buffer);
            }
        }

        if (LoggerInfo->Checksum != NULL) {
            EtwpFree(LoggerInfo->Checksum);
        }
    }
    if (bAddAppendFlag) {
        LoggerInfo->LogFileMode = SavedLogFileMode;
    }
    return EtwpSetDosError(Status);
}

PVOID
EtwpGetTraceBuffer(
    IN PWMI_LOGGER_CONTEXT Logger,
    IN PSYSTEM_THREAD_INFORMATION pThread,
    IN ULONG GroupType,
    IN ULONG RequiredSize
    )
{
    PSYSTEM_TRACE_HEADER Header;
    PWMI_BUFFER_HEADER Buffer;
    THREAD_BASIC_INFORMATION ThreadInfo;
    KERNEL_USER_TIMES ThreadCpu;
    NTSTATUS Status;
    ULONG BytesUsed;
    PCLIENT_ID Cid;

    RequiredSize += sizeof (SYSTEM_TRACE_HEADER);   // add in header

    RequiredSize = (ULONG) ALIGN_TO_POWER2(RequiredSize, WmiTraceAlignment);

    Buffer = (PWMI_BUFFER_HEADER) Logger->BufferSpace;

    if (RequiredSize > Logger->BufferSize - sizeof(WMI_BUFFER_HEADER)) {
        EtwpSetDosError(ERROR_BUFFER_OVERFLOW);
        return NULL;
    }

    if (RequiredSize > (Logger->BufferSize - Buffer->Offset)) {
        IO_STATUS_BLOCK IoStatus;

        if (Buffer->Offset < Logger->BufferSize) {
            RtlFillMemory(
                    (char *) Buffer + Buffer->Offset,
                    Logger->BufferSize - Buffer->Offset,
                    0xFF);
        }
        Buffer->BufferType = WMI_BUFFER_TYPE_RUNDOWN;
        Buffer->BufferFlag = WMI_BUFFER_FLAG_FLUSH_MARKER;

        Status = NtWriteFile(
                    Logger->LogFileHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    Buffer,
                    Logger->BufferSize,
                    NULL,
                    NULL);
        Buffer->Offset = sizeof(WMI_BUFFER_HEADER);
        if (!NT_SUCCESS(Status)) {
            return NULL;
        }
        Logger->BuffersWritten++;
    }
    Header = (PSYSTEM_TRACE_HEADER) ((char*)Buffer + Buffer->Offset);

    if (Logger->UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) {
        LARGE_INTEGER Frequency;
        ULONGLONG Counter = 0;
        Status = NtQueryPerformanceCounter((PLARGE_INTEGER)&Counter,
                                            &Frequency);
        Header->SystemTime.QuadPart = Counter;
    } else if (Logger->UsePerfClock == EVENT_TRACE_CLOCK_CPUCYCLE) {
        Header->SystemTime.QuadPart = EtwpGetCycleCount();
    } else {
        Header->SystemTime.QuadPart = EtwpGetSystemTime();
    }

    Header->Header = (GroupType << 16) + RequiredSize;
    Header->Marker = SYSTEM_TRACE_MARKER;

    if (pThread == NULL) {
        Status = NtQueryInformationThread(
                    NtCurrentThread(),
                    ThreadBasicInformation,
                    &ThreadInfo,
                    sizeof ThreadInfo, NULL);
        if (NT_SUCCESS(Status)) {
            Cid = &ThreadInfo.ClientId;
            Header->ThreadId = HandleToUlong(Cid->UniqueThread);
            Header->ProcessId = HandleToUlong(Cid->UniqueProcess);
        }

        Status = NtQueryInformationThread(
                    NtCurrentThread(),
                    ThreadTimes,
                    &ThreadCpu, sizeof ThreadCpu, NULL);
        if (NT_SUCCESS(Status)) {
            Header->KernelTime = (ULONG) (ThreadCpu.KernelTime.QuadPart
                                      / Logger->TimerResolution);
            Header->UserTime   = (ULONG) (ThreadCpu.UserTime.QuadPart
                                      / Logger->TimerResolution);
        }
    }
    else {
        Cid = &pThread->ClientId;
        Header->ThreadId = HandleToUlong(Cid->UniqueThread);
        Header->ProcessId = HandleToUlong(Cid->UniqueProcess);
        Header->KernelTime = (ULONG) (pThread->KernelTime.QuadPart
                                / Logger->TimerResolution);
        Header->UserTime = (ULONG) (pThread->UserTime.QuadPart
                                / Logger->TimerResolution);
    }

    Buffer->Offset += RequiredSize;
    // If there is room, throw in a end of buffer marker.

    BytesUsed = Buffer->Offset;
    if ( BytesUsed <= (Logger->BufferSize-sizeof(ULONG)) ) {
        *((long*)((char*)Buffer+Buffer->Offset)) = -1;
    }
    return (PVOID) ( (char*) Header + sizeof(SYSTEM_TRACE_HEADER) );
}


VOID
EtwpCopyPropertiesToInfo(
    IN PEVENT_TRACE_PROPERTIES Properties,
    IN PWMI_LOGGER_INFORMATION Info
    )
{
    ULONG SavedBufferSize = Info->Wnode.BufferSize;

    RtlCopyMemory(&Info->Wnode, &Properties->Wnode, sizeof(WNODE_HEADER));

    Info->Wnode.BufferSize = SavedBufferSize;

    Info->BufferSize            = Properties->BufferSize;
    Info->MinimumBuffers        = Properties->MinimumBuffers;
    Info->MaximumBuffers        = Properties->MaximumBuffers;
    Info->NumberOfBuffers       = Properties->NumberOfBuffers;
    Info->FreeBuffers           = Properties->FreeBuffers;
    Info->EventsLost            = Properties->EventsLost;
    Info->BuffersWritten        = Properties->BuffersWritten;
    Info->LoggerThreadId        = Properties->LoggerThreadId;
    Info->MaximumFileSize       = Properties->MaximumFileSize;
    Info->EnableFlags           = Properties->EnableFlags;
    Info->LogFileMode           = Properties->LogFileMode;
    Info->FlushTimer            = Properties->FlushTimer;
    Info->LogBuffersLost        = Properties->LogBuffersLost;
    Info->AgeLimit              = Properties->AgeLimit;
    Info->RealTimeBuffersLost   = Properties->RealTimeBuffersLost;
}

VOID
EtwpCopyInfoToProperties(
    IN PWMI_LOGGER_INFORMATION Info,
    IN PEVENT_TRACE_PROPERTIES Properties
    )
{
    ULONG SavedSize = Properties->Wnode.BufferSize;
    RtlCopyMemory(&Properties->Wnode, &Info->Wnode, sizeof(WNODE_HEADER));
    Properties->Wnode.BufferSize = SavedSize;

    Properties->BufferSize            = Info->BufferSize;
    Properties->MinimumBuffers        = Info->MinimumBuffers;
    Properties->MaximumBuffers        = Info->MaximumBuffers;
    Properties->NumberOfBuffers       = Info->NumberOfBuffers;
    Properties->FreeBuffers           = Info->FreeBuffers;
    Properties->EventsLost            = Info->EventsLost;
    Properties->BuffersWritten        = Info->BuffersWritten;
    Properties->LoggerThreadId        = Info->LoggerThreadId;
    Properties->MaximumFileSize       = Info->MaximumFileSize;
    Properties->EnableFlags           = Info->EnableFlags;
    Properties->LogFileMode           = Info->LogFileMode;
    Properties->FlushTimer            = Info->FlushTimer;
    Properties->LogBuffersLost        = Info->LogBuffersLost;
    Properties->AgeLimit              = Info->AgeLimit;
    Properties->RealTimeBuffersLost   = Info->RealTimeBuffersLost;
}

NTSTATUS
EtwpThreadRunDown(
    IN PWMI_LOGGER_CONTEXT Logger,
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo,
    IN ULONG StartFlag,
    IN BOOLEAN bExtended
    )
{
    PSYSTEM_THREAD_INFORMATION pThreadInfo;
    ULONG GroupType;
    ULONG i;
    ULONG Size;
    ULONG SystemThreadInfoSize;
    PWMI_EXTENDED_THREAD_INFORMATION ThreadInfo;
    PWMI_EXTENDED_THREAD_INFORMATION64 ThreadInfo64;

    pThreadInfo = (PSYSTEM_THREAD_INFORMATION) (pProcessInfo+1);

    GroupType = EVENT_TRACE_GROUP_THREAD +
                ((StartFlag) ? EVENT_TRACE_TYPE_DC_START
                             : EVENT_TRACE_TYPE_DC_END);
    if (!KernelWow64) { // normal case

        Size = sizeof(WMI_EXTENDED_THREAD_INFORMATION);

        SystemThreadInfoSize = (bExtended)  
                               ? sizeof(SYSTEM_EXTENDED_THREAD_INFORMATION)
                               : sizeof(SYSTEM_THREAD_INFORMATION);
        for (i=0; i < pProcessInfo->NumberOfThreads; i++) {
            if (pThreadInfo == NULL)
                break;
            ThreadInfo = (PWMI_EXTENDED_THREAD_INFORMATION)
                          EtwpGetTraceBuffer( Logger,
                                              pThreadInfo,
                                              GroupType,
                                              Size );
            if (ThreadInfo) {
                ThreadInfo->ProcessId =
                    HandleToUlong(pThreadInfo->ClientId.UniqueProcess);
                ThreadInfo->ThreadId =
                    HandleToUlong(pThreadInfo->ClientId.UniqueThread);

                if (bExtended) {
                    PSYSTEM_EXTENDED_THREAD_INFORMATION pExtThreadInfo;
                    pExtThreadInfo = (PSYSTEM_EXTENDED_THREAD_INFORMATION) 
                                     pThreadInfo;
                    ThreadInfo->StackBase = pExtThreadInfo->StackBase;
                    ThreadInfo->StackLimit = pExtThreadInfo->StackLimit;

                    ThreadInfo->StartAddr = 
                                pExtThreadInfo->ThreadInfo.StartAddress;
                    ThreadInfo->Win32StartAddr = 
                                pExtThreadInfo->Win32StartAddress;
                    ThreadInfo->UserStackBase = 0;
                    ThreadInfo->UserStackLimit = 0;
                    ThreadInfo->WaitMode = -1;
                }
                else {
                    ThreadInfo->StackBase = 0;
                    ThreadInfo->StackLimit = 0;
                    ThreadInfo->StartAddr = 0;
                    ThreadInfo->Win32StartAddr = 0;
                    ThreadInfo->UserStackBase = 0;
                    ThreadInfo->UserStackLimit = 0;
                    ThreadInfo->WaitMode = -1;
                }
            }
            pThreadInfo  = (PSYSTEM_THREAD_INFORMATION) 
                           ( (char*)pThreadInfo +SystemThreadInfoSize );
        }
    }
    else { // KernelWow64
        Size = sizeof(WMI_EXTENDED_THREAD_INFORMATION64);

        SystemThreadInfoSize = (bExtended)  
                               ? sizeof(SYSTEM_EXTENDED_THREAD_INFORMATION)
                               : sizeof(SYSTEM_THREAD_INFORMATION);
        for (i=0; i < pProcessInfo->NumberOfThreads; i++) {
            if (pThreadInfo == NULL)
                break;
            ThreadInfo64 = (PWMI_EXTENDED_THREAD_INFORMATION64)
                           EtwpGetTraceBuffer( Logger,
                                               pThreadInfo,
                                               GroupType,
                                               Size );

            if (ThreadInfo64) {
                ThreadInfo64->ProcessId =
                    HandleToUlong(pThreadInfo->ClientId.UniqueProcess);
                ThreadInfo64->ThreadId =
                    HandleToUlong(pThreadInfo->ClientId.UniqueThread);

                if (bExtended) {
                    PSYSTEM_EXTENDED_THREAD_INFORMATION pExtThreadInfo;
                    pExtThreadInfo = 
                        (PSYSTEM_EXTENDED_THREAD_INFORMATION) pThreadInfo;
                    ThreadInfo64->StackBase64 = 0;
                    ThreadInfo64->StackBase64 = 
                        (ULONG64)(pExtThreadInfo->StackBase);
                    ThreadInfo64->StackLimit64 = 0;
                    ThreadInfo64->StackLimit64 = 
                        (ULONG64)(pExtThreadInfo->StackLimit);
                    ThreadInfo64->StartAddr64 = 0;
                    ThreadInfo64->StartAddr64 = 
                        (ULONG64)(pExtThreadInfo->ThreadInfo.StartAddress);
                    ThreadInfo64->Win32StartAddr64 = 0;
                    ThreadInfo64->Win32StartAddr64 = 
                        (ULONG64)(pExtThreadInfo->Win32StartAddress);
                    ThreadInfo64->UserStackBase64 = 0;
                    ThreadInfo64->UserStackLimit64 = 0;
                    ThreadInfo64->WaitMode = -1;
                }
                else {
                    ThreadInfo64->StackBase64 = 0;
                    ThreadInfo64->StackLimit64 = 0;
                    ThreadInfo64->StartAddr64 = 0;
                    ThreadInfo64->Win32StartAddr64 = 0;
                    ThreadInfo64->UserStackBase64 = 0;
                    ThreadInfo64->UserStackLimit64 = 0;
                    ThreadInfo64->WaitMode = -1;
                }
            }
            pThreadInfo  = (PSYSTEM_THREAD_INFORMATION)
                           ( (char*)pThreadInfo + SystemThreadInfoSize );
        }
    }
    return STATUS_SUCCESS;
}

void
EtwpLogImageLoadEvent(
    IN HANDLE ProcessID,
    IN PWMI_LOGGER_CONTEXT pLogger,
    IN PRTL_PROCESS_MODULE_INFORMATION pModuleInfo,
    IN PSYSTEM_THREAD_INFORMATION pThreadInfo
)
{
    UNICODE_STRING wstrModuleName;
    ANSI_STRING    astrModuleName;
    ULONG          sizeModuleName;
    ULONG          sizeBuffer;
    PCHAR          pAuxInfo;
    PWMI_IMAGELOAD_INFORMATION ImageLoadInfo;
    PWMI_IMAGELOAD_INFORMATION64 ImageLoadInfo64;

    if ((pLogger == NULL) || (pModuleInfo == NULL) || (pThreadInfo == NULL))
        return;

    RtlInitAnsiString( & astrModuleName, pModuleInfo->FullPathName);

    sizeModuleName = sizeof(WCHAR) * (astrModuleName.Length);
    if (!KernelWow64) { // normal case

        sizeBuffer     = sizeModuleName + sizeof(WCHAR)
                       + FIELD_OFFSET (WMI_IMAGELOAD_INFORMATION, FileName);

        ImageLoadInfo = (PWMI_IMAGELOAD_INFORMATION)
                         EtwpGetTraceBuffer(
                            pLogger,
                            pThreadInfo,
                            EVENT_TRACE_GROUP_PROCESS + EVENT_TRACE_TYPE_LOAD,
                            sizeBuffer);

        if (ImageLoadInfo == NULL) {
            return;
        }

        ImageLoadInfo->ImageBase = pModuleInfo->ImageBase;
        ImageLoadInfo->ImageSize = pModuleInfo->ImageSize;
        ImageLoadInfo->ProcessId = HandleToUlong(ProcessID);

        wstrModuleName.Buffer    = (LPWSTR) &ImageLoadInfo->FileName[0];

        wstrModuleName.MaximumLength = (USHORT) sizeModuleName + sizeof(WCHAR);
        RtlAnsiStringToUnicodeString(& wstrModuleName, & astrModuleName, FALSE);
    }
    else { // KernelWow64
        sizeBuffer     = sizeModuleName + sizeof(WCHAR)
                       + FIELD_OFFSET (WMI_IMAGELOAD_INFORMATION64, FileName);

        ImageLoadInfo64 = (PWMI_IMAGELOAD_INFORMATION64)
                         EtwpGetTraceBuffer(
                            pLogger,
                            pThreadInfo,
                            EVENT_TRACE_GROUP_PROCESS + EVENT_TRACE_TYPE_LOAD,
                            sizeBuffer);

        if (ImageLoadInfo64 == NULL) {
            return;
        }

        ImageLoadInfo64->ImageBase64 = 0;
        ImageLoadInfo64->ImageBase64 = (ULONG64)(pModuleInfo->ImageBase);
        ImageLoadInfo64->ImageSize64 = 0;
        ImageLoadInfo64->ImageSize64 = (ULONG64)(pModuleInfo->ImageSize);
        ImageLoadInfo64->ProcessId = HandleToUlong(ProcessID);

        wstrModuleName.Buffer    = (LPWSTR) &ImageLoadInfo64->FileName[0];

        wstrModuleName.MaximumLength = (USHORT) sizeModuleName + sizeof(WCHAR);
        RtlAnsiStringToUnicodeString(& wstrModuleName, & astrModuleName, FALSE);

    }
}

ULONG
EtwpSysModuleRunDown(
    IN PWMI_LOGGER_CONTEXT        pLogger,
    IN PSYSTEM_THREAD_INFORMATION pThreadInfo
    )
{
    NTSTATUS   status = STATUS_SUCCESS;
    char     * pLargeBuffer1;
    ULONG      ReturnLength;
    ULONG      CurrentBufferSize;

    ULONG                           i;
    PRTL_PROCESS_MODULES            pModules;
    PRTL_PROCESS_MODULE_INFORMATION pModuleInfo;

    pLargeBuffer1 = EtwpMemReserve(MAX_BUFFER_SIZE);

    if (pLargeBuffer1 == NULL)
    {
        status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    if (EtwpMemCommit(pLargeBuffer1, BUFFER_SIZE) == NULL)
    {
        status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    CurrentBufferSize = BUFFER_SIZE;

retry:
    status = NtQuerySystemInformation(
                    SystemModuleInformation,
                    pLargeBuffer1,
                    CurrentBufferSize,
                    &ReturnLength);

    if (status == STATUS_INFO_LENGTH_MISMATCH)
    {
        // Increase buffer size. ReturnLength shows how much we need. Add
        // another 4K buffer for additional modules loaded since this call. 
        //
        if (CurrentBufferSize < ReturnLength) {
            CurrentBufferSize = ReturnLength;
        }
        CurrentBufferSize = PAGESIZE_MULTIPLE(CurrentBufferSize + SMALL_BUFFER_SIZE);

        if (EtwpMemCommit(pLargeBuffer1, CurrentBufferSize) == NULL)
        {
            status = STATUS_NO_MEMORY;
            goto Cleanup;
        }
        goto retry;
    }

    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

    pModules = (PRTL_PROCESS_MODULES) pLargeBuffer1;

    for (i = 0, pModuleInfo = & (pModules->Modules[0]);
         i < pModules->NumberOfModules;
         i ++, pModuleInfo ++)
    {
        EtwpLogImageLoadEvent(NULL, pLogger, pModuleInfo, pThreadInfo);
    }

Cleanup:
    if (pLargeBuffer1)
    {
        EtwpMemFree(pLargeBuffer1);
    }
    return EtwpSetDosError(EtwpNtStatusToDosError(status));
}

ULONG
EtwpProcessModuleRunDown(
    IN PWMI_LOGGER_CONTEXT        pLogger,
    IN HANDLE                     ProcessID,
    IN PSYSTEM_THREAD_INFORMATION pThreadInfo)
{
    NTSTATUS               status = STATUS_SUCCESS;
    ULONG                  i;
    PRTL_DEBUG_INFORMATION pLargeBuffer1 = NULL;

    pLargeBuffer1 = RtlCreateQueryDebugBuffer(0, FALSE);
    if (pLargeBuffer1 == NULL)
    {
        status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    status = RtlQueryProcessDebugInformation(
                    ProcessID,
                    RTL_QUERY_PROCESS_NONINVASIVE |  RTL_QUERY_PROCESS_MODULES,
                    pLargeBuffer1);

    if ( !NT_SUCCESS(status) || (pLargeBuffer1->Modules == NULL) )
    {
        goto Cleanup;
    }


    //
    // RtlQueryProcessDebugInformation call is returning a buffer from an 
    // untrusted source with pointers and offset and it isn't validating it. 
    // Therefore we can not assume that this buffer is trustworthy. 
    // Since we elevated our privilege to SE_DEBUG_PRIVILEGE we need a
    // condition handler here to exit cleanly and reset the privilege. 
    //
    //

    try {

        for (i = 0; i < pLargeBuffer1->Modules->NumberOfModules; i ++)
        {
            EtwpLogImageLoadEvent(
                    ProcessID,
                    pLogger,
                    & (pLargeBuffer1->Modules->Modules[i]),
                    pThreadInfo);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_NOACCESS;
    }

Cleanup:
    if (pLargeBuffer1)
    {
        RtlDestroyQueryDebugBuffer(pLargeBuffer1);
    }
    return EtwpSetDosError(EtwpNtStatusToDosError(status));
}

NTSTATUS
EtwpProcessRunDown(
    IN PWMI_LOGGER_CONTEXT Logger,
    IN ULONG StartFlag,
    IN ULONG fEnableFlags
    )
{
    PSYSTEM_PROCESS_INFORMATION  pProcessInfo;
    PSYSTEM_THREAD_INFORMATION   pThreadInfo;
    char* LargeBuffer1;
    NTSTATUS status;
    ULONG ReturnLength;
    ULONG CurrentBufferSize;
    ULONG GroupType;
    ULONG TotalOffset = 0;
    OBJECT_ATTRIBUTES objectAttributes;
    BOOLEAN WasEnabled = TRUE;
    BOOLEAN bExtended = TRUE;

    LargeBuffer1 = EtwpMemReserve ( MAX_BUFFER_SIZE );
    if (LargeBuffer1 == NULL) {
        return STATUS_NO_MEMORY;
    }

    if (EtwpMemCommit (LargeBuffer1, BUFFER_SIZE) == NULL) {
        return STATUS_NO_MEMORY;
    }

    CurrentBufferSize = BUFFER_SIZE;
    retry:
    if (bExtended) {
        status = NtQuerySystemInformation(
                    SystemExtendedProcessInformation,
                    LargeBuffer1,
                    CurrentBufferSize,
                    &ReturnLength
                    );
    }
    else {
        status = NtQuerySystemInformation(
                    SystemProcessInformation,
                    LargeBuffer1,
                    CurrentBufferSize,
                    &ReturnLength
                    );
    }

    if (status == STATUS_INFO_LENGTH_MISMATCH) {

        //
        // Increase buffer size.
        //
        if (CurrentBufferSize < ReturnLength) {
            CurrentBufferSize = ReturnLength;
        }
        CurrentBufferSize = 
                      PAGESIZE_MULTIPLE(CurrentBufferSize + SMALL_BUFFER_SIZE);

        if (EtwpMemCommit (LargeBuffer1, CurrentBufferSize) == NULL) {
            return STATUS_NO_MEMORY;
        }
        goto retry;
    }

    if (!NT_SUCCESS(status)) {

        if (bExtended) {
            bExtended = FALSE;
            goto retry;
        }

        EtwpMemFree(LargeBuffer1);
        return(status);
    }


    //
    // Adjust Privileges to obtain the module information
    //


    if (fEnableFlags & EVENT_TRACE_FLAG_IMAGE_LOAD) {
        status = RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE,
                                    TRUE, FALSE, &WasEnabled);
        if (!NT_SUCCESS(status)) {
            status = RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE,
                            TRUE, TRUE, &WasEnabled);
        }

        if (!NT_SUCCESS(status)) {
            EtwpMemFree(LargeBuffer1);
            return  (status);
        }
    }


    TotalOffset = 0;
    pProcessInfo = (SYSTEM_PROCESS_INFORMATION *) LargeBuffer1;
    while (TRUE) {
        ULONG Size;
        ULONG Length = 0;
        ULONG SidLength = 0;
        PUCHAR AuxPtr;
        PULONG_PTR AuxInfo;
        ANSI_STRING s;
        HANDLE Token;
        HANDLE pProcess;
        PCLIENT_ID Cid;
        ULONG TempInfo[128];
        PWMI_PROCESS_INFORMATION WmiProcessInfo;
        PWMI_PROCESS_INFORMATION64 WmiProcessInfo64;

        GroupType = EVENT_TRACE_GROUP_PROCESS +
                    ((StartFlag) ? EVENT_TRACE_TYPE_DC_START
                                 : EVENT_TRACE_TYPE_DC_END);

        pThreadInfo = (PSYSTEM_THREAD_INFORMATION) (pProcessInfo+1);
        if (pProcessInfo->NumberOfThreads > 0) {
            Cid = (PCLIENT_ID) &pThreadInfo->ClientId;
        }
        else {
            Cid = NULL;
        }

        // if at termination, rundown thread first before process
        if ( (!StartFlag) &&
             (fEnableFlags & EVENT_TRACE_FLAG_THREAD) ){
            status = EtwpThreadRunDown(Logger,
                                       pProcessInfo,
                                       StartFlag,
                                       bExtended);
            if (!NT_SUCCESS(status)) {
                break;
            }

        }

        if (fEnableFlags & EVENT_TRACE_FLAG_PROCESS) {

            Length = 1;
            if ( pProcessInfo->ImageName.Buffer  &&
                     pProcessInfo->ImageName.Length > 0 ) {
                status = RtlUnicodeStringToAnsiString(
                                     &s,
                                     (PUNICODE_STRING)&pProcessInfo->ImageName,
                                     TRUE);
                if (NT_SUCCESS(status)) {
                    Length = s.Length + 1;
                }
            }

            InitializeObjectAttributes(
                    &objectAttributes, 0, 0, NULL, NULL);
            status = NtOpenProcess(
                                  &pProcess,
                                  PROCESS_QUERY_INFORMATION,
                                  &objectAttributes,
                                  Cid);
            if (NT_SUCCESS(status)) {
                status = NtOpenProcessToken(
                                      pProcess,
                                      TOKEN_READ,
                                      &Token);
                if (NT_SUCCESS(status)) {

                    status = NtQueryInformationToken(
                                             Token,
                                             TokenUser,
                                             TempInfo,
                                             256,
                                             &SidLength);
                    NtClose(Token);
                }
                NtClose(pProcess);
            }
            if ( (!NT_SUCCESS(status)) || SidLength <= 0) {
                TempInfo[0] = 0;
                SidLength = sizeof(ULONG);
            }

            if (!KernelWow64) {  // normal case
                Size = FIELD_OFFSET(WMI_PROCESS_INFORMATION, Sid);
                Size += Length + SidLength;
                WmiProcessInfo = (PWMI_PROCESS_INFORMATION)
                                  EtwpGetTraceBuffer( Logger,
                                                      pThreadInfo,
                                                      GroupType,
                                                      Size);
                if (WmiProcessInfo == NULL) {
                    status = STATUS_NO_MEMORY;
                    break;
                }
                WmiProcessInfo->ProcessId = 
                                  HandleToUlong( pProcessInfo->UniqueProcessId);
                WmiProcessInfo->ParentId = 
                    HandleToUlong( pProcessInfo->InheritedFromUniqueProcessId);
                WmiProcessInfo->SessionId = pProcessInfo->SessionId;

                WmiProcessInfo->PageDirectoryBase = 
                                                pProcessInfo->PageDirectoryBase;
                WmiProcessInfo->ExitStatus = 0;

                AuxPtr = (PUCHAR) (&WmiProcessInfo->Sid);

                RtlCopyMemory(AuxPtr, &TempInfo[0], SidLength);
                AuxPtr += SidLength;

                if (Length > 1) {
                    RtlCopyMemory(AuxPtr, s.Buffer, Length - 1);
                    AuxPtr += (Length - 1);
                    RtlFreeAnsiString(&s);
                }
                *AuxPtr = '\0';
                AuxPtr++;
            }
            else { // KernelWow64
                Size = FIELD_OFFSET(WMI_PROCESS_INFORMATION64, Sid);
                if (SidLength != sizeof(ULONG)) {
                    Size += Length + SidLength + 8;
                }
                else {
                    Size += Length + SidLength;
                }
                WmiProcessInfo64 = (PWMI_PROCESS_INFORMATION64)
                                   EtwpGetTraceBuffer( Logger,
                                                      pThreadInfo,
                                                      GroupType,
                                                      Size);
                if (WmiProcessInfo64 == NULL) {
                    status = STATUS_NO_MEMORY;
                    break;
                }
                WmiProcessInfo64->ProcessId = 
                                  HandleToUlong( pProcessInfo->UniqueProcessId);
                WmiProcessInfo64->ParentId = 
                     HandleToUlong( pProcessInfo->InheritedFromUniqueProcessId);
                WmiProcessInfo64->SessionId = pProcessInfo->SessionId;
                WmiProcessInfo64->PageDirectoryBase64 = 0;
                WmiProcessInfo64->PageDirectoryBase64 = 
                                     (ULONG64)(pProcessInfo->PageDirectoryBase);
                WmiProcessInfo64->ExitStatus = 0;

                // We need to widen TOKEN_USER structure before copying SID.
                // Technically, what follows is not the correct way to extend 
                // SID for Wow64. The correct way is to widen the pointer 
                // inside the TOKEN_USER structure within the returned SID blob.
                // However, we don't really care what the pointer value is, we 
                // just need to know that it is not 0. Hence, we copy 
                // TOKEN_USER first, leave some space out, and we copy the 
                // actual SID.

                AuxPtr = (PUCHAR) (&WmiProcessInfo64->Sid);
                if (SidLength > 8) {
                    RtlCopyMemory(AuxPtr, &TempInfo[0], 8);
                    RtlCopyMemory((AuxPtr + 16), &(TempInfo[2]), SidLength - 8);
                    AuxPtr += SidLength + 8;
                }
                else if (SidLength == sizeof(ULONG)) { 
                    RtlCopyMemory(AuxPtr, &TempInfo[0], SidLength);
                    AuxPtr += SidLength;
                }
                else { // This really cannot/should not happen.
                    RtlCopyMemory(AuxPtr, &TempInfo[0], SidLength);
                    AuxPtr += SidLength + 8;
                }
                if (Length > 1) {
                    RtlCopyMemory(AuxPtr, s.Buffer, Length - 1);
                    AuxPtr += (Length - 1);
                    RtlFreeAnsiString(&s);
                }
                *AuxPtr = '\0';
                AuxPtr++;
            }
        }


        // if at beginning, trace threads after process
        if (StartFlag) {

            if (fEnableFlags & EVENT_TRACE_FLAG_THREAD) {
                EtwpThreadRunDown(Logger, pProcessInfo, StartFlag, bExtended);
            }

            if (fEnableFlags & EVENT_TRACE_FLAG_IMAGE_LOAD) {
                if (pProcessInfo->UniqueProcessId == 0) {
                    EtwpSysModuleRunDown(Logger, pThreadInfo);
                }
                else
                    EtwpProcessModuleRunDown(
                            Logger,
                            (HANDLE) pProcessInfo->UniqueProcessId,
                            pThreadInfo);
            }
        }
        if (pProcessInfo->NextEntryOffset == 0) {
            break;
        }
        TotalOffset += pProcessInfo->NextEntryOffset;
        pProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&LargeBuffer1[TotalOffset];
    }

    //
    // Restore privileges back to what it was before
    //


    if ( (fEnableFlags & EVENT_TRACE_FLAG_IMAGE_LOAD) && WasEnabled ) {
        status = RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE,
                                    FALSE,
                                    FALSE,
                                    &WasEnabled);
        if (!NT_SUCCESS(status)) {
            status = RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE,
                                    FALSE,
                                    TRUE,
                                    &WasEnabled);
        }
    }

    EtwpMemFree(LargeBuffer1);

    return status;
}

VOID
EtwpInitString(
    IN PVOID Destination,
    IN PVOID Buffer,
    IN ULONG Size
    )
{
    PSTRING s = (PSTRING) Destination;

    s->Buffer = Buffer;
    s->Length = 0;
    if (Buffer != NULL)
        s->MaximumLength = (USHORT) Size;
    else
        s->MaximumLength = 0;
}

ULONG 
EtwpRelogHeaderToLogFile(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo,
    IN PSYSTEM_TRACE_HEADER RelogProp
    )
{
    PTRACE_LOGFILE_HEADER RelogFileHeader;
    LPWSTR FileName = NULL;
    ULONG RelogPropSize;
    HANDLE LogFile = INVALID_HANDLE_VALUE;
    ULONG BufferSize;
    IO_STATUS_BLOCK IoStatus;
    PWMI_BUFFER_HEADER Buffer;
    LPWSTR FileNameBuffer = NULL;
    PUCHAR BufferSpace;
    NTSTATUS Status;

    RelogFileHeader = (PTRACE_LOGFILE_HEADER) ((PUCHAR)RelogProp +
                                               sizeof(SYSTEM_TRACE_HEADER) );
    RelogPropSize = RelogProp->Packet.Size;
    FileName = (LPWSTR) LoggerInfo->LogFileName.Buffer;
    if (FileName == NULL) {
        return EtwpSetDosError(ERROR_BAD_PATHNAME);
    }
    LogFile = EtwpCreateFileW(
                FileName,
                GENERIC_WRITE,
                FILE_SHARE_READ, 
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
                
                
                                

    if (LogFile == INVALID_HANDLE_VALUE) {
        return EtwpGetLastError();
    }

    LoggerInfo->LogFileHandle = LogFile;
    LoggerInfo->NumberOfProcessors = RelogFileHeader->NumberOfProcessors;

    if (8 == RelogFileHeader->PointerSize && 4 == sizeof(PVOID)) {

        LoggerInfo->Wnode.ClientContext = 
                           *((PULONG)((PUCHAR)RelogFileHeader + 
                           FIELD_OFFSET(TRACE_LOGFILE_HEADER, ReservedFlags) + 
                           8));
    }
    else if (4 == RelogFileHeader->PointerSize && 8 == sizeof(PVOID)) {

        LoggerInfo->Wnode.ClientContext = 
                           *((PULONG)((PUCHAR)RelogFileHeader + 
                           FIELD_OFFSET(TRACE_LOGFILE_HEADER, ReservedFlags) 
                           - 8));
    }
    else {
        LoggerInfo->Wnode.ClientContext = RelogFileHeader->ReservedFlags;
    }

    BufferSize = LoggerInfo->BufferSize * 1024;
    BufferSpace   = EtwpAlloc(BufferSize);
    if (BufferSpace == NULL) {
        NtClose(LogFile);
        return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
    }

    // initialize buffer first
    RtlZeroMemory(BufferSpace, BufferSize);
    Buffer         = (PWMI_BUFFER_HEADER) BufferSpace;
    Buffer->Offset = sizeof(WMI_BUFFER_HEADER);

    //
    // We are making this an Application Trace always. 
    // However, if two application traces are relogged
    // the Guidmaps are not really consolidated. 
    //

    Buffer->Wnode.Guid   = LoggerInfo->Wnode.Guid;
    RelogFileHeader->LogFileMode = EVENT_TRACE_RELOG_MODE;

    Buffer->Wnode.BufferSize = BufferSize;
    Buffer->ClientContext.Alignment = (UCHAR)WmiTraceAlignment;
    Buffer->Wnode.Flags   = WNODE_FLAG_TRACED_GUID;
    RelogFileHeader->BuffersWritten = 1;
    LoggerInfo->BuffersWritten = 1;
    Buffer->Offset = sizeof(WMI_BUFFER_HEADER) + RelogPropSize;
    //
    // Copy the Old LogFileHeader 
    //
    RtlCopyMemory((char*) Buffer + sizeof(WMI_BUFFER_HEADER),
                  RelogProp,
                  RelogPropSize 
                 );

    if (Buffer->Offset < BufferSize) {
        RtlFillMemory(
                (char *) Buffer + Buffer->Offset,
                BufferSize - Buffer->Offset,
                0xFF);
    }
    Buffer->BufferType = WMI_BUFFER_TYPE_RUNDOWN;
    Buffer->BufferFlag = WMI_BUFFER_FLAG_FLUSH_MARKER;
    Status = NtWriteFile(
            LogFile,
            NULL,
            NULL,
            NULL,
            &IoStatus,
            BufferSpace,
            BufferSize,
            NULL,
            NULL);
    NtClose(LogFile);

    LogFile = EtwpCreateFileW(
                 FileName,
                 GENERIC_WRITE,
                 FILE_SHARE_READ,
                 NULL,
                 OPEN_EXISTING,
                 FILE_FLAG_NO_BUFFERING,
                 NULL
                 );

    EtwpFree(BufferSpace);

    if (LogFile == INVALID_HANDLE_VALUE) {
        return EtwpGetLastError();
    }
    LoggerInfo->LogFileHandle = LogFile;

    return ERROR_SUCCESS;

}

ULONG
EtwpFixLogFileHeaderForWow64(
    IN PWMI_LOGGER_INFORMATION LoggerInfo,
    IN OUT PTRACE_LOGFILE_HEADER LogfileHeader
    )
{
    PUCHAR TempSpace = NULL;
    PULONG64 Ulong64Ptr;
    ULONG SizeNeeded = 0;
    
    if (LoggerInfo == NULL || LogfileHeader == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    SizeNeeded = sizeof(TRACE_LOGFILE_HEADER)
                    + LoggerInfo->LoggerName.Length + sizeof(WCHAR)
                    + LoggerInfo->LogFileName.Length + sizeof(WCHAR)
                    + 8;

    TempSpace = EtwpAlloc(SizeNeeded);
    if (TempSpace == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlZeroMemory(TempSpace, SizeNeeded);
    RtlCopyMemory(TempSpace, LogfileHeader, sizeof(TRACE_LOGFILE_HEADER));
    Ulong64Ptr = (PULONG64)(TempSpace + FIELD_OFFSET(TRACE_LOGFILE_HEADER, LoggerName));
    *Ulong64Ptr = (ULONG64)((PUCHAR)LogfileHeader + sizeof(TRACE_LOGFILE_HEADER) + 8);
    RtlCopyMemory((TempSpace + sizeof(TRACE_LOGFILE_HEADER) + 8),
                    LogfileHeader->LoggerName,
                    LoggerInfo->LoggerName.Length + sizeof(WCHAR));
    Ulong64Ptr++;
    *Ulong64Ptr = (ULONG64)((PUCHAR)LogfileHeader + sizeof(TRACE_LOGFILE_HEADER) 
                            + LoggerInfo->LoggerName.Length  + sizeof(WCHAR) + 8);
    RtlCopyMemory((TempSpace + sizeof(TRACE_LOGFILE_HEADER) + LoggerInfo->LoggerName.Length + sizeof(WCHAR) + 8),
                    LogfileHeader->LogFileName,
                    LoggerInfo->LogFileName.Length + sizeof(WCHAR));
    Ulong64Ptr++;
    RtlCopyMemory((PUCHAR)Ulong64Ptr, &(LogfileHeader->TimeZone), 
            sizeof(TRACE_LOGFILE_HEADER) - FIELD_OFFSET(TRACE_LOGFILE_HEADER, TimeZone));

    RtlCopyMemory(LogfileHeader, TempSpace, SizeNeeded);
    EtwpFree(TempSpace);
    return ERROR_SUCCESS;

}

ULONG
EtwpAddLogHeaderToLogFile(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo,
    IN PWMI_REF_CLOCK              RefClock,
    IN     ULONG                   Update
    )
/*++

Routine Description:

    This routine creates a new log file header or updates an existing header 
    from an existing log file. StartTrace() and ControlTrace() will call this 
    routine to keep the log file header persistent.

    Notes:
    - Special cases should be considered carefully. 
        1. Append case: The headers should be read from a file first, and 
           certain parameters specified by the user cannot be updated depending
           on the value in the existing log file. In particular, the SystemTime
           field in the header need to be updated accordingly.
        2. Update case: Similar to the case above. However, we do QueryTrace()
           to get the latest info on the logging session instead of reading a 
           file header. Append and Update modes are mutually exclusive.
        3. Circular log file case: StartBuffers gets set to 
           LoggerInfo.BuffersWritten for correct post processing.
        4. Private logger case: LoggerInfo->Checksum is NULL if this is a 
           private logger.
        5. Prealloc log file case: This is the routine that extends the file 
           size. It happens at the end of the routine, just before closing and 
           opening the file for the second time.
        6. WOW64 case: Collecting kernel data under Wow64 is special in the
           sense that kernel events will still have 64-bit pointer while some 
           user mode events (DCSTART and DCEND and Image events) will have
           thunked pointers. For this reason, all events for kernel tracing on
           ia64 machines will have 64-bit pointers and PointerSize will be 
           adjusted to 8 bytes if under Wow64.
        7. PrivateLogger: We should not call QueryLogger from this routine.
           That will result in a deadlock since pump thread is executing
           this code and it could block on the call waiting for itself
           to respond. 
           
    - LogFile is a handle to a log file.  When this routine returns with success
      a handle to the logfile is open. If you call START or UPDATE ioctl 
      then it will be closed in the kernel (whether or not the call succeeds). 
    - Checksum, FileNameBuffer and Logger.BufferSpace are temporary spaces 
      allocated and used in this routine. Make sure they are freed before
      all exits.

Arguments:

    LoggerInfo      Structure that keeps the information about a logger under 
                    consideration. This will be properly updated. LogFileHandle
                    field will have a valid handle to a newly created log file.
    RefClock        Reference clock. The SystemTIme field in the header will be
                    updated accordingly. Used only in kernel trace.
    Update          Whether this is an update operation or not. Certain 
                    parameters in the LoggerInfo (notably BufferSize) will not 
                    be updated if this is TRUE.

Return Value:

    The status of performing the action requested.

--*/
{
    NTSTATUS Status;
    HANDLE LogFile = INVALID_HANDLE_VALUE;
    ULONG BufferSize;
    ULONG MemorySize;
    ULONG TraceKernel;
    SYSTEM_BASIC_INFORMATION SystemInfo;
    WMI_LOGGER_CONTEXT Logger;
    IO_STATUS_BLOCK IoStatus;
    PWMI_BUFFER_HEADER Buffer;
    FILE_POSITION_INFORMATION FileInfo;
    LPWSTR FileName = NULL;
    LPWSTR FileNameBuffer = NULL;
    ULONG HeaderSize;
    ULONG AppendPointerSize = 0;

    struct WMI_LOGFILE_HEADER {
           WMI_BUFFER_HEADER    BufferHeader;
           SYSTEM_TRACE_HEADER  SystemHeader;
           TRACE_LOGFILE_HEADER LogFileHeader;
    };
    struct WMI_LOGFILE_HEADER LoggerBuffer;
    BOOLEAN bLogFileAppend =
                    (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_APPEND)
                  ? (TRUE) : (FALSE);

    if ((LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_NEWFILE)  &&
        (LoggerInfo->LogFileName.Length > 0)) {
        HRESULT hr;

        FileName = (LPWSTR) EtwpAlloc(LoggerInfo->LogFileName.Length + 64);
        if (FileName == NULL) {
            return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
        }
        //
        // The LogFilePatten has already been validated
        //
        hr = StringCbPrintfW(FileName, 
                             LoggerInfo->LogFileName.Length + 64, 
                             LoggerInfo->LogFileName.Buffer, 
                             1);
        if (FAILED(hr)) {
            EtwpFree(FileName);
            return HRESULT_CODE(hr);
        }
        
        FileNameBuffer = FileName;
    }
    if (FileName == NULL)
        FileName = (LPWSTR) LoggerInfo->LogFileName.Buffer;

    //
    // If it is Append Mode, we need to open the file and make sure the
    // pick up the BufferSize
    //

    if ( bLogFileAppend ) {

        FILE_STANDARD_INFORMATION FileStdInfo;
        
        ULONG ReadSize   = sizeof(WMI_BUFFER_HEADER)
                         + sizeof(SYSTEM_TRACE_HEADER)
                         + sizeof(TRACE_LOGFILE_HEADER);
        ULONG nBytesRead = 0;

        //
        //  Update and Append do not mix. To Append LoggerInfo
        //  must have LogFileName
        //

        if ( (Update) || (LoggerInfo->LogFileName.Length <= 0) ) {
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            return EtwpSetDosError(ERROR_INVALID_PARAMETER);
        }

        LogFile = EtwpCreateFileW(FileName,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
        if (LogFile == INVALID_HANDLE_VALUE) {
            // cannot OPEN_EXISTING, assume that logfile is not there and
            // create a new one.
            //
            bLogFileAppend = FALSE;
            LoggerInfo->LogFileMode = LoggerInfo->LogFileMode
                                    & (~ (EVENT_TRACE_FILE_MODE_APPEND));
        }
        else {
            // read TRACE_LOGFILE_HEADER structure and update LoggerInfo
            // members.
            //
            Status = EtwpReadFile(LogFile,
                              (LPVOID) & LoggerBuffer,
                              ReadSize,
                              & nBytesRead,
                              NULL);
            if (nBytesRead < ReadSize) {
                NtClose(LogFile);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                return EtwpSetDosError(ERROR_BAD_PATHNAME);
            }
            if (  LoggerBuffer.LogFileHeader.LogFileMode
                & EVENT_TRACE_FILE_MODE_CIRCULAR) {
                NtClose(LogFile);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                return EtwpSetDosError(ERROR_BAD_PATHNAME);
            }
            AppendPointerSize = LoggerBuffer.LogFileHeader.PointerSize;
            LoggerInfo->BufferSize =
                            LoggerBuffer.LogFileHeader.BufferSize / 1024;

            //
            // Check to see if the values from the logfile are valid. 
            //
            if ( (LoggerInfo->BufferSize == 0)  || 
                 (LoggerInfo->BufferSize  > MAX_ETW_BUFFERSIZE) ) {
                NtClose(LogFile);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                return EtwpSetDosError(ERROR_INVALID_DATA);
            }

            //
            // If it's append, the GuidMap buffers are not accounted for
            // in the BuffersWritten count. The starttrace call will fail
            // on checksum error if it is not adjusted properly. However, 
            // this will trash the GuidMap entries in this file.
            //
            Status = NtQueryInformationFile(
                        LogFile,
                        &IoStatus,
                        &FileStdInfo,
                        sizeof(FILE_STANDARD_INFORMATION),
                        FileStandardInformation
                            );
            if (NT_SUCCESS(Status)) {
                ULONG64 FileSize = FileStdInfo.AllocationSize.QuadPart;
                ULONG64 BuffersWritten = 0;

                if (LoggerBuffer.LogFileHeader.BufferSize > 0) {
                    BuffersWritten = FileSize / 
                                 (ULONG64)LoggerBuffer.LogFileHeader.BufferSize;
                }
                LoggerInfo->BuffersWritten = (ULONG)BuffersWritten;
                LoggerBuffer.LogFileHeader.BuffersWritten = (ULONG)BuffersWritten;
            }
            else {
               NtClose(LogFile);
               if (FileNameBuffer != NULL) {
                   EtwpFree(FileNameBuffer);
               }
                return EtwpNtStatusToDosError(Status);
            }

            LoggerInfo->MaximumFileSize =
                            LoggerBuffer.LogFileHeader.MaximumFileSize;

            // Write back logfile append mode so EtwpFinalizeLogFile() correctly
            // update BuffersWritten field
            //
            FileInfo.CurrentByteOffset.QuadPart =
                            LOGFILE_FIELD_OFFSET(EndTime);
            Status = NtSetInformationFile(LogFile,
                                          & IoStatus,
                                          & FileInfo,
                                          sizeof(FILE_POSITION_INFORMATION),
                                          FilePositionInformation);
            if (!NT_SUCCESS(Status)) {
                NtClose(LogFile);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                return EtwpSetDosError(EtwpNtStatusToDosError(Status));
            }
            LoggerBuffer.LogFileHeader.EndTime.QuadPart = 0;
            Status = NtWriteFile(LogFile,
                                 NULL,
                                 NULL,
                                 NULL,
                                 & IoStatus,
                                 & LoggerBuffer.LogFileHeader.EndTime,
                                 sizeof(LARGE_INTEGER),
                                 NULL,
                                 NULL);
            if (! NT_SUCCESS(Status)) {
                NtClose(LogFile);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                return EtwpSetDosError(EtwpNtStatusToDosError(Status));
            }

            // build checksum structure
            //
            if (LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
                LoggerInfo->Checksum = NULL;
            }
            else {
                LoggerInfo->Checksum = EtwpAlloc(
                        sizeof(WNODE_HEADER) + sizeof(TRACE_LOGFILE_HEADER));
                if (LoggerInfo->Checksum != NULL) {
                    PBYTE ptrChecksum = LoggerInfo->Checksum;
                    RtlCopyMemory(ptrChecksum,
                                  & LoggerBuffer.BufferHeader,
                                  sizeof(WNODE_HEADER));
                    ptrChecksum += sizeof(WNODE_HEADER);
                    RtlCopyMemory(ptrChecksum,
                                  & LoggerBuffer.LogFileHeader,
                                  sizeof(TRACE_LOGFILE_HEADER));
                }
                else {
                    NtClose(LogFile);
                    if (FileNameBuffer != NULL) {
                        EtwpFree(FileNameBuffer);
                    }
                    return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
                }
            }
        }
    }

    // get the system parameters first

    LoggerInfo->LogFileHandle = NULL;

    Status = NtQuerySystemInformation(
                SystemBasicInformation,
                &SystemInfo, sizeof (SystemInfo), NULL);

    if (!NT_SUCCESS(Status)) {
        if (LogFile != INVALID_HANDLE_VALUE) {
            // for APPEND case, the file is already opened
            NtClose(LogFile);
        }
        if (FileNameBuffer != NULL) {
            EtwpFree(FileNameBuffer);
        }
        if (LoggerInfo->Checksum != NULL) {
            EtwpFree(LoggerInfo->Checksum);
        }
        LoggerInfo->Checksum = NULL;
        return EtwpSetDosError(EtwpNtStatusToDosError(Status));
    }

    // choose some logical default value for buffer size if user
    // has not provided one

    MemorySize = (ULONG)(SystemInfo.NumberOfPhysicalPages * SystemInfo.PageSize
                    / 1024 / 1024);
    if (MemorySize <= 64) {
        BufferSize      = SystemInfo.PageSize;
    }
    else if (MemorySize <= 512) {
        BufferSize      = SystemInfo.PageSize * 2;
    }
    else {
        BufferSize      = 64 * 1024;        // allocation size
    }

    if (LoggerInfo->BufferSize > 1024)      // limit to 1Mb
        BufferSize = 1024 * 1024;
    else if (LoggerInfo->BufferSize > 0)
        BufferSize = LoggerInfo->BufferSize * 1024;

    TraceKernel = IsEqualGUID(&LoggerInfo->Wnode.Guid, &SystemTraceControlGuid);
    if (!TraceKernel) {
        GUID guid;
        RtlZeroMemory(&guid, sizeof(GUID));
        if (IsEqualGUID(&LoggerInfo->Wnode.Guid, &guid)) {
            // Generate a Guid for this logger stream
            // This will ensure  buffer filtering at the WMI service
            // based on this GUID.
            UUID uid;
            EtwpUuidCreate(&uid);
            LoggerInfo->Wnode.Guid = uid;
        }
    }

    if (LoggerInfo->LogFileName.Length <= 0) {
        if (LogFile != INVALID_HANDLE_VALUE) {
            // for APPEND case, the file is already opened
            NtClose(LogFile);
        }
        if (FileNameBuffer != NULL) {
            EtwpFree(FileNameBuffer);
        }
        if (LoggerInfo->Checksum != NULL) {
            EtwpFree(LoggerInfo->Checksum);
        }
        LoggerInfo->Checksum = NULL;
        return  ERROR_SUCCESS; //goto SendToKm;
    }
    //
    // We assumed the exposed API has checked for either RealTime or FileName
    // is provided

    //
    // If this is an Update call, then we need to pick up the original
    // buffer size for the LogFileHeader.
    // Otherwise, use the one computed above.
    //
    // For private loggers, a valid logger info is already given. Also, 
    // we can't afford nested IOCTL/MBReply for doing QueryLogger.
    //
    if (!Update) {
        LoggerInfo->BufferSize = BufferSize / 1024;
    }
    else if (!(LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE)) {
        // Update case. This block is not needed for private loggers.
        PWMI_LOGGER_INFORMATION pTempLoggerInfo;
        PWCHAR strLoggerName = NULL;
        PWCHAR strLogFileName = NULL;
        ULONG ErrCode;
        ULONG SizeNeeded = sizeof(WMI_LOGGER_INFORMATION) + MAXSTR * sizeof(WCHAR) * 2;
        ULONG CurrentProcWow = FALSE;

        SizeNeeded = (SizeNeeded +7) & ~7;
        pTempLoggerInfo = EtwpAlloc(SizeNeeded);
        if (pTempLoggerInfo == NULL) {
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
        }
        RtlZeroMemory(pTempLoggerInfo, SizeNeeded);
        pTempLoggerInfo->Wnode.BufferSize = SizeNeeded;
        pTempLoggerInfo->Wnode.Flags |= WNODE_FLAG_TRACED_GUID;
        pTempLoggerInfo->Wnode.HistoricalContext = LoggerInfo->Wnode.HistoricalContext;
        pTempLoggerInfo->Wnode.Guid = LoggerInfo->Wnode.Guid;

        strLoggerName = (PWCHAR) ( ((PUCHAR) pTempLoggerInfo)
                                    + sizeof(WMI_LOGGER_INFORMATION));
        EtwpInitString(&pTempLoggerInfo->LoggerName,
                       strLoggerName,
                       MAXSTR * sizeof(WCHAR));
        if (LoggerInfo->LoggerName.Length > 0) {
            RtlCopyUnicodeString( &pTempLoggerInfo->LoggerName,
                                  &LoggerInfo->LoggerName);
        }


        strLogFileName = (PWCHAR) ( ((PUCHAR) pTempLoggerInfo)
                                    + sizeof(WMI_LOGGER_INFORMATION)
                                    + MAXSTR * sizeof(WCHAR) );
        EtwpInitString(&pTempLoggerInfo->LogFileName,
                       strLogFileName,
                       MAXSTR * sizeof(WCHAR) );

        //
        // Call QueryLogger
        //
        ErrCode = EtwpQueryLogger(pTempLoggerInfo, FALSE);

        if (ErrCode != ERROR_SUCCESS) {
            EtwpFree(pTempLoggerInfo);
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            if (LoggerInfo->Checksum != NULL) {
                EtwpFree(LoggerInfo->Checksum);
            }
            LoggerInfo->Checksum = NULL;
            return EtwpSetDosError(ErrCode);
        }
        BufferSize = pTempLoggerInfo->BufferSize * 1024;
        if (!TraceKernel && (sizeof(PVOID) != 8)) {
            // For kernel trace, the pointer size is always 64 on ia64, 
            // whether or not under Wow64.
            // Get Wow64 information, set the flag, and adjust the pointer size.
            ULONG_PTR ulp;
            Status = NtQueryInformationProcess(
                        NtCurrentProcess(),
                        ProcessWow64Information,
                        &ulp,
                        sizeof(ULONG_PTR),
                        NULL);
            if (NT_SUCCESS(Status) && (ulp != 0)) {
                CurrentProcWow = TRUE;
            }
        }
        if ( (pTempLoggerInfo->Wow && !TraceKernel && 8 == sizeof(PVOID)) || 
            (CurrentProcWow && !(pTempLoggerInfo->Wow)) ) {
            // We're trying to do 64 bit mode update on a non-kernel logger 
            // that started in 32 bit, or vice versa. We don't allow this.
            EtwpFree(pTempLoggerInfo);
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            if (LoggerInfo->Checksum != NULL) {
                EtwpFree(LoggerInfo->Checksum);
            }
            LoggerInfo->Checksum = NULL;
            return EtwpSetDosError(ERROR_NOT_SUPPORTED);
        }
        EtwpFree(pTempLoggerInfo);
    }

    //
    // Now open the file for writing synchronously for the logger
    // others may want to read it as well.
    // For logfile append mode, logfile has been opened previously
    //
    if (!bLogFileAppend) {
        LogFile = EtwpCreateFileW(
                    FileName,
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);

        if (LogFile == INVALID_HANDLE_VALUE) {
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            return EtwpGetLastError();
        }
    }

    LoggerInfo->LogFileHandle = LogFile;
    //
    // All returns for error cases should have NtClose(LogFile). 
    //

    if (TraceKernel && (sizeof(PVOID) != 8)) {
        // For kernel trace, the pointer size is always 64 on ia64, 
        // whether or not under Wow64.
        // Get Wow64 information, set the flag, and adjust the pointer size.
        ULONG_PTR ulp;
        Status = NtQueryInformationProcess(
                    NtCurrentProcess(),
                    ProcessWow64Information,
                    &ulp,
                    sizeof(ULONG_PTR),
                    NULL);
        if (NT_SUCCESS(Status) && (ulp != 0)) {
            KernelWow64 = TRUE;
        }
    }

    //
    // Before Allocating the Buffer for Logfile Header make
    // sure the buffer size is atleast as large as the LogFileHeader
    //
    if (!KernelWow64) { 
        HeaderSize =  sizeof(LoggerBuffer)
                            + LoggerInfo->LoggerName.Length + sizeof(WCHAR)
                            + LoggerInfo->LogFileName.Length + sizeof(WCHAR);
    }
    else {
        HeaderSize =  sizeof(LoggerBuffer)
                            + LoggerInfo->LoggerName.Length + sizeof(WCHAR)
                            + LoggerInfo->LogFileName.Length + sizeof(WCHAR)
                            + 8;
    }

    if (HeaderSize > BufferSize) {
        //
        //  Round it to the nearest power of 2 and check for max size 1 MB
        //
        double dTemp = log (HeaderSize / 1024.0) / log (2.0);
        ULONG lTemp = (ULONG) (dTemp + 0.99);
        HeaderSize = (1 << lTemp);
        if (HeaderSize > 1024) {
            NtClose(LogFile);
            LoggerInfo->LogFileHandle = NULL;
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            if (LoggerInfo->Checksum != NULL) {
                EtwpFree(LoggerInfo->Checksum);
            }
            LoggerInfo->Checksum = NULL;
            return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
        }
        LoggerInfo->BufferSize = HeaderSize;
        BufferSize = HeaderSize * 1024;
    }

    //
    // allocate a buffer to write logger header and process/thread
    // rundown information
    //
    Logger.LogFileHandle   = LogFile;
    Logger.BufferSize      = BufferSize;
    Logger.TimerResolution = SystemInfo.TimerResolution;
    Logger.BufferSpace     = EtwpAlloc(BufferSize);
    if (Logger.BufferSpace == NULL) {
        NtClose(LogFile);
        LoggerInfo->LogFileHandle = NULL;
        if (FileNameBuffer != NULL) {
            EtwpFree(FileNameBuffer);
        }
        if (LoggerInfo->Checksum != NULL) {
            EtwpFree(LoggerInfo->Checksum);
        }
        LoggerInfo->Checksum = NULL;
        return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
    }
    //
    // All returns for error cases must have EtwpFree(Logger.BufferSpace) also.
    //

    Logger.UsePerfClock = LoggerInfo->Wnode.ClientContext;

    // initialize buffer first
    RtlZeroMemory(Logger.BufferSpace, BufferSize);
    Buffer         = (PWMI_BUFFER_HEADER) Logger.BufferSpace;
    Buffer->Offset = sizeof(WMI_BUFFER_HEADER);
    if (TraceKernel) {
        Buffer->Wnode.Guid   = SystemTraceControlGuid;
    }
    else {
        Buffer->Wnode.Guid   = LoggerInfo->Wnode.Guid;
    }
    Buffer->Wnode.BufferSize = BufferSize;
    Buffer->ClientContext.Alignment = (UCHAR)WmiTraceAlignment;
    Buffer->Wnode.Flags      = WNODE_FLAG_TRACED_GUID;
    Buffer->BufferType       = WMI_BUFFER_TYPE_RUNDOWN;
    Buffer->BufferFlag       = WMI_BUFFER_FLAG_FLUSH_MARKER;

    if (bLogFileAppend) {
        ULONG CurrentPointerSize = sizeof(PVOID);
        if (KernelWow64) {
            CurrentPointerSize = sizeof(ULONG64);
        }
        if (AppendPointerSize != CurrentPointerSize) {
            NtClose(LogFile);
            LoggerInfo->LogFileHandle = NULL;
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            if (LoggerInfo->Checksum != NULL) {
                EtwpFree(LoggerInfo->Checksum);
            }
            LoggerInfo->Checksum = NULL;
            return EtwpSetDosError(ERROR_INVALID_PARAMETER);
        }
        Logger.BuffersWritten  = LoggerBuffer.LogFileHeader.BuffersWritten;
        EtwpSetFilePointer(LogFile, 0, NULL, FILE_END);
    }
    else {
        PTRACE_LOGFILE_HEADER LogfileHeader;
        LARGE_INTEGER CurrentTime;
        LARGE_INTEGER Frequency;
        ULONG CpuNum = 0, CpuSpeed;
        PPEB Peb;
        
        Status = NtQueryPerformanceCounter(&CurrentTime, &Frequency);

        Logger.BuffersWritten  = 0;
        if (!KernelWow64) {
            HeaderSize =  sizeof(TRACE_LOGFILE_HEADER)
                            + LoggerInfo->LoggerName.Length + sizeof(WCHAR)
                            + LoggerInfo->LogFileName.Length + sizeof(WCHAR);
        }
        else {
            HeaderSize =  sizeof(TRACE_LOGFILE_HEADER)
                            + LoggerInfo->LoggerName.Length + sizeof(WCHAR)
                            + LoggerInfo->LogFileName.Length + sizeof(WCHAR)
                            + 8;
        }
        LogfileHeader = (PTRACE_LOGFILE_HEADER)
                        EtwpGetTraceBuffer(
                            &Logger,
                            NULL,
                            EVENT_TRACE_GROUP_HEADER + EVENT_TRACE_TYPE_INFO,
                            HeaderSize
                            );
        if (LogfileHeader == NULL) {
            NtClose(LogFile);
            LoggerInfo->LogFileHandle = NULL;
            EtwpFree(Logger.BufferSpace);
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            if (LoggerInfo->Checksum != NULL) {
                EtwpFree(LoggerInfo->Checksum);
            }
            LoggerInfo->Checksum = NULL;
            return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
        }


        LogfileHeader->PerfFreq = Frequency;
        LogfileHeader->ReservedFlags = Logger.UsePerfClock;
        if (NT_SUCCESS(EtwpGetCpuSpeed(&CpuNum, &CpuSpeed))) {
            LogfileHeader->CpuSpeedInMHz = CpuSpeed;
        }

        //
        // Start and End Times are wall clock time
        //
        if (RefClock != NULL) {
            PSYSTEM_TRACE_HEADER Header;
            LogfileHeader->StartTime = RefClock->StartTime;
            Header = (PSYSTEM_TRACE_HEADER) ( (char *) LogfileHeader - 
                                                  sizeof(SYSTEM_TRACE_HEADER) );
            Header->SystemTime = RefClock->StartPerfClock;
        }
        else {
            LogfileHeader->StartTime.QuadPart = EtwpGetSystemTime();
        }

        Peb = NtCurrentPeb();

        LogfileHeader->BufferSize = BufferSize;
        LogfileHeader->VersionDetail.MajorVersion =
                                         (UCHAR)Peb->OSMajorVersion;
        LogfileHeader->VersionDetail.MinorVersion =
                                         (UCHAR)Peb->OSMinorVersion;
        LogfileHeader->VersionDetail.SubVersion = TRACE_VERSION_MAJOR;
        LogfileHeader->VersionDetail.SubMinorVersion = TRACE_VERSION_MINOR;
        LogfileHeader->ProviderVersion = Peb->OSBuildNumber;
        LogfileHeader->StartBuffers = 1;
        LogfileHeader->LogFileMode
                = LoggerInfo->LogFileMode & (~(EVENT_TRACE_REAL_TIME_MODE));
        LogfileHeader->NumberOfProcessors = SystemInfo.NumberOfProcessors;
        if (LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE)
        {
            LoggerInfo->NumberOfProcessors = SystemInfo.NumberOfProcessors;
        }
        LogfileHeader->MaximumFileSize = LoggerInfo->MaximumFileSize;

        LogfileHeader->TimerResolution = SystemInfo.TimerResolution;

        LogfileHeader->LoggerName = (PWCHAR) ( (PUCHAR) LogfileHeader
                                    + sizeof(TRACE_LOGFILE_HEADER) );
        LogfileHeader->LogFileName = (PWCHAR) ((PUCHAR)LogfileHeader->LoggerName
                                    + LoggerInfo->LoggerName.Length
                                    + sizeof (WCHAR));
        RtlCopyMemory(LogfileHeader->LoggerName,
                    LoggerInfo->LoggerName.Buffer,
                    LoggerInfo->LoggerName.Length + sizeof(WCHAR));
        RtlCopyMemory(LogfileHeader->LogFileName,
                    LoggerInfo->LogFileName.Buffer,
                    LoggerInfo->LogFileName.Length + sizeof(WCHAR));
        EtwpGetTimeZoneInformation(&LogfileHeader->TimeZone);
        LogfileHeader->PointerSize = sizeof(PVOID);
        if (KernelWow64) {
            LogfileHeader->PointerSize = sizeof(ULONG64);
        }

        if (LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
            LoggerInfo->Checksum = NULL;
        }
        else {
            LoggerInfo->Checksum = EtwpAlloc(
                            sizeof(WNODE_HEADER)
                          + sizeof(TRACE_LOGFILE_HEADER));
            if (LoggerInfo->Checksum != NULL) {
                PBYTE ptrChecksum = LoggerInfo->Checksum;
                RtlCopyMemory(ptrChecksum, Buffer, sizeof(WNODE_HEADER));
                ptrChecksum += sizeof(WNODE_HEADER);
                RtlCopyMemory(
                    ptrChecksum, LogfileHeader, sizeof(TRACE_LOGFILE_HEADER));
            }
            else {
                NtClose(LogFile);
                LoggerInfo->LogFileHandle = NULL;
                EtwpFree(Logger.BufferSpace);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                if (LoggerInfo->Checksum != NULL) {
                    EtwpFree(LoggerInfo->Checksum);
                }
                LoggerInfo->Checksum = NULL;
                return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
            }
        }
        if (KernelWow64) {
            // ****************** IMPORTANT!!! *********************************
            // We need to fix the kernel logfile header here so that the 
            // header looks like 64 bit structure under Wow64. After this point,
            // BuffersWritten and StartBuffers are updated in this function 
            // after this point, but they are below the troublesome pointers, 
            // so it's OK.
            ULONG FixedHeaderStatus = EtwpFixLogFileHeaderForWow64(
                                                            LoggerInfo, 
                                                            LogfileHeader
                                                            );
            if (FixedHeaderStatus != ERROR_SUCCESS) {
                NtClose(LogFile);
                LoggerInfo->LogFileHandle = NULL;
                EtwpFree(Logger.BufferSpace);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                if (LoggerInfo->Checksum != NULL) {
                    EtwpFree(LoggerInfo->Checksum);
                }
                LoggerInfo->Checksum = NULL;
                return EtwpSetDosError(FixedHeaderStatus);
            }
        }
    }

    //
    // Dump the hardware config to File at the Start if it is a kernel logger
    //
    if (!Update) {
        if (TraceKernel) {
            ULONG EnableFlags = LoggerInfo->EnableFlags;
            PPERFINFO_GROUPMASK PGroupMask;
            HeaderSize = sizeof (PERFINFO_GROUPMASK); 

            PGroupMask  = (PPERFINFO_GROUPMASK) 
                          EtwpGetTraceBuffer( &Logger,
                                              NULL, 
                                              EVENT_TRACE_GROUP_HEADER + EVENT_TRACE_TYPE_EXTENSION, 
                                              HeaderSize );

            if (PGroupMask == NULL) {
                NtClose(LogFile);
                LoggerInfo->LogFileHandle = NULL;
                EtwpFree(Logger.BufferSpace);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                if (LoggerInfo->Checksum != NULL) {
                    EtwpFree(LoggerInfo->Checksum);
                }
                LoggerInfo->Checksum = NULL;
                return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
            }

            RtlZeroMemory( PGroupMask, HeaderSize);

            if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
                PTRACE_ENABLE_FLAG_EXTENSION tFlagExt;

                tFlagExt = (PTRACE_ENABLE_FLAG_EXTENSION)
                           &LoggerInfo->EnableFlags;
                EnableFlags = *(PULONG)((PCHAR)LoggerInfo + tFlagExt->Offset);
                if (tFlagExt->Length) {
                    RtlCopyMemory( PGroupMask, 
                                   (PCHAR)LoggerInfo + tFlagExt->Offset, 
                                   tFlagExt->Length * sizeof(ULONG));
                }
            } else {
                PGroupMask->Masks[0] = EnableFlags;
            }

            EtwpDumpHardwareConfig(&Logger);
            EtwpProcessRunDown( &Logger, TRUE, EnableFlags );
        } 
        else {
            if(IsEqualGUID(&NtdllTraceGuid, &LoggerInfo->Wnode.Guid)  && 
                                                           IsHeapLogging(NULL)){
                //Currently the return status of DumpHeapSnapShot is ignored.
                DumpHeapSnapShot(&Logger);
            }
        }
    }

    Buffer = (PWMI_BUFFER_HEADER) Logger.BufferSpace;
    // flush the last buffer
    if ( (Buffer->Offset < Logger.BufferSize)     &&
         (Buffer->Offset > sizeof(WMI_BUFFER_HEADER)) )
    {
        RtlFillMemory(
                (char *) Buffer + Buffer->Offset,
                Logger.BufferSize - Buffer->Offset,
                0xFF);
        Status = NtWriteFile(
                LogFile,
                NULL,
                NULL,
                NULL,
                &IoStatus,
                Logger.BufferSpace,
                BufferSize,
                NULL,
                NULL);

        Logger.BuffersWritten++;
    }

    if ((LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) ) {
        // We need to write the number of StartBuffers in the
        // Circular Logfile header to process it properly.

        FileInfo.CurrentByteOffset.QuadPart =
                            LOGFILE_FIELD_OFFSET(StartBuffers);

        Status = NtSetInformationFile(
                             LogFile,
                             &IoStatus,
                             &FileInfo,
                             sizeof(FILE_POSITION_INFORMATION),
                             FilePositionInformation
                             );
        if (!NT_SUCCESS(Status)) {
            NtClose(LogFile);
            LoggerInfo->LogFileHandle = NULL;
            EtwpFree(Logger.BufferSpace);
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            if (LoggerInfo->Checksum != NULL) {
                EtwpFree(LoggerInfo->Checksum);
            }
            LoggerInfo->Checksum = NULL;
            return EtwpSetDosError(EtwpNtStatusToDosError(Status));
        }

        Status = NtWriteFile(
                            LogFile,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatus,
                            &Logger.BuffersWritten,
                            sizeof(ULONG),
                            NULL,
                            NULL
                            );
        if (NT_SUCCESS(Status)) {
            PTRACE_LOGFILE_HEADER pLogFileHeader;

            NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);

            //
            // update StartBuffers in Checksum
            //
            if ( !(LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE)) {
                // Checksum should have been allocated and copied 
                // if not a private logger
                if (LoggerInfo->Checksum == NULL) {
                    NtClose(LogFile);
                    LoggerInfo->LogFileHandle = NULL;
                    EtwpFree(Logger.BufferSpace);
                    if (FileNameBuffer != NULL) {
                        EtwpFree(FileNameBuffer);
                    }
                    if (LoggerInfo->Checksum != NULL) {
                        EtwpFree(LoggerInfo->Checksum);
                    }
                    LoggerInfo->Checksum = NULL;
                    return EtwpSetDosError(ERROR_INVALID_DATA);
                }
                pLogFileHeader = (PTRACE_LOGFILE_HEADER)
                       (((PUCHAR) LoggerInfo->Checksum) + sizeof(WNODE_HEADER));
                pLogFileHeader->StartBuffers = Logger.BuffersWritten;
            }
        }
    }

    //
    // As a last thing update the Number of BuffersWritten so far
    // in the header and also update the checksum. This is to prevent
    // Logger failing Update calls under high load.
    //

    FileInfo.CurrentByteOffset.QuadPart =
                    LOGFILE_FIELD_OFFSET(BuffersWritten);

    Status = NtSetInformationFile(
                             LogFile,
                             &IoStatus,
                             &FileInfo,
                             sizeof(FILE_POSITION_INFORMATION),
                             FilePositionInformation
                             );
    if (!NT_SUCCESS(Status)) {
        NtClose(LogFile);
        LoggerInfo->LogFileHandle = NULL;
        EtwpFree(Logger.BufferSpace);
        if (FileNameBuffer != NULL) {
            EtwpFree(FileNameBuffer);
        }
        if (LoggerInfo->Checksum != NULL) {
            EtwpFree(LoggerInfo->Checksum);
        }
        LoggerInfo->Checksum = NULL;
        return EtwpSetDosError(EtwpNtStatusToDosError(Status));
    }

    Status = NtWriteFile(
                        LogFile,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        &Logger.BuffersWritten,
                        sizeof(ULONG),
                        NULL,
                        NULL
                        );
    if (NT_SUCCESS(Status)) {
        PTRACE_LOGFILE_HEADER pLogFileHeader;

        NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);

        // update StartBuffers in Checksum
        //
        if ( !(LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE)) {
            // Checksum should have been allocated and copied 
            // if not a private logger
            if (LoggerInfo->Checksum == NULL) {
                NtClose(LogFile);
                LoggerInfo->LogFileHandle = NULL;
                EtwpFree(Logger.BufferSpace);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                if (LoggerInfo->Checksum != NULL) {
                    EtwpFree(LoggerInfo->Checksum);
                }
                LoggerInfo->Checksum = NULL;
                return EtwpSetDosError(ERROR_INVALID_DATA);
            }
            pLogFileHeader = (PTRACE_LOGFILE_HEADER)
                     (((PUCHAR) LoggerInfo->Checksum) + sizeof(WNODE_HEADER));
            pLogFileHeader->BuffersWritten = Logger.BuffersWritten;
        }
    }

    // Extend the file size if in PREALLOCATE mode
    if (LoggerInfo->MaximumFileSize && 
        (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_PREALLOCATE)) {
        IO_STATUS_BLOCK IoStatusBlock;
        FILE_END_OF_FILE_INFORMATION EOFInfo;
        if (!(LoggerInfo->LogFileMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE)) { // normal case

            EOFInfo.EndOfFile.QuadPart = ((ULONGLONG)LoggerInfo->MaximumFileSize) * (1024 * 1024);

            Status = NtSetInformationFile(LogFile,
                                          &IoStatusBlock,
                                          &EOFInfo,
                                          sizeof(FILE_END_OF_FILE_INFORMATION),
                                          FileEndOfFileInformation);
            if (!NT_SUCCESS(Status)) {
                NtClose(LogFile);
                LoggerInfo->LogFileHandle = NULL;
                EtwpFree(Logger.BufferSpace);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                if (LoggerInfo->Checksum != NULL) {
                    EtwpFree(LoggerInfo->Checksum);
                }
                LoggerInfo->Checksum = NULL;
                return EtwpSetDosError(EtwpNtStatusToDosError(Status));
            }
        }
        else { // Using KBytes as file size unit

            EOFInfo.EndOfFile.QuadPart = ((ULONGLONG)LoggerInfo->MaximumFileSize) * 1024;

            Status = NtSetInformationFile(LogFile,
                                          &IoStatusBlock,
                                          &EOFInfo,
                                          sizeof(FILE_END_OF_FILE_INFORMATION),
                                          FileEndOfFileInformation);
            if (!NT_SUCCESS(Status)) {
                NtClose(LogFile);
                LoggerInfo->LogFileHandle = NULL;
                EtwpFree(Logger.BufferSpace);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                if (LoggerInfo->Checksum != NULL) {
                    EtwpFree(LoggerInfo->Checksum);
                }
                LoggerInfo->Checksum = NULL;
                return EtwpSetDosError(EtwpNtStatusToDosError(Status));
            }
        }
    }

    NtClose(LogFile);

    LogFile = EtwpCreateFileW(
                 FileName,
                 GENERIC_WRITE,
                 FILE_SHARE_READ,
                 NULL,
                 OPEN_EXISTING,
                 FILE_FLAG_NO_BUFFERING,
                 NULL
                 );
    if (FileNameBuffer != NULL) {
        EtwpFree(FileNameBuffer);
    }
    EtwpFree(Logger.BufferSpace);

    if (LogFile == INVALID_HANDLE_VALUE) {
        if (LoggerInfo->Checksum != NULL) {
            EtwpFree(LoggerInfo->Checksum);
        }
        LoggerInfo->Checksum = NULL;
        return EtwpGetLastError();
    }
    LoggerInfo->LogFileHandle = LogFile;
    LoggerInfo->BuffersWritten = Logger.BuffersWritten;
    return ERROR_SUCCESS;
}

ULONG
WmiUnregisterGuids(
    IN WMIHANDLE WMIHandle,
    IN LPGUID    Guid,
    OUT ULONG64  *LoggerContext
)
/*++

Routine Description:

    This routine informs WMI that a data provider is no longer available
    to receive requests for the guids previously registered. WMI will
    unregister any guids registered with this handle.

Arguments:

    WMIHandle - Handle returned from WMIRegisterGuids that represents
                the guids whose data is not longer available.
    Guid -      Pointer to the control Guid which is unregistering

    LoggerContext - Returned value of the LoggerContext

Return Value:

    Returns status code

--*/
{
    ULONG Status;
    ULONG ReturnSize;
    WMIUNREGGUIDS UnregGuids;

    UnregGuids.RequestHandle.Handle64 = (ULONG64)WMIHandle;
    UnregGuids.Guid = *Guid;

    Status = EtwpSendWmiKMRequest(NULL,
                                         IOCTL_WMI_UNREGISTER_GUIDS,
                                         &UnregGuids,
                                         sizeof(WMIUNREGGUIDS),
                                         &UnregGuids,
                                         sizeof(WMIUNREGGUIDS),
                                         &ReturnSize,
                                         NULL);

    //
    // Once the Guid has been unregistered from the kernel we will not get
    // any new notifications. We still need to wait for any notifications
    // currently in progress. The following call will check for it and 
    // block until it is okay to delete the data structures. 
    //

    if (Status == ERROR_SUCCESS) 
    {
        Status = EtwpRemoveFromGNList(Guid, 
                                    (PVOID) WMIHandle);
    }

    EtwpSetDosError(Status);
    return(Status);

}

ULONG
EtwpFlushLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
/*++

Routine Description:

    This is the actual routine to communicate with the kernel to Flush
    the logger. All the required parameters must be in LoggerInfo.

    This is an internal routine and it assumes that the LoggerInfo
    structure has been set up correctly and does not perform any 
    additional checks on the structure. 

Arguments:

    LoggerInfo      The actual parameters to be passed to and return from
                    kernel.

Return Value:

    The status of performing the action requested.

--*/
{
    ULONG Status;
    ULONG BufferSize;
    PTRACE_ENABLE_CONTEXT pContext;

    if (LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
        Status = EtwpSendUmLogRequest(
                    WmiFlushLoggerCode,
                    LoggerInfo
                    );
    }
    else {

        Status = EtwpSendWmiKMRequest(
                    NULL,
                    IOCTL_WMI_FLUSH_LOGGER,
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    &BufferSize,
                    NULL
                    );
    }

    return EtwpSetDosError(Status);
}

NTSTATUS
EtwpGetCpuSpeed(
    OUT DWORD* CpuNum,
    OUT DWORD* CpuSpeed
    )
{
        PWCHAR Buffer = NULL;
    NTSTATUS Status;
        ULONG DataLength;
    DWORD Size = MAXSTR;
    HANDLE Handle = INVALID_HANDLE_VALUE;
    HRESULT hr;


        *CpuSpeed = 0;

        Buffer = RtlAllocateHeap (RtlProcessHeap(),0,DEFAULT_ALLOC_SIZE);
    if (Buffer == NULL) {
        return STATUS_NO_MEMORY;
    }

    hr = StringCbPrintfW(Buffer, 
                         DEFAULT_ALLOC_SIZE, 
                         L"%ws\\%u", CPU_ROOT, *CpuNum
                        );

    if (FAILED(hr) ) {
        RtlFreeHeap (RtlProcessHeap(),0,Buffer);
        return STATUS_NO_MEMORY;
    }

    Status = EtwpRegOpenKey(Buffer, &Handle);

    if (NT_SUCCESS(Status)) {
        StringCbCopyW(Buffer, DEFAULT_ALLOC_SIZE, MHZ_VALUE_NAME);
        Size = sizeof(DWORD);
        Status = EtwpRegQueryValueKey(Handle,
                                   (LPWSTR) Buffer,
                                   Size,
                                   CpuSpeed,
                                   &DataLength
                                   );
        NtClose(Handle);
    }

    RtlFreeHeap (RtlProcessHeap(),0,Buffer);
        
        return Status;
}

#endif


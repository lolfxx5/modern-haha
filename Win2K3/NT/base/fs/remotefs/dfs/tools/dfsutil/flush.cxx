//--------------------------------------------------------------------------
//
//  Copyright (C) 1999, Microsoft Corporation
//
//  File:       flush.cxx
//
//--------------------------------------------------------------------------

#define UNICODE 1

#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <shellapi.h>
#include <winldap.h>
#include <stdlib.h>
#include <dsgetdc.h>
#include <lm.h>
#include <lmdfs.h>
#include "struct.hxx"
#include "dfsfsctl.h"
#include "messages.h"
#include <dfsutil.hxx>
#include "misc.hxx"

WCHAR wszEverything[] = L"*";

DWORD
PktFlush(
    LPWSTR EntryPath)
{
    DWORD dwErr = STATUS_SUCCESS;
    NTSTATUS NtStatus;
    HANDLE DriverHandle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING DfsDriverName;
    ULONG Type = 0;

    DebugInformation((L"PktFlush(%ws)\r\n", EntryPath));

    if (EntryPath == NULL)
        EntryPath = wszEverything;

    DebugInformation((L"EntryPath=[%ws]\r\n", EntryPath));

    RtlInitUnicodeString(&DfsDriverName, DFS_DRIVER_NAME);

    InitializeObjectAttributes(
        &objectAttributes,
        &DfsDriverName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    NtStatus = NtCreateFile(
                    &DriverHandle,
                    SYNCHRONIZE | FILE_WRITE_DATA,
                    &objectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_OPEN_IF,
                    FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0);

    if (!NT_SUCCESS(NtStatus)) {
        dwErr = RtlNtStatusToDosError(NtStatus);
        DebugInformation((L"NtCreateFile returned 0x%x\r\n", NtStatus));
        goto Cleanup;
    }

    NtStatus = NtFsControlFile(
                   DriverHandle,
                   NULL,       // Event,
                   NULL,       // ApcRoutine,
                   NULL,       // ApcContext,
                   &IoStatusBlock,
                   FSCTL_DFS_PKT_FLUSH_CACHE,
                   EntryPath,
                   wcslen(EntryPath) * sizeof(WCHAR),
                   NULL,
                   0);

    NtClose(DriverHandle);

    if (!NT_SUCCESS(NtStatus)) {
        DebugInformation((L"NtFsControlFile returned 0x%x\r\n", NtStatus));
        ErrorMessage(NtStatus);
    } else {
        CommandSucceeded = TRUE;
    }

    dwErr = RtlNtStatusToDosError(NtStatus);

Cleanup:

    if (dwErr != ERROR_SUCCESS)
        DebugInformation((L"PktFlush exit %d\r\n", dwErr));

    return(dwErr);
}

DWORD
SpcFlush(
    LPWSTR EntryPath)
{
    DWORD dwErr = STATUS_SUCCESS;
    NTSTATUS NtStatus;
    HANDLE DriverHandle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING DfsDriverName;
    ULONG Type = 0;

    DebugInformation((L"SpcFlush(%ws)\r\n", EntryPath));

    if (EntryPath == NULL)
        EntryPath = wszEverything;

    RtlInitUnicodeString(&DfsDriverName, DFS_DRIVER_NAME);

    InitializeObjectAttributes(
        &objectAttributes,
        &DfsDriverName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    NtStatus = NtCreateFile(
                    &DriverHandle,
                    SYNCHRONIZE | FILE_WRITE_DATA,
                    &objectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_OPEN_IF,
                    FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0);

    if (!NT_SUCCESS(NtStatus)) {
        dwErr = RtlNtStatusToDosError(NtStatus);
        DebugInformation((L"NtCreateFile returned 0x%x\r\n", NtStatus));
        goto Cleanup;
    }

    DebugInformation((L"EntryPath=[%ws]\r\n", EntryPath));

    NtStatus = NtFsControlFile(
                   DriverHandle,
                   NULL,       // Event,
                   NULL,       // ApcRoutine,
                   NULL,       // ApcContext,
                   &IoStatusBlock,
                   FSCTL_DFS_PKT_FLUSH_SPC_CACHE,
                   EntryPath,
                   wcslen(EntryPath) * sizeof(WCHAR),
                   NULL,
                   0);

    NtClose(DriverHandle);

    if (!NT_SUCCESS(NtStatus)) {
        DebugInformation((L"NtFsControlFile returned 0x%x\r\n", NtStatus));
    } else {
        CommandSucceeded = TRUE;
    }
    dwErr = RtlNtStatusToDosError(NtStatus);

Cleanup:

    if (dwErr != ERROR_SUCCESS)
        DebugInformation((L"SpcFlush exit %d\r\n", dwErr));

    return(dwErr);
}


DWORD
PurgeMupCache(
    LPWSTR ServerName)
{
    DWORD dwErr = STATUS_SUCCESS;
    NTSTATUS NtStatus;
    HANDLE DriverHandle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING DfsDriverName;
    ULONG Type = 0;

    //
    // Currently no server name is ever sent.
    //
    DebugInformation((L"PurgeMupCache(%ws)\r\n", ServerName));

    if (ServerName == NULL)
        ServerName = L"*";

    RtlInitUnicodeString(&DfsDriverName, DFS_DRIVER_NAME);

    InitializeObjectAttributes(
        &objectAttributes,
        &DfsDriverName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    NtStatus = NtCreateFile(
                    &DriverHandle,
                    SYNCHRONIZE | FILE_WRITE_DATA,
                    &objectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_OPEN_IF,
                    FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0);

    if (!NT_SUCCESS(NtStatus)) {
        dwErr = RtlNtStatusToDosError(NtStatus);
        DebugInformation((L"NtCreateFile returned 0x%x\r\n", NtStatus));
        goto Cleanup;
    }

    DebugInformation((L"ServerName=[%ws]\r\n", ServerName));

    NtStatus = NtFsControlFile(
                   DriverHandle,
                   NULL,       // Event,
                   NULL,       // ApcRoutine,
                   NULL,       // ApcContext,
                   &IoStatusBlock,
                   FSCTL_DFS_CSC_SERVER_ONLINE,
                   ServerName,
                   wcslen(ServerName) * sizeof(WCHAR),
                   NULL,
                   0);

    NtClose(DriverHandle);

    if (!NT_SUCCESS(NtStatus)) {
        DebugInformation((L"NtFsControlFile returned 0x%x\r\n", NtStatus));
    } else {
        CommandSucceeded = TRUE;
    }
    dwErr = RtlNtStatusToDosError(NtStatus);

Cleanup:

    if (dwErr != ERROR_SUCCESS)
        DebugInformation((L"PurgeMupCache exit %d\r\n", dwErr));

    return(dwErr);
}


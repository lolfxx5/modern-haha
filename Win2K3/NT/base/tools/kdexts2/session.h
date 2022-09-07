/*++

Copyright (c) 2000  Microsoft Corporation

Module Name:

    session.hxx

Abstract:

    This file declares routines to handle sessions.

Author:

    Jason Hartman (JasonHa) 2000-12-21

Environment:

    User Mode

--*/

#ifndef _SESSION_H_
#define _SESSION_H_


#define CURRENT_SESSION -1
#define DEFAULT_SESSION -2
#define INVALID_SESSION -3

typedef HRESULT (* PoolFilterFunc)(
    ULONG64 PoolAddr,
    ULONG TagFilter,
    ULONG64 PoolHeader,
    PDEBUG_VALUE Tag,
    ULONG BlockSize,
    BOOL bQuotaWithTag,
    PVOID Context
    );


typedef struct _ALLOCATION_STATS {
    ULONG AllocatedPages;
    ULONG LargePages;
    ULONG LargeAllocs;
    ULONG FreePages;
    ULONG ExpansionPages;
    ULONG Allocated;                // Number of allocated entries
    ULONG AllocatedSize;            // Size in Pool Blocks
    ULONG Free;                     // Number of free entries
    ULONG FreeSize;                 // Size in Pool Blocks
    ULONG Indeterminate;            // Number of entries with interdeterminable alloc/free status
    ULONG IndeterminateSize;        // Size in Pool Blocks
} ALLOCATION_STATS, *PALLOCATION_STATS;


extern ULONG   SessionId;
extern CHAR    SessionStr[16];


void SessionInit(PDEBUG_CLIENT Client);
void SessionExit();


HRESULT
GetSessionPhysicalAddress(
    PDEBUG_CLIENT Client,
    ULONG Session,
    ULONG64 VirtAddr,
    PULONG64 PhysAddr
    );


HRESULT
GetCurrentSession(
    PULONG64 CurSessionSpace,
    PULONG CurSessionId
    );


HRESULT
GetSessionSpace(
    ULONG Session,
    PULONG64 SessionSpace,
    PULONG64 SessionProcess
    );



#define SEARCH_POOL_NONPAGED        0x0001
#define SEARCH_POOL_PAGED           0x0002
#define SEARCH_POOL_LARGE_ONLY      0x0004
#define SEARCH_POOL_PRINT_LARGE     0x0008
#define SEARCH_POOL_PRINT_UNREAD    0x0010

#define INVALID_UNIQUE_STATE    0

HRESULT
SearchSessionPool(
    PDEBUG_CLIENT Client,
    ULONG Session,
    ULONG TagName,
    FLONG Flags,
    ULONG64 RestartAddr,
    PoolFilterFunc Filter,
    PALLOCATION_STATS AllocStats,
    PVOID Context
    );


#endif  _SESSION_H_


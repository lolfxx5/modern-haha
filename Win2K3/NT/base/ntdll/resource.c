/*++

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    resource.c

Abstract:

    This module implements the executive functions to acquire and release
    a shared resource.

Author:

    Mark Lucovsky       (markl)     04-Aug-1989

Environment:

    These routines are statically linked in the caller's executable and
    are callable only from user mode.  They make use of Nt system
    services.

Revision History:

--*/

#include "ldrp.h"

#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <heap.h>
#include "wmiumkm.h"
#include "NtdllTrc.h"


//
// Define the desired access for semaphores.
//

#define DESIRED_EVENT_ACCESS \
                (EVENT_QUERY_STATE | EVENT_MODIFY_STATE | SYNCHRONIZE)

#define DESIRED_SEMAPHORE_ACCESS \
                (SEMAPHORE_QUERY_STATE | SEMAPHORE_MODIFY_STATE | SYNCHRONIZE)

VOID RtlDumpResource( IN PRTL_RESOURCE Resource );

extern BOOLEAN LdrpShutdownInProgress;
extern HANDLE LdrpShutdownThreadId;

RTL_CRITICAL_SECTION DeferredCriticalSection;

HANDLE GlobalKeyedEventHandle=NULL;

//#define RTLP_USE_GLOBAL_KEYED_EVENT 1

#define RtlpIsKeyedEvent(xxHandle) ((((ULONG_PTR)xxHandle)&1) != 0)
#define RtlpSetKeyedEventHandle(xxHandle) ((HANDLE)(((ULONG_PTR)xxHandle)|1))

#define MAX_SLIST_DEPTH 10


BOOLEAN
ProtectHandle (
    IN HANDLE hObject
    )
{
    NTSTATUS Status;
    OBJECT_HANDLE_FLAG_INFORMATION HandleInfo;

    Status = NtQueryObject (hObject,
                            ObjectHandleFlagInformation,
                            &HandleInfo,
                            sizeof (HandleInfo),
                            NULL);

    if (NT_SUCCESS(Status)) {

        HandleInfo.ProtectFromClose = TRUE;

        Status = NtSetInformationObject (hObject,
                                         ObjectHandleFlagInformation,
                                         &HandleInfo,
                                         sizeof (HandleInfo));
        if (NT_SUCCESS( Status )) {
            return TRUE;
        }
    }

    return FALSE;
}


BOOLEAN
UnProtectHandle (
    IN HANDLE hObject
    )
{
    NTSTATUS Status;
    OBJECT_HANDLE_FLAG_INFORMATION HandleInfo;

    Status = NtQueryObject (hObject,
                            ObjectHandleFlagInformation,
                            &HandleInfo,
                            sizeof (HandleInfo),
                            NULL);

    if (NT_SUCCESS(Status)) {

        HandleInfo.ProtectFromClose = FALSE;

        Status = NtSetInformationObject (hObject,
                                         ObjectHandleFlagInformation,
                                         &HandleInfo,
                                         sizeof (HandleInfo));
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }
    }

    return FALSE;
}


SLIST_HEADER RtlCriticalSectionDebugSList;

RTL_CRITICAL_SECTION_DEBUG RtlpStaticDebugInfo[64];
PRTL_CRITICAL_SECTION_DEBUG RtlpStaticDebugInfoEnd;


PVOID
RtlpAllocateDebugInfo (
    VOID
    )
{
    PVOID p;

    p = RtlInterlockedPopEntrySList (&RtlCriticalSectionDebugSList);

    if (p == NULL) {

        p = RtlAllocateHeap (NtCurrentPeb()->ProcessHeap,
                             0,
                             sizeof(RTL_CRITICAL_SECTION_DEBUG));

        if (p == NULL) {
            KdPrint(( "NTDLL: Unable to allocate debug information from heap\n"));
        }
    }

    return p;
}


VOID
RtlpFreeDebugInfo (
    IN PRTL_CRITICAL_SECTION_DEBUG DebugInfo
    )
{
    ASSERT (RtlpStaticDebugInfoEnd != NULL);

    if ((RtlQueryDepthSList (&RtlCriticalSectionDebugSList) < MAX_SLIST_DEPTH) ||
        ((DebugInfo >= RtlpStaticDebugInfo) &&
         (DebugInfo < RtlpStaticDebugInfoEnd))) {

        RtlInterlockedPushEntrySList (&RtlCriticalSectionDebugSList,
                                      (PSLIST_ENTRY) DebugInfo);
    } else {

        //
        // Free it to the heap.
        //

        RtlFreeHeap(NtCurrentPeb()->ProcessHeap, 0, DebugInfo);
    }

    return;
}


NTSTATUS
RtlpInitDeferredCriticalSection (
    VOID
    )
{
    ULONG Size;
    NTSTATUS Status;
    PRTL_CRITICAL_SECTION_DEBUG p;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING Name;
    HANDLE Handle;


    //
    // Open the global out of memory keyed event if it's not already set up.
    //

    Name.Buffer = L"\\KernelObjects\\CritSecOutOfMemoryEvent";
    Name.Length = sizeof (L"\\KernelObjects\\CritSecOutOfMemoryEvent") - sizeof (WCHAR);
    InitializeObjectAttributes (&oa, &Name, 0, NULL, NULL);

    Status = NtOpenKeyedEvent (&Handle,
                               KEYEDEVENT_WAKE|KEYEDEVENT_WAIT,
                               &oa);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

#if DBG
    ProtectHandle (Handle);
#endif // DBG

    GlobalKeyedEventHandle = RtlpSetKeyedEventHandle (Handle);


    InitializeListHead (&RtlCriticalSectionList);

    //
    // Create the critical section SLIST.  Populate it with initial entries
    // so it can be used by ntdll routines before the first heap is created.
    //

    RtlInitializeSListHead (&RtlCriticalSectionDebugSList);

    Size = sizeof (RtlpStaticDebugInfo) / sizeof (RtlpStaticDebugInfo[0]);

    RtlpStaticDebugInfoEnd = RtlpStaticDebugInfo + Size;

    //
    // Chain all the free blocks together and then push them as a set.
    //
    for (p = RtlpStaticDebugInfo; p <= RtlpStaticDebugInfo + Size - 2; p++) {
        ((PSLIST_ENTRY) p)->Next = (PSLIST_ENTRY)(p + 1);
    }

    ((PSLIST_ENTRY) p)->Next = NULL;
    

    InterlockedPushListSList (&RtlCriticalSectionDebugSList,
                              (PSLIST_ENTRY) &RtlpStaticDebugInfo[0],
                              (PSLIST_ENTRY) &RtlpStaticDebugInfo[Size - 1],
                              Size);

    //
    // Initialize the critical section lock & the deferred critical section.
    //

    ASSERT (sizeof (RTL_CRITICAL_SECTION_DEBUG) == sizeof (RTL_RESOURCE_DEBUG));

    Status = RtlInitializeCriticalSectionAndSpinCount (&RtlCriticalSectionLock,
                                                       1000);

    if (NT_SUCCESS (Status)) {
        Status = RtlInitializeCriticalSectionAndSpinCount (&DeferredCriticalSection,
                                                           1000);
    }

    return Status;
}


BOOLEAN
NtdllOkayToLockRoutine(
    IN PVOID Lock
    )
{
    UNREFERENCED_PARAMETER (Lock);

    return TRUE;
}


VOID
RtlInitializeResource (
    IN PRTL_RESOURCE Resource
    )

/*++

Routine Description:

    This routine initializes the input resource variable

Arguments:

    Resource - Supplies the resource variable being initialized

Return Value:

    None

--*/

{
    NTSTATUS Status;
    PRTL_RESOURCE_DEBUG ResourceDebugInfo;
    ULONG SpinCount;

    //
    //  Initialize the lock fields, the count indicates how many are waiting
    //  to enter or are in the critical section, LockSemaphore is the object
    //  to wait on when entering the critical section.  SpinLock is used
    //  for the add interlock instruction.
    //

    SpinCount = 1024 * (NtCurrentPeb()->NumberOfProcessors - 1);
    if (SpinCount > 12000) {
        SpinCount = 12000;
    }

    Status = RtlInitializeCriticalSectionAndSpinCount (&Resource->CriticalSection, SpinCount);
    if (!NT_SUCCESS (Status)) {
        RtlRaiseStatus(Status);
    }

    Resource->CriticalSection.DebugInfo->Type = RTL_RESOURCE_TYPE;
    ResourceDebugInfo = (PRTL_RESOURCE_DEBUG) RtlpAllocateDebugInfo();

    if (ResourceDebugInfo == NULL) {
        RtlDeleteCriticalSection (&Resource->CriticalSection);
        RtlRaiseStatus(STATUS_NO_MEMORY);
    }

    //
    // NOTICE-2002/03/21-ELi
    // Only the ResourceDebugInfo->ContentionCount is used by the resource
    // code, so the other fields are not initialized
    //
    ResourceDebugInfo->ContentionCount = 0;
    Resource->DebugInfo = ResourceDebugInfo;

    //
    //  Initialize flags so there is a default value.
    //  (Some apps may set RTL_RESOURCE_FLAGS_LONG_TERM to affect timeouts.)
    //

    Resource->Flags = 0;


    //
    //  Initialize the shared and exclusive waiting counters and semaphore.
    //  The counters indicate how many are waiting for access to the resource
    //  and the semaphores are used to wait on the resource.  Note that
    //  the semaphores can also indicate the number waiting for a resource
    //  however there is a race condition in the alogrithm on the acquire
    //  side if count if not updated before the critical section is exited.
    //

    Status = NtCreateSemaphore (&Resource->SharedSemaphore,
                                DESIRED_SEMAPHORE_ACCESS,
                                NULL,
                                0,
                                MAXLONG);

    if (!NT_SUCCESS(Status)) {
        RtlDeleteCriticalSection (&Resource->CriticalSection);
        RtlpFreeDebugInfo (Resource->DebugInfo);
        RtlRaiseStatus (Status);
    }

    Resource->NumberOfWaitingShared = 0;

    Status = NtCreateSemaphore (&Resource->ExclusiveSemaphore,
                                DESIRED_SEMAPHORE_ACCESS,
                                NULL,
                                0,
                                MAXLONG);

    if (!NT_SUCCESS(Status)) {
        RtlDeleteCriticalSection (&Resource->CriticalSection);
        NtClose (Resource->SharedSemaphore);
        RtlpFreeDebugInfo (Resource->DebugInfo);
        RtlRaiseStatus (Status);
    }

    Resource->NumberOfWaitingExclusive = 0;

    //
    //  Initialize the current state of the resource
    //

    Resource->NumberOfActive = 0;

    Resource->ExclusiveOwnerThread = NULL;

    return;
}


VOID
RtlpPossibleDeadlock (
    IN PVOID ResourceOrCritSect
    )
{
    PIMAGE_NT_HEADERS NtHeaders;
    EXCEPTION_RECORD ExceptionRecord;

    NtHeaders = RtlImageNtHeader (NtCurrentPeb()->ImageBaseAddress);

    //
    // If the image is a Win32 image, then raise an exception
    // and try to get to the uae popup.
    //

    try {
        if (NtHeaders != NULL &&
            (NtHeaders->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI ||
             NtHeaders->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)) {

            ExceptionRecord.ExceptionCode = STATUS_POSSIBLE_DEADLOCK;
            ExceptionRecord.ExceptionFlags = 0;
            ExceptionRecord.ExceptionRecord = NULL;
            ExceptionRecord.ExceptionAddress = (PVOID)(ULONG_PTR) RtlRaiseException;
            ExceptionRecord.NumberParameters = 1;
            ExceptionRecord.ExceptionInformation[0] = (ULONG_PTR)ResourceOrCritSect;
            RtlRaiseException(&ExceptionRecord);
        } else {
            DbgBreakPoint();
        }
    } finally {
        ASSERT (!AbnormalTermination ());
#if !DBG
        if (AbnormalTermination ()) {

            //
            // Somebody tried to handle this exception.
            // This is illegal as it will corrupt the critical section.
            //

            DbgBreakPoint ();
        }
#endif
    }

    return;
}


BOOLEAN
RtlAcquireResourceShared (
    IN PRTL_RESOURCE Resource,
    IN BOOLEAN Wait
    )

/*++

Routine Description:

    The routine acquires the resource for shared access.  Upon return from
    the procedure the resource is acquired for shared access.

Arguments:

    Resource - Supplies the resource to acquire

    Wait - Indicates if the call is allowed to wait for the resource
        to become available or must return immediately

Return Value:

    BOOLEAN - TRUE if the resource is acquired and FALSE otherwise

--*/

{
    NTSTATUS Status;
    ULONG TimeoutCount;
    PLARGE_INTEGER TimeoutTime;

    //
    //  Enter the critical section
    //

    RtlEnterCriticalSection (&Resource->CriticalSection);

    //
    //  If it is not currently acquired for exclusive use then we can acquire
    //  the resource for shared access.  Note that this can potentially
    //  starve an exclusive waiter however, this is necessary given the
    //  ability to recursively acquire the resource shared.  Otherwise we
    //  might/will reach a deadlock situation where a thread tries to acquire
    //  the resource recursively shared but is blocked by an exclusive waiter.
    //
    //  The test to reenable not starving an exclusive waiter is:
    //
    //      if ((Resource->NumberOfWaitingExclusive == 0) &&
    //          (Resource->NumberOfActive >= 0)) {
    //

    if (Resource->NumberOfActive >= 0) {

        //
        //  The resource is ours, so indicate that we have it and
        //  exit the critical section
        //

        Resource->NumberOfActive += 1;

        RtlLeaveCriticalSection(&Resource->CriticalSection);

    //
    //  Otherwise check to see if this thread is the one currently holding
    //  exclusive access to the resource.  And if it is then we change
    //  this shared request to an exclusive recursive request and grant
    //  access to the resource.
    //

    } else if (Resource->ExclusiveOwnerThread == NtCurrentTeb()->ClientId.UniqueThread) {

        //
        //  The resource is ours (recursively) so indicate that we have it
        //  and exit the critical section
        //

        Resource->NumberOfActive -= 1;

        RtlLeaveCriticalSection(&Resource->CriticalSection);

    //
    //  Otherwise we'll have to wait for access.
    //

    } else {

        //
        //  Check if we are allowed to wait or must return immediately, and
        //  indicate that we didn't acquire the resource
        //

        if (!Wait) {

            RtlLeaveCriticalSection(&Resource->CriticalSection);

            return FALSE;

        }

        //
        //  Otherwise we need to wait to acquire the resource.
        //  To wait we will increment the number of waiting shared,
        //  release the lock, and wait on the shared semaphore
        //

        Resource->NumberOfWaitingShared += 1;
        Resource->DebugInfo->ContentionCount += 1;

        RtlLeaveCriticalSection(&Resource->CriticalSection);

        TimeoutCount = 0;

rewait:
        if (Resource->Flags & RTL_RESOURCE_FLAG_LONG_TERM) {
            TimeoutTime = NULL;
        }
        else {
            TimeoutTime = &RtlpTimeout;
        }

        Status = NtWaitForSingleObject (Resource->SharedSemaphore,
                                        FALSE,
                                        TimeoutTime);

        if (Status == STATUS_TIMEOUT) {

            DbgPrint("RTL: Acquire Shared Sem Timeout %d(%I64u secs)\n",
                     TimeoutCount, TimeoutTime->QuadPart / (-10000000));

            DbgPrint("RTL: Resource at %p\n",Resource);

            TimeoutCount += 1;

            if (TimeoutCount > 2) {

                //
                // If the image is a Win32 image, then raise an exception
                // and try to get to the uae popup
                //

                RtlpPossibleDeadlock ((PVOID) Resource);
            }
            DbgPrint("RTL: Re-Waiting\n");
            goto rewait;
        }

        if (!NT_SUCCESS(Status)) {
            RtlRaiseStatus(Status);
        }
    }

    //
    //  Now the resource is ours, for shared access
    //

    return TRUE;
}


BOOLEAN
RtlAcquireResourceExclusive (
    IN PRTL_RESOURCE Resource,
    IN BOOLEAN Wait
    )

/*++

Routine Description:

    The routine acquires the resource for exclusive access.  Upon return from
    the procedure the resource is acquired for exclusive access.

Arguments:

    Resource - Supplies the resource to acquire

    Wait - Indicates if the call is allowed to wait for the resource
        to become available or must return immediately

Return Value:

    BOOLEAN - TRUE if the resource is acquired and FALSE otherwise

--*/

{
    NTSTATUS Status;
    ULONG TimeoutCount;
    PLARGE_INTEGER TimeoutTime;
    HANDLE Thread;
    
    Thread = NtCurrentTeb()->ClientId.UniqueThread;

    //
    //  Loop until the resource is ours or exit if we cannot wait.
    //

    do {

        //
        //  Enter the critical section
        //

        RtlEnterCriticalSection (&Resource->CriticalSection);

        //
        //  If there are no shared users and it is not currently acquired for
        //  exclusive use then we can acquire the resource for exclusive
        //  access.  We also can acquire it if the resource indicates exclusive
        //  access but there isn't currently an owner.
        //

        if ((Resource->NumberOfActive == 0)

                ||

            ((Resource->NumberOfActive == -1) &&
             (Resource->ExclusiveOwnerThread == NULL))) {

            //
            //  The resource is ours, so indicate that we have it and
            //  exit the critical section
            //

            Resource->NumberOfActive = -1;

            Resource->ExclusiveOwnerThread = Thread;

            RtlLeaveCriticalSection(&Resource->CriticalSection);

            return TRUE;

        }

        //
        //  Otherwise check to see if we already have exclusive access to the
        //  resource and can simply recursively acquire it again.
        //

        if (Resource->ExclusiveOwnerThread == Thread) {

            //
            //  The resource is ours (recursively) so indicate that we have it
            //  and exit the critical section
            //

            Resource->NumberOfActive -= 1;

            RtlLeaveCriticalSection(&Resource->CriticalSection);

            return TRUE;

        }

        //
        //  Check if we are allowed to wait or must return immediately, and
        //  indicate that we didn't acquire the resource
        //

        if (!Wait) {

            RtlLeaveCriticalSection(&Resource->CriticalSection);

            return FALSE;

        }

        //
        //  Otherwise we need to wait to acquire the resource.
        //  To wait we will increment the number of waiting exclusive,
        //  release the lock, and wait on the exclusive semaphore
        //

        Resource->NumberOfWaitingExclusive += 1;
        Resource->DebugInfo->ContentionCount += 1;

        RtlLeaveCriticalSection (&Resource->CriticalSection);

        TimeoutCount = 0;

rewait:
        if (Resource->Flags & RTL_RESOURCE_FLAG_LONG_TERM) {
            TimeoutTime = NULL;
        }
        else {
            TimeoutTime = &RtlpTimeout;
        }

        Status = NtWaitForSingleObject (Resource->ExclusiveSemaphore,
                                        FALSE,
                                        TimeoutTime);

        if (Status == STATUS_TIMEOUT) {

            DbgPrint("RTL: Acquire Exclusive Sem Timeout %d (%I64u secs)\n",
                     TimeoutCount, TimeoutTime->QuadPart / (-10000000));

            DbgPrint("RTL: Resource at %p\n",Resource);

            TimeoutCount += 1;

            if (TimeoutCount > 2) {

                //
                // If the image is a Win32 image, then raise an exception
                // and try to get to the uae popup.
                //

                RtlpPossibleDeadlock ((PVOID) Resource);
            }

            DbgPrint("RTL: Re-Waiting\n");
            goto rewait;
        }
        if (!NT_SUCCESS(Status)) {
            RtlRaiseStatus(Status);
        }
    } while (TRUE);
}


VOID
RtlReleaseResource (
    IN PRTL_RESOURCE Resource
    )

/*++

Routine Description:

    This routine release the input resource.  The resource can have been
    acquired for either shared or exclusive access.

Arguments:

    Resource - Supplies the resource to release

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    LONG PreviousCount;

    //
    //  Enter the critical section
    //

    RtlEnterCriticalSection (&Resource->CriticalSection);

    //
    //  Test if the resource is acquired for shared or exclusive access
    //

    if (Resource->NumberOfActive > 0) {

        //
        //  Releasing shared access to the resource, so decrement
        //  the number of shared users
        //

        Resource->NumberOfActive -= 1;

        //
        //  If the resource is now available and there is a waiting
        //  exclusive user then give the resource to the waiting thread
        //

        if ((Resource->NumberOfActive == 0) &&
            (Resource->NumberOfWaitingExclusive > 0)) {

            //
            //  Set the resource state to exclusive (but not owned),
            //  decrement the number of waiting exclusive, and release
            //  one exclusive waiter
            //

            Resource->NumberOfActive = -1;
            Resource->ExclusiveOwnerThread = NULL;

            Resource->NumberOfWaitingExclusive -= 1;

            Status = NtReleaseSemaphore (Resource->ExclusiveSemaphore,
                                         1,
                                         &PreviousCount);

            if (!NT_SUCCESS(Status)) {
                RtlLeaveCriticalSection (&Resource->CriticalSection);
                RtlRaiseStatus(Status);
            }
        }

    } else if (Resource->NumberOfActive < 0) {

        //
        //  Releasing exclusive access to the resource, so increment the
        //  number of active by one.  And continue testing only
        //  if the resource is now available.
        //

        Resource->NumberOfActive += 1;

        if (Resource->NumberOfActive == 0) {

            //
            //  The resource is now available.  Remove ourselves as the
            //  owner thread
            //

            Resource->ExclusiveOwnerThread = NULL;

            //
            //  If there is another waiting exclusive then give the resource
            //  to it.
            //

            if (Resource->NumberOfWaitingExclusive > 0) {

                //
                //  Set the resource to exclusive, and its owner undefined.
                //  Decrement the number of waiting exclusive and release one
                //  exclusive waiter
                //

                Resource->NumberOfActive = -1;
                Resource->NumberOfWaitingExclusive -= 1;

                Status = NtReleaseSemaphore (Resource->ExclusiveSemaphore,
                                             1,
                                             &PreviousCount);

                if ( !NT_SUCCESS(Status) ) {
                    RtlLeaveCriticalSection (&Resource->CriticalSection);
                    RtlRaiseStatus(Status);
                }

            //
            //  Check to see if there are waiting shared, who should now get
            //  the resource
            //

            } else if (Resource->NumberOfWaitingShared > 0) {

                //
                //  Set the new state to indicate that all of the shared
                //  requesters have access and there are no more waiting
                //  shared requesters, and then release all of the shared
                //  requsters
                //

                Resource->NumberOfActive = Resource->NumberOfWaitingShared;

                Resource->NumberOfWaitingShared = 0;

                Status = NtReleaseSemaphore (Resource->SharedSemaphore,
                                             Resource->NumberOfActive,
                                             &PreviousCount);

                if (!NT_SUCCESS(Status)) {
                    RtlLeaveCriticalSection (&Resource->CriticalSection);
                    RtlRaiseStatus(Status);
                }
            }
        }

#if DBG
    } else {

        //
        //  The resource isn't currently acquired, there is nothing to release
        //  so tell the user the mistake
        //


        DbgPrint("NTDLL - Resource released too many times %lx\n", Resource);
        DbgBreakPoint();
#endif
    }

    //
    //  Exit the critical section, and return to the caller
    //

    RtlLeaveCriticalSection (&Resource->CriticalSection);

    return;
}


VOID
RtlConvertSharedToExclusive (
    IN PRTL_RESOURCE Resource
    )

/*++

Routine Description:

    This routine converts a resource acquired for shared access into
    one acquired for exclusive access.  Upon return from the procedure
    the resource is acquired for exclusive access

Arguments:

    Resource - Supplies the resource to acquire for shared access, it
        must already be acquired for shared access

Return Value:

    None

--*/

{
    NTSTATUS Status;
    ULONG TimeoutCount;
    HANDLE Thread;

    Thread = NtCurrentTeb()->ClientId.UniqueThread;

    //
    //  Enter the critical section
    //

    RtlEnterCriticalSection(&Resource->CriticalSection);

    //
    //  If there is only one shared user (it's us) and we can acquire the
    //  resource for exclusive access.
    //

    if (Resource->NumberOfActive == 1) {

        //
        //  The resource is ours, so indicate that we have it and
        //  exit the critical section, and return
        //

        Resource->NumberOfActive = -1;

        Resource->ExclusiveOwnerThread = Thread;

        RtlLeaveCriticalSection(&Resource->CriticalSection);

        return;
    }

    //
    //  If the resource is currently acquired exclusive and it's us then
    //  we already have exclusive access
    //

    if ((Resource->NumberOfActive < 0) &&
        (Resource->ExclusiveOwnerThread == Thread)) {

        //
        //  We already have exclusive access to the resource so we'll just
        //  exit the critical section and return
        //

        RtlLeaveCriticalSection(&Resource->CriticalSection);

        return;
    }

    //
    //  If the resource is acquired by more than one shared then we need
    //  to wait to get exclusive access to the resource
    //

    if (Resource->NumberOfActive > 1) {

        TimeoutCount = 0;

        //
        //  To wait we will decrement the fact that we have the resource for
        //  shared, and then loop waiting on the exclusive lock, and then
        //  testing to see if we can get exclusive access to the resource
        //

        Resource->NumberOfActive -= 1;

        while (TRUE) {

            //
            //  Increment the number of waiting exclusive, exit and critical
            //  section and wait on the exclusive semaphore
            //

            Resource->NumberOfWaitingExclusive += 1;
            Resource->DebugInfo->ContentionCount += 1;

            RtlLeaveCriticalSection(&Resource->CriticalSection);

            do {
                Status = NtWaitForSingleObject (Resource->ExclusiveSemaphore,
                                                FALSE,
                                                &RtlpTimeout);

                if (NT_SUCCESS(Status)) {

                    if (Status != STATUS_TIMEOUT) {

                        //
                        // Success !
                        //

                        break;
                    }

                    DbgPrint("RTL: Convert Exclusive Sem Timeout %d (%I64u secs)\n",
                        TimeoutCount, RtlpTimeout.QuadPart / (-10000000));

                    DbgPrint("RTL: Resource at %p\n",Resource);

                    TimeoutCount += 1;

                    if (TimeoutCount > 2) {

                        //
                        // If the image is a Win32 image, then raise an
                        // exception and try to get to the uae popup.
                        //

                        RtlpPossibleDeadlock ((PVOID) Resource);
                    }

                    DbgPrint("RTL: Re-Waiting\n");
                }
                else {
                    RtlRaiseStatus (Status);
                }

            } while (TRUE);

            //
            //  Enter the critical section
            //

            RtlEnterCriticalSection(&Resource->CriticalSection);

            //
            //  If there are no shared users and it is not currently acquired
            //  for exclusive use then we can acquire the resource for
            //  exclusive access.  We can also acquire it if the resource
            //  indicates exclusive access but there isn't currently an owner
            //

            if ((Resource->NumberOfActive == 0)

                    ||

                ((Resource->NumberOfActive == -1) &&
                 (Resource->ExclusiveOwnerThread == NULL))) {

                //
                //  The resource is ours, so indicate that we have it and
                //  exit the critical section and return.
                //

                Resource->NumberOfActive = -1;

                Resource->ExclusiveOwnerThread = NtCurrentTeb()->ClientId.UniqueThread;

                RtlLeaveCriticalSection(&Resource->CriticalSection);

                return;
            }

            //
            //  Otherwise check to see if we already have exclusive access to
            //  the resource and can simply recursively acquire it again.
            //

            if (Resource->ExclusiveOwnerThread == NtCurrentTeb()->ClientId.UniqueThread) {

                //
                //  The resource is ours (recursively) so indicate that we have
                //  it and exit the critical section and return.
                //

                Resource->NumberOfActive -= 1;

                RtlLeaveCriticalSection(&Resource->CriticalSection);

                return;
            }
        }
    }

    //
    //  The resource is not currently acquired for shared so this is a
    //  spurious call
    //

#if DBG
    DbgPrint("NTDLL:  Failed error - SHARED_RESOURCE_CONV_ERROR\n");
    DbgBreakPoint();
#endif
    RtlLeaveCriticalSection(&Resource->CriticalSection);
}


VOID
RtlConvertExclusiveToShared (
    IN PRTL_RESOURCE Resource
    )

/*++

Routine Description:

    This routine converts a resource acquired for exclusive access into
    one acquired for shared access.  Upon return from the procedure
    the resource is acquired for shared access

Arguments:

    Resource - Supplies the resource to acquire for shared access, it
        must already be acquired for exclusive access

Return Value:

    None

--*/

{
    LONG PreviousCount;
    NTSTATUS Status;

    //
    //  Enter the critical section
    //

    RtlEnterCriticalSection(&Resource->CriticalSection);

    //
    //  Release our exclusive access.
    //

    if (Resource->NumberOfActive == -1) {

        Resource->ExclusiveOwnerThread = NULL;

        //
        //  Check to see if there are waiting shared, who should now get the
        //  resource along with us
        //

        if (Resource->NumberOfWaitingShared > 0) {

            //
            //  Set the new state to indicate that all of the shared requesters
            //  have access including us, and there are no more waiting shared
            //  requesters, and then release all of the shared requsters
            //

            Resource->NumberOfActive = Resource->NumberOfWaitingShared + 1;

            Resource->NumberOfWaitingShared = 0;

            Status = NtReleaseSemaphore (Resource->SharedSemaphore,
                                         Resource->NumberOfActive - 1,
                                         &PreviousCount);

            if (!NT_SUCCESS(Status)) {
                RtlLeaveCriticalSection(&Resource->CriticalSection);
                RtlRaiseStatus(Status);
            }

        } else {

            //
            //  There is no one waiting for shared access so it's only ours
            //

            Resource->NumberOfActive = 1;
        }

    }
    else {

        //
        //  The resource is not currently acquired for exclusive, or we've
        //  recursively acquired it, so this must be a spurious call
        //

#if DBG
        DbgPrint("NTDLL:  Failed error - SHARED_RESOURCE_CONV_ERROR\n");
        DbgBreakPoint();
#endif
    }

    RtlLeaveCriticalSection(&Resource->CriticalSection);

    return;
}


VOID
RtlDeleteResource (
    IN PRTL_RESOURCE Resource
    )

/*++

Routine Description:

    This routine deletes (i.e., uninitializes) the input resource variable


Arguments:

    Resource - Supplies the resource variable being deleted

Return Value:

    None

--*/

{
    RtlDeleteCriticalSection (&Resource->CriticalSection);

    NtClose (Resource->SharedSemaphore);
    NtClose (Resource->ExclusiveSemaphore);

    RtlpFreeDebugInfo (Resource->DebugInfo);
    RtlZeroMemory (Resource, sizeof(*Resource));

    return;
}


VOID
RtlDumpResource (
    IN PRTL_RESOURCE Resource
    )

{
    DbgPrint("Resource @ %lx\n", Resource);

    DbgPrint(" NumberOfWaitingShared = %lx\n", Resource->NumberOfWaitingShared);
    DbgPrint(" NumberOfWaitingExclusive = %lx\n", Resource->NumberOfWaitingExclusive);

    DbgPrint(" NumberOfActive = %lx\n", Resource->NumberOfActive);

    return;
}


NTSTATUS
RtlInitializeCriticalSection (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )

/*++

Routine Description:

    This routine initializes the input critial section variable

Arguments:

    CriticalSection - Supplies the resource variable being initialized

Return Value:

    NTSTATUS.

--*/

{
    return RtlInitializeCriticalSectionAndSpinCount (CriticalSection, 0);
}



#define MAX_SPIN_COUNT          0x00ffffff
#define PREALLOCATE_EVENT_MASK  0x80000000


VOID
RtlEnableEarlyCriticalSectionEventCreation (
    VOID
    )
/*++

Routine Description:

    This routine marks the PEB of the calling process so critical section events
    are created at critical section creation time rather than at contention time.
    This allows critical processes not to have to worry about error paths later
    on at the expense of extra pool consumed.

Arguments:

    None

Return Value:

    None

--*/
{
    NtCurrentPeb ()->NtGlobalFlag |= FLG_CRITSEC_EVENT_CREATION;
}


NTSTATUS
RtlInitializeCriticalSectionAndSpinCount (
    IN PRTL_CRITICAL_SECTION CriticalSection,
    ULONG SpinCount
    )

/*++

Routine Description:

    This routine initializes the input critial section variable

Arguments:

    CriticalSection - Supplies the resource variable being initialized

    SpinCount - Supplies the spincount to use for MP collisions.

Return Value:

    NTSTATUS - Status of semaphore creation.

--*/

{
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;

    //
    //  Initialize the lock fields, the count indicates how many are waiting
    //  to enter or are in the critical section, LockSemaphore is the object
    //  to wait on when entering the critical section.  SpinLock is used
    //  for the add interlock instruction. Recursion count is the number of
    //  times the critical section has been recursively entered.
    //

    CriticalSection->LockCount = -1;
    CriticalSection->RecursionCount = 0;
    CriticalSection->OwningThread = 0;
    CriticalSection->LockSemaphore = 0;
    if ( NtCurrentPeb()->NumberOfProcessors > 1 ) {
        CriticalSection->SpinCount = SpinCount & MAX_SPIN_COUNT;
    } else {
        CriticalSection->SpinCount = 0;
    }

    ASSERT (GlobalKeyedEventHandle != NULL);

    //
    // Initialize debugging information.
    //

    DebugInfo = (PRTL_CRITICAL_SECTION_DEBUG) RtlpAllocateDebugInfo ();

    if (DebugInfo == NULL) {
        return STATUS_NO_MEMORY;
    }

    DebugInfo->Type = RTL_CRITSECT_TYPE;
    DebugInfo->ContentionCount = 0;
    DebugInfo->EntryCount = 0;

    //
    // It is important to set critical section pointers and potential
    // stack trace before we insert the resource in the process' 
    // resource list because the list can be randomly traversed from
    // other threads that check for orphaned resources.
    //

    DebugInfo->CriticalSection = CriticalSection;
    CriticalSection->DebugInfo = DebugInfo;

    //
    // Try to get a stack trace. If no trace database was created
    // then the log() function is a no op.
    //

    DebugInfo->CreatorBackTraceIndex = (USHORT) RtlLogStackBackTrace();

    //
    // If the critical section lock itself is not being initialized, then
    // synchronize the insert of the critical section in the process locks
    // list. Otherwise, insert the critical section with no synchronization.
    //

    if (CriticalSection != &RtlCriticalSectionLock) {

        RtlEnterCriticalSection(&RtlCriticalSectionLock);
        InsertTailList(&RtlCriticalSectionList, &DebugInfo->ProcessLocksList);
        RtlLeaveCriticalSection(&RtlCriticalSectionLock );

    } else {
        InsertTailList(&RtlCriticalSectionList, &DebugInfo->ProcessLocksList);
    }

    if (IsCritSecLogging(CriticalSection)) {

        PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
        PPERFINFO_TRACE_HEADER pEventHeader = NULL;
        USHORT ReqSize = sizeof(CRIT_SEC_INITIALIZE_EVENT_DATA) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

        AcquireBufferLocation (&pEventHeader, &pThreadLocalData, &ReqSize);

        if(pEventHeader && pThreadLocalData) {

            PCRIT_SEC_INITIALIZE_EVENT_DATA pCritSecInitializeEvent = (PCRIT_SEC_INITIALIZE_EVENT_DATA)
                ((SIZE_T)pEventHeader + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

            pEventHeader->Packet.Size = ReqSize;
            pEventHeader->Packet.HookId= (USHORT) PERFINFO_LOG_TYPE_CRITSEC_INITIALIZE;

            pCritSecInitializeEvent->Address = (PVOID)CriticalSection;
            pCritSecInitializeEvent->SpinCount = (PVOID)CriticalSection->SpinCount;

            ReleaseBufferLocation(pThreadLocalData);
        }
    }

    return STATUS_SUCCESS;
}


ULONG
RtlSetCriticalSectionSpinCount (
    IN PRTL_CRITICAL_SECTION CriticalSection,
    ULONG SpinCount
    )

/*++

Routine Description:

    This routine initializes the input critial section variable

Arguments:

    CriticalSection - Supplies the resource variable being initialized

Return Value:

    Returns the previous critical section spin count

--*/

{
    ULONG OldSpinCount;

    OldSpinCount = (ULONG)CriticalSection->SpinCount;

    if ( NtCurrentPeb()->NumberOfProcessors > 1 ) {
        CriticalSection->SpinCount = SpinCount & MAX_SPIN_COUNT;
    } else {
        CriticalSection->SpinCount = 0;
    }

    return OldSpinCount;
}


BOOLEAN
RtlpCreateCriticalSectionSem (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )
{
    NTSTATUS Status, Status1;
    HANDLE SemHandle;

#if defined (RTLP_USE_GLOBAL_KEYED_EVENT)
    Status = STATUS_INSUFFICIENT_RESOURCES;
    SemHandle = NULL;
#else
    Status = NtCreateEvent (&SemHandle,
                            DESIRED_EVENT_ACCESS,
                            NULL,
                            SynchronizationEvent,
                            FALSE);

#endif
    if (NT_SUCCESS (Status)) {
        if (InterlockedCompareExchangePointer (&CriticalSection->LockSemaphore,  SemHandle, NULL) != NULL) {
            Status1 = NtClose (SemHandle);
            ASSERT (NT_SUCCESS (Status1));
        } else {
#if DBG
            ProtectHandle(SemHandle);
#endif // DBG
        }
    } else {
        ASSERT (GlobalKeyedEventHandle != NULL);
        InterlockedCompareExchangePointer (&CriticalSection->LockSemaphore,
                                           GlobalKeyedEventHandle,
                                           NULL);
    }
    return TRUE;
}


VOID
RtlpCheckDeferredCriticalSection(
    IN PRTL_CRITICAL_SECTION CriticalSection
    )
{
    if (CriticalSection->LockSemaphore == NULL) {
        RtlpCreateCriticalSectionSem(CriticalSection);
    }
    return;
}


NTSTATUS
RtlDeleteCriticalSection (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )

/*++

Routine Description:

    This routine deletes (i.e., uninitializes) the input critical
    section variable


Arguments:

    CriticalSection - Supplies the resource variable being deleted

Return Value:

    NTSTATUS.

--*/

{
    NTSTATUS Status;
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
    HANDLE LockSemaphore;

    LockSemaphore = CriticalSection->LockSemaphore;

    if (LockSemaphore != NULL && !RtlpIsKeyedEvent (LockSemaphore)) {
#if DBG
        UnProtectHandle (LockSemaphore);
#endif // DBG
        Status = NtClose (LockSemaphore);
    } else {
        Status = STATUS_SUCCESS;
    }

    //
    // Remove critical section from the list
    //

    DebugInfo = NULL;
    RtlEnterCriticalSection (&RtlCriticalSectionLock);

    try {
        DebugInfo = CriticalSection->DebugInfo;
        if (DebugInfo != NULL) {
            RemoveEntryList (&DebugInfo->ProcessLocksList);
            RtlZeroMemory (DebugInfo, sizeof (*DebugInfo));
        }
    } finally {
        RtlLeaveCriticalSection (&RtlCriticalSectionLock);
    }
    if (DebugInfo != NULL) {
        RtlpFreeDebugInfo( DebugInfo );
    }
    RtlZeroMemory (CriticalSection,
                   sizeof(RTL_CRITICAL_SECTION));

    return Status;
}



//
// The following support routines are called from the machine language
// implementations of RtlEnterCriticalSection and RtlLeaveCriticalSection
// to execute the slow path logic of either waiting for a critical section
// or releasing a critical section to a waiting thread.
//
void
RtlpWaitForCriticalSection (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )
{
    NTSTATUS st;
    ULONG TimeoutCount = 0;
    PLARGE_INTEGER TimeoutTime;
    LOGICAL CsIsLoaderLock;
    HANDLE LockSemaphore;
    PTEB Teb;

    //
    // Critical sections are disabled during exit process so that
    // apps that are not careful during shutdown don't hang
    //

    CsIsLoaderLock = (CriticalSection == &LdrpLoaderLock);

    Teb = NtCurrentTeb ();
    Teb->WaitingOnLoaderLock = (ULONG)CsIsLoaderLock;

    if ( LdrpShutdownInProgress &&
        ((!CsIsLoaderLock) ||
         (CsIsLoaderLock && LdrpShutdownThreadId == Teb->ClientId.UniqueThread) ) ) {

        //
        // Slimy reinitialization of the critical section with the count
        // biased by one.  This is how the critical section would normally
        // look to the thread coming out of this function. Note that the
        // semaphore handle is leaked, but since the app is exiting, it's ok
        //

        CriticalSection->LockCount = 0;
        CriticalSection->RecursionCount = 0;
        CriticalSection->OwningThread = 0;
        CriticalSection->LockSemaphore = 0;

        Teb->WaitingOnLoaderLock = 0;

        return;
    }

    if (RtlpTimoutDisable) {
        TimeoutTime = NULL;
    } else {
        TimeoutTime = &RtlpTimeout;
    }

    LockSemaphore = CriticalSection->LockSemaphore;

    if (LockSemaphore == NULL) {
        RtlpCheckDeferredCriticalSection (CriticalSection);
        LockSemaphore = CriticalSection->LockSemaphore;
    }

    CriticalSection->DebugInfo->EntryCount++;

    do {

        CriticalSection->DebugInfo->ContentionCount++;

#if 0
        DbgPrint( "NTDLL: Waiting for CritSect: %p  owned by ThreadId: %X  Count: %u  Level: %u\n",
                  CriticalSection,
                  CriticalSection->OwningThread,
                  CriticalSection->LockCount,
                  CriticalSection->RecursionCount
                );
#endif

        if( IsCritSecLogging(CriticalSection)){

            PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
            PPERFINFO_TRACE_HEADER pEventHeader = NULL;
            USHORT ReqSize = sizeof(CRIT_SEC_COLLISION_EVENT_DATA) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

            AcquireBufferLocation(&pEventHeader, &pThreadLocalData, &ReqSize );

            if(pEventHeader && pThreadLocalData) {

                PCRIT_SEC_COLLISION_EVENT_DATA pCritSecCollEvent = (PCRIT_SEC_COLLISION_EVENT_DATA)( (SIZE_T)pEventHeader
                                                           +(SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

                pEventHeader->Packet.Size = ReqSize;
                pEventHeader->Packet.HookId= (USHORT) PERFINFO_LOG_TYPE_CRITSEC_COLLISION;

                pCritSecCollEvent->Address		    = (PVOID)CriticalSection;
                pCritSecCollEvent->SpinCount	    = (PVOID)CriticalSection->SpinCount;
                pCritSecCollEvent->LockCount	    = CriticalSection->LockCount;
                pCritSecCollEvent->OwningThread	    = (PVOID)CriticalSection->OwningThread;

                ReleaseBufferLocation(pThreadLocalData);
            }

        }

        if (!RtlpIsKeyedEvent (LockSemaphore)) {
            st = NtWaitForSingleObject (LockSemaphore,
                                        FALSE,
                                        TimeoutTime);
        } else {
            st = NtWaitForKeyedEvent (LockSemaphore,
                                      CriticalSection,
                                      FALSE,
                                      TimeoutTime);
        }
        if ( st == STATUS_TIMEOUT ) {

            //
            // This code path will be taken only if the TimeoutTime parameter for
            // Wait() was not null.
            //

            DbgPrint( "RTL: Enter Critical Section Timeout (%I64u secs) %d\n",
                      TimeoutTime->QuadPart / (-10000000), TimeoutCount
                    );
            DbgPrint( "RTL: Pid.Tid %x.%x, owner tid %x Critical Section %p - ContentionCount == %lu\n",
                    Teb->ClientId.UniqueProcess,
                    Teb->ClientId.UniqueThread,
                    CriticalSection->OwningThread,
                    CriticalSection, CriticalSection->DebugInfo->ContentionCount
                    );

            TimeoutCount += 1;

            if ((TimeoutCount > 2) && (CriticalSection != &LdrpLoaderLock)) {

                //
                // If the image is a Win32 image, then raise an exception
                // and try to get to the uae popup
                //

                RtlpPossibleDeadlock ((PVOID) CriticalSection);
            }
            DbgPrint("RTL: Re-Waiting\n");

        } else {

            if (NT_SUCCESS(st)) {

                //
                // If some errant thread calls SetEvent on a bogus handle
                // which happens to match the handle we are using in the
                // critical section, everything gets really messed up since
                // two threads now own the lock at the same time. ASSERT
                // that no other thread owns the lock if we have been
                // granted ownership.
                //

                ASSERT(CriticalSection->OwningThread == 0);

                if (CsIsLoaderLock) {
                    CriticalSection->OwningThread = Teb->ClientId.UniqueThread;
                    Teb->WaitingOnLoaderLock = 0;
                }
                return;
            }

            RtlRaiseStatus(st);
        }
    } while (TRUE);
}


VOID
RtlpUnWaitCriticalSection (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )
{
    NTSTATUS st;
    HANDLE LockSemaphore;

    LockSemaphore = CriticalSection->LockSemaphore;

    if (LockSemaphore == NULL) {
        RtlpCheckDeferredCriticalSection (CriticalSection);
        LockSemaphore = CriticalSection->LockSemaphore;
    }

    if (!RtlpIsKeyedEvent (LockSemaphore)) {
        st = NtSetEventBoostPriority (LockSemaphore);
    } else {
        st = NtReleaseKeyedEvent (LockSemaphore,
                                  CriticalSection,
                                  FALSE,
                                  0);
    }

    if (!NT_SUCCESS (st)) {
        RtlRaiseStatus(st);
    }
}


VOID
RtlpNotOwnerCriticalSection (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )
{
    LOGICAL CsIsLoaderLock;

    //
    // Critical sections are disabled during exit process so that
    // apps that are not careful during shutdown don't hang
    //

    CsIsLoaderLock = (CriticalSection == &LdrpLoaderLock);

    if ( LdrpShutdownInProgress &&
        ((!CsIsLoaderLock) ||
         (CsIsLoaderLock && LdrpShutdownThreadId == NtCurrentTeb()->ClientId.UniqueThread) ) ) {
        return;
    }

    if (NtCurrentPeb()->BeingDebugged) {
        DbgPrint( "NTDLL: Calling thread (%X) not owner of CritSect: %p  Owner ThreadId: %X\n",
                  NtCurrentTeb()->ClientId.UniqueThread,
                  CriticalSection,
                  CriticalSection->OwningThread
                );
        DbgBreakPoint();
    }

    RtlRaiseStatus (STATUS_RESOURCE_NOT_OWNED);
}


#if DBG

void
RtlpCriticalSectionIsOwned (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )
{
    //
    // The loader lock gets handled differently, so don't assert on it
    //

    if ((CriticalSection == &LdrpLoaderLock) &&
        (CriticalSection->OwningThread == NtCurrentTeb()->ClientId.UniqueThread))
        return;

    //
    // If we're being debugged, throw up a warning
    //

    if (NtCurrentPeb()->BeingDebugged) {
        DbgPrint( "NTDLL: Calling thread (%X) shouldn't enter CritSect: %p  Owner ThreadId: %X\n",
                  NtCurrentTeb()->ClientId.UniqueThread,
                  CriticalSection,
                  CriticalSection->OwningThread
                );
        DbgBreakPoint();
    }
}
#endif

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Critical section verifier
/////////////////////////////////////////////////////////////////////

//
// This variable enables the critical section verifier (abandoned locks,
// terminatethread() while holding locks, etc.). 
//

BOOLEAN RtlpCriticalSectionVerifier = FALSE;

//
// Settable from debugger to avoid a flurry of similar failures.
//

BOOLEAN RtlpCsVerifyDoNotBreak = FALSE;


VOID
RtlCheckHeldCriticalSections (
    IN HANDLE hThread,
    IN PRTL_CRITICAL_SECTION const *LocksHeld
    )
/*++

Routine Description:

    This routine is called to ensure that the given thread does not
    hold any locks other than the ones in a specified list of
    known-held locks.

Arguments:

    hThread     -- the thread to check

    LocksHeld   -- Locks known to be held by the thread

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    THREAD_BASIC_INFORMATION ThreadInfo;
    PLIST_ENTRY Entry;
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
    RTL_CRITICAL_SECTION_DEBUG ExtraDebugInfoCopy = {0}; // initialized because of W4 warning
    PRTL_CRITICAL_SECTION CriticalSection;
    RTL_CRITICAL_SECTION CritSectCopy = {0}; // initialized because of W4 warning
    PRTL_CRITICAL_SECTION const *LockHeld;
    BOOLEAN Copied;

    //
    // We do not check anything if critical section verifier is not on.
    //

    if (RtlpCriticalSectionVerifier == FALSE || RtlpCsVerifyDoNotBreak == TRUE ) {
        return;
    }

    //
    // We do not do anything if we are shutting down the process.
    //

    if (LdrpShutdownInProgress) {
        return;
    }

    //
    // Query the thread ID and TEB address.
    //

    Status = NtQueryInformationThread (hThread,
                                       ThreadBasicInformation,
                                       &ThreadInfo,
                                       sizeof(ThreadInfo),
                                       NULL);
    if (! NT_SUCCESS(Status)) {
        return;
    }

    //
    // Don't do anything else if number of owned critical sections
    // stored in the TEB is zero. The app verifier keeps this TEB
    // counter updated on all platforms instead of just x86 chk 
    // without the verifier, at least for critical sections that were not
    // entered from ntdll.
    //

    if (ThreadInfo.TebBaseAddress->CountOfOwnedCriticalSections < 1) {

        return;
    }

    //
    // Iterate the global list of critical sections
    //

    RtlEnterCriticalSection (&RtlCriticalSectionLock);

    try {

        for (Entry = RtlCriticalSectionList.Flink;
            Entry != &RtlCriticalSectionList;
            Entry = Entry->Flink) {

            DebugInfo = CONTAINING_RECORD (Entry,
                                           RTL_CRITICAL_SECTION_DEBUG,
                                           ProcessLocksList);

            CriticalSection = DebugInfo->CriticalSection;

            if (CriticalSection == &RtlCriticalSectionLock) {

                //
                // We know that we're holding this one, so there's no
                // problem -- continue on.
                //

                continue;
            }

            if (LocksHeld) {

                //
                // We have a list of okay-to-hold critical sections;
                // scan through it, looking for this critical section.
                // If we find it, we'll skip it and continue walking
                // ProcessLocksList.
                //
                
                for (LockHeld = LocksHeld;
                     *LockHeld;
                     LockHeld++) {

                    if (CriticalSection == *LockHeld) {

                        //
                        // We've found this critical section in the
                        // LocksHeld array.
                        //

                        break;
                    }
                }

                if (*LockHeld) {

                    //
                    // Our caller expected the thread to be holding
                    // this critical section; skip it, and continue
                    // walking through ProcessLocksList.
                    //

                    continue;
                }
            }

            Copied = TRUE;

            try {

                CritSectCopy = *CriticalSection;
            }
            except (EXCEPTION_EXECUTE_HANDLER) {

                Copied = FALSE;
            }

            if (Copied == FALSE) {

                //
                // Exception while reading the contents of the critsect.  
                // The critsect has probably been decommitted without a call to
                // RtlDeleteCriticalSection.
                //
                // You might think the entry could be deleted from the list,
                // but it can't... there may be another RTL_CRITICAL_SECTION out
                // there that is truly allocated, and whose DebugInfo pointer
                // points at this DebugInfo.  In that case, when that critsect
                // is deleted, the RtlCriticalSectionList is corrupted.
                //

                VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_IN_FREED_MEMORY,
                                "undeleted critical section in freed memory",
                                CriticalSection, "Critical section address",
                                DebugInfo, "Critical section debug info address",
                                RtlpGetStackTraceAddress (DebugInfo->CreatorBackTraceIndex), 
                                "Initialization stack trace. Use dds to dump it if non-NULL.",
                                NULL, "" );
            }
            else if(CritSectCopy.DebugInfo != DebugInfo) {

                //
                // Successfully read the critical section structure but
                // the current debug info field of this critical section
                // doesn't point to the current DebugInfo - it was probably
                // initialized more than one time or simply corrupted.
                // 
                // Try to make a copy of the DebugInfo currently pointed 
                // by our critical section. This might fail if the critical
                // section is corrupted.
                //

                Copied = TRUE;

                try {

                    ExtraDebugInfoCopy = *(CritSectCopy.DebugInfo);
                }
                except (EXCEPTION_EXECUTE_HANDLER) {

                    Copied = FALSE;
                }

                if (Copied == FALSE) {

                    //
                    // Exception while reading the contents of the debug info.
                    // The current critical section structure is corrupted.
                    //

                    VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_CORRUPTED,
                                   "corrupted critical section",
                                   CriticalSection, 
                                   "Critical section address",
                                   CritSectCopy.DebugInfo, 
                                   "Invalid debug info address of this critical section",
                                   DebugInfo, 
                                   "Address of the debug info found in the active list.",
                                   RtlpGetStackTraceAddress (DebugInfo->CreatorBackTraceIndex), 
                                   "Initialization stack trace. Use dds to dump it if non-NULL." );
                }
                else {

                    // 
                    // Successfully read this second debug info 
                    // of the same critical section. 
                    //

                    VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_DOUBLE_INITIALIZE,
                                   "double initialized or corrupted critical section",
                                   CriticalSection, 
                                   "Critical section address.",
                                   DebugInfo, 
                                   "Address of the debug info found in the active list.",
                                   RtlpGetStackTraceAddress (DebugInfo->CreatorBackTraceIndex), 
                                   "First initialization stack trace. Use dds to dump it if non-NULL.",
                                   RtlpGetStackTraceAddress (ExtraDebugInfoCopy.CreatorBackTraceIndex), 
                                   "Second initialization stack trace. Use dds to dump it if non-NULL.");
                }
            }
            else if (CritSectCopy.OwningThread == ThreadInfo.ClientId.UniqueThread
                     && CritSectCopy.LockCount != -1) {

                //
                // The thread has a critical section locked. Since this API is called
                // whenever the thread is in a context that does not allow this
                // we will have to report the issue (e.g. thread exits or is 
                // terminated, a thread pool work item finished, etc.).
                //

                VERIFIER_STOP (APPLICATION_VERIFIER_EXIT_THREAD_OWNS_LOCK,
                               "Thread is in a state in which it cannot own a critical section",
                               ThreadInfo.ClientId.UniqueThread, "Thread identifier",
                               CriticalSection, "Critical section address",
                               DebugInfo, "Critical section debug info address",
                               RtlpGetStackTraceAddress (DebugInfo->CreatorBackTraceIndex), "Initialization stack trace. Use dds to dump it if non-NULL." );
            }
        }
    }
    finally {

        //
        // Release the CS list lock.
        //

        RtlLeaveCriticalSection (&RtlCriticalSectionLock);
    }
}


VOID
RtlCheckForOrphanedCriticalSections (
    IN HANDLE hThread
    )
/*++

Routine Description:

    This routine is called from kernel32's ExitThread, TerminateThread
    and SuspendThread in an effort to track calls that kill threads while 
    they own critical sections.

Arguments:

    hThread     -- thread to be killed

Return Value:

    None.

--*/
{
    // The work is performed by RtlCheckHeldCriticalSections, which we
    // call with the following okay-to-be-held critical section.

    PRTL_CRITICAL_SECTION const LocksHeld[] = {
        &LdrpLoaderLock,
        NULL
    };

    RtlCheckHeldCriticalSections(hThread, LocksHeld);
}

/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    wait.c

Abstract:

    This module implements the generic kernel wait routines. Functions
    are provided to delay execution, wait for multiple objects, wait for
    a single object, and ot set a client event and wait for a server event.

    N.B. This module is written to be a fast as possible and not as small
        as possible. Therefore some code sequences are duplicated to avoid
        procedure calls. It would also be possible to combine wait for
        single object into wait for multiple objects at the cost of some
        speed. Since wait for single object is the most common case, the
        two routines have been separated.

Author:

    David N. Cutler (davec) 23-Mar-89

Environment:

    Kernel mode only.

Revision History:

--*/

#include "ki.h"

//
// Test for alertable condition.
//
// If alertable is TRUE and the thread is alerted for a processor
// mode that is equal to the wait mode, then return immediately
// with a wait completion status of ALERTED.
//
// Else if alertable is TRUE, the wait mode is user, and the user APC
// queue is not empty, then set user APC pending, and return immediately
// with a wait completion status of USER_APC.
//
// Else if alertable is TRUE and the thread is alerted for kernel
// mode, then return immediately with a wait completion status of
// ALERTED.
//
// Else if alertable is FALSE and the wait mode is user and there is a
// user APC pending, then return immediately with a wait completion
// status of USER_APC.
//

#define TestForAlertPending(Alertable) \
    if (Alertable) { \
        if (Thread->Alerted[WaitMode] != FALSE) { \
            Thread->Alerted[WaitMode] = FALSE; \
            WaitStatus = STATUS_ALERTED; \
            break; \
        } else if ((WaitMode != KernelMode) && \
                  (IsListEmpty(&Thread->ApcState.ApcListHead[UserMode])) == FALSE) { \
            Thread->ApcState.UserApcPending = TRUE; \
            WaitStatus = STATUS_USER_APC; \
            break; \
        } else if (Thread->Alerted[KernelMode] != FALSE) { \
            Thread->Alerted[KernelMode] = FALSE; \
            WaitStatus = STATUS_ALERTED; \
            break; \
        } \
    } else if (Thread->ApcState.UserApcPending & WaitMode) { \
        WaitStatus = STATUS_USER_APC; \
        break; \
    }

VOID
KiAdjustQuantumThread (
    IN PKTHREAD Thread
    )

/*++

Routine Description:

    If the current thread is not a time critical or real time thread, then
    adjust its quantum in accordance with the adjustment that would have
    occurred if the thread had actually waited.

    N.B. This routine is entered at SYNCH_LEVEL and exits at the wait
         IRQL of the subject thread after having exited the scheduler.

Arguments:

    Thread - Supplies a pointer to the current thread.

Return Value:

    None.

--*/

{

    PKPRCB Prcb;
    PKPROCESS Process;
    PKTHREAD NewThread;

    //
    // Acquire the thread lock and the PRCB lock.
    //
    // If the thread is not a real time or time critical thread, then adjust
    // the thread quantum.
    //

    Prcb = KeGetCurrentPrcb();
    KiAcquireThreadLock(Thread);
    KiAcquirePrcbLock(Prcb);
    if ((Thread->Priority < LOW_REALTIME_PRIORITY) &&
        (Thread->BasePriority < TIME_CRITICAL_PRIORITY_BOUND)) {

        Thread->Quantum -= WAIT_QUANTUM_DECREMENT;
        if (Thread->Quantum <= 0) {

            //
            // Quantum end has occurred. Adjust the thread priority.
            //

            Process = Thread->ApcState.Process;
            Thread->Quantum = Process->ThreadQuantum;

            //
            // Compute the new thread priority and attempt to reschedule the
            // current processor as if a quantum end had occurred.
            //
            // N.B. The new priority will never be greater than the previous
            //      priority.
            //

            Thread->Priority = KiComputeNewPriority(Thread, 1);
            if (Prcb->NextThread == NULL) {
                if ((NewThread = KiSelectReadyThread(Thread->Priority, Prcb)) != NULL) {
                    NewThread->State = Standby;
                    Prcb->NextThread = NewThread;
                }

            } else {
                Thread->Preempted = FALSE;
            }
        }
    }

    //
    // Release the thread lock, release the PRCB lock, exit the scheduler,
    // and return.
    //

    KiReleasePrcbLock(Prcb);
    KiReleaseThreadLock(Thread);
    KiExitDispatcher(Thread->WaitIrql);
    return;
}

//
// The following macro initializes thread local variables for the delay
// execution thread kernel service while context switching is disabled.
//
// N.B. IRQL must be raised to DPC level prior to the invocation of this
//      macro.
//
// N.B. Initialization is done in this manner so this code does not get
//      executed inside the dispatcher lock.
//

#define InitializeDelayExecution()                                          \
    Thread->WaitBlockList = WaitBlock;                                      \
    Thread->WaitStatus = 0;                                                 \
    WaitBlock->NextWaitBlock = WaitBlock;                                   \
    Timer->Header.WaitListHead.Flink = &WaitBlock->WaitListEntry;           \
    Timer->Header.WaitListHead.Blink = &WaitBlock->WaitListEntry;           \
    Thread->Alertable = Alertable;                                          \
    Thread->WaitMode = WaitMode;                                            \
    Thread->WaitReason = DelayExecution;                                    \
    Thread->WaitListEntry.Flink = NULL;                                     \
    StackSwappable = KiIsKernelStackSwappable(WaitMode, Thread);            \
    Thread->WaitTime = KiQueryLowTickCount()
        
NTSTATUS
KeDelayExecutionThread (
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Interval
    )

/*++

Routine Description:

    This function delays the execution of the current thread for the specified
    interval of time.

Arguments:

    WaitMode  - Supplies the processor mode in which the delay is to occur.

    Alertable - Supplies a boolean value that specifies whether the delay
        is alertable.

    Interval - Supplies a pointer to the absolute or relative time over which
        the delay is to occur.

Return Value:

    The wait completion status. A value of STATUS_SUCCESS is returned if
    the delay occurred. A value of STATUS_ALERTED is returned if the wait
    was aborted to deliver an alert to the current thread. A value of
    STATUS_USER_APC is returned if the wait was aborted to deliver a user
    APC to the current thread.

--*/

{

    LARGE_INTEGER DueTime;
    LARGE_INTEGER NewTime;
    PLARGE_INTEGER OriginalTime;
    PKPRCB Prcb;
    PRKQUEUE Queue;
    LOGICAL StackSwappable;
    PKTHREAD Thread;
    PRKTIMER Timer;
    PKWAIT_BLOCK WaitBlock;
    NTSTATUS WaitStatus;

    //
    // Set constant variables.
    //

    Thread = KeGetCurrentThread();
    OriginalTime = Interval;
    Timer = &Thread->Timer;
    WaitBlock = &Thread->WaitBlock[TIMER_WAIT_BLOCK];

    //
    // If the dispatcher database is already held, then initialize the thread
    // local variables. Otherwise, raise IRQL to DPC level, initialize the
    // thread local variables, and lock the dispatcher database.
    //

    if (Thread->WaitNext == FALSE) {
        goto WaitStart;
    }

    Thread->WaitNext = FALSE;
    InitializeDelayExecution();

    //
    // Start of delay loop.
    //
    // Note this loop is repeated if a kernel APC is delivered in the middle
    // of the delay or a kernel APC is pending on the first attempt through
    // the loop.
    //

    do {

        //
        // Test to determine if a kernel APC is pending.
        //
        // If a kernel APC is pending, the special APC disable count is zero,
        // and the previous IRQL was less than APC_LEVEL, then a kernel APC
        // was queued by another processor just after IRQL was raised to
        // DISPATCH_LEVEL, but before the dispatcher database was locked.
        //
        // N.B. that this can only happen in a multiprocessor system.
        //

        if (Thread->ApcState.KernelApcPending &&
            (Thread->SpecialApcDisable == 0) &&
            (Thread->WaitIrql < APC_LEVEL)) {

            //
            // Unlock the dispatcher database and lower IRQL to its previous
            // value. An APC interrupt will immediately occur which will result
            // in the delivery of the kernel APC if possible.
            //

            KiRequestSoftwareInterrupt(APC_LEVEL);
            KiUnlockDispatcherDatabase(Thread->WaitIrql);

        } else {

            //
            // Test for alert pending.
            //

            TestForAlertPending(Alertable);

            //
            // Insert the timer in the timer tree.
            //
            // N.B. The constant fields of the timer wait block are
            //      initialized when the thread is initialized. The
            //      constant fields include the wait object, wait key,
            //      wait type, and the wait list entry link pointers.
            //

            Prcb = KeGetCurrentPrcb();
            if (KiInsertTreeTimer(Timer, *Interval) == FALSE) {
                goto NoWait;
            }

            DueTime.QuadPart = Timer->DueTime.QuadPart;

            //
            // If the current thread is processing a queue entry, then attempt
            // to activate another thread that is blocked on the queue object.
            //

            Queue = Thread->Queue;
            if (Queue != NULL) {
                KiActivateWaiterQueue(Queue);
            }

            //
            // Set the thread wait parameters, set the thread dispatcher
            // state to Waiting, and insert the thread in the wait list if
            // the kernel stack of the current thread is swappable.
            //

            Thread->State = Waiting;
            if (StackSwappable != FALSE) {
                InsertTailList(&Prcb->WaitListHead, &Thread->WaitListEntry);
            }

            //
            // Set swap busy for the current thread, unlock the dispatcher
            // database, and switch to a new thread.
            //
            // Control is returned at the original IRQL.
            //

            ASSERT(Thread->WaitIrql <= DISPATCH_LEVEL);

            KiSetContextSwapBusy(Thread);
            KiUnlockDispatcherDatabaseFromSynchLevel();
            WaitStatus = (NTSTATUS)KiSwapThread(Thread, Prcb);

            //
            // If the thread was not awakened to deliver a kernel mode APC,
            // then return the wait status.
            //

            if (WaitStatus != STATUS_KERNEL_APC) {
                if (WaitStatus == STATUS_TIMEOUT) {
                    WaitStatus = STATUS_SUCCESS;
                }

                return WaitStatus;
            }

            //
            // Reduce the time remaining before the time delay expires.
            //

            Interval = KiComputeWaitInterval(OriginalTime,
                                             &DueTime,
                                             &NewTime);
        }

        //
        // Raise IRQL to SYNCH level, initialize the thread local variables,
        // and lock the dispatcher database.
        //

WaitStart:

        Thread->WaitIrql = KeRaiseIrqlToSynchLevel();
        InitializeDelayExecution();
        KiLockDispatcherDatabaseAtSynchLevel();

    } while (TRUE);

    //
    // The thread is alerted or a user APC should be delivered. Unlock the
    // dispatcher database, lower IRQL to its previous value, and return the
    // wait status.
    //

    KiUnlockDispatcherDatabase(Thread->WaitIrql);
    return WaitStatus;

    //
    // The wait has been satisfied without actually waiting.
    //
    // If the wait time is zero, then unlock the dispatcher database and
    // yield execution. Otherwise, unlock the dispatcher database, remain
    // at SYNCH level, adjust the thread quantum, exit the dispatcher, and
    // and return the wait completion status.
    //

NoWait:

    if ((Interval->LowPart | Interval->HighPart) == 0) {
        KiUnlockDispatcherDatabase(Thread->WaitIrql);
        return NtYieldExecution();

    } else {
        KiUnlockDispatcherDatabaseFromSynchLevel();
        KiAdjustQuantumThread(Thread);
        return STATUS_SUCCESS;
    }
}

//
// The following macro initializes thread local variables for the wait
// for multiple objects kernel service while context switching is disabled.
//
// N.B. IRQL must be raised to DPC level prior to the invocation of this
//      macro.
//
// N.B. Initialization is done in this manner so this code does not get
//      executed inside the dispatcher lock.
//

#define InitializeWaitMultiple()                                            \
    Thread->WaitBlockList = WaitBlockArray;                                 \
    Index = 0;                                                              \
    do {                                                                    \
        WaitBlock = &WaitBlockArray[Index];                                 \
        WaitBlock->Object = Object[Index];                                  \
        WaitBlock->WaitKey = (CSHORT)(Index);                               \
        WaitBlock->WaitType = WaitType;                                     \
        WaitBlock->Thread = Thread;                                         \
        WaitBlock->NextWaitBlock = &WaitBlockArray[Index + 1];              \
        Index += 1;                                                         \
    } while (Index < Count);                                                \
    WaitBlock->NextWaitBlock = &WaitBlockArray[0];                          \
    WaitTimer->NextWaitBlock = &WaitBlockArray[0];                          \
    Thread->WaitStatus = 0;                                                 \
    InitializeListHead(&Timer->Header.WaitListHead);                        \
    Thread->Alertable = Alertable;                                          \
    Thread->WaitMode = WaitMode;                                            \
    Thread->WaitReason = (UCHAR)WaitReason;                                 \
    Thread->WaitListEntry.Flink = NULL;                                     \
    StackSwappable = KiIsKernelStackSwappable(WaitMode, Thread);            \
    Thread->WaitTime = KiQueryLowTickCount()

NTSTATUS
KeWaitForMultipleObjects (
    IN ULONG Count,
    IN PVOID Object[],
    IN WAIT_TYPE WaitType,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL,
    IN PKWAIT_BLOCK WaitBlockArray OPTIONAL
    )

/*++

Routine Description:

    This function waits until the specified objects attain a state of
    Signaled. The wait can be specified to wait until all of the objects
    attain a state of Signaled or until one of the objects attains a state
    of Signaled. An optional timeout can also be specified. If a timeout
    is not specified, then the wait will not be satisfied until the objects
    attain a state of Signaled. If a timeout is specified, and the objects
    have not attained a state of Signaled when the timeout expires, then
    the wait is automatically satisfied. If an explicit timeout value of
    zero is specified, then no wait will occur if the wait cannot be satisfied
    immediately. The wait can also be specified as alertable.

Arguments:

    Count - Supplies a count of the number of objects that are to be waited
        on.

    Object[] - Supplies an array of pointers to dispatcher objects.

    WaitType - Supplies the type of wait to perform (WaitAll, WaitAny).

    WaitReason - Supplies the reason for the wait.

    WaitMode  - Supplies the processor mode in which the wait is to occur.

    Alertable - Supplies a boolean value that specifies whether the wait is
        alertable.

    Timeout - Supplies a pointer to an optional absolute of relative time over
        which the wait is to occur.

    WaitBlockArray - Supplies an optional pointer to an array of wait blocks
        that are to used to describe the wait operation.

Return Value:

    The wait completion status. A value of STATUS_TIMEOUT is returned if a
    timeout occurred. The index of the object (zero based) in the object
    pointer array is returned if an object satisfied the wait. A value of
    STATUS_ALERTED is returned if the wait was aborted to deliver an alert
    to the current thread. A value of STATUS_USER_APC is returned if the
    wait was aborted to deliver a user APC to the current thread.

--*/

{

    PKPRCB CurrentPrcb;
    LARGE_INTEGER DueTime;
    ULONG Index;
    LARGE_INTEGER NewTime;
    PKMUTANT Objectx;
    PLARGE_INTEGER OriginalTime;
    PRKQUEUE Queue;
    LOGICAL StackSwappable;
    PRKTHREAD Thread;
    PRKTIMER Timer;
    PRKWAIT_BLOCK WaitBlock;
    NTSTATUS WaitStatus;
    PKWAIT_BLOCK WaitTimer;

    //
    // Set constant variables.
    //

    Thread = KeGetCurrentThread();
    OriginalTime = Timeout;
    Timer = &Thread->Timer;
    WaitTimer = &Thread->WaitBlock[TIMER_WAIT_BLOCK];

    //
    // If a wait block array has been specified, then the maximum number of
    // objects that can be waited on is specified by MAXIMUM_WAIT_OBJECTS.
    // Otherwise the builtin wait blocks in the thread object are used and
    // the maximum number of objects that can be waited on is specified by
    // THREAD_WAIT_OBJECTS. If the specified number of objects is not within
    // limits, then bug check.
    //

    if (ARGUMENT_PRESENT(WaitBlockArray)) {
        if (Count > MAXIMUM_WAIT_OBJECTS) {
            KeBugCheck(MAXIMUM_WAIT_OBJECTS_EXCEEDED);
        }

    } else {
        if (Count > THREAD_WAIT_OBJECTS) {
            KeBugCheck(MAXIMUM_WAIT_OBJECTS_EXCEEDED);
        }

        WaitBlockArray = &Thread->WaitBlock[0];
    }

    ASSERT(Count != 0);

    //
    // If the dispatcher database is already held, then initialize the thread
    // local variables. Otherwise, raise IRQL to DPC level, initialize the
    // thread local variables, and lock the dispatcher database.
    //

    if (Thread->WaitNext == FALSE) {
        goto WaitStart;
    }

    Thread->WaitNext = FALSE;
    InitializeWaitMultiple();

    //
    // Start of wait loop.
    //
    // Note this loop is repeated if a kernel APC is delivered in the middle
    // of the wait or a kernel APC is pending on the first attempt through
    // the loop.
    //

    do {

        //
        // Test to determine if a kernel APC is pending.
        //
        // If a kernel APC is pending, the special APC disable count is zero,
        // and the previous IRQL was less than APC_LEVEL, then a kernel APC
        // was queued by another processor just after IRQL was raised to
        // DISPATCH_LEVEL, but before the dispatcher database was locked.
        //
        // N.B. that this can only happen in a multiprocessor system.
        //

        if (Thread->ApcState.KernelApcPending &&
            (Thread->SpecialApcDisable == 0) &&
            (Thread->WaitIrql < APC_LEVEL)) {

            //
            // Unlock the dispatcher database and lower IRQL to its previous
            // value. An APC interrupt will immediately occur which will result
            // in the delivery of the kernel APC if possible.
            //

            KiRequestSoftwareInterrupt(APC_LEVEL);
            KiUnlockDispatcherDatabase(Thread->WaitIrql);

        } else {

            //
            // Construct wait blocks and check to determine if the wait is
            // already satisfied. If the wait is satisfied, then perform
            // wait completion and return. Else put current thread in a wait
            // state if an explicit timeout value of zero is not specified.
            //

            Index = 0;
            if (WaitType == WaitAny) {
                do {

                    //
                    // Test if wait can be satisfied immediately.
                    //
    
                    Objectx = (PKMUTANT)Object[Index];
    
                    ASSERT(Objectx->Header.Type != QueueObject);
    
                    //
                    // If the object is a mutant object and the mutant object
                    // has been recursively acquired MINLONG times, then raise
                    // an exception. Otherwise if the signal state of the mutant
                    // object is greater than zero, or the current thread is
                    // the owner of the mutant object, then satisfy the wait.
                    //

                    if (Objectx->Header.Type == MutantObject) {
                        if ((Objectx->Header.SignalState > 0) ||
                            (Thread == Objectx->OwnerThread)) {
                            if (Objectx->Header.SignalState != MINLONG) {
                                KiWaitSatisfyMutant(Objectx, Thread);
                                WaitStatus = (NTSTATUS)(Index | Thread->WaitStatus);
                                goto NoWait;

                            } else {
                                KiUnlockDispatcherDatabase(Thread->WaitIrql);
                                ExRaiseStatus(STATUS_MUTANT_LIMIT_EXCEEDED);
                            }
                        }

                    //
                    // If the signal state is greater than zero, then satisfy
                    // the wait.
                    //

                    } else if (Objectx->Header.SignalState > 0) {
                        KiWaitSatisfyOther(Objectx);
                        WaitStatus = (NTSTATUS)(Index);
                        goto NoWait;
                    }

                    Index += 1;

                } while(Index < Count);

            } else {
                do {

                    //
                    // Test if wait can be satisfied.
                    //
    
                    Objectx = (PKMUTANT)Object[Index];
    
                    ASSERT(Objectx->Header.Type != QueueObject);
    
                    //
                    // If the object is a mutant object and the mutant object
                    // has been recursively acquired MINLONG times, then raise
                    // an exception. Otherwise if the signal state of the mutant
                    // object is less than or equal to zero and the current
                    // thread is not the  owner of the mutant object, then the
                    // wait cannot be satisfied.
                    //

                    if (Objectx->Header.Type == MutantObject) {
                        if ((Thread == Objectx->OwnerThread) &&
                            (Objectx->Header.SignalState == MINLONG)) {
                            KiUnlockDispatcherDatabase(Thread->WaitIrql);
                            ExRaiseStatus(STATUS_MUTANT_LIMIT_EXCEEDED);

                        } else if ((Objectx->Header.SignalState <= 0) &&
                                  (Thread != Objectx->OwnerThread)) {
                            break;
                        }

                    //
                    // If the signal state is less than or equal to zero, then
                    // the wait cannot be satisfied.
                    //

                    } else if (Objectx->Header.SignalState <= 0) {
                        break;
                    }

                    Index += 1;

                } while(Index < Count);

                //
                // If all objects have been scanned, then satisfy the wait.
                //

                if (Index == Count) {
                    WaitBlock = &WaitBlockArray[0];
                    do {
                        Objectx = (PKMUTANT)WaitBlock->Object;
                        KiWaitSatisfyAny(Objectx, Thread);
                        WaitBlock = WaitBlock->NextWaitBlock;
                    } while (WaitBlock != &WaitBlockArray[0]);

                    WaitStatus = (NTSTATUS)Thread->WaitStatus;
                    goto NoWait;
                }
            }

            //
            // Test for alert pending.
            //

            TestForAlertPending(Alertable);

            //
            // Check to determine if a timeout value is specified.
            //

            if (ARGUMENT_PRESENT(Timeout)) {

                //
                // If the timeout value is zero, then return immediately without
                // waiting.
                //

                if (Timeout->QuadPart == 0) {
                    WaitStatus = (NTSTATUS)(STATUS_TIMEOUT);
                    goto NoWait;
                }

                //
                // Initialize a wait block for the thread specific timer,
                // initialize timer wait list head, insert the timer in the
                // timer tree, and increment the number of wait objects.
                //
                // N.B. The constant fields of the timer wait block are
                //      initialized when the thread is initialized. The
                //      constant fields include the wait object, wait key,
                //      wait type, and the wait list entry link pointers.
                //

                if (KiInsertTreeTimer(Timer, *Timeout) == FALSE) {
                    WaitStatus = (NTSTATUS)STATUS_TIMEOUT;
                    goto NoWait;
                }

                WaitBlock->NextWaitBlock = WaitTimer;
                DueTime.QuadPart = Timer->DueTime.QuadPart;
            }

            //
            // Insert wait blocks in object wait lists.
            //

            WaitBlock = &WaitBlockArray[0];
            do {
                Objectx = (PKMUTANT)WaitBlock->Object;
                InsertTailList(&Objectx->Header.WaitListHead, &WaitBlock->WaitListEntry);
                WaitBlock = WaitBlock->NextWaitBlock;
            } while (WaitBlock != &WaitBlockArray[0]);

            //
            // If the current thread is processing a queue entry, then attempt
            // to activate another thread that is blocked on the queue object.
            //

            Queue = Thread->Queue;
            if (Queue != NULL) {
                KiActivateWaiterQueue(Queue);
            }

            //
            // Set the thread wait parameters, set the thread dispatcher state
            // to Waiting, and insert the thread in the wait list.
            //

            CurrentPrcb = KeGetCurrentPrcb();
            Thread->State = Waiting;
            if (StackSwappable != FALSE) {
                InsertTailList(&CurrentPrcb->WaitListHead, &Thread->WaitListEntry);
            }

            //
            // Set swap busy for the current thread, unlock the dispatcher
            // database, and switch to a new thread.
            //
            // Control is returned at the original IRQL.
            //

            ASSERT(Thread->WaitIrql <= DISPATCH_LEVEL);

            KiSetContextSwapBusy(Thread);
            KiUnlockDispatcherDatabaseFromSynchLevel();
            WaitStatus = (NTSTATUS)KiSwapThread(Thread, CurrentPrcb);

            //
            // If the thread was not awakened to deliver a kernel mode APC,
            // then return the wait status.
            //

            if (WaitStatus != STATUS_KERNEL_APC) {
                return WaitStatus;
            }

            if (ARGUMENT_PRESENT(Timeout)) {

                //
                // Reduce the amount of time remaining before timeout occurs.
                //

                Timeout = KiComputeWaitInterval(OriginalTime,
                                                &DueTime,
                                                &NewTime);
            }
        }

        //
        // Raise IRQL to SYNCH level, initialize the thread local variables,
        // and lock the dispatcher database.
        //

WaitStart:
        Thread->WaitIrql = KeRaiseIrqlToSynchLevel();
        InitializeWaitMultiple();
        KiLockDispatcherDatabaseAtSynchLevel();

    } while (TRUE);

    //
    // The thread is alerted or a user APC should be delivered. Unlock the
    // dispatcher database, lower IRQL to its previous value, and return
    // the wait status.
    //

    KiUnlockDispatcherDatabase(Thread->WaitIrql);
    return WaitStatus;

    //
    // The wait has been satisfied without actually waiting.
    //
    // Unlock the dispatcher database and remain at SYNCH level.
    //

NoWait:

    KiUnlockDispatcherDatabaseFromSynchLevel();

    //
    // Adjust the thread quantum, exit the scheduler, and return the wait
    // completion status.
    //

    KiAdjustQuantumThread(Thread);
    return WaitStatus;
}

//
// The following macro initializes thread local variables for the wait
// for single object kernel service while context switching is disabled.
//
// N.B. IRQL must be raised to DPC level prior to the invocation of this
//      macro.
//
// N.B. Initialization is done in this manner so this code does not get
//      executed inside the dispatcher lock.
//

#define InitializeWaitSingle()                                              \
    Thread->WaitBlockList = WaitBlock;                                      \
    WaitBlock->Object = Object;                                             \
    WaitBlock->WaitKey = (CSHORT)(STATUS_SUCCESS);                          \
    WaitBlock->WaitType = WaitAny;                                          \
    Thread->WaitStatus = 0;                                                 \
    if (ARGUMENT_PRESENT(Timeout)) {                                        \
        WaitBlock->NextWaitBlock = WaitTimer;                               \
        WaitTimer->NextWaitBlock = WaitBlock;                               \
        Timer->Header.WaitListHead.Flink = &WaitTimer->WaitListEntry;       \
        Timer->Header.WaitListHead.Blink = &WaitTimer->WaitListEntry;       \
    } else {                                                                \
        WaitBlock->NextWaitBlock = WaitBlock;                               \
    }                                                                       \
    Thread->Alertable = Alertable;                                          \
    Thread->WaitMode = WaitMode;                                            \
    Thread->WaitReason = (UCHAR)WaitReason;                                 \
    Thread->WaitListEntry.Flink = NULL;                                     \
    StackSwappable = KiIsKernelStackSwappable(WaitMode, Thread);            \
    Thread->WaitTime = KiQueryLowTickCount()

NTSTATUS
KeWaitForSingleObject (
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )

/*++

Routine Description:

    This function waits until the specified object attains a state of
    Signaled. An optional timeout can also be specified. If a timeout
    is not specified, then the wait will not be satisfied until the object
    attains a state of Signaled. If a timeout is specified, and the object
    has not attained a state of Signaled when the timeout expires, then
    the wait is automatically satisfied. If an explicit timeout value of
    zero is specified, then no wait will occur if the wait cannot be satisfied
    immediately. The wait can also be specified as alertable.

Arguments:

    Object - Supplies a pointer to a dispatcher object.

    WaitReason - Supplies the reason for the wait.

    WaitMode  - Supplies the processor mode in which the wait is to occur.

    Alertable - Supplies a boolean value that specifies whether the wait is
        alertable.

    Timeout - Supplies a pointer to an optional absolute of relative time over
        which the wait is to occur.

Return Value:

    The wait completion status. A value of STATUS_TIMEOUT is returned if a
    timeout occurred. A value of STATUS_SUCCESS is returned if the specified
    object satisfied the wait. A value of STATUS_ALERTED is returned if the
    wait was aborted to deliver an alert to the current thread. A value of
    STATUS_USER_APC is returned if the wait was aborted to deliver a user
    APC to the current thread.

--*/

{

    PKPRCB CurrentPrcb;
    LARGE_INTEGER DueTime;
    LARGE_INTEGER NewTime;
    PKMUTANT Objectx;
    PLARGE_INTEGER OriginalTime;
    PRKQUEUE Queue;
    LOGICAL StackSwappable;
    PRKTHREAD Thread;
    PRKTIMER Timer;
    PKWAIT_BLOCK WaitBlock;
    NTSTATUS WaitStatus;
    PKWAIT_BLOCK WaitTimer;

    //
    // Collect call data.
    //

#if defined(_COLLECT_WAIT_SINGLE_CALLDATA_)

    RECORD_CALL_DATA(&KiWaitSingleCallData);

#endif

    ASSERT((PsGetCurrentThread()->StartAddress != (PVOID)(ULONG_PTR)KeBalanceSetManager) || (ARGUMENT_PRESENT(Timeout)));

    //
    // Set constant variables.
    //

    Thread = KeGetCurrentThread();
    Objectx = (PKMUTANT)Object;
    OriginalTime = Timeout;
    Timer = &Thread->Timer;
    WaitBlock = &Thread->WaitBlock[0];
    WaitTimer = &Thread->WaitBlock[TIMER_WAIT_BLOCK];

    //
    // If the dispatcher database is already held, then initialize the thread
    // local variables. Otherwise, raise IRQL to DPC level, initialize the
    // thread local variables, and lock the dispatcher database.
    //

    if (Thread->WaitNext == FALSE) {
        goto WaitStart;
    }

    Thread->WaitNext = FALSE;
    InitializeWaitSingle();

    //
    // Start of wait loop.
    //
    // Note this loop is repeated if a kernel APC is delivered in the middle
    // of the wait or a kernel APC is pending on the first attempt through
    // the loop.
    //

    do {

        //
        // Test to determine if a kernel APC is pending.
        //
        // If a kernel APC is pending, the special APC disable count is zero,
        // and the previous IRQL was less than APC_LEVEL, then a kernel APC
        // was queued by another processor just after IRQL was raised to
        // DISPATCH_LEVEL, but before the dispatcher database was locked.
        //
        // N.B. that this can only happen in a multiprocessor system.
        //

        if (Thread->ApcState.KernelApcPending &&
            (Thread->SpecialApcDisable == 0) &&
            (Thread->WaitIrql < APC_LEVEL)) {

            //
            // Unlock the dispatcher database and lower IRQL to its previous
            // value. An APC interrupt will immediately occur which will result
            // in the delivery of the kernel APC if possible.
            //

            KiRequestSoftwareInterrupt(APC_LEVEL);
            KiUnlockDispatcherDatabase(Thread->WaitIrql);

        } else {

            //
            // If the object is a mutant object and the mutant object has been
            // recursively acquired MINLONG times, then raise an exception.
            // Otherwise if the signal state of the mutant object is greater
            // than zero, or the current thread is the owner of the mutant
            // object, then satisfy the wait.
            //

            ASSERT(Objectx->Header.Type != QueueObject);

            if (Objectx->Header.Type == MutantObject) {
                if ((Objectx->Header.SignalState > 0) ||
                    (Thread == Objectx->OwnerThread)) {
                    if (Objectx->Header.SignalState != MINLONG) {
                        KiWaitSatisfyMutant(Objectx, Thread);
                        WaitStatus = (NTSTATUS)(Thread->WaitStatus);
                        goto NoWait;

                    } else {
                        KiUnlockDispatcherDatabase(Thread->WaitIrql);
                        ExRaiseStatus(STATUS_MUTANT_LIMIT_EXCEEDED);
                    }
                }

            //
            // If the signal state is greater than zero, then satisfy the wait.
            //

            } else if (Objectx->Header.SignalState > 0) {
                KiWaitSatisfyOther(Objectx);
                WaitStatus = (NTSTATUS)(0);
                goto NoWait;
            }

            //
            // Construct a wait block for the object.
            //

            //
            // Test for alert pending.
            //

            TestForAlertPending(Alertable);

            //
            // The wait cannot be satisifed immediately. Check to determine if
            // a timeout value is specified.
            //

            if (ARGUMENT_PRESENT(Timeout)) {

                //
                // If the timeout value is zero, then return immediately without
                // waiting.
                //

                if (Timeout->QuadPart == 0) {
                    WaitStatus = (NTSTATUS)(STATUS_TIMEOUT);
                    goto NoWait;
                }

                //
                // Insert the timer in the timer tree.
                //
                // N.B. The constant fields of the timer wait block are
                //      initialized when the thread is initialized. The
                //      constant fields include the wait object, wait key,
                //      wait type, and the wait list entry link pointers.
                //

                if (KiInsertTreeTimer(Timer, *Timeout) == FALSE) {
                    WaitStatus = (NTSTATUS)STATUS_TIMEOUT;
                    goto NoWait;
                }

                DueTime.QuadPart = Timer->DueTime.QuadPart;
            }

            //
            // Insert wait block in object wait list.
            //

            InsertTailList(&Objectx->Header.WaitListHead, &WaitBlock->WaitListEntry);

            //
            // If the current thread is processing a queue entry, then attempt
            // to activate another thread that is blocked on the queue object.
            //

            Queue = Thread->Queue;
            if (Queue != NULL) {
                KiActivateWaiterQueue(Queue);
            }

            //
            // Set the thread wait parameters, set the thread dispatcher state
            // to Waiting, and insert the thread in the wait list.
            //

            Thread->State = Waiting;
            CurrentPrcb = KeGetCurrentPrcb();
            if (StackSwappable != FALSE) {
                InsertTailList(&CurrentPrcb->WaitListHead, &Thread->WaitListEntry);
            }

            //
            // Set swap busy for the current thread, unlock the dispatcher
            // database, and switch to a new thread.
            //
            // Control is returned at the original IRQL.
            //

            ASSERT(Thread->WaitIrql <= DISPATCH_LEVEL);

            KiSetContextSwapBusy(Thread);
            KiUnlockDispatcherDatabaseFromSynchLevel();
            WaitStatus = (NTSTATUS)KiSwapThread(Thread, CurrentPrcb);

            //
            // If the thread was not awakened to deliver a kernel mode APC,
            // then return wait status.
            //

            if (WaitStatus != STATUS_KERNEL_APC) {
                return WaitStatus;
            }

            if (ARGUMENT_PRESENT(Timeout)) {

                //
                // Reduce the amount of time remaining before timeout occurs.
                //

                Timeout = KiComputeWaitInterval(OriginalTime,
                                                &DueTime,
                                                &NewTime);
            }
        }

        //
        // Raise IRQL to SYNCH level, initialize the thread local variables,
        // and lock the dispatcher database.
        //

WaitStart:
        Thread->WaitIrql = KeRaiseIrqlToSynchLevel();
        InitializeWaitSingle();
        KiLockDispatcherDatabaseAtSynchLevel();

    } while (TRUE);

    //
    // The thread is alerted or a user APC should be delivered. Unlock the
    // dispatcher database, lower IRQL to its previous value, and return
    // the wait status.
    //

    KiUnlockDispatcherDatabase(Thread->WaitIrql);
    return WaitStatus;

    //
    // The wait has been satisfied without actually waiting.
    //
    // Unlock the dispatcher database and remain at SYNCH level.
    //

NoWait:

    KiUnlockDispatcherDatabaseFromSynchLevel();

    //
    // Adjust the thread quantum, exit the scheduler, and return the wait
    // completion status.
    //

    KiAdjustQuantumThread(Thread);
    return WaitStatus;
}

NTSTATUS
KiSetServerWaitClientEvent (
    IN PKEVENT ServerEvent,
    IN PKEVENT ClientEvent,
    IN ULONG WaitMode
    )

/*++

Routine Description:

    This function sets the specified server event and waits on specified
    client event. The wait is performed such that an optimal switch to
    the waiting thread occurs if possible. No timeout is associated with
    the wait, and thus, the issuing thread will wait until the client event
    is signaled or an APC is delivered.

Arguments:

    ServerEvent - Supplies a pointer to a dispatcher object of type event.

    ClientEvent - Supplies a pointer to a dispatcher object of type event.

    WaitMode  - Supplies the processor mode in which the wait is to occur.

Return Value:

    The wait completion status. A value of STATUS_SUCCESS is returned if
    the specified object satisfied the wait. A value of STATUS_USER_APC is
    returned if the wait was aborted to deliver a user APC to the current
    thread.

--*/

{

    //
    // Set sever event and wait on client event atomically.
    //

    KeSetEvent(ServerEvent, EVENT_INCREMENT, TRUE);
    return KeWaitForSingleObject(ClientEvent,
                                 WrEventPair,
                                 (KPROCESSOR_MODE)WaitMode,
                                 FALSE,
                                 NULL);
}

PLARGE_INTEGER
FASTCALL
KiComputeWaitInterval (
    IN PLARGE_INTEGER OriginalTime,
    IN PLARGE_INTEGER DueTime,
    IN OUT PLARGE_INTEGER NewTime
    )

/*++

Routine Description:

    This function recomputes the wait interval after a thread has been
    awakened to deliver a kernel APC.

Arguments:

    OriginalTime - Supplies a pointer to the original timeout value.

    DueTime - Supplies a pointer to the previous due time.

    NewTime - Supplies a pointer to a variable that receives the
        recomputed wait interval.

Return Value:

    A pointer to the new time is returned as the function value.

--*/

{

    //
    // If the original wait time was absolute, then return the same
    // absolute time. Otherwise, reduce the wait time remaining before
    // the time delay expires.
    //

    if (OriginalTime->HighPart >= 0) {
        return OriginalTime;

    } else {
        KiQueryInterruptTime(NewTime);
        NewTime->QuadPart -= DueTime->QuadPart;
        return NewTime;
    }
}

VOID
FASTCALL
KiWaitForFastMutexEvent (
    IN PFAST_MUTEX Mutex
    )

/*++

Routine Description:

    This function increments the fast mutex contention count and waits on
    the event assocated with the fast mutex.

Arguments:

    Mutex - Supplies a pointer to a fast mutex.

Return Value:

    None.

--*/

{

    //
    // Increment contention count and wait for ownership to be granted.
    //

    Mutex->Contention += 1;
    KeWaitForSingleObject(&Mutex->Event, WrMutex, KernelMode, FALSE, NULL);
    return;
}

VOID
FASTCALL
KiWaitForGuardedMutexEvent (
    IN PKGUARDED_MUTEX Mutex
    )

/*++

Routine Description:

    This function increments the guarded mutex contention count and waits on
    the event assocated with the guarded mutex.

Arguments:

    Mutex - Supplies a pointer to a guarded mutex.

Return Value:

    None.

--*/

{

    //
    // Increment contention count and wait for ownership to be granted.
    //

    Mutex->Contention += 1;
    KeWaitForSingleObject(&Mutex->Event, WrMutex, KernelMode, FALSE, NULL);
    return;
}

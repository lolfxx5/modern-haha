/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    FspDisp.c

Abstract:

    This module implements the main dispatch procedure/thread for the RDBSS Fsp

Author:

    Joe Linn     [JoeLinn]    1-aug-1994

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop
#include "NtDspVec.h"
#include <ntddnfs2.h>
#include <ntddmup.h>

//
//  Define our local debug trace level
//

#define Dbg                              (DEBUG_TRACE_FSP_DISPATCHER)

#ifndef MONOLITHIC_MINIRDR
PIO_WORKITEM RxIoWorkItem;
#endif

//
//  Internal support routine, spinlock wrapper.
//

PRX_CONTEXT
RxRemoveOverflowEntry (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN WORK_QUEUE_TYPE Type
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxFspDispatch)
#endif

#ifdef RDBSSLOG
//this stuff must be in nonpaged memory
                     ////       1 2 3 4 5 6 7
char RxFsp_SurrogateFormat[] = "%S%S%N%N%N%N%N";
                             ////   2  3   4         5        6   7
char RxFsp_ActualFormat[]    = "Fsp %s/%lx %08lx irp %lx thrd %lx #%lx";

#endif //ifdef RDBSSLOG

#ifndef MONOLITHIC_MINIRDR

VOID
RxFspDispatchEx (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
/*++

Routine Description:

    This routine calls RxFspDispatch which is the main FSP thread routine that
    is executed to receive and dispatch IRP requests.

Arguments:

    Context - The RxContext being queued to the FSP.

Notes:

    None.

--*/
{
    RxFspDispatch( Context );
    return;
}

#endif

VOID
RxFspDispatch (
    IN PVOID Context
    )
/*++

Routine Description:

    This is the main FSP thread routine that is executed to receive
    and dispatch IRP requests.  Each FSP thread begins its execution here.
    There is one thread created at system initialization time and subsequent
    threads created as needed.

Arguments:

    Context - The RxContext being queued to the FSP.

Notes:

    This routine never exits

--*/
{
    NTSTATUS Status;

    PRX_CONTEXT RxContext = (PRX_CONTEXT)Context;
    RX_TOPLEVELIRP_CONTEXT TopLevelContext;
    PRDBSS_DEVICE_OBJECT RxDeviceObject;
    WORK_QUEUE_TYPE WorkQueueType;
    DWORD CurrentIrql;

    PIRP Irp = RxContext->CurrentIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    PAGED_CODE();

    CurrentIrql = KeGetCurrentIrql();


    //
    //  If this request has an associated volume device object, remember it.
    //

    if (FileObject != NULL ) {

        RxDeviceObject = CONTAINING_RECORD( IrpSp->DeviceObject, RDBSS_DEVICE_OBJECT, DeviceObject );

        //
        //  currently, we use the wrapper's device object for all throttling.....
        //

        RxDeviceObject = RxFileSystemDeviceObject;

    } else {

        RxDeviceObject = NULL;
    }

    if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_FSP_DELAYED_OVERFLOW_QUEUE )) {
        WorkQueueType = DelayedWorkQueue;
    } else if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_FSP_CRITICAL_OVERFLOW_QUEUE )) {
        WorkQueueType = CriticalWorkQueue;
    } else {
        ASSERT(!"Valid RXCONTEXT Work Queue Type");
    }

    //
    //  We'll do all of the work within an exception handler that
    //  will be invoked if ever some underlying operation gets into
    //  trouble.
    //

    while (TRUE) {

        //
        //   Grab the current irp
        //  

        Irp = RxContext->CurrentIrp;

        RxDbgTrace( 0, Dbg, ("RxFspDispatch: IrpC = 0x%08lx\n", RxContext) );
        
        ASSERT( RxContext->MajorFunction <= IRP_MJ_MAXIMUM_FUNCTION );
        ASSERT( RxContext->PostRequest == FALSE );

        RxContext->LastExecutionThread = PsGetCurrentThread();

        RxLog(( RxFsp_SurrogateFormat, RxFsp_ActualFormat, RXCONTX_OPERATION_NAME( RxContext->MajorFunction, TRUE ), RxContext->MinorFunction, RxContext, Irp, RxContext->LastExecutionThread, RxContext->SerialNumber ));
        RxWmiLog( LOG,
                  RxFspDispatch,
                  LOGPTR( RxContext)
                  LOGPTR( Irp )
                  LOGPTR( RxContext->LastExecutionThread)
                  LOGULONG( RxContext->SerialNumber)
                  LOGUCHAR( RxContext->MinorFunction)
                  LOGARSTR( RXCONTX_OPERATION_NAME( RxContext->MajorFunction, TRUE ) ) );

        //
        //  Now because we are the Fsp we will force the RxContext to
        //  indicate true on Wait.
        //

        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_WAIT | RX_CONTEXT_FLAG_IN_FSP );

        //
        //  If this Irp was top level, note it in our thread local storage.
        //

        FsRtlEnterFileSystem();

        if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_RECURSIVE_CALL )) {
            
            RxTryToBecomeTheTopLevelIrp( &TopLevelContext,
                                         (PIRP)FSRTL_FSP_TOP_LEVEL_IRP,
                                         RxContext->RxDeviceObject,
                                         TRUE ); //  force
        } else {
            
            RxTryToBecomeTheTopLevelIrp( &TopLevelContext,
                                         Irp,
                                         RxContext->RxDeviceObject,
                                         TRUE ); //  force
        }

        try {

            ASSERT( RxContext->ResumeRoutine != NULL );

            if (FlagOn( RxContext->MinorFunction, IRP_MN_DPC ) && (Irp->Tail.Overlay.Thread == NULL)) {

                ASSERT( (RxContext->MajorFunction == IRP_MJ_WRITE) ||
                        (RxContext->MajorFunction == IRP_MJ_READ) );

                Irp->Tail.Overlay.Thread = PsGetCurrentThread();
            }

            do {
                
                BOOLEAN NoCompletion = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_NO_COMPLETE_FROM_FSP );

                Status = RxContext->ResumeRoutine( RxContext, Irp );

                if (NoCompletion) {
                    NOTHING;
                } else if ((Status != STATUS_PENDING) && (Status != STATUS_RETRY)) {
                        
                    Status = RxCompleteRequest( RxContext, Status );
                }
            } while (Status == STATUS_RETRY);

        } except( RxExceptionFilter( RxContext, GetExceptionInformation() )) {

            //
            //  We had some trouble trying to perform the requested
            //  operation, so we'll abort the I/O request with
            //  the error status that we get back from the
            //  execption code.
            //

            (VOID) RxProcessException( RxContext, GetExceptionCode() );
        }

        RxUnwindTopLevelIrp( &TopLevelContext );

        FsRtlExitFileSystem();

        //
        //  If there are any entries on this volume's overflow queue, service
        //  them.
        //

        if (RxDeviceObject != NULL) {

            //
            //  We have a volume device object so see if there is any work
            //  left to do in its overflow queue.
            //

            RxContext = RxRemoveOverflowEntry( RxDeviceObject,WorkQueueType );

            //
            //  There wasn't an entry, break out of the loop and return to
            //  the Ex Worker thread.
            //

            if (RxContext == NULL) {
                break;
            }

            continue;
        } else {
            break;
        }
    }

#ifdef DBG
    if(KeGetCurrentIrql() >= APC_LEVEL) {
        
        DbgPrint( "High Irql RxContext=%x Irql On Entry=%x\n", RxContext, CurrentIrql);
        //  DbgBreakPoint();
    }
#endif

    return;
}

//
//  Internal support routine, spinlock wrapper.
//

PRX_CONTEXT
RxRemoveOverflowEntry (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN WORK_QUEUE_TYPE WorkQueueType)
{
    PRX_CONTEXT RxContext;
    KIRQL SavedIrql;

    KeAcquireSpinLock( &RxDeviceObject->OverflowQueueSpinLock, &SavedIrql );

    if (RxDeviceObject->OverflowQueueCount[WorkQueueType] > 0) {
        
        PVOID Entry;

        //
        //  There is overflow work to do in this volume so we'll
        //  decrement the Overflow count, dequeue the IRP, and release
        //  the Event
        //

        RxDeviceObject->OverflowQueueCount[WorkQueueType] -= 1;

        Entry = RemoveHeadList( &RxDeviceObject->OverflowQueue[WorkQueueType] );

        //
        //  Extract the RxContext, Irp, and IrpSp, and loop.
        //

        RxContext = CONTAINING_RECORD( Entry,
                                       RX_CONTEXT,
                                       OverflowListEntry );

        RxContext->OverflowListEntry.Flink = NULL;

        ClearFlag( RxContext->Flags, RX_CONTEXT_FLAG_FSP_CRITICAL_OVERFLOW_QUEUE | RX_CONTEXT_FLAG_FSP_DELAYED_OVERFLOW_QUEUE );
    
    } else {
        
        RxContext = NULL;
        InterlockedDecrement( &RxDeviceObject->PostedRequestCount[WorkQueueType] );
    }

    KeReleaseSpinLock( &RxDeviceObject->OverflowQueueSpinLock, SavedIrql );

    return RxContext;
}

BOOLEAN
RxCancelOperationInOverflowQueue (
    PRX_CONTEXT RxContext
    )
/*++

Routine Description:

    This routine cancels the operation in the overflow queue

Arguments:

    RxContext    The context of the operation being synchronized

--*/
{
    BOOLEAN CancelledRequest = FALSE;

    PRDBSS_DEVICE_OBJECT RxDeviceObject;

    KIRQL SavedIrql;

    //
    //  currently, we use the wrapper's device object for all throttling.....
    //

    RxDeviceObject = RxFileSystemDeviceObject;

    KeAcquireSpinLock( &RxDeviceObject->OverflowQueueSpinLock, &SavedIrql );

    if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_FSP_CRITICAL_OVERFLOW_QUEUE | RX_CONTEXT_FLAG_FSP_DELAYED_OVERFLOW_QUEUE) &&
        (RxContext->OverflowListEntry.Flink != NULL)) {
        
        //
        //  Remove the entry from the overflow queue
        //

        RemoveEntryList( &RxContext->OverflowListEntry );
        RxContext->OverflowListEntry.Flink = NULL;

        if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_FSP_CRITICAL_OVERFLOW_QUEUE )) {
            RxDeviceObject->OverflowQueueCount[CriticalWorkQueue] -= 1;
        } else {
            RxDeviceObject->OverflowQueueCount[DelayedWorkQueue] -= 1;
        }

        ClearFlag( RxContext->Flags, RX_CONTEXT_FLAG_FSP_CRITICAL_OVERFLOW_QUEUE | RX_CONTEXT_FLAG_FSP_DELAYED_OVERFLOW_QUEUE );
        CancelledRequest = TRUE;
    }

    KeReleaseSpinLock( &RxDeviceObject->OverflowQueueSpinLock, SavedIrql );

    if (CancelledRequest) {
        
        RxRemoveOperationFromBlockingQueue( RxContext );
        RxCompleteRequest( RxContext, STATUS_CANCELLED );
    }

    return CancelledRequest;
}


//
//  The following constant is the maximum number of ExWorkerThreads that we
//  will allow to be servicing a particular target device at any one time.
//

#define FSP_PER_DEVICE_THRESHOLD         (1)


VOID
RxPrePostIrp (
    IN PVOID Context,
    IN PIRP Irp
    )
/*++

Routine Description:

    This routine performs any neccessary work before RxStatus(PENDING) is
    returned with the Fsd thread.  This routine is called within the
    filesystem and by the oplock package. The main issue is that we are about
    to leave the user's process so we need to get a systemwide address for
    anything in his address space that we require.

Arguments:

    Context - Pointer to the RxContext to be queued to the Fsp

    Irp - I/O Request Packet.

Return Value:

    None.

--*/
{
    PRX_CONTEXT RxContext = (PRX_CONTEXT) Context;
    PIO_STACK_LOCATION IrpSp;

    //
    //  If there is no Irp, we are done.
    //

    if (Irp == NULL) {
        return;
    }

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_NO_PREPOSTING_NEEDED )) {
         
        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_NO_PREPOSTING_NEEDED );

        //
        //  We need to lock the user's buffer, unless this is an MDL-read,
        //  in which case there is no user buffer.
        //
        //  **** we need a better test than non-MDL (read or write)!
        //

        if ((RxContext->MajorFunction == IRP_MJ_READ) || (RxContext->MajorFunction == IRP_MJ_WRITE)) {

            //
            //  If not an Mdl request, lock the user's buffer.
            //

            if (!FlagOn( RxContext->MinorFunction, IRP_MN_MDL )) {

                RxLockUserBuffer( RxContext,
                                  Irp,
                                  (RxContext->MajorFunction == IRP_MJ_READ) ? IoWriteAccess : IoReadAccess,
                                  IrpSp->Parameters.Write.Length );
            }

        //
        //  We also need to check whether this is a query file operation.
        //

        } else if ((RxContext->MajorFunction == IRP_MJ_DIRECTORY_CONTROL) && 
                   (RxContext->MinorFunction == IRP_MN_QUERY_DIRECTORY)) {

            RxLockUserBuffer( RxContext,
                              Irp,
                              IoWriteAccess,
                              IrpSp->Parameters.QueryDirectory.Length );

        //
        //  We also need to check whether this is a query ea operation.
        //

        } else if (RxContext->MajorFunction == IRP_MJ_QUERY_EA) {

            RxLockUserBuffer( RxContext,
                              Irp,
                              IoWriteAccess,
                              IrpSp->Parameters.QueryEa.Length );

        //
        //  We also need to check whether this is a set ea operation.
        //

        } else if (RxContext->MajorFunction == IRP_MJ_SET_EA) {

            RxLockUserBuffer( RxContext,
                              Irp,
                              IoReadAccess,
                              IrpSp->Parameters.SetEa.Length );
        }

        //
        //  Mark that we've already returned pending to the user
        //

        IoMarkIrpPending( Irp );
    }

    return;
}

#ifdef RDBSSLOG
//this stuff must be in nonpaged memory
                     ////           1 2 3 4 5 6 7
char RxFsdPost_SurrogateFormat[] = "%S%S%N%N%N%N%N";
                             ////        2  3   4         5        6    7
char RxFsdPost_ActualFormat[]    = "POST %s/%lx %08lx irp %lx thrd %lx #%lx";

#endif //ifdef RDBSSLOG

NTSTATUS
RxFsdPostRequest (
    IN PRX_CONTEXT RxContext
    )
/*++

Routine Description:

    This routine enqueues the request packet specified by RxContext to the
    Ex Worker threads.  This is a FSD routine.

Arguments:

    RxContext - Pointer to the RxContext to be queued to the Fsp

Return Value:

    RxStatus(PENDING)

--*/
{
    PIRP Irp = RxContext->CurrentIrp;

    if(!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_NO_PREPOSTING_NEEDED )) {
        
        RxPrePostIrp( RxContext, Irp );
    }

    RxLog(( RxFsdPost_SurrogateFormat, 
            RxFsdPost_ActualFormat,
            RXCONTX_OPERATION_NAME( RxContext->MajorFunction, TRUE ),
            RxContext->MinorFunction,
            RxContext, 
            RxContext->CurrentIrp,
            RxContext->LastExecutionThread,
            RxContext->SerialNumber ));
    RxWmiLog( LOG,
              RxFsdPostRequest,
              LOGPTR( RxContext )
              LOGPTR( RxContext->CurrentIrp )
              LOGPTR( RxContext->LastExecutionThread )
              LOGULONG( RxContext->SerialNumber )
              LOGUCHAR( RxContext->MinorFunction )
              LOGARSTR( RXCONTX_OPERATION_NAME( RxContext->MajorFunction,TRUE ) ) );

    RxAddToWorkque( RxContext, Irp );

    return STATUS_PENDING;
}



VOID
RxAddToWorkque (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
/*++

Routine Description:

    This routine is called to acually store the posted Irp to the Fsp
    workque.

Arguments:

    RxContext - Pointer to the RxContext to be queued to the Fsp

    Irp - I/O Request Packet.

Return Value:

    None.

--*/
{
    KIRQL SavedIrql;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    WORK_QUEUE_TYPE WorkQueueType;
    BOOLEAN PostToWorkerThread = FALSE;
                          
    ULONG IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

    //
    //  Send it off.....
    //

    RxContext->PostRequest = FALSE;

    //
    //  Check if this request has an associated file object, and thus volume
    //  device object.
    //
    
    if ((RxContext->MajorFunction == IRP_MJ_DEVICE_CONTROL) &&
        (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_REDIR_QUERY_PATH)) {
        WorkQueueType = DelayedWorkQueue;
        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_FSP_DELAYED_OVERFLOW_QUEUE );
    } else {
        WorkQueueType = CriticalWorkQueue;
        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_FSP_CRITICAL_OVERFLOW_QUEUE );
    }

    if (FileObject != NULL) {
        
        PRDBSS_DEVICE_OBJECT RxDeviceObject;
        LONG RequestCount;

        RxDeviceObject = CONTAINING_RECORD( IrpSp->DeviceObject, RDBSS_DEVICE_OBJECT, DeviceObject );

        //
        // currently, we use the wrapper's device object for all throttling.....
        //

        RxDeviceObject = RxFileSystemDeviceObject;

        //
        //  Check to see if this request should be sent to the overflow
        //  queue.  If not, then send it off to an exworker thread.
        //

        KeAcquireSpinLock( &RxDeviceObject->OverflowQueueSpinLock, &SavedIrql );

        RequestCount = InterlockedIncrement(&RxDeviceObject->PostedRequestCount[WorkQueueType]);

        PostToWorkerThread = (RequestCount > FSP_PER_DEVICE_THRESHOLD);

        if (PostToWorkerThread) {

            //
            //  We cannot currently respond to this IRP so we'll just enqueue it
            //  to the overflow queue on the volume.
            //

            InterlockedDecrement( &RxDeviceObject->PostedRequestCount[WorkQueueType] );

            InsertTailList( &RxDeviceObject->OverflowQueue[WorkQueueType],
                            &RxContext->OverflowListEntry );

            RxDeviceObject->OverflowQueueCount[WorkQueueType] += 1;

            KeReleaseSpinLock( &RxDeviceObject->OverflowQueueSpinLock, SavedIrql );

            return;

        } else {

            KeReleaseSpinLock( &RxDeviceObject->OverflowQueueSpinLock, SavedIrql );

        }

    } else {

        PostToWorkerThread = TRUE;

    }

#ifndef MONOLITHIC_MINIRDR

    IoQueueWorkItem( RxIoWorkItem, RxFspDispatchEx, WorkQueueType, RxContext );

#else 

    ExInitializeWorkItem( &RxContext->WorkQueueItem, RxFspDispatch, RxContext );

    ExQueueWorkItem( (PWORK_QUEUE_ITEM)&RxContext->WorkQueueItem, WorkQueueType );

#endif

    return;
}


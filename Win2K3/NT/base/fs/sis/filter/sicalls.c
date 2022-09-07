/*++

Copyright (c) 1997-1999  Microsoft Corporation

Module Name:

    sicalls.c

Abstract:

	Routines to make calls into the underlying file system for the
	Single Instance Store.  Generally, these are similar to the
	similar Zw calls, but they take file objects rather than handles
	and call down directly to the driver below SIS rather than going
	through the entire stack.

Authors:

    Bill Bolosky & Scott Cutshall, Summer, 1997

Environment:

    Kernel mode


Revision History:


--*/

#include "sip.h"

//
// Context used to communicate between SipQueryInformationFile and
// SiQueryInformationCompleted.
//
typedef struct _SI_QUERY_COMPLETION_CONTEXT {
		//
		// An event to indicate that the irp has completed.
		//
		KEVENT				event[1];

		//
		// The status copied out of the completed irp.
		//
		IO_STATUS_BLOCK		Iosb[1];

} SI_QUERY_COMPLETION_CONTEXT, *PSI_QUERY_COMPLETION_CONTEXT;

NTSTATUS
SiQueryInformationCompleted(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
	)
/*++

Routine Description:

	IO completion routine for Irps generated by SipQueryInformationFile.
	Since this irp originated with SIS, there is no place to pass it up
	to.  So, we copy the completion status into a return buffer, set
	an event indicating that the operation is done and free the irp.

Arguments:
	DeviceObject	- For the SIS device

	Irp				- The irp that's completing

	Context			- Pointer to a SI_QUERY_COMPLETION_CONTEXT; see the
						definition for a description of the contents.

Return Value:

	STATUS_MORE_PROCESSING_REQUIRED

--*/
{
	PSI_QUERY_COMPLETION_CONTEXT completionContext = (PSI_QUERY_COMPLETION_CONTEXT)Context;

	UNREFERENCED_PARAMETER(DeviceObject);

	*completionContext->Iosb = Irp->IoStatus;

	KeSetEvent(completionContext->event,IO_NO_INCREMENT,FALSE);

	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SipQueryInformationFile(
    IN PFILE_OBJECT 			FileObject,
	IN PDEVICE_OBJECT			DeviceObject,
    IN ULONG 					InformationClass,
    IN ULONG 					Length,
    OUT PVOID 					Information,
    OUT PULONG					ReturnedLength		OPTIONAL
    )

/*++

Routine Description:

    This routine returns the requested information about a specified file.
    The information returned is determined by the class that is specified,
    and it is placed into the caller's output buffer.  This call sends the
	irp directly to the driver immediately below SIS, so the file object
	must be something that's understood by the stack beneath SIS.

Arguments:
    FileObject - Supplies a pointer to the file object about which the requested
        information is returned.

	DeviceObject - the SIS device object for the device on which this file
		lies.

    InformationClass - Specifies the type of information which should be
        returned about the file/volume.

    Length - Supplies the length of the buffer in bytes.

    Information - Supplies a buffer to receive the requested information
        returned about the file.  This buffer must not be pageable and must
        reside in system space.

    ReturnedLength - Supplies a variable that is to receive the length of the
        information written to the buffer.

Return Value:

    The status returned is the final completion status of the operation.

--*/
{
	PDEVICE_EXTENSION	deviceExtension = DeviceObject->DeviceExtension;

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

	return SipQueryInformationFileUsingGenericDevice(
				FileObject,
				deviceExtension->AttachedToDeviceObject,
				InformationClass,
				Length,
				Information,
				ReturnedLength);
}

NTSTATUS
SipQueryInformationFileUsingGenericDevice(
    IN PFILE_OBJECT 			FileObject,
	IN PDEVICE_OBJECT			DeviceObject,
    IN ULONG 					InformationClass,
    IN ULONG 					Length,
    OUT PVOID 					Information,
    OUT PULONG					ReturnedLength		OPTIONAL
    )

/*++

Routine Description:

    This routine returns the requested information about a specified file.
    The information returned is determined by the class that is specified,
    and it is placed into the caller's output buffer.

Arguments:
    FileObject - Supplies a pointer to the file object about which the requested
        information is returned.

	DeviceObject - the SIS device object for the device on which this file
		lies.

    InformationClass - Specifies the type of information which should be
        returned about the file/volume.

    Length - Supplies the length of the buffer in bytes.

    Information - Supplies a buffer to receive the requested information
        returned about the file.  This buffer must not be pageable and must
        reside in system space.

    ReturnedLength - Supplies a variable that is to receive the length of the
        information written to the buffer.

Return Value:

    The status returned is the final completion status of the operation.

--*/

{
    PIRP 							irp;
    NTSTATUS 						status;
    PIO_STACK_LOCATION 				irpSp;
	SI_QUERY_COMPLETION_CONTEXT		completionContext[1];

    PAGED_CODE();

	ASSERT(IoGetRelatedDeviceObject(FileObject) == IoGetAttachedDevice(DeviceObject));
	ASSERT(!(FileObject->Flags & FO_STREAM_FILE));	// can't do this stuff on stream files

    //
    // Allocate and initialize the I/O Request Packet (IRP) for this operation.
    // The allocation is performed with an exception handler in case the
    // caller does not have enough quota to allocate the packet.
    //

    irp = IoAllocateIrp( DeviceObject->StackSize, FALSE );
    if (!irp) {

        //
        // An IRP could not be allocated.  Cleanup and return an appropriate
        // error status code.  Set a valid return Length.
        //

	    if (NULL != ReturnedLength) {
	        *ReturnedLength = 0;
	    }

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->RequestorMode = KernelMode;

    //
    // Fill in the service independent parameters in the IRP.
    //

    irp->UserEvent = (PKEVENT) NULL;
    irp->UserIosb = completionContext->Iosb;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;
    irp->Overlay.AsynchronousParameters.UserApcContext = NULL;

    //
    // Get a pointer to the stack location for the first driver.  This will be
    // used to pass the original function codes and parameters.
    //

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    irpSp->FileObject = FileObject;

    //
    // Set the system buffer address to the address of the caller's buffer and
    // set the flags so that the buffer is not deallocated.
    //

    irp->AssociatedIrp.SystemBuffer = Information;
    irp->Flags = IRP_BUFFERED_IO;

	KeInitializeEvent(completionContext->event,NotificationEvent,FALSE);

    //
    // Copy the caller's parameters to the service-specific portion of the
    // IRP.
    //

    irpSp->Parameters.QueryFile.Length = Length;
    irpSp->Parameters.QueryFile.FileInformationClass = InformationClass;

	IoSetCompletionRoutine(
			irp,
			SiQueryInformationCompleted,
			completionContext,
			TRUE,
			TRUE,
			TRUE);

    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //

    status = IoCallDriver( DeviceObject, irp );

    //
    // Wait for the operation to complete and obtain the final status from
    // the completion context, which gets it from the completed irp.
    //

    if (status == STATUS_PENDING) {
        status = KeWaitForSingleObject( completionContext->event,
                                        Executive,
                                        KernelMode,
										FALSE,
                                        NULL );
        status = completionContext->Iosb->Status;
    }

	if (NULL != ReturnedLength) {
	    *ReturnedLength = (ULONG)completionContext->Iosb->Information;
	}
    return status;
}

//
// Context used to communicate between SipSetInformationFile and
// SiSetInformationCompleted.
//
typedef struct _SI_SET_COMPLETION_CONTEXT {
		//
		// An event to indicate that the irp has completed.
		//
		KEVENT				event[1];

		//
		// The status copied out of the completed irp.
		//
		IO_STATUS_BLOCK		Iosb[1];

} SI_SET_COMPLETION_CONTEXT, *PSI_SET_COMPLETION_CONTEXT;


NTSTATUS
SiSetInformationCompleted(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
	)
/*++

Routine Description:

	IO completion routine for Irps generated by SipSetInformationFileUsingGenericDevice.
	Since this irp originated with SIS, there is no place to pass it up
	to.  So, we copy the completion status into a return buffer, set
	an event indicating that the operation is done and free the irp.

Arguments:
	DeviceObject	- For the SIS device

	Irp				- The irp that's completing

	Context			- Pointer to a SI_SET_COMPLETION_CONTEXT; see the
						definition for a description of the contents.

Return Value:

	STATUS_MORE_PROCESSING_REQUIRED

--*/
{
	PSI_SET_COMPLETION_CONTEXT completionContext = (PSI_SET_COMPLETION_CONTEXT)Context;

	UNREFERENCED_PARAMETER(DeviceObject);

	*completionContext->Iosb = Irp->IoStatus;

	KeSetEvent(completionContext->event,IO_NO_INCREMENT,FALSE);

	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SipSetInformationFile(
    IN PFILE_OBJECT 			FileObject,
	IN PDEVICE_OBJECT			DeviceObject,
    IN FILE_INFORMATION_CLASS	InformationClass,
    IN ULONG 					Length,
    IN PVOID 					Information
    )

/*++

Routine Description:

    This routine returns the requested information about a specified file.
    The information returned is determined by the class that is specified,
    and it is placed into the caller's output buffer.

	All this routine really does is to pull the filesystem device object
	out of the SIS device extension and pass it into the generic version
	of set information file.

Arguments:
    FileObject - Supplies a pointer to the file object about which the requested
        information is returned.

    FsInformationClass - Specifies the type of information which should be
        returned about the file/volume.

    Length - Supplies the length of the buffer in bytes.

	DeviceObject - the SIS device object for the device on which this file
		lies.

    FsInformation - Supplies a buffer to receive the requested information
        returned about the file.  This buffer must not be pageable and must
        reside in system space.

    ReturnedLength - Supplies a variable that is to receive the length of the
        information written to the buffer.

Return Value:

    The status returned is the final completion status of the operation.

--*/
{
	PDEVICE_EXTENSION	deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

	return SipSetInformationFileUsingGenericDevice(
				FileObject,
				deviceExtension->AttachedToDeviceObject,
				InformationClass,
				Length,
				Information);
}

NTSTATUS
SipSetInformationFileUsingGenericDevice(
    IN PFILE_OBJECT 			FileObject,
	IN PDEVICE_OBJECT			DeviceObject,
    IN FILE_INFORMATION_CLASS	InformationClass,
    IN ULONG 					Length,
    IN PVOID 					Information
    )

/*++

Routine Description:

    This routine returns the requested information about a specified file.
    The information returned is determined by the class that is specified,
    and it is placed into the caller's output buffer.

Arguments:
    FileObject - Supplies a pointer to the file object about which the requested
        information is returned.

    FsInformationClass - Specifies the type of information which should be
        returned about the file/volume.

    Length - Supplies the length of the buffer in bytes.

	DeviceObject - The device object onto which to make the call.  This must be
		an appropriate device for the given file.

    FsInformation - Supplies a buffer to receive the requested information
        returned about the file.  This buffer must not be pageable and must
        reside in system space.

    ReturnedLength - Supplies a variable that is to receive the length of the
        information written to the buffer.

Return Value:

    The status returned is the final completion status of the operation.

--*/
{
    PIRP 							irp;
    NTSTATUS 						status;
    PIO_STACK_LOCATION 				irpSp;
	SI_SET_COMPLETION_CONTEXT		completionContext[1];

    PAGED_CODE();

	ASSERT(IoGetRelatedDeviceObject(FileObject) == IoGetAttachedDevice(DeviceObject));
	ASSERT(!(FileObject->Flags & FO_STREAM_FILE));	// can't do this stuff on stream files

	ASSERT(InformationClass != FilePositionInformation);	// This call isn't supported in this routine
	ASSERT(InformationClass != FileTrackingInformation);	// This call isn't supported in this routine
	ASSERT(InformationClass != FileModeInformation);		// This call isn't supported in this routine
	ASSERT(InformationClass != FileCompletionInformation);	// This call isn't supported in this routine

    //
    // Allocate and initialize the I/O Request Packet (IRP) for this operation.
    // The allocation is performed with an exception handler in case the
    // caller does not have enough quota to allocate the packet.
    //

    irp = IoAllocateIrp( DeviceObject->StackSize, FALSE );
    if (!irp) {

        //
        // An IRP could not be allocated.  Cleanup and return an appropriate
        // error status code.
        //

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->RequestorMode = KernelMode;

    //
    // Fill in the service independent parameters in the IRP.
    //

    irp->UserEvent = (PKEVENT) NULL;
    irp->UserIosb = completionContext->Iosb;

	irp->Flags = IRP_SYNCHRONOUS_API;

    irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;
    irp->Overlay.AsynchronousParameters.UserApcContext = NULL;

    //
    // Get a pointer to the stack location for the first driver.  This will be
    // used to pass the original function codes and parameters.
    //

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
    irpSp->FileObject = FileObject;
	if (FileDispositionInformation == InformationClass) {
		//
		// Auditing code needs to know the handle used for a delete.
		// We're in kernel mode, so we don't need to provide one.
		// Null out the field.
		//
		irpSp->Parameters.SetFile.DeleteHandle = NULL;
	}

    //
    // Set the system buffer address to the address of the caller's buffer and
    // set the flags so that the buffer is not deallocated.
    //

    irp->AssociatedIrp.SystemBuffer = Information;

    irp->Flags |= IRP_BUFFERED_IO;

	KeInitializeEvent(completionContext->event,NotificationEvent,FALSE);

    //
    // Copy the caller's parameters to the service-specific portion of the
    // IRP.
    //

    irpSp->Parameters.SetFile.Length = Length;
    irpSp->Parameters.SetFile.FileInformationClass = InformationClass;

	IoSetCompletionRoutine(
			irp,
			SiSetInformationCompleted,
			completionContext,
			TRUE,
			TRUE,
			TRUE);

    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //

    status = IoCallDriver( DeviceObject, irp );

    //
    // Wait for the operation to complete and obtain the final status from
    // the completion context, which gets it from the completed irp.
    //

    if (status == STATUS_PENDING) {
        status = KeWaitForSingleObject( completionContext->event,
                                        Executive,
                                        KernelMode,
										FALSE,
                                        NULL );
        status = completionContext->Iosb->Status;
    }

    return status;
}


//
// Context used to communicate between SipFsControlFile and
// SiFsControlCompleted.
//

#if DBG
//used to detect if we are returning from IOCallDriver without STATUS_PENDING
//being returned but with the completion routine not yet called.
#define SIMAGIC_INIT        0xBad4Babe
#define SIMAGIC_COMPLETED   0xBad2Babe
#endif

typedef struct _SI_FS_CONTROL_COMPLETION_CONTEXT {

#if DBG
        ULONG               magic;
#endif

		//
		// An event to indicate that the irp has completed.
		//
		KEVENT				event[1];

		//
		// The status copied out of the completed irp.
		//
		IO_STATUS_BLOCK		Iosb[1];

} SI_FS_CONTROL_COMPLETION_CONTEXT, *PSI_FS_CONTROL_COMPLETION_CONTEXT;

NTSTATUS
SiFsControlCompleted(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
	)
/*++

Routine Description:

	IO completion routine for Irps generated by SipQueryInformationFile.
	Since this irp originated with SIS, there is no place to pass it up
	to.  So, we copy the completion status into a return buffer, set
	an event indicating that the operation is done, copy the output
	data into the caller's buffer if appropriate, and free the irp

Arguments:
	DeviceObject	- For the SIS device

	Irp				- The irp that's completing

	Context			- Pointer to a SI_FS_CONTROL_COMPLETION_CONTEXT; see the
						definition for a description of the contents.

Return Value:

	STATUS_MORE_PROCESSING_REQUIRED

--*/
{
	PSI_FS_CONTROL_COMPLETION_CONTEXT completionContext = (PSI_FS_CONTROL_COMPLETION_CONTEXT)Context;

	UNREFERENCED_PARAMETER(DeviceObject);

    ASSERT(completionContext->magic == SIMAGIC_INIT);
	*completionContext->Iosb = Irp->IoStatus;

	//
	// Handle cleanup processing for buffered IO.
	//
	if (Irp->Flags & IRP_BUFFERED_IO) {
		if (Irp->Flags & IRP_INPUT_OPERATION &&
			Irp->IoStatus.Status != STATUS_VERIFY_REQUIRED &&
			!NT_ERROR(Irp->IoStatus.Status)) {

            //
            // Copy the information from the system buffer to the caller's
            // buffer.  We don't need to worry about problems with the copy
			// because only KernelMode callers can use this interface.
            //

            RtlCopyMemory( Irp->UserBuffer,
                           Irp->AssociatedIrp.SystemBuffer,
                           Irp->IoStatus.Information );
		}

        //
        // Free the buffer if needed.
        //

        if (Irp->Flags & IRP_DEALLOCATE_BUFFER) {
            ExFreePool( Irp->AssociatedIrp.SystemBuffer );
        }

	}

#if DBG
    completionContext->magic = SIMAGIC_COMPLETED;
#endif

	KeSetEvent(completionContext->event,IO_NO_INCREMENT,FALSE);

	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SipFsControlFile(
	IN PFILE_OBJECT				fileObject,
	IN PDEVICE_OBJECT			DeviceObject,
	IN ULONG					ioControlCode,
	IN PVOID					inputBuffer,
	IN ULONG					inputBufferLength,
	OUT PVOID					outputBuffer,
	IN ULONG					outputBufferLength,
	OUT PULONG					returnedOutputBufferLength	OPTIONAL)
/*++

Routine Description:

	Call a file system control given a file object.  This file object must
	be recognized by the components underlying SIS on the driver stack.

Arguments:
    FileObject - Supplies a pointer to the file object on which to make the
		fsctl call

	DeviceObject - The device object onto which to make the call.  This must be
		an appropriate device for the given file.

	ioControlCode - The fsctl itself

	inputBuffer	- pointer to the buffer containing input data for the fsctl.  May be
			NULL is inputBufferLength is zero

	inputBufferLength - the size of the input buffer

	outputBuffer - buffer into which to place the data returned from the fsctl call.
		May be NULL is outputBufferLength is 0.

	outputBufferLength - the length of the output buffer.

	returnedOutputBufferLength - the actual count of bytes returned by the fsctl call.

Return Value:

    The status returned is the final completion status of the operation.

--*/
{
	PDEVICE_EXTENSION	deviceExtension = DeviceObject->DeviceExtension;

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

	return SipFsControlFileUsingGenericDevice(
				fileObject,
				deviceExtension->AttachedToDeviceObject,
				ioControlCode,
				inputBuffer,
				inputBufferLength,
				outputBuffer,
				outputBufferLength,
				returnedOutputBufferLength);
}


NTSTATUS
SipFsControlFileUsingGenericDevice(
	IN PFILE_OBJECT				fileObject,
	IN PDEVICE_OBJECT			DeviceObject,
	IN ULONG					ioControlCode,
	IN PVOID					inputBuffer,
	IN ULONG					inputBufferLength,
	OUT PVOID					outputBuffer,
	IN ULONG					outputBufferLength,
	OUT PULONG					returnedOutputBufferLength	OPTIONAL)
/*++

Routine Description:

	Call a file system control given a file object.

Arguments:
    FileObject - Supplies a pointer to the file object on which to make the
		fsctl call

	DeviceObject - The device object onto which to make the call.  This must be
		an appropriate device for the given file.

	ioControlCode - The fsctl itself

	inputBuffer	- pointer to the buffer containing input data for the fsctl.  May be
			NULL is inputBufferLength is zero

	inputBufferLength - the size of the input buffer

	outputBuffer - buffer into which to place the data returned from the fsctl call.
		May be NULL is outputBufferLength is 0.

	outputBufferLength - the length of the output buffer.

	returnedOutputBufferLength - the actual count of bytes returned by the fsctl call.

Return Value:

    The status returned is the final completion status of the operation.

--*/
{
	PIO_STACK_LOCATION					irpSp;
	PIRP								irp;
	SI_FS_CONTROL_COMPLETION_CONTEXT	completionContext[1];
	NTSTATUS							status;
	ULONG								method;

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	ASSERT(IoGetRelatedDeviceObject(fileObject) == IoGetAttachedDevice(DeviceObject));

#if		DBG
	if ((BJBDebug & 0x00800000) && (FSCTL_SET_REPARSE_POINT == ioControlCode)) {
		//
		// Fail the call for debugging 
		//
		DbgPrint("SIS: SipFsControlFile: intentionally failing FSCTL_SET_REPARSE_POINT for debugging reasons\n");
		return STATUS_UNSUCCESSFUL;
	}
#endif	// DBG

	//
	// Temporary hack: we need to do FSCTL_SET_REPARSE_POINT and FSCTL_DELETE_REPARSE_POINT on file objects
	// that the user opened read only.  In order to get NTFS to do this, we set the WriteAccess bit in the file
	// object.  Remove this hack once there's a more permanent fix in NTFS.
	//
	// We also now need to do this for FSCTL_SET_SPARSE.
	//
	if ((FSCTL_SET_REPARSE_POINT == ioControlCode) || (FSCTL_DELETE_REPARSE_POINT == ioControlCode) ||
		(FSCTL_SET_SPARSE == ioControlCode)) {
		fileObject->WriteAccess = TRUE;
	}

	//
	// The method is the input/output method of the IO control.  It is the low
	// two bits of the IO control code.
	//
	method = ioControlCode & 3;

	irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

	if (!irp) {
		SIS_MARK_POINT();
		return STATUS_INSUFFICIENT_RESOURCES;
	}

    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->Tail.Overlay.AuxiliaryBuffer = (PVOID) NULL;
    irp->RequestorMode = KernelMode;
    irp->PendingReturned = FALSE;
    irp->Cancel = FALSE;
    irp->CancelRoutine = (PDRIVER_CANCEL) NULL;

	irp->UserEvent = NULL;
	irp->UserIosb = NULL;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
    irp->Overlay.AsynchronousParameters.UserApcContext = NULL;

	irpSp = IoGetNextIrpStackLocation(irp);
	irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
	irpSp->FileObject = fileObject;

    irpSp->Parameters.DeviceIoControl.OutputBufferLength = outputBufferLength;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = inputBufferLength;
    irpSp->Parameters.DeviceIoControl.IoControlCode = ioControlCode;

    irp->MdlAddress = (PMDL) NULL;
    irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;

	switch (method) {
		case 0:
				if (inputBufferLength || outputBufferLength) {
					irp->AssociatedIrp.SystemBuffer = 
						ExAllocatePoolWithTag(NonPagedPool, 
							(inputBufferLength > outputBufferLength) ? inputBufferLength : outputBufferLength,
							' siS');

					if (irp->AssociatedIrp.SystemBuffer == NULL) {
						IoFreeIrp(irp);
						return STATUS_INSUFFICIENT_RESOURCES;
					}

					if (ARGUMENT_PRESENT(inputBuffer)) {
						RtlCopyMemory(	irp->AssociatedIrp.SystemBuffer,
										inputBuffer,
										inputBufferLength);
					}
					irp->Flags = IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;
					irp->UserBuffer = outputBuffer;

					if (ARGUMENT_PRESENT(outputBuffer)) {
						irp->Flags |= IRP_INPUT_OPERATION;
					}
				} else {
					irp->Flags = 0;
					irp->UserBuffer = NULL;
				}
				break;
		case 1:
		case 2:
				ASSERT(FALSE && "WRITEME");
		case 3:
				irpSp->Parameters.DeviceIoControl.Type3InputBuffer = inputBuffer;
				irp->Flags = 0;
				irp->UserBuffer = outputBuffer;
				break;
		default:
			ASSERT(FALSE && "SIS: SipFsControlFile: bogus method number\n");
	}

#if DBG
    completionContext->magic = SIMAGIC_INIT;
#endif
	KeInitializeEvent(completionContext->event,NotificationEvent,FALSE);

	IoSetCompletionRoutine(
			irp,
			SiFsControlCompleted,
			completionContext,
			TRUE,
			TRUE,
			TRUE);
			
	status = IoCallDriver(DeviceObject, irp);

	if (status == STATUS_PENDING) {

		status = KeWaitForSingleObject(completionContext->event,Executive,KernelMode,FALSE, NULL);
		ASSERT(status == STATUS_SUCCESS);
		status = completionContext->Iosb->Status;
	}

#if DBG
    ASSERT(completionContext->magic == SIMAGIC_COMPLETED);
    if (completionContext->magic != SIMAGIC_COMPLETED) {    //wait for completion routine

		DbgPrint("SIS: SipFsControlFile: waiting for completion routine!\n");
		status = KeWaitForSingleObject(completionContext->event,Executive,KernelMode,FALSE, NULL);
		ASSERT(status == STATUS_SUCCESS);
		status = completionContext->Iosb->Status;
    }
#endif

	SIS_MARK_POINT_ULONG(completionContext->Iosb->Information);

	if (NULL != returnedOutputBufferLength) {
		*returnedOutputBufferLength =
            (ULONG)completionContext->Iosb->Information;
	}

#if DBG
    completionContext->magic = 0;   //mark it as freed
#endif
	return status;
	
}


//
// Context used to communicate between SipFlushBuffersFile and
// SiFlushBuffersCompleted.
//
typedef struct _SI_FLUSH_COMPLETION_CONTEXT {
		//
		// An event to indicate that the irp has completed.
		//
		KEVENT				event[1];

		//
		// The status copied out of the completed irp.
		//
		IO_STATUS_BLOCK		Iosb[1];
} SI_FLUSH_COMPLETION_CONTEXT, *PSI_FLUSH_COMPLETION_CONTEXT;

NTSTATUS
SiFlushBuffersCompleted(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
	)
/*++

Routine Description:

	IO completion routine for Irps generated by SipFlushBuffersFile.
	Since this irp originated with SIS, there is no place to pass it up
	to.  So, we copy the completion status into a return buffer, set
	an event indicating that the operation is done and free the irp.

Arguments:
	DeviceObject	- For the SIS device

	Irp				- The irp that's completing

	Context			- Pointer to a SI_FLUSH_COMPLETION_CONTEXT; see the
						definition for a description of the contents.

Return Value:

	STATUS_MORE_PROCESSING_REQUIRED

--*/
{
	PSI_QUERY_COMPLETION_CONTEXT completionContext = (PSI_QUERY_COMPLETION_CONTEXT)Context;

	UNREFERENCED_PARAMETER(DeviceObject);

	*completionContext->Iosb = Irp->IoStatus;

	KeSetEvent(completionContext->event,IO_NO_INCREMENT,FALSE);

	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SipFlushBuffersFile(
    IN PFILE_OBJECT 			FileObject,
	IN PDEVICE_OBJECT			DeviceObject
    )

/*++

Routine Description:

	This routine flushes the cache for a given file object.  It is
	synchronous; it doesn't return until the flush is completed.

Arguments:
    FileObject - Supplies a pointer to the file object about which the requested
        information is returned.

	DeviceObject - the SIS device object for the device on which this file
		lies.

Return Value:

    The status returned is the final completion status of the operation.

--*/

{
    PIRP 							irp;
    NTSTATUS 						status;
    PIO_STACK_LOCATION 				irpSp;
	PDEVICE_EXTENSION				deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
	SI_FLUSH_COMPLETION_CONTEXT		completionContext[1];

    PAGED_CODE();

	ASSERT(IoGetRelatedDeviceObject(FileObject) == IoGetAttachedDevice(DeviceObject));
	ASSERT(!(FileObject->Flags & FO_STREAM_FILE));	// can't do this stuff on stream files

    //
    // Allocate and initialize the I/O Request Packet (IRP) for this operation.
    // The allocation is performed with an exception handler in case the
    // caller does not have enough quota to allocate the packet.
    //

    irp = IoAllocateIrp( deviceExtension->AttachedToDeviceObject->StackSize, FALSE );
    if (!irp) {

        //
        // An IRP could not be allocated.  Cleanup and return an appropriate
        // error status code.
        //

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->RequestorMode = KernelMode;

    //
    // Fill in the service independent parameters in the IRP.
    //

    irp->UserEvent = (PKEVENT) NULL;
    irp->UserIosb = completionContext->Iosb;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;
    irp->Overlay.AsynchronousParameters.UserApcContext = NULL;

    //
    // Get a pointer to the stack location for the first driver.  This will be
    // used to pass the original function codes and parameters.
    //

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_FLUSH_BUFFERS;
    irpSp->FileObject = FileObject;

    irp->Flags = IRP_SYNCHRONOUS_API;

	KeInitializeEvent(completionContext->event,NotificationEvent,FALSE);

    //
    // The flush buffers irp doesn't have any parameters.
    //

	IoSetCompletionRoutine(
			irp,
			SiFlushBuffersCompleted,
			completionContext,
			TRUE,
			TRUE,
			TRUE);

    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //

    status = IoCallDriver( deviceExtension->AttachedToDeviceObject, irp );

    //
    // Wait for the operation to complete and obtain the final status from
    // the completion context, which gets it from the completed irp.
    //

    if (status == STATUS_PENDING) {
        status = KeWaitForSingleObject( completionContext->event,
                                        Executive,
                                        KernelMode,
										FALSE,
                                        NULL );
        status = completionContext->Iosb->Status;
    }

    return status;
}

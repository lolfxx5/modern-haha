/*++

Copyright (c) 1996-1997  Microsoft Corporation

Module Name:

    clusdskp.h

Abstract:

    Private header file for the cluster disk driver.

Authors:

    Rod Gamache     30-Mar-1997

Environment:

    kernel mode only

Notes:

Revision History:


--*/

#define _NTDDK_ // [HACKHACK] to make ProbeForRead work. Better to include ntddk instead of ntos //

#include "ntos.h"
#include "zwapi.h"
#include "stdarg.h"
#include "stdio.h"
#include "ntddscsi.h"
#include "ntdddisk.h"
#include "clusdef.h"

#if 1                // turn on tagging all the time
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'kdSC')
#endif


//
// Global definitions
//

#define CLUSDISK_ROOT_DEVICE                L"\\Device\\ClusDisk0"
#define CLUSDISK_SIGNATURE_DISK_NAME        L"DiskName"
#define CLUSDISK_SIGNATURE_FIELD            L"\\12345678"
#define CLUSDISK_PARAMETERS_KEYNAME         L"\\Parameters"
#define CLUSDISK_SIGNATURE_KEYNAME          L"\\Signatures"
#define CLUSDISK_AVAILABLE_DISKS_KEYNAME    L"\\AvailableDisks"
#define CLUSDISK_SINGLE_BUS_KEYNAME         L"SingleBus"

#define CLUSSVC_VALUENAME_MANAGEDISKSONSYSTEMBUSES L"ManageDisksOnSystemBuses"

#define ID_CLEANUP              ' nlC'
#define ID_CLEANUP_DEV_OBJ      'OnlC'
#define ID_CLUSTER_RESET_IOCTL  'tsRI'
#define ID_RESET                ' tsR'
#define ID_CLUSTER_ARB_RESET    'tsRA'
#define ID_RESET_BUSSES         'BtsR'
#define ID_GET_PARTITION        'traP'
#define ID_GET_GEOMETRY         'moeG'

#define UNINITIALIZED_DISK_NUMBER           (ULONG)-1

#define MAX_BUSSES          20              // Maximum number of shared busses

#define MAX_BUFFER_SIZE     256             // Maximum buffer size

#define MAX_RETRIES 2

//
// KeEnterCriticalRegion is required if resource
// acquisition is done at a PASSIVE level in the context
// of non-kernel thread.
//
// KeEnterCriticalRegion() == KeGetCurrentThread()->KernelApcDisable -= 1;
//
// guarantees that the thread in which we execute cannot get
// suspeneded in APC while we own the global resource.
//

#define ACQUIRE_EXCLUSIVE( _lock ) \
    do { KeEnterCriticalRegion();ExAcquireResourceExclusiveLite(_lock, TRUE); } while(0)

#define ACQUIRE_SHARED( _lock ) \
    do { KeEnterCriticalRegion();ExAcquireResourceSharedLite(_lock, TRUE); } while(0)

#define RELEASE_EXCLUSIVE( _lock ) \
    do { ExReleaseResourceLite( _lock );KeLeaveCriticalRegion(); } while(0)

#define RELEASE_SHARED( _lock ) \
    do { ExReleaseResourceLite( _lock );KeLeaveCriticalRegion(); } while(0)


// #define RESERVE_TIMER   3 // [GN] moved to cluster\inc\diskarbp.h

#if DBG
#define ClusDiskPrint(x)  ClusDiskDebugPrint x
#define WCSLEN_ASSERT( _buf )   ( wcslen( _buf ) < (sizeof( _buf ) / sizeof( WCHAR )))
#else
#define ClusDiskPrint(x)
#define WCSLEN_ASSERT( _buf )
#endif  // DBG


//
// Error log messages
//
#define CLUSDISK_BAD_DEVICE L"Skipping device. Possible filter driver installed!"


//
// Macros

#define IsAlpha( c ) \
    ( ((c) >= 'a' && (c) <= 'z') || ((c) >='A' && (c) <= 'Z') )

//
// Device Extension
//

typedef struct _CLUS_DEVICE_EXTENSION {

    //
    // Back pointer to this extension's device object
    //

    PDEVICE_OBJECT DeviceObject;

    //
    // device object to which clusdisk's device obj is attached
    //

    PDEVICE_OBJECT TargetDeviceObject;

    //
    // back ptr to clusdisk Partition0 Device Object
    //

    PDEVICE_OBJECT PhysicalDevice;

    //
    // The SCSI_ADDRESS for this device
    //

    SCSI_ADDRESS    ScsiAddress;

    //
    // Disk signature
    //

    ULONG           Signature;

    //
    // Disk number for reference on verifying attach
    //

    ULONG          DiskNumber;

    //
    // Disk State. This is only maintained in the physical or partition
    // zero extension.
    //

    ULONG          DiskState;

    //
    // Reservation timer - valid on the physical (partition0) extension
    //

    ULONG          ReserveTimer;

    //
    // Time last reserve started.  Not protected by a lock as only one
    // routine ever updates this value.
    //

    LARGE_INTEGER   LastReserveStart;

    //
    // Event flag for use with reservation IRP.
    // Use is controlled by the TimerBusy flag.
    //

    KEVENT          Event;

    //
    // The bus type.  E.G. Scsi, SSA, etc.
    //

    ULONG           BusType;

    //
    // Last reserve failure.
    //

    NTSTATUS        ReserveFailure;

    //
    // Waiting IOCTL's looking for Reserve Failure notification.
    //

    LIST_ENTRY      WaitingIoctls;

    //
    // Work queue item context.
    //

    WORK_QUEUE_ITEM WorkItem;

    //
    // Reservation time IRP
    //

    BOOLEAN         PerformReserves;

    //
    // Work queue item busy.
    //

    BOOLEAN         TimerBusy;

    //
    // Attached state. True if this device object is supposed to be
    // attached. False if not sure.
    //

    BOOLEAN         AttachValid;

    //
    // Device is detached.
    //

    BOOLEAN         Detached;

    //
    // Flag to indicate offline/terminate is in process.
    //

    BOOLEAN         OfflinePending;

    //
    // The driver object for use on repartitioning. RNGFIX -???
    //

    PDRIVER_OBJECT DriverObject;

    //
    // The partition number for the last extension created
    // only maintained in the physical or partition zero extension.
    //

    ULONG          LastPartitionNumber;

    //
    // context value for when we deregister from disk device
    // notifications
    //

    PVOID          DiskNotificationEntry;

    //
    // context value for when we deregister from mounted device
    // notifications
    //

    PVOID          VolumeNotificationEntry;

    // [GN]
    //      Physical Sector Size of the device
    //      If SectorSize == 0 then
    //      persistent writing is disabled

    ULONG          SectorSize;

    //
    //      Physical Sector which is used
    //      for persistent reservations.

    ULONG          ArbitrationSector;

    //
    //      For tracing purposes:
    //      Approximate time of the last write to
    //      the disk. (Approximate, because we
    //      update this field without holding any locks)
    //

    LARGE_INTEGER  LastWriteTime;

    //
    // P0 object stores an array of handles to all volumes on
    // this disk to be dismounted when offline happens
    // First entry in this array is a number of handles in the array
    //

    PHANDLE VolumeHandles;

    //
    // Lock to prevent removal while I/O in progress.
    //

    IO_REMOVE_LOCK  RemoveLock;

    //
    // Keep track of paging files, crash dump files, and hibernation files.
    //

    KEVENT          PagingPathCountEvent;
    ULONG           PagingPathCount;
    ULONG           HibernationPathCount;
    ULONG           DumpPathCount;

    //
    // Cache partition info when possible.
    //

    PDRIVE_LAYOUT_INFORMATION_EX    DriveLayout;
    ULONG                       DriveLayoutSize;
    ERESOURCE                   DriveLayoutLock;

    //
    // Number of arbitration writes and reserves in progress.
    //

    LONG    ArbWriteCount;
    LONG    ReserveCount;

    //
    // Time when the last reserve successfully completed.
    // Protected by ReserveInfoLock as this value could be updated
    // by multiple threads.
    //

    LARGE_INTEGER   LastReserveEnd;

    //
    // Lock to control access to LastReserveEnd
    //

    ERESOURCE       ReserveInfoLock;

} CLUS_DEVICE_EXTENSION, *PCLUS_DEVICE_EXTENSION;

#define DEVICE_EXTENSION_SIZE sizeof(CLUS_DEVICE_EXTENSION)

//
// Device list entry
//

typedef struct _DEVICE_LIST_ENTRY {
    struct _DEVICE_LIST_ENTRY *Next;
    ULONG   Signature;
    PDEVICE_OBJECT DeviceObject;
    BOOLEAN Attached;
    BOOLEAN LettersAssigned;
    BOOLEAN FreePool;
} DEVICE_LIST_ENTRY, *PDEVICE_LIST_ENTRY;

typedef struct _SCSI_BUS_ENTRY {
    struct _SCSI_BUS_ENTRY *Next;
    UCHAR   Port;
    UCHAR   Path;
    USHORT  Reserved;
} SCSI_BUS_ENTRY, *PSCSI_BUS_ENTRY;

typedef enum _ClusterBusType {
    RootBus,
    ScsiBus,
    UnknownBus
} ClusterBusType;


typedef struct _WORK_CONTEXT {
    LIST_ENTRY      ListEntry;
    PDEVICE_OBJECT  DeviceObject;
    KEVENT          CompletionEvent;
    NTSTATUS        FinalStatus;
    PVOID           Context;
    PIO_WORKITEM    WorkItem;
} WORK_CONTEXT, *PWORK_CONTEXT;

//
// Flags for ClusDiskpReplaceHandleArray
//

enum {
    DO_DISMOUNT         = 0x00000001,
    RELEASE_REMOVE_LOCK = 0x00000002,
    CLEANUP_STORAGE     = 0x00000004,
    SET_PART0_EVENT     = 0x00000008,
};

typedef struct _REPLACE_CONTEXT {
    PCLUS_DEVICE_EXTENSION  DeviceExtension;
    PHANDLE                 NewValue;           // OPTIONAL
    PHANDLE                 OldValue;
    PKEVENT                 Part0Event;
    ULONG                   Flags;
} REPLACE_CONTEXT, *PREPLACE_CONTEXT;

typedef struct _HALTPROC_CONTEXT {
    PCLUS_DEVICE_EXTENSION  DeviceExtension;
    PHANDLE                 FileHandle;
} HALTPROC_CONTEXT, *PHALTPROC_CONTEXT;

typedef struct _VOL_STATE_INFO {
    PIO_WORKITEM    WorkItem;
    ULONG           NewDiskState;
} VOL_STATE_INFO, *PVOL_STATE_INFO;

typedef struct _DEVICE_CHANGE_CONTEXT {
    PIO_WORKITEM    WorkItem;
    PCLUS_DEVICE_EXTENSION  DeviceExtension;
    UNICODE_STRING          SymbolicLinkName;
    ULONG                   Signature;
    ULONG                   DeviceNumber;
    ULONG                   PartitionNumber;
    SCSI_ADDRESS            ScsiAddress;
} DEVICE_CHANGE_CONTEXT, *PDEVICE_CHANGE_CONTEXT;

//
// Structure for offline to preserve snapshots.
//

typedef struct _OFFLINE_ENTRY {
    ULONG DiskNumber;
    ULONG PartitionNumber;
    BOOLEAN OfflineSent;
    struct _OFFLINE_ENTRY * Next;
} OFFLINE_ENTRY, *POFFLINE_ENTRY;

//
// Structure to offline disk in sync thread instead of using
// worker thread (async).
//

typedef struct _OFFLINE_DISK_ENTRY {
    PCLUS_DEVICE_EXTENSION DeviceExtension;
    struct _OFFLINE_DISK_ENTRY * Next;
} OFFLINE_DISK_ENTRY, *POFFLINE_DISK_ENTRY;

//
// Info for synchronous reserves and arbitration
//

typedef NTSTATUS ( * ArbFunction)( IN PCLUS_DEVICE_EXTENSION DeviceExtenion,
                                   IN PVOID Context );

typedef enum _ArbIoType {
    ArbIoWrite,
    ArbIoReserve,
    ArbIoInvalid
} ArbIoType;

typedef struct _ARB_RESERVE_COMPLETION {
    LARGE_INTEGER   IoStartTime;
    LARGE_INTEGER   IoEndTime;
    ULONG           RetriesLeft;
    ArbIoType       Type;
    PVOID           LockTag;
    PDEVICE_OBJECT  DeviceObject;
    PCLUS_DEVICE_EXTENSION  DeviceExtension;
    PIO_WORKITEM    WorkItem;
    NTSTATUS        FinalStatus;
    ArbFunction     RetryRoutine;           // Optional routine called if I/O fails & RetriesLeft > 0
    ArbFunction     FailureRoutine;         // Optional routine called if I/O fails & RetriesLeft == 0
    ArbFunction     PostCompletionRoutine;  // Optional routine called if I/O succeeds
} ARB_RESERVE_COMPLETION, *PARB_RESERVE_COMPLETION;


//
// Function declarations
//


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
ClusDiskScsiInitialize(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID NextDisk,
    IN ULONG Count
    );

VOID
ClusDiskUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
ClusDiskCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClusDiskClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClusDiskCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClusDiskRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClusDiskWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClusDiskIoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
ClusDiskDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClusDiskRootDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ClusDiskShutdownFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClusDiskNewDiskCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

NTSTATUS
ClusDiskSetLayoutCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

NTSTATUS
ClusDiskDismountDevice(
    IN ULONG    DiskNumber,
    IN BOOLEAN  ForceDismount
    );

BOOLEAN
ClusDiskAttached(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG    DiskNumber
    );

BOOLEAN
ClusDiskVerifyAttach(
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
AddAttachedDevice(
    IN ULONG Signature,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
MatchDevice(
    IN ULONG Signature,
    OUT PDEVICE_OBJECT *DeviceObject
    );

NTSTATUS
GetScsiAddress(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_ADDRESS ScsiAddress
    );

VOID
ClusDiskWriteDiskInfo(
    IN ULONG Signature,
    IN ULONG DiskNumber,
    IN LPWSTR KeyName
    );

PDRIVE_LAYOUT_INFORMATION_EX
ClusDiskGetPartitionInfo(
    PCLUS_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
ClusDiskTryAttachDevice(
    ULONG          Signature,
    ULONG          NextDisk,
    PDRIVER_OBJECT DriverObject,
    BOOLEAN        InstallMode
    );

NTSTATUS
ClusDiskAttachDevice(
    ULONG          Signature,
    ULONG          NextDisk,
    PDRIVER_OBJECT DriverObject,
    BOOLEAN        Reset,
    BOOLEAN        *StopProcessing,
    BOOLEAN        InstallMode
    );

NTSTATUS
ClusDiskDetachDevice(
    ULONG          Signature,
    PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DismountDevice(
    IN HANDLE FileHandle
    );

NTSTATUS
ClusDiskGetDiskGeometry(
    PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
GetDiskGeometry(
    PDEVICE_OBJECT DeviceObject,
    PMEDIA_TYPE MediaType
    );

PDRIVE_LAYOUT_INFORMATION_EX
GetPartitionInfo(
    PDEVICE_OBJECT DeviceObject,
    NTSTATUS       *Status
    );

NTSTATUS
ResetScsiDevice(
    IN HANDLE ScsiportHandle,
    IN PSCSI_ADDRESS ScsiAddress
    );

NTSTATUS
ReserveScsiDevice(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PVOID Context
    );

VOID
ReleaseScsiDevice(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension
    );

BOOLEAN
AttachedDevice(
    IN ULONG Signature,
    OUT PDEVICE_OBJECT *DeviceObject
    );

NTSTATUS
EnableHaltProcessing(
    IN KIRQL *Irql
    );

NTSTATUS
DisableHaltProcessing(
    IN KIRQL *Irql
    );

VOID
ClusDiskEventCallback(
    IN CLUSNET_EVENT_TYPE   EventType,
    IN CL_NODE_ID           NodeId,
    IN CL_NETWORK_ID        NetworkId
    );

VOID
ClusDiskLogError(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN ULONG SequenceNumber,
    IN UCHAR MajorFunctionCode,
    IN UCHAR RetryCount,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN NTSTATUS SpecificIOStatus,
    IN ULONG LengthOfText,
    IN PWCHAR Text
    );

NTSTATUS
DismountPartition(
    IN PDEVICE_OBJECT TargetDevice,
    IN ULONG DiskNumber,
    IN ULONG PartNumber
    );


#if DBG
VOID
ClusDiskDebugPrint(
    IN ULONG Level,
    IN PCHAR DebugMessage,
    ...
    );
#endif


VOID
GetSymbolicLink(
    IN PWCHAR Root,
    IN OUT PWCHAR Path
    );

NTSTATUS
ClusDiskGetTargetDevice(
    IN ULONG                        DiskNumber,
    IN ULONG                        PartitionNumber,
    OUT PDEVICE_OBJECT              * DeviceObject OPTIONAL,
    IN OUT PUNICODE_STRING          UnicodeString,
    OUT PDRIVE_LAYOUT_INFORMATION_EX   * PartitionInfo OPTIONAL,
    OUT PSCSI_ADDRESS               ScsiAddress OPTIONAL,
    IN BOOLEAN                      Reset
    );

//[GN]
NTSTATUS
ArbitrationInitialize(
    VOID
    );

VOID
ArbitrationDone(
    VOID
    );

VOID
ArbitrationTick(
    VOID
    );

VOID
ArbitrationWrite(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
VerifyArbitrationArgumentsIfAny(
    IN PULONG                 InputData,
    IN LONG                   InputSize
    );

VOID
ProcessArbitrationArgumentsIfAny(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PULONG                 InputData,
    IN LONG                   InputSize
    );

NTSTATUS
ProcessArbitrationEscape(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PULONG                 InputData,
    IN LONG                   InputSize,
    IN OUT PULONG             OutputSize
    );

NTSTATUS
SimpleDeviceIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG          Ioctl,
    IN PVOID          InBuffer,
    IN ULONG          InBufferSize,
    IN PVOID          OutBuffer,
    IN ULONG          OutBufferSize);

NTSTATUS
ClusDiskInitRegistryString(
    OUT PUNICODE_STRING UnicodeString,
    IN  LPWSTR          KeyName,
    IN  ULONG           KeyNameChars
    );

NTSTATUS
ClusDiskAddSignature(
    IN PUNICODE_STRING  UnicodeString,
    IN ULONG   Signature,
    IN BOOLEAN Volatile
    );

NTSTATUS
ClusDiskDeleteSignature(
    IN PUNICODE_STRING  UnicodeString,
    IN ULONG   Signature
    );

ULONG
ClusDiskIsSignatureDisk(
    IN ULONG Signature
    );

NTSTATUS
ClusDiskMarkIrpPending(
    PIRP                Irp,
    PDRIVER_CANCEL      CancelRoutine
    );

VOID
ClusDiskCompletePendingRequest(
    IN PIRP                 Irp,
    IN NTSTATUS             Status,
    PCLUS_DEVICE_EXTENSION  DeviceExtension
    );

VOID
ClusDiskIrpCancel(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

MEDIA_TYPE
GetMediaType(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
GetScsiPortNumber(
    IN ULONG DiskSignature,
    IN PUCHAR DiskPortNumber
    );

NTSTATUS
IsDiskClusterCapable(
   IN UCHAR PortNumber,
   OUT PBOOLEAN IsCapable
   );

NTSTATUS
GetBootTimeSystemRoot(
    IN OUT PWCHAR        Path
    );

NTSTATUS
GetRunTimeSystemRoot(
    IN OUT PWCHAR        Path
    );

NTSTATUS
GetSystemRootPort(
    VOID
    );

VOID
ResetScsiBusses(
    VOID
    );

NTSTATUS
GetDriveLayout(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PDRIVE_LAYOUT_INFORMATION_EX *DriveLayout,
    BOOLEAN UpdateCachedLayout,
    BOOLEAN FlushStorageDrivers
    );

NTSTATUS
ClusDiskInitialize(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
LockVolumes(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
ClusDiskHaltProcessingWorker(
    IN PVOID Context
    );

VOID
SendOfflineDirect(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
ClusDiskRescanWorker(
    IN PVOID Context
    );

VOID
ClusDiskTickHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

NTSTATUS
RegistryQueryValue(
    PVOID hKey,
    LPWSTR pValueName,
    PULONG pulType,
    PVOID pData,
    PULONG pulDataSize
    );

NTSTATUS
ClusDiskCreateHandle(
    OUT PHANDLE     pHandle,
    IN  ULONG       DiskNumber,
    IN  ULONG       PartitionNumber,
    IN  ACCESS_MASK DesiredAccess
    );

VOID
ClusDiskCompletePendedIrps(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN ULONG        Offline
    );

NTSTATUS
ClusDiskOfflineEntireDisk(
    IN PDEVICE_OBJECT Part0DeviceObject
    );

NTSTATUS
ClusDiskDismountVolumes(
    IN PDEVICE_OBJECT Part0DeviceObject,
    IN BOOLEAN RelRemLock
    );

NTSTATUS
ClusDiskForwardIrpSynchronous(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
ClusDiskReservationWorker(
    IN PCLUS_DEVICE_EXTENSION  DeviceExtension
    );

VOID
ClusDiskpReplaceHandleArray(
    PDEVICE_OBJECT DeviceObject,
    PWORK_CONTEXT WorkContext
    );

VOID
ClusDiskpOpenFileHandles(
    PDEVICE_OBJECT Part0DeviceObject,
    PWORK_CONTEXT WorkContext
    );

NTSTATUS
EjectVolumes(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ReclaimVolumes(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ProcessDelayedWorkAsynchronous(
    PDEVICE_OBJECT DeviceObject,
    PVOID WorkerRoutine,
    PVOID Context
    );

NTSTATUS
ProcessDelayedWorkSynchronous(
    PDEVICE_OBJECT DeviceObject,
    PVOID WorkerRoutine,
    PVOID Context
    );

VOID
EnableHaltProcessingWorker(
    PDEVICE_OBJECT DeviceObject,
    PWORK_CONTEXT WorkContext
    );

VOID
DisableHaltProcessingWorker(
    PDEVICE_OBJECT DeviceObject,
    PWORK_CONTEXT WorkContext
    );

NTSTATUS
GetRegistryValue(
    PUNICODE_STRING KeyName,
    PWSTR ValueName,
    PULONG ReturnValue
    );

NTSTATUS
SetVolumeState(
    PCLUS_DEVICE_EXTENSION PhysicalDisk,
    ULONG NewDiskState
    );

VOID
SetVolumeStateWorker(
    PDEVICE_OBJECT DeviceObject,
    PVOID Context
    );

NTSTATUS
AttachSignatureList(
    PDEVICE_OBJECT DeviceObject,
    PULONG InBuffer,
    ULONG InBufferLen
    );

NTSTATUS
DetachSignatureList(
    PDEVICE_OBJECT DeviceObject,
    PULONG InBuffer,
    ULONG InBufferLen
    );

NTSTATUS
IsVolumeMounted(
    IN ULONG DiskNumber,
    IN ULONG PartNumber,
    OUT BOOLEAN *IsMounted
    );

NTSTATUS
SendFtdiskIoctlSync(
    PDEVICE_OBJECT TargetObject,
    IN ULONG DiskNumber,
    IN ULONG PartNumber,
    ULONG Ioctl
    );

NTSTATUS
ClusDiskDeviceChangeNotification(
    IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION DeviceChangeNotification,
    IN PCLUS_DEVICE_EXTENSION      DeviceExtension
    );

NTSTATUS
ClusDiskDeviceChangeNotificationWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

NTSTATUS
ClusDiskVolumeChangeNotification(
    IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION DeviceChangeNotification,
    IN PCLUS_DEVICE_EXTENSION      DeviceExtension
    );

NTSTATUS
ClusDiskVolumeChangeNotificationWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

NTSTATUS
ProcessDeviceArrival(
    IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION DeviceChangeNotification,
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN BOOLEAN VolumeArrival
    );

NTSTATUS
CleanupDeviceList(
    PDEVICE_OBJECT DeviceObject
    );

VOID
CleanupDeviceListWorker(
    PDEVICE_OBJECT DeviceObject,
    PVOID Context
    );

NTSTATUS
CreateVolumeObject(
    PCLUS_DEVICE_EXTENSION ZeroExtension,
    ULONG DiskNumber,
    ULONG PartitionNumber,
    PDEVICE_OBJECT TargetDev
    );

NTSTATUS
WaitForAttachCompletion(
    PCLUS_DEVICE_EXTENSION DeviceExtension,
    BOOLEAN WaitForInit,
    BOOLEAN CheckPhysDev
    );

NTSTATUS
GetReserveInfo(
    PVOID   InOutBuffer,
    ULONG   InSize,
    ULONG*  OutSize
    );

NTSTATUS
SetDiskState(
    PVOID InBuffer,
    ULONG InBufferLength,
    ULONG OutBufferLength,
    ULONG *BytesReturned
    );

NTSTATUS
ArbReserveCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

RequeueArbReserveIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

NTSTATUS
CheckReserveTiming(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PVOID Context
    );

NTSTATUS
HandleReserveFailure(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PVOID Context
    );

#if DBG

//
// RemoveLock tracing functions.
//

NTSTATUS
AcquireRemoveLock(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN OPTIONAL PVOID Tag
    );

VOID
ReleaseRemoveLock(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN PVOID Tag
    );

VOID
ReleaseRemoveLockAndWait(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN PVOID Tag
    );

//
// Debug print helper routines
//

PCHAR
PnPMinorFunctionString (
    UCHAR MinorFunction
    );

PCHAR
BoolToString(
    BOOLEAN Value
    );

PCHAR
DiskStateToString(
    ULONG DiskState
    );


#else

#define ReleaseRemoveLock(RemoveLock, Tag)          IoReleaseRemoveLock(RemoveLock, Tag)

NTSTATUS
AcquireRemoveLock(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN OPTIONAL PVOID Tag
    );

VOID
ReleaseRemoveLockAndWait(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN PVOID Tag
    );

#endif

#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(ClusdiskLH,(b25a9257,9a39,43df,9f35,b0976e28e843), \
      WPP_DEFINE_BIT(DEFAULT) \
      WPP_DEFINE_BIT(CREATE)  \
      WPP_DEFINE_BIT(CLOSE)   \
      WPP_DEFINE_BIT(CLEANUP) \
      WPP_DEFINE_BIT(UNPEND)  \
      WPP_DEFINE_BIT(LEGACY)  \
   )                          \
   WPP_DEFINE_CONTROL_GUID(ClusdiskHB,(7f827e76,1a10,11d3,ba86,00c04f8eed00), \
      WPP_DEFINE_BIT(RESERVE) \
      WPP_DEFINE_BIT(READ)    \
      WPP_DEFINE_BIT(WRITE)   \
      WPP_DEFINE_BIT(TICK)    \
   )

BugChecks and what to do about them:

Bugchecks with no descriptions are either checked builds
only or very rare.  If you get one of these and a kernel
debugger is available do the following
    kb
    !process 0 7
    !vm
    !errlog

Note:
Please use following format for modifications in this file, it helps
the debugger to extract the description text from this file:
<BUGCODE>    <value>
<text>
PARAMETERS
  <ParamId1> - <text>
   VALUES:    - If parameter values are explained.
        <paramId1-value> : <text>
             <ParamId2> - <text>
             <ParamId3> - <text>
             <paramid4> - <text>
        <paramId1-value> : <text>
             <ParamId2> - <text>
                 VALUES:    - If parameter values are explained.
                     <paramId2-value> : <text>
                 END_VALUES
             <ParamId3> - <text>

             <paramid4> - <text>
DESCRIPTION - if more description text for bugcheck follows
<text>
% Any text present just as a comment, not part of the description

APC_INDEX_MISMATCH               (0x1)
This is a kernel internal error which can occur only on a checked build.
The most common reason to see such a bugcheck would occur when a
filesystem had a mismatched number of KeEnterCriticalRegion calls compared
to KeLeaveCriticalRegion calls.  This check is made on exit from a system
call.
PARAMETERS
    1 - address of system function (system call)
    2 - Thread->ApcStateIndex << 8 | Previous ApcStateIndex
    3 - Thread->KernelApcDicable
    4 - Previous KernelApcDisable

DEVICE_QUEUE_NOT_BUSY            (0x2)

INVALID_AFFINITY_SET             (0x3)

INVALID_DATA_ACCESS_TRAP         (0x4)

INVALID_PROCESS_ATTACH_ATTEMPT   (0x5)

INVALID_PROCESS_DETACH_ATTEMPT   (0x6)

INVALID_SOFTWARE_INTERRUPT       (0x7)

IRQL_NOT_DISPATCH_LEVEL          (0x8)

IRQL_NOT_GREATER_OR_EQUAL        (0x9)

IRQL_NOT_LESS_OR_EQUAL           (0xA)
PARAMETERS
        1 - memory referenced
        2 - IRQL
        3 - value 0 = read operation, 1 = write operation
        4 - address which referenced memory

DESCRIPTION
An attempt was made to access a pagable (or completely invalid) address at an
interrupt request level (IRQL) that is too high.  This is usually
caused by drivers using improper addresses.

If a kernel debugger is available get the stack backtrace.


NO_EXCEPTION_HANDLING_SUPPORT    (0xB)

MAXIMUM_WAIT_OBJECTS_EXCEEDED    (0xC)

MUTEX_LEVEL_NUMBER_VIOLATION     (0xD)

NO_USER_MODE_CONTEXT             (0xE)

SPIN_LOCK_ALREADY_OWNED          (0xF)

SPIN_LOCK_NOT_OWNED              (0x10)

THREAD_NOT_MUTEX_OWNER           (0x11)

TRAP_CAUSE_UNKNOWN               (0x12)

PARAMETERS
    1 - Unexpected interrupt.
    2 - Unknown floating point exception.
    3 - The enabled and asserted status bits (see processor definition).

EMPTY_THREAD_REAPER_LIST         (0x13)

CREATE_DELETE_LOCK_NOT_LOCKED    (0x14)

LAST_CHANCE_CALLED_FROM_KMODE    (0x15)

CID_HANDLE_CREATION              (0x16)

CID_HANDLE_DELETION              (0x17)

REFERENCE_BY_POINTER             (0x18)

BAD_POOL_HEADER                  (0x19)

The pool is already corrupt at the time of the current request.
This may or may not be due to the caller.
The internal pool links must be walked to figure out a possible cause of
the problem, and then special pool applied to the suspect tags or the driver
verifier to a suspect driver.
PARAMETERS
   1 -
    VALUES:
    3 : the pool freelist is corrupt.
       Parameter 2 - the pool entry being checked.
       Parameter 3 - the read back flink freelist value (should be the same as 2).
       Parameter 4 - the read back blink freelist value (should be the same as 2).

    5 : the adjacent pool block headers are corrupt.
       Parameter 2 - One entry whose headers are not consistent.
       Parameter 3 - (reserved)
%       Parameter 3 - the line number inside pool.c (generally not useful).
       Parameter 4 - Another entry whose headers are not consistent.

    6 : the pool block header previous size is corrupt (too large).
       Parameter 2 - One incorrectly calculated entry.
       Parameter 3 - (reserved)
%       Parameter 3 - the line number inside pool.c (generally not useful).
       Parameter 4 - The bad entry that caused the miscalculation.

    7 : the pool block header size is corrupt.
       Parameter 2 - 0.
       Parameter 3 - (reserved)
%       Parameter 3 - the line number inside pool.c (generally not useful).
       Parameter 4 - The bad pool entry.

    8 : the pool block header size is corrupt.
       Parameter 2 - 0.
       Parameter 3 - (reserved)
%       Parameter 3 - the line number inside pool.c (generally not useful).
       Parameter 4 - The bad pool entry (should have nonzero size but doesn't).

    9 : the pool block header size is corrupt (too large).
       Parameter 2 - One incorrectly calculated entry.
       Parameter 3 - (reserved)
%       Parameter 3 - the line number inside pool.c (generally not useful).
       Parameter 4 - The bad entry that caused the miscalculation.

    0xA : a pool block header size is corrupt.
       Parameter 2 - The pool entry we were looking for within the page.
       Parameter 3 - (reserved)
%       Parameter 3 - the line number inside pool.c (generally not useful).
       Parameter 4 - The VA of the page that should have contained the pool entry.

    END_VALUES

MEMORY_MANAGEMENT                (0x1A)

PARAMETERS
    1 - The subtype of the bugcheck:
        VALUES:
            0x1 : The fork clone block reference count is corrupt.  Only occurs
                  on checked builds.
          0x777 : The caller is unlocking a system cache address that is not
                  currently locked.  (This address was either never mapped or
                  is being unlocked twice.)
          0x778 : The system is using the very last system cache view address,
                  instead of preserving it.
      0x780-781 : The PTEs mapping the argument system cache view have been
                  corrupted.
         0x1000 : A caller of MmGetSystemAddressForMdl* tried to map a fully-
                  cached physical page as non-cached.  This action would cause
                  a conflicting hardware translation buffer entry, and so it
                  was refused by the operating system.  Since the caller
                  specified "bugcheck on failure" in the requesting MDL, the
                  system had no choice but to bugcheck in this instance.
         0x1010 : The caller is unlocking a pageable section that is not
                  currently locked.  (This section was either never locked or
                  is being unlocked twice.)
         0x1234 : The caller is trying lock a nonexistent pageable section.
         0x1235 : The caller is trying to protect an MDL with an invalid
                  mapping.
         0x3451 : The PTEs of an outswapped kernel thread stack are corrupt.
    0x8888-8889 : Internal memory management structures are corrupt.
        0x41283 : The working set index encoded in the PTE is corrupt.
        0x41284 : A PTE or the working set list is corrupt.
        0x41286 : The caller is trying to free an invalid pool address.
        0x41785 : The working set list is corrupt.
        0x61940 : A PDE has been unexpectedly invalidated.
          Other : An unknown memory management error occurred.
        END_VALUES

DESCRIPTION
    # Any other values for parameter 1 must be individually examined.

PFN_SHARE_COUNT                  (0x1B)

PFN_REFERENCE_COUNT              (0x1C)

NO_SPIN_LOCK_AVAILABLE           (0x1D)

KMODE_EXCEPTION_NOT_HANDLED      (0x1E)
PARAMETERS
    1 - The exception code that was not handled
        VALUES
        0x80000002: (STATUS_DATATYPE_MISALIGNMENT) An unaligned data reference was encountered.
                    The trap frame will supply additional information.
        0x80000003: This means a hard coded breakpoint or assertion was hit, but this system was booted
                    /NODEBUG.  This is not supposed to happen as developers should never have
                    hardcoded breakpoints in retail code, but ...
                    If this happens, make sure a debugger gets connected, and the
                    system is booted /DEBUG.  This will let us see why this breakpoint is
                    happening.
        END_VALUES
    2 - The address that the exception occurred at
    3 - Parameter 0 of the exception
    4 - Parameter 1 of the exception

DESCRIPTION
This is a very common bugcheck.  Usually the exception address pinpoints
the driver/function that caused the problem.  Always note this address
as well as the link date of the driver/image that contains this address.

SHARED_RESOURCE_CONV_ERROR       (0x1F)

KERNEL_APC_PENDING_DURING_EXIT   (0x20)
PARAMETERS
    1 - The address of the APC found pending during exit.
    2 - The thread's APC disable count
    3 - The current IRQL

DESCRIPTION
The key data item is the thread's APC disable count.
If this is non-zero, then this is the source of the problem.

The APC disable count is decremented each time a driver calls
KeEnterCriticalRegion, KeInitializeMutex, or FsRtlEnterFileSystem.  The APC
disable count is incremented each time a driver calls KeLeaveCriticalRegion,
KeReleaseMutex, or FsRtlExitFileSystem.  Since these calls should always be in
pairs, this value should be zero when a thread exits.  A negative value
indicates that a driver has disabled APC calls without re-enabling them.  A
positive value indicates that the reverse is true.

If you ever see this error, be very suspicious of all drivers installed on the
machine -- especially unusual or non-standard drivers.  Third party file
system redirectors are especially suspicious since they do not generally
receive the heavy duty testing that NTFS, FAT, RDR, etc receive.

This current IRQL should also be 0.  If it is not, that a driver's
cancelation routine can cause this bugcheck by returning at an elevated
IRQL.  Always attempt to note what you were doing/closing at the
time of the crash, and note all of the installed drivers at the time of
the crash.  This symptom is usually a severe bug in a third party
driver.

QUOTA_UNDERFLOW                  (0x21)

FILE_SYSTEM                      (0x22)

FAT_FILE_SYSTEM                  (0x23)
%   All file system bug checks have encoded in their first ULONG
%   the source file and the line within the source file that generated
%   the bugcheck.  The high 16-bits contains a number to identify the
%   file and low 16-bits is the source line within the file where
%   the bug check call occurs.  For example, 0x00020009 indicates
%   that the FAT file system bugcheck occurred in source file #2 and
%   line #9.
%
%   The file system calls bug check in multiple places and this will
%   help us identify the actual source line that generated the bug
%   check.

    If you see FatExceptionFilter on the stack then the 2nd and 3rd
    parameters are the exception record and context record. Do a .cxr
    on the 3rd parameter and then kb to obtain a more informative stack
    trace.

NTFS_FILE_SYSTEM                 (0x24)
%   All file system bug checks have encoded in their first ULONG
%   the source file and the line within the source file that generated
%   the bugcheck.  The high 16-bits contains a number to identify the
%   file and low 16-bits is the source line within the file where
%   the bug check call occurs.  For example, 0x00020009 indicates
%   that the NTFS file system bugcheck occurred in source file #2 and
%   line #9.
%
%   The file system calls bug check in multiple places and this will
%   help us identify the actual source line that generated the bug
%   check.

    If you see NtfsExceptionFilter on the stack then the 2nd and 3rd
    parameters are the exception record and context record. Do a .cxr
    on the 3rd parameter and then kb to obtain a more informative stack
    trace.

NPFS_FILE_SYSTEM                 (0x25)
%   All file system bug checks have encoded in their first ULONG
%   the source file and the line within the source file that generated
%   the bugcheck.  The high 16-bits contains a number to identify the
%   file and low 16-bits is the source line within the file where
%   the bug check call occurs.  For example, 0x00020009 indicates
%   that the NPFS file system bugcheck occurred in source file #2 and
%   line #9.
%
%   The file system calls bug check in multiple places and this will
%   help us identify the actual source line that generated the bug
%   check.

CDFS_FILE_SYSTEM                 (0x26)
%   All file system bug checks have encoded in their first ULONG
%   the source file and the line within the source file that generated
%   the bugcheck.  The high 16-bits contains a number to identify the
%   file and low 16-bits is the source line within the file where
%   the bug check call occurs.  For example, 0x00020009 indicates
%   that the CD file system bugcheck occurred in source file #2 and
%   line #9.
%
%   The file system calls bug check in multiple places and this will
%   help us identify the actual source line that generated the bug
%   check.

    If you see CdExceptionFilter on the stack then the 2nd and 3rd
    parameters are the exception record and context record. Do a .cxr
    on the 3rd parameter and then kb to obtain a more informative stack
    trace.

RDR_FILE_SYSTEM                  (0x27)
    If you see RxExceptionFilter on the stack then the 2nd and 3rd parameters are the
    exception record and context record. Do a .cxr on the 3rd parameter and then kb to
    obtain a more informative stack trace.

    The high 16 bits of the first parameter is the RDBSS bugcheck code, which is defined
    as follows:
     RDBSS_BUG_CHECK_CACHESUP  = 0xca550000,
     RDBSS_BUG_CHECK_CLEANUP   = 0xc1ee0000,
     RDBSS_BUG_CHECK_CLOSE     = 0xc10e0000,
     RDBSS_BUG_CHECK_NTEXCEPT  = 0xbaad0000,

%   Further the mapping between each of the above RDBSS bugcheck codes and the
%   corresponding files (all in ...\base\fs\rdr2\rdbss directory) is as follows -
%    RDBSS_BUG_CHECK_CACHESUP  ===> cachesup.c
%    RDBSS_BUG_CHECK_CLEANUP  ====> cleanup.c
%    RDBSS_BUG_CHECK_CLOSE =======> close.c
%    RDBSS_BUG_CHECK_NTEXCEPT ====> ntexcept.c

CORRUPT_ACCESS_TOKEN             (0x28)

SECURITY_SYSTEM                  (0x29)

INCONSISTENT_IRP                 (0x2A)
PARAMETERS
    1 - Address of the IRP that was found to be inconsistent

DESCRIPTION
An IRP was encountered that was in an inconsistent state; i.e., some field
or fields of the IRP were inconsistent w/the remaining state of the IRP.
An example would be an IRP that was being completed, but was still marked
as being queued to a driver's device queue.  This bugcheck code is not
currently being used in the system, but exists for debugging purposes.


PANIC_STACK_SWITCH               (0x2B)
PARAMETERS
    1 - Trap Frame
DESCRIPTION
This error indicates that the kernel mode stack was overrun. This normally
occurs when a kernel-mode driver uses too much stack space.  It can also
occur when serious data corruption occurs in the kernel.


PORT_DRIVER_INTERNAL             (0x2C)


SCSI_DISK_DRIVER_INTERNAL        (0x2D)


DATA_BUS_ERROR                   (0x2E)
This bugcheck is normally caused by a parity error in the system memory.
PARAMETERS
        1 - Virtual address that caused the fault
        2 - Physical address that caused the fault.
        3 - Processor status register (PSR)
        4 - Faulting instruction register (FIR)
DESCRIPTION
This error can also be caused by a driver accessing a bad virtual
address whose backing physical address does not exist.


INSTRUCTION_BUS_ERROR            (0x2F)


SET_OF_INVALID_CONTEXT           (0x30)

Attempt to set the stack pointer in the trap frame to a lower value than
the current stack pointer value.   This would cause the kernel run with a
stack pointer pointing to stack which is no longer valid.
PARAMETERS
    1 - New stack pointer
    2 - Old stack pointer
    3 - TrapFrame address
    4 - 0

PHASE0_INITIALIZATION_FAILED     (0x31)

    System init failed early on.  A debugger is required to analyze this.

PHASE1_INITIALIZATION_FAILED     (0x32)
PARAMETERS
    1 - NT status code that describes why the system initialization failed.
    2 - (reserved)
%   2 - Indicates location within init.c where phase 1 initialization failure occurred

UNEXPECTED_INITIALIZATION_CALL   (0x33)

CACHE_MANAGER                    (0x34)
    See the comment for FAT_FILE_SYSTEM (0x23)

NO_MORE_IRP_STACK_LOCATIONS      (0x35)
PARAMETERS
    1 - Address of the IRP

DESCRIPTION
A higher level driver has attempted to call a lower level driver through
the IoCallDriver() interface, but there are no more stack locations in the
packet, hence, the lower level driver would not be able to access its
parameters, as there are no parameters for it.  This is a disasterous
situation, since the higher level driver "thinks" it has filled in the
parameters for the lower level driver (something it MUST do before it calls
it), but since there is no stack location for the latter driver, the former
has written off of the end of the packet.  This means that some other memory
has probably been trashed at this point.

DEVICE_REFERENCE_COUNT_NOT_ZERO  (0x36)
PARAMETERS
    1 - Address of the device object

DESCRIPTION
A device driver has attempted to delete one of its device objects from the
system but the reference count for that object was non-zero, meaning that
there are still outstanding references to the device.  (The reference count
indicates the number of reasons why this device object cannot be deleted.)
This is a bug in the calling device driver.

FLOPPY_INTERNAL_ERROR            (0x37)

SERIAL_DRIVER_INTERNAL           (0x38)

SYSTEM_EXIT_OWNED_MUTEX          (0x39)

SYSTEM_UNWIND_PREVIOUS_USER      (0x3A)

SYSTEM_SERVICE_EXCEPTION         (0x3B)

INTERRUPT_UNWIND_ATTEMPTED       (0x3C)

INTERRUPT_EXCEPTION_NOT_HANDLED  (0x3D)

MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED (0x3E)
The system has multiple processors, but they are asymmetric in relation
to one another.  In order to be symmetric all processors must be of
the same type and level.  For example, trying to mix a Pentium level
processor with an 80486 would cause this bugcheck.


NO_MORE_SYSTEM_PTES              (0x3F)
PARAMETERS
        1 - PTE Type (0 - system expansion, 1 nonpaged pool expansion)
        2 - Requested size
        3 - Total free system PTEs
        4 - Total system PTEs

DESCRIPTION
No System PTEs left.  Usually caused by a driver not cleaning up
properly.  If kernel debugger available get stack trace and
"!sysptes 3".

Set HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management\TrackPtes
to a DWORD 1 value and reboot.  Then the system will save stack traces
so the guilty driver can be identified.  There is no other way to find out
which driver is neglecting to clean up the I/Os.

A bugcheck DRIVER_USED_EXCESSIVE_PTES will then occur if the system runs out of
PTEs again and the offending driver's name will be printed.

TARGET_MDL_TOO_SMALL             (0x40)

A driver has called the IoBuildPartialMdl() function and passed it an MDL
to map part of a source MDL, but the target MDL is not large enough to map
the entire range of addresses requested.  This is a driver bug.  The source
and target MDLs, as well as the address range length to be mapped are the
arguments to the IoBuildPartialMdl() function, i.e.;

    IoBuildPartialMdl(
        IN PMDL SourceMdl,
        IN OUT PMDL TargetMdl,
        IN PVOID VirtualAddress,
        IN ULONG Length
        )

MUST_SUCCEED_POOL_EMPTY          (0x41)

PARAMETERS
        1 - size of the request that could not be satisfied
        2 - number of pages used of nonpaged pool
        3 - number of > PAGE_SIZE requests from nonpaged pool
        4 - number of pages available

DESCRIPTION
No component should ever ask for must-succeed pool as if there is none left,
the system crashes.  Instead, components should ask for normal pool and
gracefully handle the scenario where the pool is temporarily empty.  This
bugcheck definitely reveals a bug in the caller (use kb to identify the caller).
In addition, the fact that the pool is empty may be either a transient condition
or possibly a leak in another component (distinguish between the 2 cases by
following the directions below).

Type kb to show the calling stack.
Type !vm 1 to display total pool usage.
Then type !poolused 2 to display per-tag nonpaged pool usage.
Then type !poolused 4 to display per-tag paged pool usage.
The crash should be looked at by the tag owner that is consuming the most pool.

ATDISK_DRIVER_INTERNAL           (0x42)

NO_SUCH_PARTITION                (0x43)

MULTIPLE_IRP_COMPLETE_REQUESTS   (0x44)
PARAMETERS
    1 - Address of the IRP

DESCRIPTION
A driver has requested that an IRP be completed (IoCompleteRequest()), but
the packet has already been completed.  This is a tough bug to find because
the easiest case, a driver actually attempted to complete its own packet
twice, is generally not what happened.  Rather, two separate drivers each
believe that they own the packet, and each attempts to complete it.  The
first actually works, and the second fails.  Tracking down which drivers
in the system actually did this is difficult, generally because the trails
of the first driver have been covered by the second.  However, the driver
stack for the current request can be found by examining the DeviceObject
fields in each of the stack locations.

INSUFFICIENT_SYSTEM_MAP_REGS     (0x45)

DEREF_UNKNOWN_LOGON_SESSION      (0x46)

REF_UNKNOWN_LOGON_SESSION        (0x47)

CANCEL_STATE_IN_COMPLETED_IRP    (0x48)
PARAMETERS
    1 - Pointer to the IRP
    2 - Cancel routine set by the driver.

DESCRIPTION
This bugcheck indicates that an I/O Request Packet (IRP) that is to be
cancelled, has a cancel routine specified in it -- meaning that the packet
is in a state in which the packet can be cancelled -- however, the packet
no longer belongs to a driver, as it has entered I/O completion.  This is
either a driver bug, or more than one driver is accessing the same packet,
which is not likely and much more difficult to find. The cancel routine
parameter will provide a clue as to which driver or stack is the culprit.

PAGE_FAULT_WITH_INTERRUPTS_OFF   (0x49)

IRQL_GT_ZERO_AT_SYSTEM_SERVICE   (0x4A)

Returning to usermode from a system call at an IRQL > PASSIVE_LEVEL.
PARAMETERS
    1 - Address of system function (system call routine)
    2 - Current IRQL
    3 - 0
    4 - 0

STREAMS_INTERNAL_ERROR           (0x4B)

FATAL_UNHANDLED_HARD_ERROR       (0x4C)

If a hard error occurs during system booting before windows is up, and
the hard error is a real error, the system will blue screen crash.

%   Note:  This will not actually appear as bug check 0x0000004C on
%   the blue screen or in the debugger.  Bugcheck.c shifts the parameters
%   left, and replaces the bug check code with the exception code.
%   So this will appear as bug check "code" 0x00000218, 0x0000021A, and
%   so forth.

Some common cases are:

    x218 - This means a necessary registry hive file could not be
           loaded.  The obvious reason is if it is corrupt or missing.
           In this case, either the Emergency Repair Disk or a
           reinstall is required.

           Some less obvious reasons are that the driver has corrupted
           the registry data while loading into memory, or the memory
           where the registry file was loaded is not actually memory.

    x21a - This means that either winlogon, or csrss (windows) died
           unexpectedly.  The exit code tells more information.  Usually
           it is c0000005 meaning that an unhandled exception crashed
           either of these processes.

    x221 - This means that a driver is corrupt, or a system DLL was
           detected to be corrupt.
%
%          We do our best to integrity check
%          drivers and important system DLLs, and if they are corrupt,
%          the blue screen displays the name of the corrupt file.
%          This prevents crashes from occuring when we stumble into the
%          corruption later.

           Safeboot or boot an alternate OS (or reinstall)
           and then make sure the on disk file that is listed as bad
           matches the version on CD and replace if necessary.  In some
           cases, random corruption can mean that there is a hardware
           problem in the I/O path to the file.

NO_PAGES_AVAILABLE               (0x4D)
PARAMETERS
        1 - Total number of dirty pages
        2 - Number of dirty pages destined for the pagefile(s).
        3 - Internal flags.
        4 - Most recent modified write error status.

DESCRIPTION
No free pages available to continue operations.
If kernel debugger available "!vm 3".

        This bugcheck can occur for the following reasons:

        1.  A driver has blocked, deadlocking the modified or mapped
            page writers.  Examples of this include mutex deadlocks or
            accesses to paged out memory in filesystem drivers, filter
            drivers, etc.  This indicates a driver bug.

            If parameter 1 or 2 is large, then this is a possibility.  Type
            "!vm 3" in the kernel debugger.

        2.  The storage driver(s) are not processing requests.  Examples
            of this are stranded queues, non-responding drives, etc.  This
            indicates a driver bug.

            If parameter 1 or 2 is large, then this is a possibility.  Type
            "!process 0 7" in the kernel debugger.

        3.  Not enough pool is available for the storage stack to write out
            modified pages.  This indicates a driver bug.

            If parameter 3 is small, then this is a possibility.  Type
            "!vm" and "!poolused 2" in the kernel debugger.

        4.  A high priority realtime thread has starved the balance set
            manager from trimming pages and/or starved the modified writer
            from writing them out.  This indicates a bug in the component
            that created this thread.

            This one is hard to determine, try "!ready"

        5.  All the processes have been trimmed to their minimums and all
            modified pages written, but still no memory is available.  The
            freed memory must be stuck in transition pages with non-zero
            reference counts - thus they cannot be put on the freelist.
            A driver is neglecting to unlock the pages preventing the
            reference counts from going to zero which would free the pages.
            This may be due to transfers that never finish and the driver
            never aborts or other driver bugs.

            If parameter 4 is large, then this is a possibility.  But it
            is very hard to find the driver.  Try "!process 0 1" and look
            for any that have a lot of locked pages.

If the problem cannot be found, then try booting with /DEBUG and a kernel
debugger attached, so if it reproduces, a debug session can be initiated
to identify the cause.

PFN_LIST_CORRUPT                 (0x4E)
PARAMETERS
    1 -
    VALUES:
      1 : A list head was corrupt
        2 - ListHead value which was corrupt
        3 - number of pages available
        4 - 0
      2 : A list entry was corrupt
        2 - entry in list being removed
        3 - highest physical page number
        4 - reference count of entry being removed
      7 : A driver has unlocked a page more times than it locked it
        2 - page frame number
        3 - current share count
        4 - 0
      0x8F : The free or zeroed page listhead is corrupt
        2 - new page
        3 - old page
        4 - 0
      0x99 : A PTE or PFN is corrupt
        2 - page frame number
        3 - current page state
        4 - 0
    END_VALUES


DESCRIPTION
Typically caused by drivers passing bad memory descriptor lists (ie: calling
MmUnlockPages twice with the same list, etc).  If a kernel debugger is
available get the stack trace.


NDIS_INTERNAL_ERROR              (0x4F)

PAGE_FAULT_IN_NONPAGED_AREA      (0x50)
PARAMETERS
        1 - memory referenced.
        2 - value 0 = read operation, 1 = write operation.
        3 - If non-zero, the instruction address which referenced the bad memory
            address.
        4 - (reserved)
%       4 - Mm internal code.

DESCRIPTION
Invalid system memory was referenced.  This cannot be protected by try-except,
it must be protected by a Probe.  Typically the address is just plain bad or it
is pointing at freed memory.

REGISTRY_ERROR                   (0x51)
PARAMETERS
        1 - (reserved)
%       1 - value 1 (indicates where Windows bugchecked)
        2 - (reserved)
%       2 - value 2 (indicates where Windows bugchecked)
        3 - depends on where Windows bugchecked, may be pointer to hive
        4 - depends on where Windows bugchecked, may be return code of
            HvCheckHive if the hive is corrupt.

DESCRIPTION
Something has gone badly wrong with the registry.  If a kernel debugger
is available, get a stack trace. It can also indicate that the registry got
an I/O error while trying to read one of its files, so it can be caused by
hardware problems or filesystem corruption.

It may occur due to a failure in a refresh operation, which is used only
in by the security system, and then only when resource limits are encountered.

MAILSLOT_FILE_SYSTEM             (0x52)

NO_BOOT_DEVICE                   (0x53)

LM_SERVER_INTERNAL_ERROR         (0x54)

DATA_COHERENCY_EXCEPTION         (0x55)

INSTRUCTION_COHERENCY_EXCEPTION  (0x56)

XNS_INTERNAL_ERROR               (0x57)

FTDISK_INTERNAL_ERROR            (0x58)

The system was booted from a revived primary partition so
the hives say the mirror is ok, when in fact it is not.
The "real" image of the hives are on the shadow.
The user must boot from the shadow.

PINBALL_FILE_SYSTEM              (0x59)
    See the comment for FAT_FILE_SYSTEM (0x23)

CRITICAL_SERVICE_FAILED          (0x5A)

SET_ENV_VAR_FAILED               (0x5B)

HAL_INITIALIZATION_FAILED        (0x5C)

UNSUPPORTED_PROCESSOR            (0x5D)
    386 - System failed because the processor is only a 386 or
    compatible.  The system requires a Pentium (or higher) compatible processor.

OBJECT_INITIALIZATION_FAILED     (0x5E)

SECURITY_INITIALIZATION_FAILED   (0x5F)

PROCESS_INITIALIZATION_FAILED    (0x60)

HAL1_INITIALIZATION_FAILED       (0x61)

OBJECT1_INITIALIZATION_FAILED    (0x62)

SECURITY1_INITIALIZATION_FAILED  (0x63)

SYMBOLIC_INITIALIZATION_FAILED   (0x64)

MEMORY1_INITIALIZATION_FAILED    (0x65)

CACHE_INITIALIZATION_FAILED      (0x66)

CONFIG_INITIALIZATION_FAILED     (0x67)

PARAMETERS
    1 - (reserved)
%   1 - indicates location in ntos\config\cmsysini that failed
    2 - location selector
    3 - NT status code

DESCRIPTION
This means the registry couldn't allocate the pool needed to contain the
registry files.  This should never happen, since it is early enough in
system initialization that there is always plenty of paged pool available.

FILE_INITIALIZATION_FAILED       (0x68)

IO1_INITIALIZATION_FAILED        (0x69)

Initialization of the I/O system failed for some reason.  There is
very little information available.  In general, setup really made
some bad decisions about the installation of the system, or the user has
reconfigured the system.

LPC_INITIALIZATION_FAILED        (0x6A)

PROCESS1_INITIALIZATION_FAILED   (0x6B)
PARAMETERS
    1 - Indicates the NT status code that caused the failure.
    2 - (reserved)
%   2 - Indicates the location in psinit.c where the failure
%       was detected.

REFMON_INITIALIZATION_FAILED     (0x6C)

SESSION1_INITIALIZATION_FAILED   (0x6D)
    1 - Indicates the NT status code that caused the failure.

DESCRIPTION
The bugcheck code (SESSION1 - SESSION5) indicates the point during
initialization when the failure was detected.
%
% The session number indicates the location in ntos\init\init.c.

SESSION2_INITIALIZATION_FAILED   (0x6E)
PARAMETERS
    1 - Indicates the NT status code that tripped Windows into thinking
        that initialization failed.

DESCRIPTION
The bugcheck code (SESSION1 - SESSION5) indicates the point during
initialization when the failure was detected.
%
% The session number indicates the location in ntos\init\init.c.

SESSION3_INITIALIZATION_FAILED   (0x6F)
PARAMETERS
    1 - Indicates the NT status code that tripped Windows into thinking
        that initialization failed.

DESCRIPTION
The bugcheck code (SESSION1 - SESSION5) indicates the point during
initialization when the failure was detected.
%
% The session number indicates the location in ntos\init\init.c.

SESSION4_INITIALIZATION_FAILED   (0x70)
PARAMETERS
    1 - Indicates the NT status code that tripped Windows into thinking
        that initialization failed.

DESCRIPTION
The bugcheck code (SESSION1 - SESSION5) indicates the point during
initialization when the failure was detected.
%
% The session number indicates the location in ntos\init\init.c.

SESSION5_INITIALIZATION_FAILED   (0x71)
PARAMETERS
    1 - Indicates the NT status code that tripped Windows into thinking
        that initialization failed.

DESCRIPTION
The bugcheck code (SESSION1 - SESSION5) indicates the point during
initialization when the failure was detected.
%
% The session number indicates the location in ntos\init\init.c.

ASSIGN_DRIVE_LETTERS_FAILED      (0x72)

CONFIG_LIST_FAILED               (0x73)
Indicates that one of the core system hives cannot be linked in the
registry tree. The hive is valid, it was loaded OK. Examine the 2nd
bugcheck argument to see why the hive could not be linked in the
registry tree.

PARAMETERS
        1 - 1
        2 - Indicates the NT status code that tripped Windows into
            thinking that it had failed to load the hive.
        3 - Index of hive in hivelist
        4 - Pointer to UNICODE_STRING containing filename of hive

DESCRIPTION
This can be either SAM, SECURITY, SOFTWARE or DEFAULT. One common reason
for this to happen is if you are out of disk space on the system drive
(in which case param 2 is 0xC000017D - STATUS_NO_LOG_SPACE) or an attempt
to allocate pool has failed (in which case param 2 is 0xC000009A -
STATUS_INSUFFICIENT_RESOURCES). Other status codes must be individually
investigated.


BAD_SYSTEM_CONFIG_INFO           (0x74)
Can indicate that the SYSTEM hive loaded by the osloader/NTLDR
was corrupt.  This is unlikely, since the osloader will check
a hive to make sure it isn't corrupt after loading it.

It can also indicate that some critical registry keys and values
are not present.  (i.e. somebody used regedt32 to delete something
that they shouldn't have)  Booting from LastKnownGood may fix
the problem, but if someone is persistent enough in mucking with
the registry they will need to reinstall or use the Emergency
Repair Disk.

PARAMETERS
        1 - (reserved)
%       1 - identifies the function
        2 - (reserved)
%       2 - identifies the line inside the function
        3 - (reserved)
%       3 - other info
        4 - usually the NT status code.

CANNOT_WRITE_CONFIGURATION       (0x75)

This will result if the SYSTEM hive file cannot be converted to a
mapped file. This usually happens if the system is out of pool and
we cannot reopen the hive.

PARAMETERS
        1 - 1
        2 - Indicates the NT status code that tripped Windows into
            thinking that it had failed to convert the hive.

DESCRIPTION
Normally you shouldn't see this as the conversion happens at early
during system initialization, so enough pool should be available.

PROCESS_HAS_LOCKED_PAGES         (0x76)
PARAMETERS
        1 - 0
        2 - process address
        3 - number of locked pages
        4 - pointer to driver stacks (if enabled) or 0 if not.

DESCRIPTION
Caused by a driver not cleaning up completely after an I/O.  Set
HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management\TrackLockedPages
to a DWORD 1 value and reboot.  Then the system will save stack traces
so the guilty driver can be identified.  There is no other way to find out
which driver is neglecting to clean up the I/Os.  When you enable this flag,
if the driver commits the error again you will see a different
bugcheck - DRIVER_LEFT_LOCKED_PAGES_IN_PROCESS (0xCB) - which can identify the
offending driver(s).

KERNEL_STACK_INPAGE_ERROR        (0x77)
PARAMETERS
        1 - status code
        2 - i/o status code
        3 - page file number
        4 - offset into page file

   1 - status code
   VALUES:
     0 : (page was retrieved from page cache)
        2 - value found in stack where signature should be
        3 - 0
        4 - address of signature on kernel stack

     1 : (page was retrieved from disk)
        2 - value found in stack where signature should be
        3 - 0
        4 - address of signature on kernel stack

     2 : (page was retrieved from disk, storage stack returned SUCCESS,
            but the Status.Information != PAGE_SIZE)
        2 - value found in stack where signature should be
        3 - 0
        4 - address of signature on kernel stack
     END_VALUES


DESCRIPTION
The requested page of kernel data could not be read in.  Caused by
bad block in paging file or disk controller error.

In the case when the first and second arguments are 0, the stack signature
in the kernel stack was not found.  Again, bad hardware.

An I/O status of c000009c (STATUS_DEVICE_DATA_ERROR) or
C000016AL (STATUS_DISK_OPERATION_FAILED)  normally indicates
the data could not be read from the disk due to a bad
block.  Upon reboot autocheck willl run and attempt to map out the bad
sector.  If the status is C0000185 (STATUS_IO_DEVICE_ERROR) and the paging
file is on a SCSI disk device, then the cabling and termination should be
checked.  See the knowledge base article on SCSI termination.

PHASE0_EXCEPTION                 (0x78)

MISMATCHED_HAL                   (0x79)
PARAMETERS
    1 - type of mismatch
    VALUES:
        1:
            The PRCB release levels mismatch.  (something is out of date)
            2 - Major PRCB level of ntoskrnl.exe
            3 - Major PRCB level of hal.dll

        2:
            The build types mismatch.
            2 - Build type of ntoskrnl.exe
            3 - Build type of hal.dll

                Build type
                    0 = Free multiprocessor enabled build
                    1 = Checked multiprocessor enabled build
                    2 = Free uniprocessor build

        3:
            The loader (ntldr) and hal versions mismatch.
            2 - Size of the loader parameter extension
            3 - MajorVersion of the loader parameter extension
            4 - MinorVersion of the loader parameter extension

     END_VALUES


DESCRIPTION
The HAL revision level and HAL configuration type does not match that
of the kernel or the machine type.  This would probably happen if the
user has manually updated either ntoskrnl.exe or hal.dll and managed to
get a conflict.

You have an MP (multi-processor) Hal and a UP (uni-processor) Kernel,
or the reverse.



KERNEL_DATA_INPAGE_ERROR         (0x7A)
PARAMETERS
        1 - lock type that was held (value 1,2,3, or PTE address)
        2 - error status (normally i/o status code)
        3 - current process (virtual address for lock type 3, or PTE)
        4 - virtual address that could not be in-paged

DESCRIPTION
The requested page of kernel data could not be read in.  Typically caused by
a bad block in the paging file or disk controller error. Also see
KERNEL_STACK_INPAGE_ERROR.

If the error status is 0xC000000E, 0xC000009C, 0xC000009D or 0xC0000185,
it means the disk subsystem has experienced a failure.
%
%  ntmsd is the best alias to take care of these types of failures.

If the error status is 0xC000009A, then it means the request failed because
a filesystem failed to make forward progress.
%
%  The filesystem alias is the right alias to look at these types of failures.




INACCESSIBLE_BOOT_DEVICE         (0x7B)
PARAMETERS
    1 - Pointer to the device object or Unicode string of ARC name

DESCRIPTION
During the initialization of the I/O system, it is possible that the driver
for the boot device failed to initialize the device that the system is
attempting to boot from, or it is possible for the file system that is
supposed to read that device to either fail its initialization or to simply
not recognize the data on the boot device as a file system structure that
it recognizes.  In the former case, the argument (#1) is the address of a
Unicode string data structure that is the ARC name of the device from which
the boot was being attempted.  In the latter case, the argument (#1) is the
address of the device object that could not be mounted.

If this is the initial setup of the system, then this error can occur if
the system was installed on an unsupported disk or SCSI controller.  Note
that some controllers are supported only by drivers which are in the Windows
Driver Library (WDL) which requires the user to do a custom install.  See
the Windows Driver Library for more information.

This error can also be caused by the installation of a new SCSI adapter or
disk controller or repartitioning the disk with the system partition.  If
this is the case, on x86 systems the boot.ini file must be edited or on ARC
systems setup must be run.  See the "Advanced Server System Administrator's
User Guide" for information on changing boot.ini.

If the argument is a pointer to an ARC name string, then the format of the
first two (and in this case only) longwords will be:

    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;

That is, the first longword will contain something like 00800020 where 20
is the actual length of the Unicode string, and the next longword will
contain the address of buffer.  This address will be in system space, so
the high order bit will be set.

If the argument is a pointer to a device object, then the format of the first
word will be:

    USHORT Type;

That is, the first word will contain a 0003, where the Type code will ALWAYS
be 0003.

Note that this makes it immediately obvious whether the argument is a pointer
to an ARC name string or a device object, since a Unicode string can never
have an odd number of bytes, and a device object will always have a Type
code of 3.

BUGCODE_NDIS_DRIVER                  (0x7C)

This is the NDIS Driver Bugcheck for Windows Server 2003 and later.
For Windows 2000 and Windows XP, see 0xD2, BUGCODE_ID_DRIVER.

PARAMETERS
        1 - NDIS BugCheck Code
            VALUES:
                1 : Driver called NdisMAllocateSharedMemory at raised IRQL
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - The length of the requested shared memory
                        4 - The current IRQL
                2 : During a call to NdisMAllocateSharedMemory, NDIS detected
                    that a previously allocated shared memory page has been
                    corrupted.
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - The shared memory page that was corrupted.
                        4 - A pointer to a NDIS_WRAPPER_CONTEXT that keeps
                            track of shared memory allocations by the driver.
                3 : A Driver called NdisMFreeSharedMemory(Async) with a shared
                    memory pointer that has already been freed.
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - The page that this shared was allocated from.
                        4 - The virtual address of the shared memory.
                4 : [Only enabled on special instrumented NDIS.] AddDevice was
                    called with a driver that is not on the list of drivers
                    that are registered with NDIS.
                        2 - A pointer to NDIS_M_DRIVER_BLOCK.
                        3 - The driver object. (A pointer to DRIVER_OBJECT)
                        4 - Not used
                5 : An Ethernet driver indicated receiving a packet using a
                    packet descriptor that is currently in use by protocol
                    stack.  (Caught by checking stack packet location)
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - The packet descriptor used by the driver.  Use
                            !ndiskd.pkt on this pointer for more info.
%                           MICROSOFT INTERNAL:  You can also use, on an x86,
%                           dd (<pointer>-38).  On IA-64, dd (<pointer>-68).
                        4 - A pointer to the packet array that contained this
                            packet descriptor. (not much of a use)
                6 : An Ethernet driver indicated receiving a packet using a
                    packet descriptor that is currently in use by protocol
                    stack. (caught by checking packet reference count)
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - The packet descriptor used by the driver.  Use
                            !ndiskd.pkt on this pointer for more info.
%                           MICROSOFT INTERNAL:  You can also use, on an x86,
%                           dd (<pointer>-38).  On IA-64, dd (<pointer>-68).
                        4 - A pointer to the packet array that contained this
                            packet descriptor. (not much of a use)
                7 : A FDDI driver indicated receiving a packet using a packet
                    descriptor that is currently in use by protocol stack.
                    (caught by checking packet reference count)
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - The packet descriptor used by the driver.  Use
                            !ndiskd.pkt on this pointer for more info.
%                           MICROSOFT INTERNAL:  You can also use, on an x86,
%                           dd (<pointer>-38).  On IA-64, dd (<pointer>-68).
                        4 - A pointer to the packet array that contained this
                            packet descriptor. (not much of a use)
                8 : A Miniport did not deregister its interrupt during the halt
                    process.
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - A pointer to NDIS_MINIPORT_INTERRUP.
                        4 - Not used.
                9 : A Miniport halted without successfully canceling all its
                    timers.
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - A pointer to miniport's timer queue. type:
                            NDIS_MINIPORT_TIMER.
                        4 - Not used.
                0xA : A miniport driver is getting unloaded prematurely.
                        2 - A pointer to NDIS_M_DRIVER_BLOCK.
                        3 - The driver object. (A pointer to DRIVER_OBJECT).
                        4 - The ref count for the miniport driver.
                0xB : A Miniport failed initialization without deregistering
                      its interrupt.
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - A pointer to NDIS_MINIPORT_INTERRUP.
                        4 - Not used.
                0xC : A Miniport failed initialization without successfully
                      canceling all its timers.
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - A pointer to miniport's timer queue. type:
                            NDIS_MINIPORT_TIMER.
                        4 - Not used.
                0xD : A Miniport did not deregister its interrupt during the
                      halt process. (halt called from initialize routine after
                      miniport returned success from its initialize handler)
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - A pointer to NDIS_MINIPORT_INTERRUP.
                        4 - Not used.
                0xE : A Miniport halted without successfully canceling all its
                      timers. (halt called from initialize routine after
                      miniport returned success from its initialize handler)
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - A pointer to miniport's timer queue. type:
                            NDIS_MINIPORT_TIMER.
                        4 - Not used.
                0xF : A Miniport driver is calling NdisMResetComplete without
                      any pending reset request.
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - Reset status.
                        4 - AddressingReset BOOLEAN. (not much of a use).
                0x10 : After resuming from a low power state, a Miniport failed
                       initialization without deregistering its interrupt.
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - A pointer to NDIS_MINIPORT_INTERRUP.
                        4 - Not used.
                0x11 : After resuming from a low power state, a Miniport failed
                       initialization without successfully canceling all its
                       timers.
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - A pointer to miniport's timer queue. type:
                            NDIS_MINIPORT_TIMER.
                        4 - Not used.
                0x12 : A miniport driver indicated receiving a packet using a
                       packet descriptor that is currently in use by protocol
                       stack. (caught by checking packet reference count)
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - The packet descriptor used by the driver.  Use
                            !ndiskd.pkt on this pointer for more info.
%                           MICROSOFT INTERNAL:  You can also use, on an x86,
%                           dd (<pointer>-38).  On IA-64, dd (<pointer>-68).
                        4 - A pointer to the packet array that contained this
                            packet descriptor. (not much of a use)
                0x13 : A Token-Ring miniport driver indicated receiving a
                       packet using a packet descriptor that is currently in
                       use by protocol stack. (caught by checking packet
                       reference count)
                        2 - A pointer to Miniport block. !ndiskd.miniport on
                            this pointer for more info.
                        3 - The packet descriptor used by the driver.  Use
                            !ndiskd.pkt on this pointer for more info.
%                           MICROSOFT INTERNAL:  You can also use, on an x86,
%                           dd (<pointer>-38).  On IA-64, dd (<pointer>-68).
                        4 - A pointer to the packet array that contained this
                            packet descriptor. (not much of a use)
            END_VALUES


INSTALL_MORE_MEMORY              (0x7D)
PARAMETERS
        1 - Number of physical pages found
        2 - Lowest physical page
        3 - Highest physical page
        4 - 0

DESCRIPTION
Not enough memory to boot Windows.

SYSTEM_THREAD_EXCEPTION_NOT_HANDLED      (0x7E)
PARAMETERS
    1 - The exception code that was not handled
        VALUES
        0x80000002: (STATUS_DATATYPE_MISALIGNMENT) An unaligned data reference was encountered.
                    The trap frame will supply additional information.
        0x80000003: This means a hard coded breakpoint or assertion was hit, but this system was booted
                    /NODEBUG.  This is not supposed to happen as developers should never have
                    hardcoded breakpoints in retail code, but ...
                    If this happens, make sure a debugger gets connected, and the
                    system is booted /DEBUG.  This will let us see why this breakpoint is
                    happening.
        END_VALUES
    2 - The address that the exception occurred at
    3 - Exception Record Address
    4 - Context Record Address

DESCRIPTION
This is a very common bugcheck.  Usually the exception address pinpoints
the driver/function that caused the problem.  Always note this address
as well as the link date of the driver/image that contains this address.

SYSTEM_THREAD_EXCEPTION_NOT_HANDLED_M      (0x1000007E)
PARAMETERS
    1 - The exception code that was not handled
    2 - The address that the exception occurred at
    3 - Exception Record Address
    4 - Context Record Address

DESCRIPTION
This is a very common bugcheck.  Usually the exception address pinpoints
the driver/function that caused the problem.  Always note this address
as well as the link date of the driver/image that contains this address.
Some common problems are exception code 0x80000003.  This means a hard
coded breakpoint or assertion was hit, but this system was booted
/NODEBUG.  This is not supposed to happen as developers should never have
hardcoded breakpoints in retail code, but ...
If this happens, make sure a debugger gets connected, and the
system is booted /DEBUG.  This will let us see why this breakpoint is
happening.

An exception code of 0x80000002 (STATUS_DATATYPE_MISALIGNMENT) indicates
that an unaligned data reference was encountered.  The trap frame will
supply additional information.

UNEXPECTED_KERNEL_MODE_TRAP      (0x7F)

This means a trap occurred in kernel mode, and it's a trap of a kind
that the kernel isn't allowed to have/catch (bound trap) or that
is always instant death (double fault).  The first number in the
bugcheck parens is the number of the trap (8 = double fault, etc)
Consult an Intel x86 family manual to learn more about what these
traps are. Here is a *portion* of those codes:

PARAMETERS
        1 - x86 trap number
             VALUES:
             0: EXCEPTION_DIVIDED_BY_ZERO
             1: EXCEPTION_DEBUG
             2: EXCEPTION_NMI
             3: EXCEPTION_INT3
             5: EXCEPTION_BOUND_CHECK
             6: EXCEPTION_INVALID_OPCODE
             7: EXCEPTION_NPX_NOT_AVAILABLE
             8: EXCEPTION_DOUBLE_FAULT
             9: EXCEPTION_NPX_OVERRUN
             A: EXCEPTION_INVALID_TSS
             B: EXCEPTION_SEGMENT_NOT_PRESENT
             C: EXCEPTION_STACK_FAULT
             D: EXCEPTION_GP_FAULT
             F: EXCEPTION_RESERVED_TRAP
            10: EXCEPTION_NPX_ERROR
            11: EXCEPTION_ALIGNMENT_CHECK
            END_VALUES

DESCRIPTION
If kv shows a taskGate
        use .tss on the part before the colon, then kv.
Else if kv shows a trapframe
        use .trap on that value
Else
        .trap on the appropriate frame will show where the trap was taken
        (on x86, this will be the ebp that goes with the procedure KiTrap)
Endif

kb will then show the corrected stack.



UNEXPECTED_KERNEL_MODE_TRAP_M              (0x1000007F)

This means a trap occurred in kernel mode, and it's a trap of a kind
that the kernel isn't allowed to have/catch (bound trap) or that
is always instant death (double fault).  The first number in the
bugcheck parens is the number of the trap (8 = double fault, etc)
Consult an Intel x86 family manual to learn more about what these
traps are. Here is a *portion* of those codes:

PARAMETERS
        1 - x86 trap number
             VALUES:
             0: EXCEPTION_DIVIDED_BY_ZERO
             1: EXCEPTION_DEBUG
             2: EXCEPTION_NMI
             3: EXCEPTION_INT3
             5: EXCEPTION_BOUND_CHECK
             6: EXCEPTION_INVALID_OPCODE
             7: EXCEPTION_NPX_NOT_AVAILABLE
             8: EXCEPTION_DOUBLE_FAULT
             9: EXCEPTION_NPX_OVERRUN
             A: EXCEPTION_INVALID_TSS
             B: EXCEPTION_SEGMENT_NOT_PRESENT
             C: EXCEPTION_STACK_FAULT
             D: EXCEPTION_GP_FAULT
             F: EXCEPTION_RESERVED_TRAP
            10: EXCEPTION_NPX_ERROR
            11: EXCEPTION_ALIGNMENT_CHECK
            END_VALUES

DESCRIPTION
If kv shows a taskGate
        use .tss on the part before the colon, then kv.
Else if kv shows a trapframe
        use .trap on that value
Else
        .trap on the appropriate frame will show where the trap was taken
        (on x86, this will be the ebp that goes with the procedure KiTrap)
Endif

kb will then show the corrected stack.


NMI_HARDWARE_FAILURE             (0x80)

This is typically due to a hardware malfunction.  The hardware supplier should
be called.

SPIN_LOCK_INIT_FAILURE           (0x81)

DFS_FILE_SYSTEM                  (0x82)

SETUP_FAILURE                    (0x85)

(NOTE:  Textmode setup no longer uses bugchecks to bail out of serious
error conditions.  Therefore, you will never encounter a bugcheck 0x85.
All bugchecks have been replaced with friendlier and (where possible)
more descriptive error messages.  Some of the former bugchecks, however,
have simply been replaced by our own bugcheck screen, and the codes for
these error conditions are the same as before.  These are documented below.)

The first extended bugcheck field is a code indicating what the
problem is, and the other fields are used differently depending on
that value.
PARAMETERS
    1 -
        VALUES:
        0:  The oem hal font is not a valid .fon format file, and so setup
            is unable to display text.
            This indicates that vgaxxx.fon on the boot floppy or CD-ROM
            is damaged.

        1:  Video initialization failed.  NO LONGER A BUGCHECK CODE.
            This error now has its own error screen, and the user is only
            presented with the two relevant parameters detailed below.

            This may indicate that the disk containing vga.sys
            (or other video driver appropriate to the machine)
            is damaged or that machine has video hardware that
            Windows cannot communicate with.

            3 - Status code from NT API call, if appropriate.

            2 - What failed:
            VALUES:
                0 : NtCreateFile of \device\video0
                1 : IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES
                2 : IOCTL_VIDEO_QUERY_AVAIL_MODES
                3: Desired video mode not supported.  This is indicative of
                   an internal setup error.
                4: IOCTL_VIDEO_SET_CURRENT_MODE (unable to set video mode)
                5: IOCTL_VIDEO_MAP_VIDEO_MEMORY
                6: IOCTL_VIDEO_LOAD_AND_SET_FONT
            END_VALUES

        2:  Out of memory.  NO LONGER A BUGCHECK CODE.
            This error now uses a more friendly error screen that works
            regardless of how far along in setup Windows is.

        3:  Keyboard initialization failed.  NO LONGER A BUGCHECK CODE.
            There are now 2 error screens for the two different possible errors
            that can occur here.
            This may indicate that the disk containing the keyboard driver
            (i8042prt.sys or kbdclass.sys) is damaged, or that the machine has
            keyboard hardware Windows cannot communicate with.

            It may also mean that the keyboard layout dll could not be loaded.

            2 - What failed:
            VALUES:
                0: NtCreateFile of \device\KeyboardClass0 failed.
                   "Setup did not find a keyboard connected to your computer."
                1: Unable to load keyboard layout dll.
                   "Setup could not load the keyboard layout file <filename>."
                   Indicates that the cd or floppy is missing a file (kbdus.dll
                   for us release, other layout dlls for localized ones).
            END_VALUES

        4:  Setup was unable to resolve the ARC device pathname of
            the device from which setup was started. This is an internal
            Setup error.

        5:  Partitioning sanity check failed.  This indicates a bug in
            a disk driver.  The parameters are meaningful only to the setup
            group.

        END_VALUES

MBR_CHECKSUM_MISMATCH            (0x8B)

This message occurs during the boot process when the MBR checksum the system
calculates does not match the checksum passed in by the loader. This is usually
an indication of a virus. There are many forms of viruses and not all can be
detected. The newer ones usually can only be detected by a virus scanner that
has recently been upgraded. Boot a write-protected disk containing a virus
scanner and attempt to clean out the infection.

PARAMETERS

    1 - Disk Signature from MBR.
    2 - MBR checksum calculated by osloader.
    3 - MBR checksum calculated by system.

KERNEL_MODE_EXCEPTION_NOT_HANDLED      (0x8E)
PARAMETERS
    1 - The exception code that was not handled
    2 - The address that the exception occurred at
    3 - Trap Frame

DESCRIPTION
This is a very common bugcheck.  Usually the exception address pinpoints
the driver/function that caused the problem.  Always note this address
as well as the link date of the driver/image that contains this address.
Some common problems are exception code 0x80000003.  This means a hard
coded breakpoint or assertion was hit, but this system was booted
/NODEBUG.  This is not supposed to happen as developers should never have
hardcoded breakpoints in retail code, but ...
If this happens, make sure a debugger gets connected, and the
system is booted /DEBUG.  This will let us see why this breakpoint is
happening.

An exception code of 0x80000002 (STATUS_DATATYPE_MISALIGNMENT) indicates
that an unaligned data reference was encountered.  The trap frame will
supply additional information.


KERNEL_MODE_EXCEPTION_NOT_HANDLED_M      (0x1000008E)
PARAMETERS
    1 - The exception code that was not handled
    2 - The address that the exception occurred at
    3 - Trap Frame

DESCRIPTION
This is a very common bugcheck.  Usually the exception address pinpoints
the driver/function that caused the problem.  Always note this address
as well as the link date of the driver/image that contains this address.
Some common problems are exception code 0x80000003.  This means a hard
coded breakpoint or assertion was hit, but this system was booted
/NODEBUG.  This is not supposed to happen as developers should never have
hardcoded breakpoints in retail code, but ...
If this happens, make sure a debugger gets connected, and the
system is booted /DEBUG.  This will let us see why this breakpoint is
happening.

An exception code of 0x80000002 (STATUS_DATATYPE_MISALIGNMENT) indicates
that an unaligned data reference was encountered.  The trap frame will
supply additional information.


PP0_INITIALIZATION_FAILED        (0x8F)

This message occurs if phase 0 initialization of the kernel-mode Plug and
Play Manager failed.

PP1_INITIALIZATION_FAILED        (0x90)

This message occurs if phase 1 initialization of the kernel-mode Plug and
Play Manager failed.  This is where most of the initialization is done,
including setting up the environment (registry, etc.) for drivers to
subsequently call during I/O init.


UP_DRIVER_ON_MP_SYSTEM           (0x92)

This message occurs if a UNIPROCESSOR only driver is loaded on a MultiProcessor
system with more than one active processor.

PARAMETERS

    1 - The Base address of the driver.

INVALID_KERNEL_HANDLE           (0x93)

This message occurs if kernel code (server, redirector, other driver, etc.)
attempts to close a handle that is not a valid handle.

PARAMETERS

    1 - The handle that NtClose was called with.

    2 -
    VALUES
        0 : means a protected handle was closed.
        1 : means an invalid handle was closed.
    END_VALUES

KERNEL_STACK_LOCKED_AT_EXIT     (0x94)

This message occurs when a thread exits while its kernel stack is
marked as not swapable

INVALID_WORK_QUEUE_ITEM         (0x96)

This message occurs when KeRemoveQueue removes a queue entry whose flink
or blink field is null.  This is almost always called by code misusing
worker thread work items, but any queue misuse can cause this.  The rule
is that an entry on a queue may only be inserted on the list once. When an
item is removed from a queue, it's flink field is set to NULL. This bugcheck
occurs when remove queue attempts to remove an entry, but the flink or blink
field is NULL. In order to debug this problem, you need to know the queue being
referenced.

In an attempt to help identify the guilty driver, this bugcheck assumes the
queue is a worker queue (ExWorkerQueue) and prints the worker routine as
parameter 4 below.

PARAMETERS

    1 - The address of the queue entry whose flink/blink field is NULL
    2 - The address of the queue being references. Usually this is one
        of the ExWorkerQueues.
    3 - The base address of the ExWorkerQueue array. This will help determine
        if the queue in question is an ExWorkerQueue and if so, the offset from
        this parameter will isolate the queue.
    4 - If this is an ExWorkerQueue (which it usually is), this is the address
        of the worker routine that would have been called if the work item was
        valid. This can be used to isolate the driver that is misusing the work
        queue.


BOUND_IMAGE_UNSUPPORTED         (0x97)

END_OF_NT_EVALUATION_PERIOD     (0x98)

Your Windows System is an evaluation unit with an expiration date. The trial
period is over.

PARAMETERS

    1 - The low order 32 bits of your installation date
    2 - The high order 32 bits of your installation date
    3 - The trial period in minutes

INVALID_REGION_OR_SEGMENT       (0x99)

ExInitializeRegion or ExInterlockedExtendRegion was called with an invalid
set of parameters.

SYSTEM_LICENSE_VIOLATION        (0x9A)

A violation of the software license agreement has occurred. This can be due to
either attempting to change the product type of an offline system, or an attempt
to change the trial period of an evaluation unit of Windows.

PARAMETERS
    1 - Violation type
    VALUES:
        0 : means that offline product type changes were attempted
            2 - if 1, product should be LanmanNT or ServerNT. If 0, should be WinNT
            3 - partial serial number
            4 - first two characters of product type from product options.

        1 : means that offline changes to the nt evaluation unit time period
            2 - registered evaluation time from source 1
            3 - partial serial number
            4 - registered evaluation time from alternate source


        2 : means that the setup key could not be opened
            2 - status code associated with the open failure

        3 : The SetupType value from the setup key is missing so gui setup
            mode could not be detected
            2 - status code associated with the key lookup failure

        4 : The SystemPrefix value from the setup key is missing
            2 - status code associated with the key lookup failure

        5 : means that offline changes were made to the number of licensed processors
            2 - see setup code
            3 - invalid value found in licensed processors
            4 - officially licensed number of processors

        6 : means that ProductOptions key could not be opened
            2 - status code associated with the open failure

        7 : means that ProductType value could not be read
            2 - status code associated with the read failure

        8 : means that Change Notify on ProductOptions failed
            2 - status code associated with the change notify failure

        9 : means that Change Notify on SystemPrefix failed
            2 - status code associated with the change notify failure

        0xA : An NTW system was converted to an NTS system

        0xB : Reference of setup key failed
            2 - status code associated with the change failure

        0xC : Reference of product options key failed
            2 - status code associated with the change failure

        0xD : Open of ProductOptions in worker thread failed
            2 - status code associated with the failure

        0xF : Open of setup key failed
            2 - status code associated with the failure

        0x10 : Failure occurred in the setup key worker thread
            2 - status code associated with the failure
            3 - 0 means set value failed, 1 means change notify failed

        0x11 : Failure occurred in the product options key worker thread
            2 - status code associated with the failure
            3 - 0 means set value failed, 1 means change notify failed

        0x12 : Could not open the LicenseInfoSuites key for the suite
            2 - status code associated with the failure

        0x13 : Could not query the LicenseInfoSuites key for the suite
            2 - status code associated with the failure

        0x14 : Could not allocate memory
            2 - size of memory alllocation

        0x15 : Could not re-set the ConcurrentLimit value for the suite key
            2 - status code associated with the failure

        0x16 : Could not open the license key for a suite product
            2 - status code associated with the failure

        0x17 : Could not re-set the ConcurrentLimit value for a suite product
            2 - status code associated with the failure

        0x18 : Could not start the change notify for the LicenseInfoSuites
            2 - status code associated with the open failure

        0x19 : A suite is running on a system that must be pdc

        0x20 : Failure occurred when enumerating the suites
            2 - status code associated with the failure

        END_VALUES


UDFS_FILE_SYSTEM              (0x9B)
%   All file system bug checks have encoded in their first ULONG
%   the source file and the line within the source file that generated
%   the bugcheck.  The high 16-bits contains a number to identify the
%   file and low 16-bits is the source line within the file where
%   the bug check call occurs.  For example, 0x00020009 indicates
%   that the UDF file system bugcheck occurred in source file #2 and
%   line #9.
%
%   The file system calls bug check in multiple places and this will
%   help us identify the actual source line that generated the bug
%   check.

    If you see UdfExceptionFilter on the stack then the 2nd and 3rd
    parameters are the exception record and context record. Do a .cxr
    on the 3rd parameter and then kb to obtain a more helpful stack
    trace.

MACHINE_CHECK_EXCEPTION         (0x9C)

A fatal Machine Check Exception has occurred.

KeBugCheckEx parameters;

    x86 Processors
        If the processor has ONLY MCE feature available (For example Intel
        Pentium), the parameters are:

        1 - Low  32 bits of P5_MC_TYPE MSR
        2 - Address of MCA_EXCEPTION structure
        3 - High 32 bits of P5_MC_ADDR MSR
        4 - Low  32 bits of P5_MC_ADDR MSR

        If the processor also has MCA feature available (For example Intel
        Pentium Pro), the parameters are:

        1 - Bank number
        2 - Address of MCA_EXCEPTION structure
        3 - High 32 bits of MCi_STATUS MSR for the MCA bank that had the error
        4 - Low  32 bits of MCi_STATUS MSR for the MCA bank that had the error

    IA64 Processors

        1 - Bugcheck Type
            1 - MCA_ASSERT
            2 - MCA_GET_STATEINFO
                SAL returned an error for SAL_GET_STATEINFO while processing MCA.
            3 - MCA_CLEAR_STATEINFO
                SAL returned an error for SAL_CLEAR_STATEINFO while processing MCA.
            4 - MCA_FATAL
                FW reported a fatal MCA.
            5 - MCA_NONFATAL
                SAL reported a recoverable MCA and we don't support currently
                support recovery or SAL generated an MCA and then couldn't
                produce an error record.
            0xB - INIT_ASSERT
            0xC - INIT_GET_STATEINFO
                  SAL returned an error for SAL_GET_STATEINFO while processing INIT event.
            0xD - INIT_CLEAR_STATEINFO
                  SAL returned an error for SAL_CLEAR_STATEINFO while processing INIT event.
            0xE - INIT_FATAL
                  Not used.
        2 - Address of log
        3 - Size of log
        4 - Error code in the case of x_GET_STATEINFO or x_CLEAR_STATEINFO

USER_MODE_HEALTH_MONITOR        (0x9E)

One or more critical user mode components failed to satisfy a health check.

Hardware mechanisms such as watchdog timers can detect that basic kernel
services are not executing. However, resource starvation issues, including
memory leaks, lock contention, and scheduling priority misconfiguration,
may block critical user mode components without blocking DPCs or
draining the nonpaged pool.

Kernel components can extend watchdog timer functionality to user mode
by periodically monitoring critical applications. This bugcheck indicates
that a user mode health check failed in a manner such that graceful
shutdown is unlikely to succeed. It restores critical services by
rebooting and/or allowing application failover to other servers.

PARAMETERS
    1 - Process that failed to satisfy a health check within the
        configured timeout
    2 - Health monitoring timeout (seconds)

DRIVER_POWER_STATE_FAILURE      (0x9F)

A driver is causing an inconsistent power state.

PARAMETERS

     1 - SubCode
     VALUES:
        1   : The device object is being freed which still has an
              outstanding power request which it has not completed
              2 - DeviceObject
        2   : The device object completed the irp for the system power
              state request, but failed to call PoStartNextPowerIrp
              2 - Optional Target device's DEVICE_OBJECT
              3 - DeviceObject
              4 - Optional DriverObject      
      0x500 : The device object completed the irp for the system power
              state request, but failed to call PoStartNextPowerIrp.
              3 - Optional Target device's DEVICE_OBJECT
              4 - DeviceObject
     END_VALUES


INTERNAL_POWER_ERROR            (0xA0)

The power policy manager experienced a fatal error.

PARAMETERS:

    1 -
    VALUES:
        1  : Error Handling power IRP.
           2 -
           VALUES:
               1  : A device has overrun its maximum number of reference counts.
                    3 - maximum number of references allowed.
               2  : Too many inrush power irps have been queue.
                    3 - maximum number of pending irps allowed.
               3  : Too many inrush power irps have been queue.
                    3 - maximum number of pending irps allowed.
               4  : Too many inrush power irps have been queue.
                    3 - maximum number of pending irps allowed.
               5  : Power IRP has been sent to a passive level deviceobject.
             END_VALUES
        2  : (POP_INTERNAL) An internal failure has occurred while attempting to process
             a power event.  To debug this, dump the stack and look for the
             function ntoskrnl!_PopInternalError.  The first argument is
             (error_code << 16) | __LINE__.  If the caller is PopExceptionFilter, then
             the first argument to this function is of type PEXCEPTION_POINTERS.
             In the debugger, type 'dt nt!_EXCEPTION_POINTERS <argument>'.
             Then type '.cxr <value of context record from the previous command>'.
             All subsequent debugger commands will show you the actual
             source of the error.  Start with a stack trace by typing 'kb'.
        3  : The checksum for a hibernation context page does not match.
             2 - Expected checksum
             3 - Actual checksum
             4 - Linenumber of the failure.
        4  : The checksum for a page about to be written to the hibernation file
             does not match its expected checksum.
             2 - Expected checksum
             3 - Actual checksum
             4 - Linenumber of the failure.
        5  : An unknown shutdown code has been sent to the system shutdown handler.
        
        7  : (POP_MISC) Unhandled exception.  To debug this, dump the stack.  Look
             for the function ntoskrnl!PopExceptionFilter.  The first
             argument to this function is of type PEXCEPTION_POINTERS.
             In the debugger, type 'dt nt!_EXCEPTION_POINTERS <argument>'.
             Then type '.cxr <value of context record from the previous command>'.
             All subsequent debugger commands will show you the actual
             source of the error.  Start with a stack trace by typing 'kb'.
        8  : (POP_SYS) A fatal error has occured while processing a system power event.
             2 -
             VALUES:
                 0x100 : An unknown device type is being processed.
                         3 - DEVICE_OBJECT
                         4 - POWER_CHANNEL_SUMMARY
             END_VALUES
        
        0x101 : Unhandled exception occured while processing a system power event.
                3 - ExceptionPointer.  To debug this, in the debugger type:
                    'dt nt!_EXCEPTION_POINTERS <argument>'.  Then type:
                    '.cxr <value of context record from the previous command>'.
                    All subsequent debugger commands will show you the actual
                    source of the error.  Start with a stack trace by typing 'kb'.
        
        
        0x102 : Hibernation orking buffer size is not page aligned
                3 - DUMP_INITIALIZATION_CONTEXT
                4 - POP_HIBER_CONTEXT
        0x103 : All working pages have failed to be accounted for during the hibernation process.
                3 - POP_HIBER_CONTEXT
        0x104 : An attempt was made to map internal hibernate memory while
                the internal memory structures were locked.
                3 - POP_HIBER_CONTEXT
        0x105 : An attempt was made to map internal hibernate memory with
                an unsupported memory type flag.
                3 - POP_HIBER_CONTEXT
        0x106 : A memory descriptor list was created during the hibernation process which
                describes memory that is not paged-aligned.
                3 - MDL
        0x107 : A data mismatch has occurred in the internal hibernation data
                structures.
                3 - POP_HIBER_CONTEXT
                4 - PO_MEMORY_RANGE_ARRAY
        0x108 : The disk subsystem failed to properly write part of the hibernation
                file.
                3 - POP_HIBER_CONTEXT
        0x109 : The checksum for the processor state data does not match its
                expected checksum.
                3 - Expected checksum
                4 - Actual checksum
        0x10A : The disk subsystem failed to properly write part of the hibernation
                file.
                3 - POP_HIBER_CONTEXT
                4 - NTSTATUS failure code        
        0x200 : An unknown device type is being checked for an idle state.
                3 - DEVICE_OBJECT
                4 - DEVICE_OBJECT_POWER_EXTENSION
        0x300 : An unknown status was returned from a battery power IRP.
                3 - DEVICE_OBJECT
                4 - IRP                        
        0x301 : The battery has entered an unknown state..
                3 - DEVICE_OBJECT
                4 - IRP                        
        0x400 : A device has overrun its maximum number of reference counts.
                3 - IO_STACK_LOCATION
                4 - DEVICE_OBJECT
        0x401 : Too many inrush power irps have been queue.
                3 - Pending IRP list.
                4 - DEVICE_OBJECT
        0x402 : Too many inrush power irps have been queue.
                3 - Pending IRP list.
                4 - DEVICE_OBJECT
        0x403 : Too many inrush power irps have been queue.
                3 - Pending IRP list.
                4 - DEVICE_OBJECT
        0x404 : Power IRP has been sent to a passive level deviceobject.
                3 - IO_STACK_LOCATION.
                4 - DEVICE_OBJECT
        0x500 : An unknown status was returned from a thermal power IRP.
              3 - IRP
              4 - DEVICE_OBJECT                        

     END_VALUES


PCI_BUS_DRIVER_INTERNAL         (0xA1)

The PCI Bus driver detected inconsistency
problems in its internal structures and could not continue.


MEMORY_IMAGE_CORRUPT            (0xA2)
On a system wake operation, various regions of memory may be CRCed to
guard against memory failures.

PARAMETERS

     1 -
     VALUES:
        2   : Table page check failure
                2 - IF PARAMETER 3 IS ZERO:
                    the page number in of the table page which failed
                    IF PARAMETER 3 IS NONZERO:
                    the page number with the failing page run index
                3 - the index which failed to match the run  (or zero)

        3   : The checksum for the range of memory listed is incorrect
                2 - starting physical page # of the range
                3 - length (in pages) of the range
                4 - the page number of the table page containing this run
     END_VALUES


ACPI_DRIVER_INTERNAL            (0xA3)

The ACPI Driver detected an internal inconsistency. The inconsistency is
so severe that continuing to run would cause serious problems.

The ACPI driver calls this when the state is so inconsistent that proceeding
would actually be dangerous. The problem may or may not be a BIOS issue, but
there is no way to tell.

%  To find the place where the driver died, look at the 2nd parameter. The high
%  word corresponds to an entry in wdm\acpi\driver\nt\debug.h. The low word
%  corresponds to a line number.


CNSS_FILE_SYSTEM_FILTER          (0xA4)
    See the comment for FAT_FILE_SYSTEM (0x23)

ACPI_BIOS_ERROR                 (0xA5)

The ACPI Bios in the system is not fully compliant with the ACPI specification.
The first value indicates where the incompatibility lies:

PARAMETERS

    1 -
    VALUES:

      1 : ACPI_ROOT_RESOURCES_FAILURE.
          ACPI cannot find the SCI Interrupt vector in the resources handed
          to it when ACPI is started.

         2 - ACPI's deviceExtension
         3 - ACPI's ResourceList
         4 -
         VALUES:
             0 : No resource list found
             1 : No IRQ resource found in list
         END_VALUES

      2 : ACPI_ROOT_PCI_RESOURCE_FAILURE
          ACPI could not process the resource list for the PCI root buses
%         There is an White Paper on the Web Site about this problem.

         2 - The ACPI Extension for the PCI bus.
         3 -
         VALUES:
            0 : ACPI cannot convert the BIOS' resource list into the proper
                format. This probably represents a flaw in the BIOS' list
                encoding procedure.
              4 - Pointer to the QUERY_RESOURCES irp
            1 : ACPI cannot convert the BIOS' resource list into the proper
                format. This probably represents a flaw in the BIOS' list
                encoding procedure.
              4 - Pointer to the QUERY_RESOURCE_REQUIREMENTS irp
            2 : ACPI found an empty resource list.
              4 - 0
            3 : ACPI could not find the current bus number in the CRS.
              4 - Pointer to the PNP CRS descriptor
            Other : Any other value is a pointer to the Resource List for PCI.
                    This indicates that there is an overlap between the list of
                    resources that PCI claims to decode and the list of memory
                    regions reported by the E820 BIOS interface.
              4 - Pointer to the E820 Memory Table
         END_VALUES

       3 : ACPI_FAILED_MUST_SUCCEED_METHOD
           ACPI tried to run a control method while creating device extensions
           to represent the ACPI namespace, but this control method failed.

         2 - The ACPI Object that was being run
         3 - return value from the interpreter
         4 - Name of the control method (in ULONG format)

       4 : ACPI_PRW_PACKAGE_EXPECTED_INTEGER
           ACPI evaluated a _PRW and expected to find an integer as a
           package element.

         2 - The ACPI Extension for which the _PRW belongs to
         3 - Pointer to the method
         4 - The DataType returned (see amli.h)

       5 : ACPI_PRW_PACKAGE_TOO_SMALL
           ACPI evaluated a _PRW and the package that came back failed to
           contain at least 2 elements. The ACPI specification requires that
           two elements to always be present in a _PRW.

         2 - The ACPI Extension for which the _PRW belongs to
         3 - Pointer to the _PRW
         4 - Number of elements in the _PRW

       6 : ACPI_PRx_CANNOT_FIND_OBJECT
           ACPI tried to find a named object named, but could not find it.

         2 - The ACPI Extension for which the _PRx belongs to
         3 - Pointer to the _PRx
         4 - Pointer to the name of the object to look for

       7 : ACPI_EXPECTED_BUFFER
           ACPI evaluated a method and expected to receive a Buffer in return.
           However, the method returned some other data type

         2 - The ACPI Extension for which the method belongs to
         3 - Pointer to the method
         4 - The DataType returned (see amli.h)

       8 : ACPI_EXPECTED_INTEGER
           ACPI evaluated a method and expected to receive an Integer in return.
           However, the method returned some other data type

         2 - The ACPI Extension for which the method belongs to
         3 - Pointer to the method
         4 - The DataType returned (see amli.h)

       9 : ACPI_EXPECTED_PACKAGE
           ACPI evaluated a method and expected to receive a Package in return.
           However, the method returned some other data type

         2 - The ACPI Extension for which the method belongs to
         3 - Pointer to the method
         4 - The DataType returned (see amli.h)

       0xA : ACPI_EXPECTED_STRING
           ACPI evaluated a method and expected to receive a String in return.
           However, the method returned some other data type

         2 - The ACPI Extension for which the method belongs to
         3 - Pointer to the method
         4 - The DataType returned (see amli.h)

       0xB : ACPI_EJD_CANNOT_FIND_OBJECT
           ACPI cannot find the object referenced to by an _EJD string

         2 - The ACPI Extension for which which the _EJD belongs to
         3 - The status returned by the interpreter
         4 - Name of the object ACPI is trying to find

       0xC : ACPI_CLAIMS_BOGUS_DOCK_SUPPORT
           ACPI provides faulty/insufficient information for dock support

         2 - The ACPI Extension for which ACPI found a dock device
         3 - Pointer to the _EJD method
         4 -
         VALUES:
            0: Bios does not claim system is dockage
            1: Duplicate device extensions for dock device
         END_VALUES

       0xD : ACPI_REQUIRED_METHOD_NOT_PRESENT
           ACPI could not find a required method/object in the namespace
           This is the bugcheck that is used if a vendor does not have an
           _HID or _ADR present

         2 - The ACPI Extension that ACPI needs the object for
         3 - The (ULONG) name of the method ACPI looked for
         4 -
         VALUES:
            0: Base Case
            1: Conflict
         END_VALUES

       0xE : ACPI_POWER_NODE_REQUIRED_METHOD_NOT_PRESENT
           ACPI could not find a requird method/object in the namespace for
           a power resource (or entity other than a "device"). This is the
           bugcheck used if a vendor does not have an _ON, _OFF, or _STA present
           for a power resource

         2 - The NS PowerResource that ACPI needs the object for
         3 - The (ULONG) name of the method ACPI looked for
         4 -
         VALUES:
            0: Base Case
         END_VALUES

       0xF : ACPI_PNP_RESOURCE_LIST_BUFFER_TOO_SMALL
           ACPI could not parse the resource descriptor
         2 - The current buffer that ACPI was parsing
         3 - The buffer's tag
         4 - The specified length of the buffer
         
       0x10 : ACPI_CANNOT_MAP_SYSTEM_TO_DEVICE_STATES
            ACPI could not map determine the system to device state mapping
            correctly.
%           There is a very long white paper about this topic.
         2 - The ACPI Extension for which ACPI is trying to do the mapping
         3 -
         VALUES:
            0 : The _PRx mapped back to a non-supported S-state
              4 - The DEVICE_POWER_STATE (ie: x+1)
            1 : ACPI cannot find a D-state to associate with the S-state
              4 - The SYSTEM_POWER_STATE that is causing the problem
            2 : The device claims to be able to wake the system when the
                system is in this S-state, but the system doesn't actually
                support this S-state.
              4 - The SYSTEM_POWER_STATE that is causing the problem
         END_VALUES

       0x11 : ACPI_SYSTEM_CANNOT_START_ACPI
            The system could not enter ACPI mode

         2 -
         VALUES:
            0 : System could not initialize AML interpreter
            1 : System could not find RSDT
            2 : System could not allocate critical driver structures
            3 : System could not load RSDT
            4 : System could not load DDBs
            5 : System cannot connect Interrupt vector
            6 : SCI_EN never becomes set in PM1 Control Register
            7 : Table checksum is incorrect
              3 - Pointer to the table that had a bad checksum
              4 - Creator Revision
            8 : Failed to load DDB
              3 - Pointer to the table that ACPI failed to load
              4 - Creator Revision
            9 : Unsupported Firmware Version
              3 - FADT Version
            0xA : System could not find MADT
            0xB : System could not find any valid Local SAPIC structures in
                  the MADT
         END_VALUES
         
        0x14 : ACPI_PNP_RESOURCE_LIST_LENGTH_TOO_LARGE
           ACPI could not parse the resource descriptor, the length exceeds
           MAXULONG.
         2 - The current buffer that ACPI was parsing
         3 - The buffer's tag
         4 - Pointer to a variable containing ULONGLONG length of the buffer


      0x2001 : ACPI_FAILED_PIC_METHOD
             ACPI tried to evaluate the PIC control method and but failed

         2 - InterruptModel (Integer)
         3 - return value from interpreter
         4 - Pointer to the PIC control method

      0x10001 : ACPI_CANNOT_ROUTE_INTERRUPTS
              ACPI tried to do interrupt routing, but failed

         2 - Pointer to the device object
         3 - Pointer to the parent of the device object
         4 - Pointer to the PRT

      0x10002 : ACPI_PRT_CANNOT_FIND_LINK_NODE
              ACPI could not find the link node referenced in a _PRT
              (This code exists in Windows 2000, but is obsolete in XP)

         2 - Pointer to the device object
         3 - Pointer to the string name ACPI is looking for, but could not find.
         4 - Pointer to the PRT. Dump this with !acpikd.nsobj <argument 4>

      0x10003 : ACPI_PRT_CANNOT_FIND_DEVICE_ENTRY
              ACPI could not find a mapping in the _PRT package for a device
              (This code exists in Windows 2000, but is obsolete in XP)

         2 - Pointer to the device object
         3 - The Device ID / Function Number. This DWORD is encoded
             as follows: Bits 5:0 are the PCI Device Number, Bits 8:6 are
             the PCI function number.
         4 - Pointer to the PRT. Dump this with !acpikd.nsobj <argument 4>

      0x10005 : ACPI_PRT_HAS_INVALID_FUNCTION_NUMBERS
              ACPI found an entry in the _PRT for which the function ID isn't
              all F's.  The generic format for a _PRT entry is that the device
              number is specified, but the function number isn't.  If it isn't
              done this way, then the machine vendor can introduce dangerous
              ambiguities.
              (This code exists in Windows 2000, but is obsolete in XP)
%
%             The Win98 behaviour is to bugcheck if it see this condition,
%             so the NT operating systems do so as well.

         2 - Pointer to the PRT object. Dump this with !acpikd.nsobj <argument 2>
         3 - Pointer to the current PRT Element. This is an index into the PRT.
         4 - The DeviceID/FunctionID of the element. This DWORD is
             encoded. Bits 15:0 are the PCI Function Number.
             Bits 31:16 are the PCI Device Number.

      0x10006 : ACPI_LINK_NODE_CANNOT_BE_DISABLED
              ACPI found a link node, but cannot disable it. Link nodes must
              be disable to allow for reprogramming

         2 - Pointer to the link node. This device is missing the _DIS method.


      0x10007 :
              The _PRT contained a reference to a vector not described in the
              I/O APIC entries MAPIC table.
         2 - The vector that couldn't be found

      0x20000 :
              The PM_TMR_BLK entry in the Fixed ACPI Description Table doesn't
              point to a working ACPI timer block.
         2 - The I/O port in the Fixed Table

      END_VALUES

DESCRIPTION
This bug check covers a great variety of ACPI problems.  If a kernel debugger
is attached, use "!analyze -v".  This command will analyze the precise problem,
and display whatever information is most useful for debugging the specific
error.

BAD_EXHANDLE                    (0xA7)

The kernel mode handle table detected an inconsistent handle table
entry state.

SESSION_HAS_VALID_POOL_ON_EXIT  (0xAB)
PARAMETERS
        1 - session ID
        2 - number of paged pool bytes that are leaking
        3 - number of nonpaged pool bytes that are leaking
        4 - total number of paged and nonpaged allocations that are leaking.
            nonpaged allocations are in the upper half of this word,
            paged allocations are in the lower half of this word.

DESCRIPTION
Caused by a session driver not freeing its pool allocations prior to a
session unload.  This indicates a bug in win32k.sys, atmfd.dll,
rdpdd.dll or a video driver.

HAL_MEMORY_ALLOCATION           (0xAC)

The HAL was unable to obtain memory for a system critical requirement.
These allocations are made early in system initialization and such a
failure is not expected.  It probably indicates some other critical error
such as pool corruption or massive consumption.

PARAMETERS
        1 - Allocation size.
        2 - 0
        3 - Pointer to string containing file name.
        4 - (reserved)
%       4 - Line number of call to KeBugCheckEx.

VIDEO_DRIVER_INIT_FAILURE       (0xB4)

The system was not able to go into graphics mode because no display drivers
were able to start.  This usually occurs if no video miniport drivers load
successfully.

ATTEMPTED_SWITCH_FROM_DPC       (0xB8)

A wait operation, attach process, or yield was attempted from a DPC routine.
This is an illegal operation and the stack track will lead to the offending
code and original DPC routine.

CHIPSET_DETECTED_ERROR          (0xB9)

SESSION_HAS_VALID_VIEWS_ON_EXIT  (0xBA)
PARAMETERS
        1 - session ID
        2 - number of mapped views that are leaking
        3 - address of this session's mapped views table
        4 - size of this session's mapped views table.

DESCRIPTION
Caused by a session driver not unmapping its mapped views prior to a
session unload.  This indicates a bug in win32k.sys, atmfd.dll,
rdpdd.dll or a video driver.

NETWORK_BOOT_INITIALIZATION_FAILED  (0xBB)
PARAMETERS
    1 - the part of network initialization that failed
    2 - the failure status
DESCRIPTION
Caused if we are booting off the network, and a critical function fails during
IO initialization. Currently the codes for the first value are:
1 - updating the registry.
2 - starting the network stack - Windows sends IOCTLs to the redirector and
    datagram receiver, then waits for the redirector to be ready. If it is not
    ready within a certain period of time, initialization fails.
3 - failed sending the DHCP IOCTL to TCP - this is how Windows informs the
    transport of its IP adress.

NETWORK_BOOT_DUPLICATE_ADDRESS  (0xBC)
PARAMETERS
    1 - the IP address, show as a hex DWORD. So an address aa.bb.cc.dd will
        appear as 0xddccbbaa.
    2 - the hardware address of the other machine.
    3 - the hardware address of the other machine.
    4 - the hardware address of the other machine. For Ethernet, a MAC address
        of aa-bb-cc-dd-ee-ff will be indicated by the second parameter
        containing 0xaabbccdd, the third parameter containing 0xeeff0000, and
        the fourth parameter containing 0x00000000.
DESCRIPTION
This indicates that when TCP/IP sent out an ARP for its IP address, it got
a response from another machine, indicating a duplicate IP address. When we
are booting off the network this is a fatal error.

INVALID_HIBERNATED_STATE    (0xBD)
The hibernated memory image does not match the current hardware configuration.
This bugcheck occurs when a system resumes from hibernate and discovers that the
hardware has been changed while the system was hibernated.

Note:  This bug check no longer exists in any version of Windows.
%  JVert wants this kept in bugcodes.w in case it is reinstated at some
%  point in the future.

PARAMETERS
    1 - hardware that was invalid
        VALUES:
        1 : Number of installed processors is less than before the hibernation
            2 - number of processors before hibernation
            3 - number of processors after hibernation
        END_VALUES

ATTEMPTED_WRITE_TO_READONLY_MEMORY    (0xBE)
An attempt was made to write to readonly memory.  The guilty driver is on the
stack trace (and is typically the current instruction pointer).

PARAMETERS
    1 - Virtual address for the attempted write.
    2 - PTE contents.
    3 - (reserved)
%   3 - Unique internal Mm information.
    4 - (reserved)
%   4 - Unique internal Mm code.

DESCRIPTION
When possible, the guilty driver's name (Unicode string) is printed on
the bugcheck screen and saved in KiBugCheckDriver.

MUTEX_ALREADY_OWNED             (0xBF)

This thread is attempting to acquire ownership of a mutex it already owns.

PARAMETERS
        1 - Address of Mutex
        2 - Thread
        3 - 0
        4 - (reserved)
%       4 - Unique value to help development isolate the instance.

SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION     (0xC1)

Special pool has detected memory corruption.  Typically the current thread's
stack backtrace will reveal the guilty party.

PARAMETERS
    4 - subclass of driver violation.
        VALUES:
        0x20 : caller is trying to free pool which is not allocated
            1 - address trying to free
            2 - (reserved)
%           2 - Mm internal code
            3 - 0.

        0x21 : caller is trying to free a bad address
            1 - address trying to free
            2 - bytes requested
            3 - bytes calculated

        0x22 : caller is trying to free a bad address
            1 - address trying to free
            2 - bytes requested
            3 - bytes calculated

        0x23 : caller is freeing an address where nearby bytes within the same page have been corrupted
            1 - address trying to free
            2 - address where bits are corrupted
            3 - (reserved)
%           3 - unique internal Mm pattern

        0x24 : caller is freeing an address where bytes after the end of the allocation have been overwritten
            1 - address trying to free
            2 - address where bits are corrupted
            3 - (reserved)
%           3 - unique internal Mm pattern

        0x30 : caller is trying to allocate pool from an incorrect IRQL level
            1 - current IRQL
            2 - pool type
            3 - number of bytes

        0x31 : caller is trying to free pool from an incorrect IRQL level
            1 - current IRQL
            2 - pool type
            3 - address trying to free

        END_VALUES


BAD_POOL_CALLER                             (0xC2)

The current thread is making a bad pool request.  Typically this is at a bad IRQL level or double freeing the same allocation, etc.

PARAMETERS
    1 - type of pool violation the caller is guilty of.
        VALUES:

    0x0 : The caller is requesting a zero byte pool allocation.
       Parameter 2 - zero.
       Parameter 3 - the pool type being allocated.
       Parameter 4 - the pool tag being used.

    0x1 :  Pool header has been corrupted
        Parameter 2 - Pointer to pool header
        Parameter 3 - First part of pool header contents
        Parameter 4 - 0

    0x2 :  Pool header has been corrupted
        Parameter 2 - Pointer to pool header
        Parameter 3 - First part of pool header contents
        Parameter 4 - 0

    0x4 :  Pool header has been corrupted
        Parameter 2 - Pointer to pool header
        Parameter 3 - First part of pool header contents
        Parameter 4 - 0

    0x6 :    Attempt to free pool which was already freed
        Parameter 2 - (reserved)
%       Parameter 2 - __LINE__
        Parameter 3 - Pointer to pool header
        Parameter 4 - Pool header contents

    0x7 :    Attempt to free pool which was already freed
        Parameter 2 - (reserved)
%       Parameter 2 - __LINE__
        Parameter 3 - Memory contents of the pool block
        Parameter 4 - Pointer to pool header

    0x8 :    Attempt to allocate pool at invalid IRQL
        Parameter 2 - Current IRQL
        Parameter 3 - Pool type
        Parameter 4 - Size of allocation

    0x9 :    Attempt to free pool at invalid IRQL
        Parameter 2 - Current IRQL
        Parameter 3 - Pool type
        Parameter 4 - Address of pool

    0xA :    Attempt to free some other component's protected pool.
        Parameter 2 - Address of pool
        Parameter 3 - Allocator's tag
        Parameter 4 - Tag being used in the attempted free.

    0xB/0xC/0xD :    Attempt to release quota on a corrupted pool allocation.
        Parameter 2 - Address of pool
        Parameter 3 - Pool allocation's tag
        Parameter 4 - Quota process pointer (bad).

    0x40 :    Attempt to free usermode address to kernel pool
        Parameter 2 - Starting address
        Parameter 3 - Start of system address space
        Parameter 4 - 0

    0x41 :    Attempt to free a non-allocated nonpaged pool address
        Parameter 2 - Starting address
        Parameter 3 - physical page frame
        Parameter 4 - highest physical page frame

    0x42 :    Attempt to free a virtual address which was never in any pool
        Parameter 2 - Address being freed.
        Parameter 3 - 0
        Parameter 4 - 0

    0x43 :    Attempt to free a virtual address which was never in any pool
        Parameter 2 - Address being freed.
        Parameter 3 - 0
        Parameter 4 - 0

    0x50 :    Attempt to free a non-allocated paged pool address
        Parameter 2 - Starting address
        Parameter 3 - Start offset in pages from beginning of paged pool
        Parameter 4 - Size in bytes of paged pool

    0x60 :    Attempt to free an invalid contiguous memory address (ie: caller of MmFreeContiguousMemory is passing a bad pointer).
        Parameter 2 - Starting address
        Parameter 3 - 0
        Parameter 4 - 0

    0x99 :    Attempt to free pool with invalid address  (or corruption in pool header)
        Parameter 2 - Address being freed
        Parameter 3 - 0
        Parameter 4 - 0

    0x9A :    Attempt to allocate must-succeed pool (this pool type has been deprecated)
        Parameter 2 - Pool type
        Parameter 3 - Size of allocation in bytes
        Parameter 4 - Allocation's pool tag

    0x9B :    Attempt to allocate pool with a tag of zero.  This would make the pool untrackable and worse, corrupt the existing tag tables.
        Parameter 2 - Pool type
        Parameter 3 - Size of allocation in bytes
        Parameter 4 - Caller's address.

    0x9C :    Attempt to allocate pool with a tag of BIG.  This would make the pool untrackable and worse, corrupt the existing tag tables.
        Parameter 2 - Pool type
        Parameter 3 - Size of allocation in bytes
        Parameter 4 - Caller's address.

    END_VALUES

DRIVER_VERIFIER_DETECTED_VIOLATION         (0xC4)

A device driver attempting to corrupt the system has been caught.  This is
because the driver was specified in the registry as being suspect (by the
administrator) and the kernel has enabled substantial checking of this driver.
If the driver attempts to corrupt the system, bugchecks 0xC4, 0xC1 and 0xA will
be among the most commonly seen crashes.

PARAMETERS
    1 - subclass of driver violation.
        VALUES
        0x00 : caller is trying to allocate zero bytes
           2 - current IRQL
           3 - pool type
           4 - number of bytes

        0x01 : caller is trying to allocate paged pool at DISPATCH_LEVEL or above
            2 - current IRQL
            3 - pool type
            4 - number of bytes

        0x02 : caller is trying to allocate nonpaged pool at an IRQL above DISPATCH_LEVEL
            2 - current IRQL
            3 - pool type
            4 - number of bytes

        0x03 : caller is trying to allocate more than one page of mustsucceed
               pool, but one page is the maximum allowed by this API.

        0x10 : caller is freeing a bad pool address
            2 - bad pool address

        0x11 : caller is trying to free paged pool at DISPATCH_LEVEL or above
            2 - current IRQL
            3 - pool type
            4 - pool address

        0x12 : caller is trying to free nonpaged pool at an IRQL above DISPATCH_LEVEL
            2 - current IRQL
            3 - pool type
            4 - pool address

        0x13 : the pool the caller is trying to free is already free.
            2 - (reserved)
%           2 - line number
            3 - pool header
            4 - pool header contents

        0x14 : the pool the caller is trying to free is already free.
            2 - (reserved)
%           2 - line number
            3 - pool header
            4 - pool header contents

        0x15 : the pool the caller is trying to free contains an active timer.
            2 - timer entry
            3 - pool type
            4 - pool address being freed

        0x16 : the pool the caller is trying to free is a bad address.
            2 - line number
            3 - pool address
            4 - 0

        0x17 : the pool the caller is trying to free contains an active ERESOURCE.
            2 - resource entry
            3 - pool type
            4 - pool address being freed

        0x30 : raising IRQL to an invalid level,
            2 -  current IRQL,
            3 -  new IRQL
        0x31 : lowering IRQL to an invalid level,
            2 -  current IRQL,
            3 -  new IRQL
            4 -  0 means the new IRQL is bad, 1 means the IRQL is invalid inside a DPC routine

        0x32 : releasing a spinlock when not at DISPATCH_LEVEL.
            2 -  current IRQL,
            3 -  spinlock address

        0x33 : acquiring a fast mutex when not at APC_LEVEL or below.
            2 -  current IRQL,
            3 -  fast mutex address

        0x34 : releasing a fast mutex when not at APC_LEVEL.
            2 -  current IRQL,
            3 -  thread APC disable count
            4 -  fast mutex address

        0x35 : kernel is releasing a spinlock when not at DISPATCH_LEVEL.
            2 -  current IRQL,
            3 -  spinlock address
            4 -  old irql.

        0x36 : kernel is releasing a queued spinlock when not at DISPATCH_LEVEL.
            2 -  current IRQL,
            3 -  spinlock number,
            4 -  old irql.

        0x37 : a resource is being acquired but APCs are not disabled.
            2 -  current IRQL,
            3 -  thread APC disable count,
            4 -  resource.

        0x38 : a resource is being released but APCs are not disabled.
            2 -  current IRQL,
            3 -  thread APC disable count,
            4 -  resource.

        0x39 : a mutex is being acquired unsafe, but irql is not APC_LEVEL on entry.
            2 -  current IRQL,
            3 -  thread APC disable count,
            4 -  mutex.

        0x3A : a mutex is being released unsafe, but irql is not APC_LEVEL on entry.
            2 -  current IRQL,
            3 -  thread APC disable count,
            4 -  mutex.

        0x3B : KeWaitXxx routine is being called at DISPATCH_LEVEL or higher.
            2 -  current irql,
            3 -  object to wait on,
            4 -  time out parameter.

        0x3C : ObReferenceObjectByHandle is being called with a bad handle.
            2 -  bad handle passed in,
            3 -  object type,
            4 -  0.

        0x3D : ExAcquireResourceExclusive is being called with a bad (unaligned) resource.
            2 -  0
            3 -  0
            4 -  bad resource address passed in.

        0x3E : KeLeaveCriticalRegion is being called for a thread that never entered a critical region.
            2 -  0
            3 -  0
            4 -  0

        0x3F : ObReferenceObject is being called on an object that has a reference count of zero or
               ObDereferenceObject is being called on an object with a reference count of zero
            2 -  Object address
            3 -  new object reference count -1 = dereference case, 1 = reference case
            4 -  0

        0x40 : acquiring a spinlock when not at DISPATCH_LEVEL.
            2 -  current IRQL,
            3 -  spinlock address

        0x41 : releasing a spinlock when not at DISPATCH_LEVEL.
            2 -  current IRQL,
            3 -  spinlock address

        0x42 : acquiring a spinlock when caller is already above DISPATCH_LEVEL.
            2 -  current IRQL,
            3 -  spinlock address

        0x51 : freeing memory where the caller has written past the end of the
               allocation, overwriting our stored bytecount.
            2 -  base address of the allocation,
            3 -  corrupt address,
            4 -  charged bytes.

        0x52 : freeing memory where the caller has written past the end of the
               allocation, overwriting our stored virtual address.
            2 -  base address of the allocation,
            3 -  hash entry,
            4 -  charged bytes.

        0x53 : freeing memory where the caller has written past the end of the
               allocation, overwriting our stored virtual address.
            2 -  base address of the allocation,
            3 -  header,
            4 -  (reserved)
%           4 -  internal verifier pointer.

        0x54 : freeing memory where the caller has written past the end of the
               allocation, overwriting our stored virtual address.
            2 -  base address of the allocation,
            3 -  pool hash size,
            4 -  listindex.

        0x59 : freeing memory where the caller has written past the end of the
               allocation, overwriting our stored virtual address.
            2 -  base address of the allocation,
            3 -  listindex,
            4 -  (reserved)
%           4 -  internal verifier pointer.

        0x60 : A driver has forgotten to free its pool allocations prior to unloading.
            2 -  paged bytes
            3 -  nonpaged bytes,
            4 -  total # of (paged+nonpaged) allocations that weren't freed.

            To get the name of the driver at fault, type
                dp ViBadDriver l1; dS @$p

            Then type !verifier 3 drivername.sys for info on the allocations
            that were leaked that caused the bugcheck.

        0x61 : A driver is unloading and allocating memory (in another thread) at the same time.
            2 -  paged bytes
            3 -  nonpaged bytes,
            4 -  total # of (paged+nonpaged) allocations that weren't freed.

            To get the name of the driver at fault, type
                dp ViBadDriver l1; dS @$p

            Then type !verifier 3 drivername.sys for info on the allocations
            that were leaked that caused the bugcheck.

        0x6F : MmProbeAndLockPages called on pages not in PFN database.  This
               is typically a driver calling this routine to lock its own
               private dualport RAM.  Not only is this not needed, it can also
               corrupt memory on machines with noncontiguous physical RAM.
               2 -  MDL address
               3 -  physical page being locked
               4 -  highest physical page in the system

        0x70 : MmProbeAndLockPages called when not at DISPATCH_LEVEL or below.
               2 -  current IRQL
               3 -  MDL address
               4 -  access mode

        0x71 : MmProbeAndLockProcessPages called when not at DISPATCH_LEVEL or below.
               2 -  current IRQL
               3 -  MDL address
               4 -  process address

        0x72 : MmProbeAndLockSelectedPages called when not at DISPATCH_LEVEL or below.
               2 -  current IRQL
               3 -  MDL address
               4 -  process address

        0x73 : MmMapIoSpace called when not at DISPATCH_LEVEL or below.
               2 -  current IRQL
               3 -  low 32 bits of the physical address (full 64 on Win64)
               4 -  number of bytes

        0x74 : MmMapLockedPages called when not at DISPATCH_LEVEL or below.
               2 -  current IRQL
               3 -  MDL address
               4 -  access mode

        0x75 : MmMapLockedPages called when not at APC_LEVEL or below.
               2 -  current IRQL
               3 -  MDL address
               4 -  access mode

        0x76 : MmMapLockedPagesSpecifyCache called when not at DISPATCH_LEVEL or below.
               2 -  current IRQL
               3 -  MDL address
               4 -  access mode

        0x77 : MmMapLockedPagesSpecifyCache called when not at APC_LEVEL or below.
               2 -  current IRQL
               3 -  MDL address
               4 -  access mode

        0x78 : MmUnlockPages called when not at DISPATCH_LEVEL or below.
               2 -  current IRQL
               3 -  MDL address
               4 -  0

        0x79 : MmUnmapLockedPages called when not at DISPATCH_LEVEL or below.
               2 -  current IRQL
               3 -  virtual address being unmapped
               4 -  MDL address

        0x7A : MmUnmapLockedPages called when not at APC_LEVEL or below.
               2 -  current IRQL
               3 -  virtual address being unmapped
               4 -  MDL address

        0x7B : MmUnmapIoSpace called when not at APC_LEVEL or below.
               2 -  current IRQL
               3 -  virtual address being unmapped
               4 -  number of bytes

        0x7C : MmUnlockPages called with an MDL whose pages were never
               successfully locked.
               2 -  MDL address
               3 -  MDL flags
               4 -  0

        0x7D : MmUnlockPages called with an MDL whose pages are from nonpaged
               pool - these should never be unlocked.
               2 -  MDL address
               3 -  MDL flags
               4 -  0

        0x80 : KeSetEvent called when not at DISPATCH_LEVEL or below.
               2 -  current IRQL
               3 -  event address
               4 -  0

        0x81 : MmMapLockedPages called without MDL_MAPPING_CAN_FAIL
               2 -  MDL address
               3 -  MDL flags
               4 -  0

        0x82 : MmMapLockedPagesSpecifyCache called without MDL_MAPPING_CAN_FAIL
               2 -  MDL address
               3 -  MDL flags
               4 -  Whether to bugcheck on failure

        0x83 : MmMapIoSpace called to map, but the caller hasn't locked down the MDL pages.
               2 -  Starting physical address to map.
               3 -  Number of bytes to map.
               4 -  The first page frame number that isn't locked down.

        0x84 : MmMapIoSpace called to map, but the caller hasn't locked down the MDL pages.
               2 -  Starting physical address to map.
               3 -  Number of bytes to map.
               4 -  The first page frame number that is on the free list.

        0x85 : MmMapLockedPages called to map, but the caller hasn't locked down the MDL pages.
               2 -  MDL address.
               3 -  Number of pages to map.
               4 -  The first page frame number that isn't locked down.

        0x86 : MmMapLockedPages called to map, but the caller hasn't locked down the MDL pages.
               2 -  MDL address.
               3 -  Number of pages to map.
               4 -  The first page frame number that is on the free list.

        0x87 : MmMapIoSpace called to map, but the caller's cachetype conflicts with an already existing mapping.
               2 -  Base physical page (shift left for physical address) of the existing (conflicting) mapping.
               3 -  Number of pages already mapped in the existing (conflicting) mapping.
               4 -  MEMORY_CACHING_TYPE of the existing (conflicting mapping).

        0x88 : MmMapIoSpace called to map a physical range as noncached or writecombined, but the caller's physical range already has an existing cached mapping.
               2 -  Base physical page (shift left for physical address) of the requested mapping.
               3 -  Number of pages in the requested mapping.
               4 -  MEMORY_CACHING_TYPE of the requested mapping.

        0x89 : An MDL is not marked as I/O but contains non-memory page addresses.
               2 -  MDL address.
               3 -  Pointer to the non-memory page in the MDL.
               4 -  The non-memory page number in the MDL.

        0x8A : MmMapLockedPages* called to map a physical range as noncached or writecombined, but the caller's physical range already has an existing cached mapping.
               2 -  MDL address.
               3 -  Conflicting physical page (shift left for physical address) of the requested mapping.
               4 -  MEMORY_CACHING_TYPE of the requested mapping.

        0x90 : A driver switched stacks. The current stack is neither a thread
               stack nor a DPC stack. Typically the driver doing this should be
               on the stack obtained from `kb' command.

        0xA0 : A CRC error was detected on the sector (synchronously).
               2 -  Request Irp.
               3 -  Device object of the lower device.
               4 -  Sector number on which the CRC error was detected.

        0xA1 : A CRC error was detected on the sector (asynchronously).
               2 -  *COPY* of original Irp (actual irp has been completed)
               3 -  Device object of the lower device.
               4 -  Sector number on which the CRC error was detected.

        0xA2 : CRCDISK checksum copies don't match (possible paging error)
               2 -  Request Irp or copy.
               3 -  Device object of the lower device.
               4 -  Sector number on which the CRC error was detected.

        END_VALUES

DESCRIPTION
        Parameter 1 = 0x1000 .. 0x1020 - deadlock verifier error codes.
               Typically the code is 0x1001 (deadlock detected) and you can
               issue a '!deadlock' KD command to get more information.
               % Please consult also the FAQ about the deadlock verifier
               % on http://stressweb/dbginfo.htm, or see the DDK docs.

DRIVER_CORRUPTED_EXPOOL           (0xC5)
PARAMETERS
        1 - memory referenced
        2 - IRQL
        3 - value 0 = read operation, 1 = write operation
        4 - address which referenced memory

DESCRIPTION
An attempt was made to access a pagable (or completely invalid) address at an
interrupt request level (IRQL) that is too high.  This is
caused by drivers that have corrupted the system pool.  Run the driver
verifier against any new (or suspect) drivers, and if that doesn't turn up
the culprit, then use gflags to enable special pool.

DRIVER_CAUGHT_MODIFYING_FREED_POOL           (0xC6)
PARAMETERS
        1 - memory referenced
        2 - value 0 = read operation, 1 = write operation
        3 - previous mode.
        4 - 4.

DESCRIPTION
An attempt was made to access freed pool memory.  The faulty component is
displayed in the current kernel stack.

TIMER_OR_DPC_INVALID                                    (0xC7)

A kernel timer or DPC was found in memory which must not contain such items.
Usually this is memory being freed.  This is usually caused by a device driver
that has not cleaned up properly before freeing memory.

PARAMETERS
        1 - What kind of object
            0   Timer Object
            1   DPC Object
            2   DPC Routine
        2 - Address of object
        3 - Start of range being checked
        4 - End of range being checked

IRQL_UNEXPECTED_VALUE                                   (0xC8)

The processor's IRQL is not what it should be at this time.  This is
usually caused by a lower level routine changing IRQL for some period
and not restoring IRQL at the end of that period (eg acquires spinlock
but doesn't release it).

PARAMETERS
        1 - (Current IRQL << 16) | (Expected IRQL << 8) | UniqueValue
DESCRIPTION
        if UniqueValue is 0 or 1
            2 = APC->KernelRoutine
            3 = APC
            4 = APC->NormalRoutine

DRIVER_VERIFIER_IOMANAGER_VIOLATION                     (0xC9)
The IO manager has caught a misbehaving driver.
PARAMETERS
        1 - Code that specifies the violation
            VALUES:
            1 : Invalid IRP passed to IoFreeIrp
                2 - the IRP passed in , 3/4 - 0
            2 : IRP still associated with a thread at IoFreeIrp
                2 - the IRP passed in , 3/4 - 0
            3 : Invalid IRP passed to IoCallDriver
                2 - the IRP passed in , 3/4 - 0
            4 : Invalid Device object passed to IoCallDriver
                2 - the Device object , 3/4 - 0
            5 : Irql not equal across call to the driver dispatch routine
                2 - the device object associated with the offending driver
                3 - the Irql before the call
                4 - the Irql after the call
            6 : IRP passed to IoCompleteRequest contains invalid status
                2 - the status
                3 - the IRP
                4 - 0
            7 : IRP passed to IoCompleteRequest still has cancel routine set
                2 - the cancel routine pointer
                3 - the IRP
                4 - 0
            8 : Call to IoBuildAsynchronousFsdRequest threw an exception
                2 - the Device object
                3 - the IRP major function
                4 - the exception status
            9 : Call to IoBuildDeviceIoControlRequest threw an exception
                2 - the Device object
                3 - the IoControlCode
                4 - the exception status
          0xA : Reinitialization of Device object timer
                2 - the Device object , 3/4 - 0
          0xB : Unused
          0xC : Invalid IOSB in IRP at APC IopCompleteRequest (appears to be on
                stack that was unwound)
                2 - the IOSB pointer , 3/4 - 0
          0xD : Invalid UserEvent in IRP at APC IopCompleteRequest (appears to
                be on stack that was unwound)
                2 - the UserEvent pointer , 3/4 - 0
          0xE : Irql > DPC at IoCompleteRequest
                2 - the current Irql
                3 - the IRP
                4 - 0
          0xF : Driver sent an create request with fileobject that's been closed or had its open cancelled.
                1 - Device Object to which the IRP is being sent
                2 - Pointer to IRP
                3 - Pointer to FileObject

%       The following table lists additional I/O Verification errors that can appear in
%       Windows 2000 and later.  In Windows 2000, these errors will only be displayed
%       if Level 2 I/O Verification is activated.

        0x200 : (Unknown) This code covers all unknown I/O Verification errors.
                2 - The address in the driver's code where the error was detected.
                3 - A pointer to the IRP.
                4 - A pointer to the DeviceObject.
        0x201 : (Fatal error) A device is deleting itself while there is another device
                beneath it in the driver stack. This may be because the caller has forgotten to call
                 IoDetachDevice first, or the lower driver may have incorrectly deleted itself.

        0x202 : (Fatal error) A driver has attempted to detach from a device object that
                is not attached to anything. This may occur if detach was called twice on the same
                 device object. (Device object specified.)

        0x203 : (Fatal error) A driver has called IoCallDriver without setting the
                CancelRoutine in the IRP to NULL. (IRP specified.)

        0x204 : (Fatal error) The caller has passed in NULL as a DeviceObject. This
                is fatal. (IRP specified.)

        0x205 : (Fatal error) The caller is forwarding an IRP that is currently queued beneath
                it. The code handling IRPs returning STATUS_PENDING in this driver appears to be
                 broken. (IRP specified.)

        0x206 : (Fatal error) The caller has incorrectly forwarded an IRP (control field
                not zeroed). The driver should use IoCopyCurrentIrpStackLocationToNext or
                IoSkipCurrentIrpStackLocation. (IRP specified.)

        0x207 : (Fatal error) The caller has manually copied the stack and has inadvertently
                 copied the upper layer's completion routine. Please use
                 IoCopyCurrentIrpStackLocationToNext. (IRP specified.)

        0x208 : (Fatal error) This IRP is about to run out of stack locations. Someone
                may have forwarded this IRP from another stack. (IRP specified.)

        0x209 : (Fatal error) The caller is completing an IRP that is currently queued
                 beneath it. The code handling IRPs returning STATUS_PENDING in this driver appears
                  to be broken. (IRP specified.)

        0x20A : (Fatal error) The caller of IoFreeIrp is freeing an IRP that is still
                in use. (Original IRP and IRP in use specified.)

        0x20B : (Fatal error) The caller of IoFreeIrp is freeing an IRP that is still
                in use. (IRP specified.)

        0x20C : (Fatal error) The caller of IoFreeIrp is freeing an IRP that is still
                queued against a thread. (IRP specified.)

        0x20D : (Fatal error) The caller of IoInitializeIrp has passed an IRP that was
                allocated with IoAllocateIrp. This is illegal and unnecessary, and has caused
                a quota leak. Check the documentation for IoReuseIrp if this IRP is being recycled.

        0x20E : (Non-fatal error) A PNP IRP has an invalid status. (Any PNP IRP must
                have its status initialized to STATUS_NOT_SUPPORTED.) (IRP specified.)

        0x20F : (Non-fatal error) A Power IRP has an invalid status. (Any Power IRP
                must have its status initialized to STATUS_NOT_SUPPORTED.) (IRP specified.)

        0x210 : (Non-fatal error) A WMI IRP has an invalid status. (Any WMI IRP must
                have its status initialized to STATUS_NOT_SUPPORTED.) (IRP specified.)

        0x211 : (Non-fatal error) The caller has forwarded an IRP while skipping a device
                 object in the stack. The caller is probably sending IRPs to the PDO instead of
                  to the device returned by IoAttachDeviceToDeviceStack. (IRP specified.)

        0x212 : (Non-fatal error) The caller has trashed or has not properly copied the
                IRP's stack. (IRP specified.)

        0x213 : (Non-fatal error) The caller has changed the status field of an IRP it
                does not understand. (IRP specified.)

        0x214 : (Non-fatal error) The caller has changed the information field of an
                IRP it does not understand. (IRP specified.)

        0x215 : (Non-fatal error) A non-successful non-STATUS_NOT_SUPPORTED IRP status
                for IRP_MJ_PNP is being passed down stack. (IRP specified.) Failed PNP IRPs
                must be completed.

        0x216 : (Non-fatal error) The previously-set IRP_MJ_PNP status has been converted
                to STATUS_NOT_SUPPORTED. (IRP specified.)

        0x217 : (Non-fatal error) The driver has not handled a required IRP. The driver must
                update the status of the IRP to indicate whether or not it has been handled.
                (IRP specified.)

        0x218 : (Non-fatal error) The driver has responded to an IRP that is reserved for
                other device objects elsewhere in the stack. (IRP specified.)

        0x219 : (Non-fatal error) A non-successful non-STATUS_NOT_SUPPORTED IRP status for
                IRP_MJ_POWER is being passed down stack. (IRP specified.) Failed POWER IRPs must be
                completed.

        0x21A : (Non-fatal error) The previously-set IRP_MJ_POWER status has been converted
                to STATUS_NOT_SUPPORTED. (IRP specified.)

        0x21B : (Non-fatal error) A driver has returned a suspicious status. This is probably
                 due to an uninitialized variable bug in the driver. (IRP specified.)

        0x21C : (Warning) The caller has copied the IRP stack but not set a completion routine.
                This is inefficient -- use IoSkipCurrentIrpStackLocation instead. (IRP specified.)

        0x21D : (Fatal error) An IRP dispatch handler has not properly detached from the
                stack below it upon receiving a remove IRP. (DeviceObject, Dispatch
                 Routine, and IRP specified.)

        0x21E : (Fatal error) An IRP dispatch handler has not properly deleted its device object
                upon receiving a remove IRP. (DeviceObject, Dispatch Routine, and IRP specified.)

        0x21F : (Non-fatal error) A driver has not filled out a dispatch routine for a required
                IRP major function. (IRP specified.)

        0x220 : (Non-fatal error) IRP_MJ_SYSTEM_CONTROL has been completed by someone other than
                the ProviderId. This IRP should either have been completed earlier or should have
                been passed down. (IRP specified, along with the DeviceObject where it was targeted.)

        0x221 : (Fatal error) An IRP dispatch handler for a PDO has deleted its device object,
                but the hardware has not been reported as missing in a bus relations query.
                (DeviceObject, Dispatch Routine, and IRP specified.)

        0x222 : (Fatal error) A Bus Filter's IRP dispatch handler has detached upon receiving
                a remove IRP when the PDO is still alive. Bus Filters must clean up in
                FastIoDetach callbacks. (DeviceObject, Dispatch Routine, and IRP specified.)

        0x223 : (Fatal error) An IRP dispatch handler for a bus filter has deleted its device
                object, but the PDO is still present. Bus filters must clean up in
                FastIoDetach callbacks. (DeviceObject, Dispatch Routine, and IRP specified.)

        0x224 : (Fatal error) An IRP dispatch handler has returned a status that is inconsistent
                 with the IRP's IoStatus.Status field. (Dispatch handler routine, IRP, IRP's
                 IoStatus.Status, and returned Status specified.)

        0x225 : (Non-fatal error) An IRP dispatch handler has returned a status that is illegal
                (0xFFFFFFFF). This is probably due to an uninitialized stack variable. To debug
                this, execute an LN command on the specified address.

        0x226 : (Fatal error) An IRP dispatch handler has returned without passing down or
                completing this IRP, or someone forgot to return STATUS_PENDING. (IRP specified.)

        0x227 : (Fatal error) An IRP completion routine is in pageable code. (This is never
                permitted.) (Routine and IRP specified.)

        0x228 : (Non-fatal error) A driver's completion routine has not marked the IRP pending if
                the PendingReturned field was set in the IRP passed to it. This may cause the
                OS to hang, especially if an error is returned by the stack. (Routine and IRP specified.)

        0x229 : (Fatal error) A cancel routine has been set for an IRP that is currently being
                processed by drivers lower in the stack, possibly stomping their cancel routine.
                (Routine and IRP specified.)

        0x22A : (Non-fatal error) The physical device object (PDO) has not responded to a
                required IRP. (IRP specified.)

        0x22B : (Non-fatal error) The physical device object (PDO) has forgotten to fill out the
                device relation list with the PDO for the TargetDeviceRelation query. (IRP specified.)

        0x22C : (Fatal error) The code implementing the TargetDeviceRelation query has not called
                ObReferenceObject on the PDO. (IRP specified.)

        0x22D : (Non-fatal error) The caller has completed a IRP_MJ_PNP it didn't understand
                instead of passing it down. (IRP specified.)

        0x22E : (Non-fatal error) The caller has completed a successful IRP_MJ_PNP instead of
                 passing it down. (IRP specified.)

        0x22F : (Non-fatal error) The caller has completed an untouched IRP_MJ_PNP (instead of
                passing the IRP down), or non-PDO has failed the IRP using illegal value of
                STATUS_NOT_SUPPORTED. (IRP specified.)

        0x230 : (Non-fatal error) The caller has completed an IRP_MJ_POWER it didn't understand
                instead of passing it down. (IRP specified.)

        0x231 : (Fatal error) The caller has completed a successful IRP_MJ_POWER instead of passing
                 it down. (IRP specified.)

        0x232 : (Non-fatal error) The caller has completed an untouched IRP_MJ_POWER (instead of
                passing the IRP down), or non-PDO has failed the IRP using illegal value of
                STATUS_NOT_SUPPORTED. (IRP specified.)

        0x233 : (Non-fatal error) The version field of the query capabilities structure in a query
                capabilities IRP was not properly initialized. (IRP specified.)

        0x234 : (Non-fatal error) The size field of the query capabilities structure in a query
                capabilities IRP was not properly initialized. (IRP specified.)

        0x235 : (Non-fatal error) The address field of the query capabilities structure in a query
                capabilities IRP was not properly initialized to -1. (IRP specified.)

        0x236 : (Non-fatal error) The UI Number field of the query capabilities structure in a
                 query capabilities IRP was not properly initialized to -1. (IRP specified.)

        0x237 : (Fatal error) A driver has sent an IRP that is restricted for system use only. (IRP specified.)

        0x238 : (Warning) The caller of IoInitializeIrp has passed an IRP that was allocated with
                IoAllocateIrp. This is illegal, unnecessary, and negatively impacts performance in normal
                use. Check the documentation for IoReuseIrp if this IRP is being recycled.

        0x239 : (Warning) The caller of IoCompleteRequest is completing an IRP that has never
                been forwarded via a call to IoCallDriver or PoCallDriver. This may be
                a bug. (IRP specified.)

        0x23A : (Fatal error) A driver has forwarded an IRP at an IRQL that is illegal for
                this major code. (IRP specified.)

        0x23B : (Non-fatal error) The caller has changed the status field of an IRP it does not
                understand. (IRP specified.)

%       The following table lists additional I/O Verification errors that can appear in
%       Windows XP and later. Some of these errors will only be revealed if Enhanced I/O
%       Verification is activated.

        0x23C : (Fatal error) A driver has completed an IRP without setting the cancel
                routine in the IRP to NULL. (IRP specified.)

        0x23D : (Non-fatal error) A driver has returned STATUS_PENDING but did not mark the IRP
                pending via a call to IoMarkIrpPending. (IRP specified.)

        0x23E : (Non-fatal error) A driver has marked an IRP pending but didn't return
                STATUS_PENDING. (IRP specified.)

        0x23F : (Fatal error) A driver has not inherited the DO_POWER_PAGABLE bit from the
                stack it has attached to. (DeviceObject specified.)

        0x240 : (Fatal error) A driver is attempting to delete a device object that has already
                been deleted via a prior call to IoDeleteDevice.

        0x241 : (Fatal error) A driver has detached its device object during a surprise remove
                IRP. (IRP and DeviceObject specified.)

        0x242 : (Fatal error) A driver has deleted its device object during a surprise remove
                 IRP. (IRP and DeviceObject specified.)

        0x243 : (Fatal error) A driver has failed to clear the DO_DEVICE_INITIALIZING flag at
                the end of AddDevice. (DeviceObject specified.)

        0x244 : (Fatal error) A driver has not copied either the DO_BUFFERED_IO or the
                DO_DIRECT_IO flag from the device object it is attaching
                to. (DeviceObject specified.)

        0x245 : (Fatal error) A driver has set both the DO_BUFFERED_IO and the DO_DIRECT_IO
                flags. These flags are mutually exclusive. (DeviceObject specified.)

        0x246 : (Fatal error) A driver has failed to copy the DeviceType field from the device
                object it is attaching to. (DeviceObject specified.)

        0x247 : (Fatal error) A driver has failed an IRP that cannot legally be failed. (IRP specified.)

        0x248 : (Fatal error) A driver has added a device object that is not a PDO to a device
                relations query. (IRP and DeviceObject specified.)

        0x249 : (Non-fatal error) A driver has enumerated two child PDOs that returned identical
                Device IDs. (Both DeviceObjects specified.)

        0x24A : (Fatal error) A driver has mistakenly called a file I/O function at an IRQL
                other than PASSIVE_LEVEL.

        0x24B : (Fatal error) A driver has succeeded IRP_MJ_PNP.IRP_MN_QUERY_DEVICE_RELATIONS(TargetRelations)
                but did not properly fill out the request or forward the IRP to the underlying
                hardware stack.  (DeviceObject specified.)

        0x24C : (Non-fatal error) A driver has returned STATUS_PENDING but did not mark the
                IRP pending via a call to IoMarkIrpPending.  (IRP specified.)

        0x24D : (Fatal error) A driver has passed an invalid device object to a function that
                requires a PDO.  (DeviceObject specified.)

           END_VALUES

PNP_DETECTED_FATAL_ERROR                    (0xCA)

PnP encountered a severe error, either as a result of a problem in a driver or
a problem in PnP itself.  The first argument describes the nature of the
problem, the second argument is the address of the PDO.  The other arguments
vary depending on argument 1.

PARAMETERS
        Argument 1 -
        VALUES:
        1 : Duplicate PDO

                A specific instance of a driver has enumerated multiple PDOs with
                identical device id and unique ids.

                2 - Newly reported PDO.

                3 - PDO of which it is a duplicate.

        2 : Invalid PDO

                An API which requires a PDO has been called with either an FDO,
                a PDO which hasn't been initialized yet (returned to PnP in a
                QueryDeviceRelation/BusRelations), or some random piece of
                memory.

                2 - Purported PDO.

                3 - Driver object

        3 : Invalid ID

                An enumerator has returned an ID which contains illegal
                characters or isn't properly terminated.  IDs must only contain
                characters in the range 0x20-7F inclusive with the exception of
                0x2C (comma) which is illegal.

                2 - PDO whose IDs were queried

                3 - Address of ID buffer

                4 - Type of ID
                VALUES
                        1 : DeviceID
                        2 : UniqueID
                        3 : HardwareIDs
                        4 : CompatibleIDs
                END_VALUES

        4 : Invalid enumeration of deleted PDO

                An enumerator has returned a PDO which it has previously deleted
                using IoDeleteDevice.

                2 - PDO with DOE_DELETE_PENDING set.

        5 : PDO freed while still linked in devnode tree.

                The object manager reference count on a PDO dropped to zero
                while the devnode was still linked in the tree.  This usually
                indicates that the driver is not adding a reference when
                returning the PDO in a query IRP.

                2 - PDO.
        END_VALUES



DRIVER_LEFT_LOCKED_PAGES_IN_PROCESS     (0xCB)
PARAMETERS
        1 - The calling address in the driver that locked the pages or if the
            IO manager locked the pages this points to the dispatch routine of
            the top driver on the stack to which the IRP was sent.
        2 - The caller of the calling address in the driver that locked the
            pages. If the IO manager locked the pages this points to the device
            object of the top driver on the stack to which the IRP was sent.
        3 - A pointer to the MDL containing the locked pages.
        4 - The number of locked pages.

DESCRIPTION
Caused by a driver not cleaning up completely after an I/O.

When possible, the guilty driver's name (Unicode string) is printed on
the bugcheck screen and saved in KiBugCheckDriver.

PAGE_FAULT_IN_FREED_SPECIAL_POOL        (0xCC)
PARAMETERS
        1 - memory referenced
        2 - value 0 = read operation, 1 = write operation
        3 - if non-zero, the address which referenced memory.
        4 - Mm internal code.

DESCRIPTION
Memory was referenced after it was freed.
This cannot be protected by try-except.

When possible, the guilty driver's name (Unicode string) is printed on
the bugcheck screen and saved in KiBugCheckDriver.

PAGE_FAULT_BEYOND_END_OF_ALLOCATION     (0xCD)
PARAMETERS
        1 - memory referenced
        2 - value 0 = read operation, 1 = write operation
        3 - if non-zero, the address which referenced memory.
        4 - Mm internal code.

DESCRIPTION
N bytes of memory was allocated and more than N bytes are being referenced.
This cannot be protected by try-except.

When possible, the guilty driver's name (Unicode string) is printed on
the bugcheck screen and saved in KiBugCheckDriver.

DRIVER_UNLOADED_WITHOUT_CANCELLING_PENDING_OPERATIONS      (0xCE)

A driver unloaded without cancelling timers, DPCs, worker threads, etc.
The broken driver's name is displayed on the screen.
PARAMETERS
        1 - memory referenced
        2 - value 0 = read operation, 1 = write operation
        3 - If non-zero, the instruction address which referenced the bad memory
            address.
        4 - Mm internal code.

TERMINAL_SERVER_DRIVER_MADE_INCORRECT_MEMORY_REFERENCE     (0xCF)
PARAMETERS
        1 - memory referenced
        2 - value 0 = read operation, 1 = write operation
        3 - If non-zero, the instruction address which referenced the bad memory
            address.
        4 - Mm internal code.

A driver has been incorrectly ported to Terminal Server.  It is referencing
session space addresses from the system process context.  Probably from
queueing an item to a system worker thread.
The broken driver's name is displayed on the screen.

DRIVER_CORRUPTED_MMPOOL     (0xD0)
PARAMETERS
        1 - memory referenced
        2 - IRQL
        3 - value 0 = read operation, 1 = write operation
        4 - address which referenced memory

An attempt was made to access a pagable (or completely invalid) address at an
interrupt request level (IRQL) that is too high.  This is
caused by drivers that have corrupted the system pool.  Run the driver
verifier against any new (or suspect) drivers, and if that doesn't turn up
the culprit, then use gflags to enable special pool.  You can also set
HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management\ProtectNonPagedPool
to a DWORD 1 value and reboot.  Then the system will unmap freed nonpaged pool,
preventing drivers (although not DMA-hardware) from corrupting the pool.

DRIVER_IRQL_NOT_LESS_OR_EQUAL           (0xD1)
PARAMETERS
        1 - memory referenced
        2 - IRQL
        3 - value 0 = read operation, 1 = write operation
        4 - address which referenced memory

DESCRIPTION
An attempt was made to access a pagable (or completely invalid) address at an
interrupt request level (IRQL) that is too high.  This is usually
caused by drivers using improper addresses.

If kernel debugger is available get stack backtrace.

BUGCODE_ID_DRIVER                       (0xD2)

This is the NDIS Driver Bugcheck for Windows 2000 and Windows XP.
For Windows Server 2003 and later, see 0x7C, BUGCODE_NDIS_DRIVER.

DESCRIPTION
The meaning of the bug check parameters cannot be determined by examining the
parameters alone.  You must also examine the text of a DbgPrint message.

For details, see either the debugger documentation or the DDK documentation.


DRIVER_PORTION_MUST_BE_NONPAGED         (0xD3)
PARAMETERS
        1 - memory referenced
        2 - IRQL
        3 - value 0 = read operation, 1 = write operation
        4 - address which referenced memory

DESCRIPTION
When possible, the guilty driver's name (Unicode string) is printed on
the bugcheck screen and saved in KiBugCheckDriver.

An attempt was made to access a pagable (or completely invalid) address at an
interrupt request level (IRQL) that is too high.  This is usually
caused by drivers marking code or data as pagable when it should be
marked nonpaged.

If kernel debugger is available get stack backtrace.

SYSTEM_SCAN_AT_RAISED_IRQL_CAUGHT_IMPROPER_DRIVER_UNLOAD    (0xD4)

A driver unloaded without cancelling lookaside lists, DPCs, worker threads, etc.
The broken driver's name is displayed on the screen.

PARAMETERS
        1 - memory referenced
        2 - IRQL
        3 - value 0 = read operation, 1 = write operation
        4 - address which referenced memory

DESCRIPTION
When possible, the guilty driver's name (Unicode string) is printed on
the bugcheck screen and saved in KiBugCheckDriver.

An attempt was made to access the driver at raised IRQL after it unloaded.

If kernel debugger is available get stack backtrace.

DRIVER_PAGE_FAULT_IN_FREED_SPECIAL_POOL        (0xD5)
PARAMETERS
        1 - memory referenced
        2 - value 0 = read operation, 1 = write operation
        3 - if non-zero, the address which referenced memory.
        4 - (reserved)
%       4 - Mm internal code.

DESCRIPTION
Memory was referenced after it was freed.
This cannot be protected by try-except.

When possible, the guilty driver's name (Unicode string) is printed on
the bugcheck screen and saved in KiBugCheckDriver.

DRIVER_PAGE_FAULT_BEYOND_END_OF_ALLOCATION     (0xD6)
PARAMETERS
        1 - memory referenced
        2 - value 0 = read operation, 1 = write operation
        3 - if non-zero, the address which referenced memory.
        4 - (reserved)
%       4 - Mm internal code.

DESCRIPTION
N bytes of memory was allocated and more than N bytes are being referenced.
This cannot be protected by try-except.

When possible, the guilty driver's name (Unicode string) is printed on
the bugcheck screen and saved in KiBugCheckDriver.

DRIVER_UNMAPPING_INVALID_VIEW                  (0xD7)
PARAMETERS
        1 - virtual address to unmap.
        2 - 1 if the view is being unmapped, 2 if the view is being committed.
        3 - 0.
        4 - 0.

DESCRIPTION
A driver (usually win32k.sys, but can be determined from the stack trace for
certain) is trying to unmap an address that was not mapped.

DRIVER_USED_EXCESSIVE_PTES                      (0xD8)
PARAMETERS
        1 - If non-null, the guilty driver's name (Unicode string).
        2 - If parameter 1 non-null, the number of PTEs used by the guilty driver.
        3 - Total free system PTEs
        4 - Total system PTEs

DESCRIPTION
No System PTEs left.  Usually caused by a driver not cleaning up
properly.  If non-null, Parameter 1 shows the name of the driver
who is consuming the most PTEs.  The calling stack also shows the name of
the driver which bugchecked.  Both drivers need to be fixed and/or the number
of PTEs increased.

When possible, the guilty driver's name (Unicode string) is printed on
the bugcheck screen and saved in KiBugCheckDriver.

LOCKED_PAGES_TRACKER_CORRUPTION                 (0xD9)

PARAMETERS
        1 - Type of error.
        VALUES:
        1 : The MDL is being inserted twice on the same process list.

                2 - Address of internal lock tracking structure.
                3 - Address of memory descriptor list.
                4 - Number of pages locked for the current process.

        2 : The MDL is being inserted twice on the systemwide list.

                2 - Address of internal lock tracking structure.
                3 - Address of memory descriptor list.
                4 - Number of pages locked for the current process.

        3 : The MDL was found twice in the process list when being freed.

                Arguments:

                2 - Address of first internal tracking structure found.
                3 - Address of internal lock tracking structure.
                4 - Address of memory descriptor list.


        4 : The MDL was found in the systemwide list on free after it was removed.

                2 - Address of internal lock tracking structure.
                3 - Address of memory descriptor list.
                4 - 0.
        END_VALUES

SYSTEM_PTE_MISUSE                               (0xDA)

The stack trace identifies the guilty driver.

PARAMETERS
        1 - Type of error.
        VALUES:
        1 : The PTE mapping being freed is a duplicate.

                2 - Address of internal lock tracking structure.
                3 - Address of memory descriptor list.
                4 - Address of duplicate internal lock tracking structure.

        2 : The number of PTE mappings being freed is incorrect.

                2 - Address of internal lock tracking structure.
                3 - Number of PTEs the system thinks should be freed.
                4 - Number of PTEs the driver is requesting to free.

        3 : The PTE mapping address being freed is incorrect.

                2 - Address of first internal tracking structure found.
                3 - The PTE address the system thinks should be freed.
                4 - The PTE address the driver is requesting to free.

        4 : The first page of the mapped MDL has changed since the MDL was mapped.

                2 - Address of internal lock tracking structure.
                3 - Page frame number the system thinks should be first in the MDL.
                4 - Page frame number that is currently first in the MDL.

        5 : The start virtual address in the MDL being freed has changed since
        the MDL was mapped.

                2 - Address of first internal tracking structure found.
                3 - The virtual address the system thinks should be freed.
                4 - The virtual address the driver is requesting to free.

        6 : The MDL being freed was never (or is currently not) mapped.

                2 - The MDL specified by the driver.
                3 - The virtual address specified by the driver.
                4 - The number of PTEs to free (specified by the driver).

        7 : The PTE range is being double allocated.

                2 - Starting PTE.
                3 - Number of PTEs.
                4 - Caller Id (system internal).

        8 : The caller is asking to free an incorrect number of PTEs.

                2 - Starting PTE.
                3 - Number of PTEs the caller is freeing.
                4 - Number of PTEs the system thinks should be freed.

        9 : The caller is asking to free PTEs where one of them is not allocated.

                2 - Starting PTE.
                3 - Number of PTEs the caller is freeing.
                4 - PTE index that the system thinks is already free.

        0xA : The caller is asking to allocate 0 PTEs.

                2 - Whether the driver requested "bugcheck on failure" in the MDL.
                3 - Number of PTEs the caller is allocating.
                4 - Type of PTE pool requested.

        0xB : The PTE list is already corrupted at the time of this allocation.
          The corrupt PTE is below the lowest possible PTE address.

                2 - Corrupt PTE.
                3 - Number of PTEs the caller is allocating.
                4 - Type of PTE pool requested.

        0xC : The PTE list is already corrupted at the time of this allocation.
          The corrupt PTE is above the lowest possible PTE address.

                2 - Corrupt PTE.
                3 - Number of PTEs the caller is allocating.
                4 - Type of PTE pool requested.

        0xD : The caller is trying to free 0 PTEs.

                2 - Starting PTE.
                3 - Number of PTEs the caller is freeing.
                4 - Type of PTE pool.

        0xE : The caller is trying to free PTEs and the guard PTE has been overwritten.

                2 - Starting PTE.
                3 - Number of PTEs the caller is freeing.
                4 - Type of PTE pool.

        0xF : The caller is trying to free a bogus PTE.
          The bogus PTE is below the lowest possible PTE address.

                2 - Bogus PTE.
                3 - Number of PTEs the caller is trying to free.
                4 - Type of PTE pool being freed.

        0x10 : The caller is trying to free a bogus PTE.
           The bogus PTE is above the highest possible PTE address.

                2 - Bogus PTE.
                3 - Number of PTEs the caller is trying to free.
                4 - Type of PTE pool being freed.

        0x11 : The caller is trying to free a bogus PTE.
           The bogus PTE is at the base of the PTE address space.

                2 - Bogus PTE.
                3 - Number of PTEs the caller is trying to free.
                4 - Type of PTE pool being freed.
        END_VALUES

DRIVER_CORRUPTED_SYSPTES           (0xDB)
PARAMETERS
        1 - memory referenced
        2 - IRQL
        3 - value 0 = read operation, 1 = write operation
        4 - address which referenced memory

DESCRIPTION
An attempt was made to access a pagable (or completely invalid) address at an
interrupt request level (IRQL) that is too high.  This is
caused by drivers that have corrupted system PTEs.  Set
HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management\TrackPtes
to a DWORD 3 value and reboot.  Then the system will save stack traces and
perform validity checks so the guilty driver can be identified.
There is no other way to find out which driver did this.  When you enable
this flag, if the driver commits the error again you will see a different
bugcheck - SYSTEM_PTE_MISUSE - and the stack trace will identify the offending
driver(s).

DRIVER_INVALID_STACK_ACCESS                   (0xDC)

A driver accessed a stack address that lies below the stack pointer of the
stack's thread.

POOL_CORRUPTION_IN_FILE_AREA                  (0xDE)

A driver corrupted pool memory used for holding pages destined for disk.
This was discovered by the memory manager when dereferencing the file.

IMPERSONATING_WORKER_THREAD                    (0xDF)

A workitem forgot to disable impersonation before it completed.

PARAMETERS
    1 - Worker Routine that caused this bugcheck.
    2 - Parameter passed to this worker routine.
    3 - Pointer to the Workitem.

ACPI_BIOS_FATAL_ERROR                         (0xE0)

Your computer (BIOS) has reported that a component in your system is too faulty
for Windows to be able to run.

You can determine which component is faulty by running the diagnostic disk or
tool that came with your computer.

If you do not have this tool, you must contact your system vendor and report
this error message to them.  They will be able to assist you in correcting this
hardware problem, thereby allowing Windows to operate.  Microsoft cannot assist
in the debugging of this problem -- only the hardware vendor can interpret the
bug check parameters and debug this error.

WORKER_THREAD_RETURNED_AT_BAD_IRQL            (0xE1)
PARAMETERS
        1 - address of worker routine (do ln on this to find guilty driver)
        2 - IRQL returned at (should have been 0, but isn't).
        3 - workitem parameter
        4 - workitem address

MANUALLY_INITIATED_CRASH                      (0xE2)

The user manually initiated this crash dump.

RESOURCE_NOT_OWNED                            (0xE3)

A thread tried to release a resource it did not own.

PARAMETERS
        1 - Address of resource
        2 - Address of thread
        3 - Address of owner table if there is one

WORKER_INVALID                                (0xE4)

A executive worker item was found in memory which must not contain such
items.  Usually this is memory being freed.  This is usually caused by
a device driver that has not cleaned up properly before freeing memory.

PARAMETERS
        1 - Code position indicator
        2 - Address of worker item
        3 - Start of pool block
        4 - End of pool block


DRIVER_VERIFIER_DMA_VIOLATION                     (0xE6)

An illegal DMA operation was attempted by a driver being verified.
PARAMETERS
        1- Violation code.
            VALUES:
            0x03 : Double-freed DMA common buffer.
            0x04 : Double-freed DMA adapter channel.
            0x05 : Double-freed DMA map register.
            0x06 : Double-freed DMA scatter-gather list.
            0x0E : Buffer not locked. DMA transfer has been
                   attempted with a PAGED buffer.
            0x0F : Boundary overrun. Driver or DMA hardware has
                   written outside of its allocation.
            0x18 : Adapter already released. A DMA operation has been
                   attempted using an adapter that no longer exists.
            0x19 : The driver passed a null DMA_ADAPTER value to a
                   HAL routine.
            0x1B : The driver passed an address and an MDL to a HAL
                   routine, but this address is not contained in
                   this MDL.
            0x1D : The driver attempted to map an address that was
                   already mapped.
            END_VALUES


INVALID_FLOATING_POINT_STATE                  (0xE7)

While restoring the previously saved floating point state for a thread,
the state was found to be invalid.  The first argument indicates which
validity check failed.
PARAMETERS
   1 -  indicates which validity check failed.
   VALUES:
        0 : Saved context flags field is invalid, either FLOAT_SAVE_VALID
            is not set or some of the reserved bits are non-zero.  Second
            argument is the flags field.

        1 : The current processor interrupt priority level (IRQL) is not
            the same as when the floating point context was saved.
            Second argument is saved IRQL, third is current IRQL.

        2 : The saved context does not belong to the current thread.
            Second argument is the saved address of the thread this
            floating point context belongs to, third argument is the
            current thread.
    END_VALUES


INVALID_CANCEL_OF_FILE_OPEN                    (0xE8)

The fileobject passed to IoCancelFileOpen is invalid. It should have reference
of 1. The driver that called IoCancelFileOpen is at fault.

PARAMETERS
    1 - FileObject passed to IoCancelFileOpen
    2 - DeviceObject passed to IoCancelFileOpen

ACTIVE_EX_WORKER_THREAD_TERMINATION           (0xE9)

An executive worker thread is being terminated without having gone through
the worker thread rundown code.  A stack trace should indicate the cause.

PARAMETERS
        1 - The exiting ETHREAD.

THREAD_STUCK_IN_DEVICE_DRIVER                 (0xEA)

The device driver is spinning in an infinite loop, most likely waiting for
hardware to become idle. This usually indicates problem with the hardware
itself or with the device driver programming the hardware incorrectly.

PARAMETERS
        1 - Pointer to a stuck thread object.  Do .thread then kb on it to find
            the hung location.
        2 - Pointer to a DEFERRED_WATCHDOG object.
        3 - Pointer to offending driver name.
        4 - Number of times this error occurred.  If a debugger is attached,
            this error is not always fatal -- see DESCRIPTION below.  On the
            blue screen, this will always equal 1.

DESCRIPTION

If the kernel debugger is connected and running when watchdog detects a
timeout condition then DbgBreakPoint() will be called instead of KeBugCheckEx()
and detailed message including bugcheck arguments will be printed to the
debugger. This way we can identify an offending thread, set breakpoints in it,
and hit go to return to the spinning code to debug it further. Because
KeBugCheckEx() is not called the .bugcheck directive will not return bugcheck
information in this case. The arguments are already printed out to the kernel
debugger. You can also retrieve them from a global variable via
"dd watchdog!g_WdBugCheckData l5" (use dq on NT64).

On MP machines it is possible to hit a timeout when the spinning thread is
interrupted by hardware interrupt and ISR or DPC routine is running at the time
of the bugcheck (this is because the timeout's work item can be delivered and
handled on the second CPU and the same time). If this is the case you will have
to look deeper at the offending thread's stack (e.g. using dds) to determine
spinning code which caused the timeout to occur.


THREAD_STUCK_IN_DEVICE_DRIVER_M                 (0x100000EA)

The device driver is spinning in an infinite loop, most likely waiting for
hardware to become idle. This usually indicates problem with the hardware
itself or with the device driver programming the hardware incorrectly.

PARAMETERS
        1 - Pointer to a stuck thread object.  Do .thread then kb on it to find
            the hung location.
        2 - Pointer to a DEFERRED_WATCHDOG object.
        3 - Pointer to offending driver name.
        4 - Number of times "intercepted" bugcheck 0xEA was hit (see notes).

DESCRIPTION

If the kernel debugger is connected and running when watchdog detects a
timeout condition then DbgBreakPoint() will be called instead of KeBugCheckEx()
and detailed message including bugcheck arguments will be printed to the
debugger. This way we can identify an offending thread, set breakpoints in it,
and hit go to return to the spinning code to debug it further. Because
KeBugCheckEx() is not called the .bugcheck directive will not return bugcheck
information in this case. The arguments are already printed out to the kernel
debugger. You can also retrieve them from a global variable via
"dd watchdog!g_WdBugCheckData l5" (use dq on NT64).

On MP machines it is possible to hit a timeout when the spinning thread is
interrupted by hardware interrupt and ISR or DPC routine is running at the time
of the bugcheck (this is because the timeout's work item can be delivered and
handled on the second CPU and the same time). If this is the case you will have
to look deeper at the offending thread's stack (e.g. using dds) to determine
spinning code which caused the timeout to occur.



DIRTY_MAPPED_PAGES_CONGESTION                 (0xEB)
PARAMETERS
        1 - Total number of dirty pages
        2 - Number of dirty pages destined for the pagefile(s).
        3 - Internal flags.
        4 - Most recent modified write error status.

DESCRIPTION
No free pages available to continue operations.

If kernel debugger available, type "!vm 3".

        This bugcheck usually occurs for the following reasons:

        1.  A driver has blocked, deadlocking the modified or mapped
            page writers.  Examples of this include mutex deadlocks or
            accesses to paged out memory in filesystem drivers, filter
            drivers, etc.  This indicates a driver bug.

            If parameter 1 or 2 is large, then this is a possibility.  Type
            "!vm 3" in the kernel debugger.

        2.  The storage driver(s) are not processing requests.  Examples
            of this are stranded queues, non-responding drives, etc.  This
            indicates a driver bug.

            If parameter 1 or 2 is large, then this is a possibility.  Type
            "!process 0 7" in the kernel debugger.

        3.  Not enough pool is available for the storage stack to write out
            modified pages.  This indicates a driver bug.

            If parameter 3 is small, then this is a possibility.  Type
            "!vm" and "!poolused 2" in the kernel debugger.

SESSION_HAS_VALID_SPECIAL_POOL_ON_EXIT  (0xEC)
PARAMETERS
        1 - session ID
        2 - number of special pool pages that are leaking

DESCRIPTION
Caused by a session driver not freeing its pool allocations prior to a
session unload.  This indicates a bug in win32k.sys, atmfd.dll,
rdpdd.dll or a video driver.


UNMOUNTABLE_BOOT_VOLUME (0xED)

        The IO subsystem attempted to mount the boot volume and it failed.

PARAMETERS
        1 - Device object of the boot volume
        2 - Status code from the filesystem on why it failed to mount the volume

CRITICAL_PROCESS_DIED (0xEF)

        A critical system process died

PARAMETERS
        1 - Process object

SCSI_VERIFIER_DETECTED_VIOLATION (0xF1)
PARAMETERS
        1 - Error code:
        VALUES
        1000 : Miniport passed bad params to ScsiPortInitialize

            2 - First argument to ScsiPortInitialize
            3 - Second argument to ScsiPortInitialize

        1001 : Miniport stalled processor too long

            2 - Delay in microseconds supplied by miniport

        1002 : Miniport routine executed too long

            2 - Address of routine that ran too long
            3 - Address of miniport's HwDeviceExtension
            4 - Duration of the routine in microseconds

        1003 : Miniport completed a request multiple times

            2 - Address of miniport's HwDeviceExtension
            3 - Address of SRB of multiply completed request

        1004 : Miniport has completed request with bad status

            2 - Address of SRB
            3 - Address of miniport's HwDeviceExtension

        1005 : Miniport has asked for the next LU request while
               an untagged request is active

            2 - Address of miniport's HwDeviceExtension
            3 - Address of logical unit extension

        1006 : Miniport called ScsiportGetPhysicalAddress with a bad VA

            2 - Address of miniport's HwDeviceExtension
            3 - VA supplied by the miniport

        1007 : Miniport had outstanding requests at the end of a bus reset period

            2 - Address of adapter extension
            3 - Address of miniport's HwDeviceExtension
        END_VALUES


DESCRIPTION
The SCSI verifier has detected an error in a SCSI miniport driver being verified.

HARDWARE_INTERRUPT_STORM (0xF2)

Note:  This bug check no longer exists in any version of Windows.
%  JVert wants this kept in bugcodes.w in case it is reinstated at some
%  point in the future.

PARAMETERS
     1 - address of the ISR (or first ISR in the chain) connected to the storming interrupt vector
     2 - ISR context value
     3 - address of the interrupt object for the storming interrupt vector
     4 - 0x1 if the ISR is not chained, 0x2 if the ISR is chained

DESCRIPTION
This bugcheck will show up on the screen when the kernel
detects an interrupt "storm".  An interrupt storm is defined as a level
triggered interrupt signal staying in the asserted state.  This is fatal
to the system in the manner that the system will hard hang, or "bus lock".

This can happen because of the following:

 -  A piece of hardware does not release its interrupt signal after being told
    to do so by the device driver
 -  A device driver does not instruct its hardware to release the interrupt
    signal because it does not believe the interrupt was initiated from its
    hardware
 -  A device driver claims the interrupt even though the interrupt was not
    initiated from its hardware.  Note that this can only occur when multiple
    devices are sharing the same IRQ.
 -  The ELCR (edge level control register) is set incorrectly.
 -  Edge and Level interrupt triggered devices share an IRQ (e.g. COM port and
    PCI SCSI controller).

All of these cases will instantly hard hang your system.  Instead of hard
hanging the system, this bugcheck is initiated since in many cases it can
identify the culprit.

When the bugcheck occurs, the module containing the ISR (interrupt service
routine) of the storming IRQ is displayed on the screen.  This is an
example of what you would see:

*** STOP: 0x000000F2 (0xFCA7C55C, 0x817B9B28, 0x817D2AA0, 0x00000002)

An interrupt storm has caused the system to hang.

*** Address FCA7C55C base at FCA72000, Datestamp 3A72BDEF - ACPI.sys

In the event the fourth parameter is a 0x00000001, the module pointed to
is very likely the culprit.  Either the driver is broken, or the hardware
is malfunctioning.

In the event the fourth parameter is a 0x00000002, the module pointed to
is the first ISR in the chain, and is never guaranteed to be the culprit.
A user experiencing this bugcheck repeatedly should try to isolate the
problem by looking for devices that are on the same IRQ as the one for
which the module is a driver for (in this case, the same IRQ that ACPI
is using).  In the future, we may be able to list all devices on a
chained ISR.

%  If you see a bugcheck like this, you should send mail to WINIRQTR to
%  have the system debugged.  People on this alias are well trained to debug
%  this type of failure and can typically isolate the faulty hardware or
%  device driver quickly.

DISORDERLY_SHUTDOWN               (0xF3)

No free pages available to continue operations.

Because applications are not terminated and drivers are
not unloaded, they can continue to access pages even after
the modified writer has terminated.  This can cause the
system to run out of pages since the pagefile(s) cannot be used.

PARAMETERS
        1 - Total number of dirty pages
        2 - Number of dirty pages destined for the pagefile(s).
        3 - Internal flags.
        4 - Most recent modified write error status.


CRITICAL_OBJECT_TERMINATION       (0xF4)

A process or thread crucial to system operation has unexpectedly exited or been
terminated.

Several processes and threads are necessary for the operation of the
system; when they are terminated (for any reason), the system can no
longer function.

PARAMETERS
        1 - Terminating object type
            VALUES:
                3 : Process
                6 : Thread
            END_VALUES
        2 - Terminating object
        3 - Process image file name
        4 - Explanatory message (ascii)


PCI_VERIFIER_DETECTED_VIOLATION   (0xF6)

The PCI driver detected an error in a device or BIOS being verified.

PARAMETERS
        1 - Failure detected
            VALUES:
                1 : An active bridge was reprogrammed by the BIOS during a docking event
                2 : The PMCSR register was not updated within the spec mandated time
                3 : A driver has written to OS controlled portions of a PCI device's config space
            END_VALUES


DRIVER_OVERRAN_STACK_BUFFER       (0xF7)

A driver has overrun a stack-based buffer.  This overrun could potentially
allow a malicious user to gain control of this machine.

DESCRIPTION

A driver overran a stack-based buffer (or local variable) in a way that would
have overwritten the function's return address and jumped back to an arbitrary
address when the function returned.  This is the classic "buffer overrun"
hacking attack and the system has been brought down to prevent a malicious user
from gaining complete control of it.

Do a kb to get a stack backtrace -- the last routine on the stack before the
buffer overrun handlers and bugcheck call is the one that overran its local
variable(s).

RAMDISK_BOOT_INITIALIZATION_FAILED  (0xF8)

An initialization failure occurred while attempting to boot from the RAM disk.

PARAMETERS
        1 - Action that failed
            VALUES:
                1 : No LoaderXIPRom descriptor was found in the loader memory list
                2 : Unable to open the RAM disk driver (ramdisk.sys, \Device\Ramdisk)
                3 : FSCTL_CREATE_RAM_DISK failed
                4 : Unable to create GUID string from binary GUID
                5 : Unable to create symbolic link pointing to RAM disk device
            END_VALUES
        2 - NT status code

DRIVER_RETURNED_STATUS_REPARSE_FOR_VOLUME_OPEN  (0xF9)

A driver returned STATUS_REPARSE to a Create request with no trailing names.
STATUS_REPARSE should be returned only for Create requests with trailing names
as that indicates the driver is supporting name spaces.

PARAMETERS
        1 - Device object that was opened.
        2 - Device object to which the Create IRP was issued.
        3 - New name of the file (to be reparsed) (UNICODE string)
        4 - Information returned by the driver for the Create IRP

HTTP_DRIVER_CORRUPTED (0xFA)

The HTTP kernel driver (http.sys) reached a corrupted state and can not
recover.

PARAMETERS
        1 - State identifier
        VALUES:
            1 : A work item is invalid. This will eventually result in
                  corruption of the thread pool and an access violation.
                2 - Address of work item
                3 - Name of file containing work item check, __FILE__
                4 - Line number in file, __LINE__
        END_VALUES

BUGCODE_PSS_MESSAGE                  (0x4000007C)

Generic bugcode message.

ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY    (0xFC)
An attempt was made to execute to non-executable memory.  The guilty driver
is on the stack trace (and is typically the current instruction pointer).

PARAMETERS
    1 - Virtual address for the attempted execute.
    2 - PTE contents.
    3 - (reserved)
%   3 - Unique internal Mm information.
    4 - (reserved)
%   4 - Unique internal Mm code.

DESCRIPTION
When possible, the guilty driver's name (Unicode string) is printed on
the bugcheck screen and saved in KiBugCheckDriver.

DIRTY_NOWRITE_PAGES_CONGESTION                (0xFD)
PARAMETERS
        1 - Total number of dirty pages
        2 - Number of nowrite dirty pages.
        3 - Internal flags.
        4 - Most recent modified write error status.

DESCRIPTION
No free pages available to continue operations.

If kernel debugger available, type "!vm 3 and !memusage 1".

        This bugcheck usually occurs because the component owning the
        modified nowrite pages failed to write out these pages after
        marking the relevant files as dont-write to memory management.
        This indicates a driver bug.

BUGCODE_USB_DRIVER                            (0xFE)
PARAMETERS
        1 - USB Bugcheck Code:
VALUES
        1 : INTERNAL_ERROR An internal error has occured in the USB stack
           
        2 : BAD_URB The USB client driver has submitted a URB that is 
               still attached to another IRP still pending in the bus
               driver.  
 
            2 - Address of pending IRP.
            3 - Address of IRP passed in.
            4 - Address URB that caused the error.

        3 : MINIPORT_ERROR The USB miniport driver has generated a 
               bugcheck. This is usually in response to catastrophic 
               hardware failure.

        4 : IRP_URB_DOUBLE_SUBMIT The caller has submitted an irp 
               that is already pending in the USB bus driver.
        
            2 - Address of IRP
            3 - Address of URB
                       
        
DESCRIPTION
USB Driver bugcheck, first parameter is USB bugcheck code.
        

WINLOGON_FATAL_ERROR                 (0xC000021A)

The Winlogon process terminated unexpectedly.

MANUALLY_INITIATED_CRASH1            (0xDEADDEAD)

The user manually initiated this crash dump.


%   NOTE:  See the beginning of this file for formatting instructions.
%   If you are adding new bugcodes to this file, make sure you place
%   them in the proper numerical order!

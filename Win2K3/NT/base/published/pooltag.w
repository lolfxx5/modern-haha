rem
rem    Pooltag.txt
rem
rem    This file lists the tags used for pool allocations by kernel mode components
rem    and drivers.
rem
rem    The file has the following format:
rem       <PoolTag> - <binary-name> - <Description>
rem
rem    This file is updated occasionaly and made available to the public through
rem    Microsoft Product Support Services. To check for updates, go to:
rem    http://support.microsoft.com and search for pooltag.txt.
rem



8042 - i8042prt.sys - PS/2 kb and mouse
ARPC - atmarpc.sys  - ATM ARP Client
ATMU - atmuni.sys   - ATM UNI Call Manager
Atom - <unknown>    - Atom Tables
Abos - <unknown>    - Abiosdsk
AcdM - <unknown>    - TDI AcdObjectInfoG
AcdN - <unknown>    - TDI AcdObjectInfoG
Acp* - acpi.sys     - ACPI Pooltags
ACPI - acpi.sys - ACPI
Adap - <unknown>    - Adapter objects
Adbe - <unknown>    - Adobe's font driver
AECi - <unknown>    - filter object interface for MS acoustic echo canceller
Afd? - afd.sys      - AFD objects
AfdA - afd.sys      -     Afd EA buffer
AfdB - afd.sys      -     Afd data buffer
AfdC - afd.sys      -     Afd connection structure
AfdD - afd.sys      -     Afd debug data
AfdE - afd.sys      -     Afd endpoint structure
AfdF - afd.sys      -     Afd TransmitFile info
AfdG - afd.sys      -     Afd group table
AfdI - afd.sys      -     Afd TDI data
AfdL - afd.sys      -     Afd local address buffer
AfdP - afd.sys      -     Afd poll info
AfdQ - afd.sys      -     Afd work queue item
AfdR - afd.sys      -     Afd remote address buffer
AfdS - afd.sys      -     Afd security info
AfdT - afd.sys      -     Afd transport info
AfdW - afd.sys      -     Afd work item
AfdX - afd.sys      -     Afd context buffer
Afda - afd.sys      -     Afd APC buffer (NT 3.51 only)
Afdc - afd.sys      -     Afd connect data buffer
Afdd - afd.sys      -     Afd disconnect data buffer
Afdf - afd.sys      -     Afd TransmitFile debug data
Afdh - afd.sys      -     Afd address list change buffer
Afdi - afd.sys      -     Afd "set inline mode" buffer
Afdl - afd.sys      -     Afd lookaside lists buffer
Afdp - afd.sys      -     Afd transport IRP buffer
Afdq - afd.sys      -     Afd routing query buffer
Afdr - afd.sys      -     Afd ERESOURCE buffer
Afdt - afd.sys      -     Afd transport address buffer
Afp  - <unknown>    - SFM File server
Aml* - <unknown>    - ACPI AMLI Pooltags
APIC - pnpapic.sys  - I/O APIC Driver
ArbA - nt!arb       - ARBITER_ALLOCATION_STATE_TAG
ArbL - nt!arb       - ARBITER_ORDERING_LIST_TAG
ArbM - nt!arb       - ARBITER_MISC_TAG
ArbR - nt!arb       - ARBITER_RANGE_LIST_TAG
Arp? - <unknown>    - ATM ARP server objects, atmarps.sys
ArpA - <unknown>    -     AtmArpS address
ArpB - <unknown>    -     AtmArpS buffer space
ArpI - <unknown>    -     AtmArpS Interface structure
ArpK - <unknown>    -     AtmArpS ARP block
ArpM - <unknown>    -     AtmArpS MARS structure
ArpR - <unknown>    -     AtmArpS NDIS request
ArpS - <unknown>    -     AtmArpS SAP structure
Asy1 - <unknown>    - ndis / ASYNC_IOCTX_TAG
Asy2 - <unknown>    - ndis / ASYNC_INFO_TAG
Asy3 - <unknown>    - ndis / ASYNC_ADAPTER_TAG
Asy4 - <unknown>    - ndis / ASYNC_FRAME_TAG
AtC  - <unknown>    - IDE disk configuration
AtD  - <unknown>    - atdisk.c
ATIX - <unknown>    - WDM mini drivers for ATI tuner, xbar, etc.
Atk  - <unknown>    - Appletalk transport
AtmA - <unknown>    - Atoms
AtmT - <unknown>    - Atom tables
AuxL - <unknown>    - EXIFS Auxlist

Bat? - <unknown>    - Battery Class drivers
BatC - <unknown>    -     Composite battery driver
BatM - <unknown>    -     Control method battery driver
BatS - <unknown>    -     Smart battery driver
Batt - <unknown>    -     Battery class driver
BCSP - bthbcsp.sys  - Bluetooth BCSP minidriver
BIG  - nt!mm        - Large session pool allocations (ntos\ex\pool.c)
Bmfd - <unknown>    - Font related stuff
BT3C - bt3c.sys     - Bluetooth 3COM minidriver
BT8x - <unknown>    - WDM mini drivers for Brooktree 848,829, etc.
BTHP - bthport.sys  - Bluetooth port driver (generic)
BthS - bthport.sys  - Bluetooth port driver (security)
BTME - bthenum.sys  - Bluetooth enumerator
BTMO - bthmodem.sys - Bluetooth modem
BTPT - <unknown>    - Bluetooth transport protocol library
BTSR - bthser.sys   - Bluetooth serial minidriver
BTUR - bthuart.sys  - Bluetooth UART minidriver
Bu*  - <unknown>    - burneng.sys from adaptec

Call - nt!ex        - kernel callback object signature
call - <unknown>    - debugging call tables
CBRe - <unknown>    - CallbackRegistration
Cc   - nt!cc        - Cache Manager allocations (catch-all)
CcBc - nt!cc        - Cache Manager Bcb from pool
CcBr - nt!cc        - Cache Manager Bitmap range
CcBz - nt!cc        - Cache Manager Bcb Zone
CcDw - nt!cc        - Cache Manager Deferred Write
CcEv - nt!cc        - Cache Manager Event
CcFn - nt!cc        - Cache Manager File name for popups
CcOb - nt!cc        - Cache Manager Overlap Bcb
CcPB - nt!ccpf      - Prefetcher trace buffer
CcPC - nt!ccpf      - Prefetcher context
CcPD - nt!ccpf      - Prefetcher trace dump
CcPF - nt!ccpf      - Prefetcher file name
CcPI - nt!ccpf      - Prefetcher intermediate table
CcPL - nt!ccpf      - Prefetcher read list
CcPM - nt!ccpf      - Prefetcher metadata
CcPS - nt!ccpf      - Prefetcher scenario
CcPT - nt!ccpf      - Prefetcher trace
CcPc - nt!cc        - Cache Manager Private Cache Map
CcPf - nt!ccpf      - Prefetcher
CcPp - nt!ccpf      - Prefetcher instructions
CcPq - nt!ccpf      - Prefetcher query buffer
CcPs - nt!ccpf      - Prefetcher section table
CcPv - nt!ccpf      - Prefetcher volume info
CcPw - nt!ccpf      - Prefetcher boot worker
CcSc - nt!cc        - Cache Manager Shared Cache Map
CcVa - nt!cc        - Cache Manager Initial array of Vacbs
CcVl - nt!cc        - Cache Manager Vacb Level structures (large streams)
CcVp - nt!cc        - Cache Manager Array of Vacb pointers for a cached stream
CctX - <unknown>    - EXIFX FCB Commit CTX
CcWq - nt!cc        - Cache Manager Work Queue Item
CcWK - nt!cc        - Kernel Cache Manager lookaside list
CcWk - nt!cc        - Kernel Cache Manager lookaside list
CcZe - nt!cc        - Cache Manager Buffer of Zeros
CdA  - <unknown>    - CdAudio filter driver
Cdcc - cdfs.sys     - CDFS Ccb
Cddn - cdfs.sys     - CDFS CdName in dirent
Cdee - cdfs.sys     - CDFS Search expression for enumeration
Cdfd - cdfs.sys     - CDFS Data Fcb
Cdfi - cdfs.sys     - CDFS Index Fcb
Cdfl - cdfs.sys     - CDFS Filelock
CdFn - cdfs.sys     - CDFS Filename buffer
Cdfn - cdfs.sys     - CDFS Nonpaged Fcb
Cdfs - cdfs.sys     - CDFS General Allocation
Cdft - cdfs.sys     - CDFS Fcb Table entry
Cdgs - cdfs.sys     - CDFS Generated short name
Cdic - cdfs.sys     - CDFS Irp Context
Cdil - cdfs.sys     - CDFS Irp Context lite
Cdio - cdfs.sys     - CDFS Io context for async reads
Cdma - cdfs.sys     - CDFS Mcb array
Cdpe - cdfs.sys     - CDFS Prefix Entry
CdPn - cdfs.sys     - CDFS CdName in path entry
Cdpn - cdfs.sys     - CDFS Prefix Entry name
Cdsp - cdfs.sys     - CDFS Buffer for spanning path table
Cdtc - cdfs.sys     - CDFS TOC
Cdun - cdfs.sys     - CDFS Buffer for upcased name
Cdvd - cdfs.sys     - CDFS Buffer for volume descriptor
Cdvp - cdfs.sys     - CDFS Vpb allocated in filesystem
CM   - nt!cm        - Configuration Manager (registry)
CmcD - hal.dll      - HAL CMC Driver Log
CmcK - hal.dll      - HAL CMC Kernel Log
CmcT - hal.dll      - HAL CMC temporary Log
CMCa - nt!cm        - Configuration Manager Cache (registry)
CMDc - nt!cm        - Configuration Manager Cache (registry)
CMkb - nt!cm        - registry key control blocks
CMpb - nt!cm        - registry post blocks
CMnb - nt!cm        - registry notify blocks
CMpe - nt!cm        - registry post events
CMpa - nt!cm        - registry post apcs
CMsb - nt!cm        - registry stash buffer
CmVn - nt!cm        - captured value name
CMVw - nt!cm        - registry mapped view of file
CMVa - nt!cm        - value cache value tag
CMVI - nt!cm        - value index cache tag
CMSc - nt!cm        - security cache pooltag
CMSb - nt!cm       - internal stash buffer pool tag
CMNb - nt!cm        - notification block pool tag
CMIn - nt!cm        - index hint pool tag
CM?? - nt!cm        - Internal Configuration manager allocations
CMDa - nt!cm       - value data cache pool tag
CMAl - nt!cm        -internal registry memory allocator pool tag
COMX - serial.sys   - serial driver allocations
Cont - <unknown>    - Contiguous physical memory allocations for device drivers
CopW - <unknown>    - EXIFS CopyOnWrite
CpeD - hal.dll      - HAL CPE Driver Log
CpeK - hal.dll      - HAL CMC Kernel Log
CpeT - hal.dll      - HAL CPE temporary Log
CPnp - classpnp.sys - ClassPnP transfer packets
CrtH - <unknown>    - EXIFS Create Header
CSdk - <unknown>    - Cluster Disk Filter driver
CSnt - <unknown>    - Cluster Network driver
CTE  - <unknown>    - Common transport environment (ntos\inc\cxport.h, used by tdi)
Cvli - <unknown>    - EXIFS Cached Volume Info

D2d  - <unknown>    - Device Object to DosName rtns (ntos\rtl\dev2dos.c)
D3Dd - <unknown>    - DX D3D driver (embedded in a display driver like s3mvirge.dll)
D851 - <unknown>    - 8514a video driver
Dacl - <unknown>    - Temp allocations for object DACLs
Dati - <unknown>    - ati video driver
DbLo - <unknown>    - Debug logging
dcam - <unknown>    - WDM mini driver for IEEE 1394 digital camera
Dcl  - <unknown>    - cirrus video driver
Ddk  - <unknown>    - Default for driver allocated memory (user's of ntddk.h)
Devi - <unknown>    - Device objects

Dh 0 - <unknown>    - DirectDraw/3D default object
Dh 1 - <unknown>    - DirectDraw/3D DirectDraw object
Dh 2 - <unknown>    - DirectDraw/3D Surface object
Dh 3 - <unknown>    - DirectDraw/3D Direct3D context object
Dh 4 - <unknown>    - DirectDraw/3D VideoPort object
Dh 5 - <unknown>    - DirectDraw/3D MotionComp object

Dfb  - <unknown>    - framebuf video driver
Dfs  - <unknown>    - Distributed File System
dfsr - <unknown>    - RDBSS IRP allocation
Dfsm - <unknown>    - Eng event allocation (ENG_KEVENTALLOC,ENG_ALLOC) in ntgdi\gre
Dire - <unknown>    - Directory objects
Dlck - <unknown>    - deadlock verifier (part of driver verifier) structures
Dmga - <unknown>    - mga (matrox) video driver
DmH? - <unknown>    - DirectMusic hardware synthesizer
Dmio - <unknown>    - Logical Disk Manager driver
Dmip - dmio.sys - Logical Disk Manager driver
Dmiq - dmio.sys - Logical Disk Manager driver
Dmld - <unknown>    - Logical Disk Manager loader
DmS? - <unknown>    - DirectMusic kernel software synthesizer
Dmst - <unknown>    - Logical Disk Manager driver static initialization
Dndt - <unknown>    - Device node
Dnod - <unknown>    - Device node structure
DOPE - <unknown>    - Device Object Power Extension (po component)
DPrf - <unknown>    - Disk performance filter driver
DPwr - nt!pnp       - PnP power management
Dps5 - <unknown>    - NT5 PostScript printer driver
Dpsh - <unknown>    - Postcript driver heap memory
Dpsi - <unknown>    - psidisp video driver
Dpsm - <unknown>    - Postcript driver memory
Dqv  - <unknown>    - qv (qvision) video driver
Driv - <unknown>    - Driver objects
Drsd - <unknown>    - Rasdd Printer Driver Pool Tag.
Dtga - <unknown>    - tga video driver
Dump - <unknown>    - Bugcheck dump allocations
Dun5 - <unknown>    - NT5 Universal printer driver

DV?? - <unknown>    - RDR2 DAV MiniRedir Tags
DVCx - <unknown>    - AsyncEngineContext, DAV MiniRedir
DVEx - <unknown>    - Exchange, DAV MiniRedir
DVFi - <unknown>    - FileInfo, DAV MiniRedir
DVFn - <unknown>    - FileName, DAV MiniRedir
DVRw - <unknown>    - ReadWrite, DAV MiniRedir
DVSc - <unknown>    - SrvCall, DAV MiniRedir
DVSh - <unknown>    - SharedHeap, DAV MiniRedir

Dvga - <unknown>    - vga 16 color video driver
Dvg2 - <unknown>    - vga 256 color video driver
Dvg6 - <unknown>    - vga 64K color video driver
Dvgr - <unknown>    - vga for risc video driver
DW32 - <unknown>    - W32 video driver
Dwd  - <unknown>    - wd90c24a video driver
Dwp9 - <unknown>    - weitekp9 video driver
Dxga - <unknown>    - XGA video driver

Efsm - <unknown>    - EFS driver
Efsc - <unknown>    - EFS driver

Efst - <unknown>    -  EXIFS FS Statistics

Envr - <unknown>    - Environment strings
Err  - <unknown>    - Error strings
Evel - <unknown>    - EFS file system filter driver lookaside list
Even - <unknown>    - Event objects
Evid - <unknown>    - Rtl Event ID's
ExWl - <unknown>    - Executive worker list entry

Fat  - fastfat.sys  - Fat File System allocations
FatB - fastfat.sys  - Fat allocation bitmaps
FatC - fastfat.sys  - Fat Ccbs
FatD - fastfat.sys  - Fat pool dirents
FatE - fastfat.sys  - Fat EResources
FatF - fastfat.sys  - Fat Fcbs
FatI - fastfat.sys  - Fat IrpContexts
FatL - fastfat.sys  - Fat FAT entry lookup buffer on verify
FatN - fastfat.sys  - Fat Nonpaged Fcbs
FatO - fastfat.sys  - Fat I/O buffer
FatP - fastfat.sys  - Fat output for query retrieval pointers (caller frees)
FatQ - fastfat.sys  - Fat buffered user buffer
FatR - fastfat.sys  - Fat repinned Bcb
FatS - fastfat.sys  - Fat stashed Bpb
FatT - fastfat.sys  - Fat directory allocation bitmaps
FatV - fastfat.sys  - Fat Vcb stat bucket
Fatv - fastfat.sys  - Fat events
FatW - fastfat.sys  - Fat FAT windowing structure
FatX - fastfat.sys  - Fat IO contexts
Fatb - fastfat.sys  - Fat Bcb arrays
Fatd - fastfat.sys  - Fat EA data
Fate - fastfat.sys  - Fat EA set headers
Fatf - fastfat.sys  - Fat deferred flush contexts
Fati - fastfat.sys  - Fat IO run descriptor
Fatn - fastfat.sys  - Fat filename buffer
Fatr - fastfat.sys  - Fat verification-time rootdir snapshot
Fats - fastfat.sys  - Fat verification-time boot sector
Fatv - fastfat.sys  - Fat backpocket Vpb
Fatx - fastfat.sys  - Fat delayed close contexts

Fcbl - <unknown>    - EXIFS FCBlock
fboX - <unknown>    - EXIFS FOBXVF List

File - <unknown>    - File objects
Flop - <unknown>    - floppy driver

Flst - <unknown>    - EXIFS Freelist

FLex - <unknown>    - exclusive file lock
FLfl - <unknown>    - exported (non-private) file lock
FLli - <unknown>    - per-file lock information
FLln - <unknown>    - shared lock tree node
FLsh - <unknown>    - shared file lock
FLwl - <unknown>    - waiting lock

Fsrc - <unknown>    - Filesystem recognizer (fsrec.sys)

Fstb - <unknown>    - ntos\fstub
FstB - <unknown>    - ntos\fstub

flnk - <unknown>    - font link tag used in ntgdi\gre

FS?? - nt!fsrtl     - File System Run Time allocations
FSfm - nt!fsrtl     - File System Run Time Fast Mutex Lookaside List
FSim - nt!fsrtl     - File System Run Time Mcb Initial Mapping Lookaside List
FSrt - nt!fsrtl     - File System Run Time allocations (DO NOT USE!)
FSmg - nt!fsrtl     - File System Run Time
FSrd - nt!fsrtl     - File System Run Time
FSrm - nt!fsrtl     - File System Run Time
FSrn - nt!fsrtl     - File System Run Time
FSrN - nt!fsrtl     - File System Run Time
FSro - nt!fsrtl     - File System Run Time
FSrs - nt!fsrtl     - File System Run Time
FSun - nt!fsrtl     - File System Run Time

FTrc - <unknown>    - Fault tolerance Slist tag.
FtC  - <unknown>    - Fault tolerance driver
FtM  - <unknown>    - Fault tolerance driver
FtS  - <unknown>    - Fault tolerance driver
FtT  - <unknown>    - Fault tolerance driver
FtU  - <unknown>    - Fault tolerance driver
FtV  - <unknown>    - Fault tolerance driver
       <unknown>
G??? - <unknown>    - Gdi Objects
G    - <unknown>    -     Gdi Generic allocations
Gcac - <unknown>    -     Gdi glyph cache
Gcap - <unknown>    -     Gdi capture buffer
Gcsl - <unknown>    -     Gdi string resource script names
Gdbr - <unknown>    -     Gdi driver brush realization
Gdd  - <unknown>    -     Gdi ddraw PKEY_VALUE_FULL_INFORMATION
Gdda - <unknown>    -     Gdi ddraw attach list
GddD - <unknown>    -     Gdi ddraw dummy page
Gdde - <unknown>    -     Gdi ddraw event
Gddf - <unknown>    -     Gdi ddraw driver heaps
Gddp - <unknown>    -     Gdi ddraw driver caps
Gddv - <unknown>    -     Gdi ddraw driver video memory list
Gdwd - win32k.sys   -     Gdi watchdog support objects
Gdxd - <unknown>    -     Gdi ddraw VPE directdraw object
Gdxs - <unknown>    -     Gdi ddraw VPE surface, videoport, capture object
Gdxx - <unknown>    -     Gdi ddraw VPE DXAPI object
Gdev - <unknown>    -     Gdi GDITAG_DEVMODE
GDev - <unknown>    -     Gdi pdev
Gdrs - <unknown>    -     Gdi GDITAG_DRVSUP
Gebr - <unknown>    -     Gdi ENGBRUSH
gEdg - <unknown>    -     Gdi gradient fill triangle
Gffv - <unknown>    -     Gdi FONTFILEVIEW
gFil - <unknown>    -     Gdi FILEVIEW
GFil - <unknown>    -     Gdi engine descriptor list
Gfsb - <unknown>    -     Gdi font sustitution list
Gfsm - <unknown>    -     Gdi Fast mutex
Ggdv - <unknown>    -     Gdi GDITAG_GDEVICE
Gglb - <unknown>    -     Gdi temp buffer
Ggls - <unknown>    -     Gdi glyphset
Ggb  - <unknown>    -     Gdi glyph bits
Ggbl - <unknown>    -     Gdi look aside buffers
Ghmg - <unknown>    -     Gdi handle manager objects
Gini - <unknown>    -     Gdi fast mutex
Gh ? - win32k.sys   -     Gdi Handle manager specific object types: defined in w32\ntgdi\inc\ntgdistr.h
Gla? - win32k.sys   -     Gdi handle manager specific object types allocated from lookaside memory: defined in w32\ntgdi\inc\ntgdistr.h
Gldv - <unknown>    -     Gdi Ldev
Glnk - <unknown>    -     Gdi PFELINK
Gmap - <unknown>    -     Gdi font map signature table
Gpff - <unknown>    -     Gdi physical font file
Gpft - <unknown>    -     Gdi font table
Gsem - <unknown>    -     Gdi Semaphores
Gsp  - <unknown>    -     Gdi sprite
Gspr - <unknown>    -     Gdi sprite grow range
Gtmp - <unknown>    -     Gdi temporary allocations
Gtmw - <unknown>    -     Gdi TMW_INTERNAL
Gxlt - <unknown>    -     Gdi Xlate

Gfcb - <unknown>    - EXIFS Grow FCB


Hal  - hal.dll      - Hardware Abstraction Layer
Hpfs - <unknown>    - Pinball (aka Hpfs) allocations
HisC - <unknown>    - histogram filter driver
Hist - <unknown>    - histogram filter driver
HidP - <unknown>    - HID Parser
HidC - <unknown>    - HID Class
HdCl - <unknown>    - HID Client Sample Driver
HpMM - pnpmem.sys   - HotPlug Memory Driver

HCID - bthport.sys  - Bluetooth port driver HCI debug
HCIT - bthport.sys  - Bluetooth port driver (HCI)

HcRs - hcaport.sys - HCAPORT_TAG_RESOURCE_LIST
HcEv - hcaport.sys - HCAPORT_TAG_EVENT
HcdI - hcaport.sys - HCAPORT_TAG_HWID
HcEn - hcaport.sys - HCAPORT_TAG_ENUM
HcMc - hcaport.sys - HCAPORT_TAG_MISC
HcDr - hcaport.sys - HCAPORT_TAG_DEVICE_RELATIONS
HcBm - hcaport.sys - HCAPORT_TAG_BITMAP
HcOb - hcaport.sys - HCAPORT_TAG_OBJECT
HcCq - hcaport.sys - HCAPORT_TAG_CQUEUE
HcPr - hcaport.sys - HCAPORT_TAG_PROTD
HPmi - hcaport.sys - HCAPORT_TAG_PMI_EXTENSION
HcMa - hcaport.sys - HCAPORT_TAG_MAD
HcMp - hcaport.sys - HCAPORT_TAG_MINIPORT
HcCm - hcaport.sys - HCAPORT_TAG_CM
HcUc - hcaport.sys - HCAPORT_TAG_UCONTEXT
HcMr - hcaport.sys - HCAPORT_TAG_REMOVE_LOCK
Hmgo - hcaport.sys - HCAPORT_TAG_WQ_MG_INFO
Hioc - hcaport.sys - HCAPORT_TAG_IOC_SERVICE_TABLE

hSVD - mrxdav.sys - Shared Heap Tag

HT01 - <unknown>    - GDI Halftone AddCachedDCI() for CurCDCIData
HT02 - <unknown>    - GDI Halftone GetCachedDCI() for Threshold
HT03 - <unknown>    - GDI Halftone FindCachedSMP() for CurCSMPData
HT04 - <unknown>    - GDI Halftone FindCachedSMP() for CurCSMPBmp
HT05 - <unknown>    - GDI Halftone HT_CreateDeviceHalftoneInfo() for HT_DHI
HT06 - <unknown>    - GDI Halftone pDCIAdjClr() for DEVCLRADJ
HT07 - <unknown>    - GDI Halftone ComputeRGB555LUT() for RGBLUT
HT08 - <unknown>    - GDI Halftone ColorTriadSrcToDev() for RGB-XYZ
HT09 - <unknown>    - GDI Halftone ColorTriadSrcToDev() for CRTX-FD6XYZ Cache
HT10 - <unknown>    - GDI Halftone CreateDyesColorMappingTable() for DevPrims
HT11 - <unknown>    - GDI Halftone CreateDyesColorMappingTable() for DyeMappingTable
HT12 - <unknown>    - GDI Halftone ThresholdsFromYData() for pYData
HT13 - <unknown>    - GDI Halftone ComputeHTCellRegress() for pThresholds
HT14 - <unknown>    - GDI Halftone CalculateStretch() for InputSI/pSrcMaskLine
HT15 - <unknown>    - GDI Halftone CalculateStretch() for PrimColorInfo

IBCM - wibcm.sys - CM_INSTANCE_TAG Windows Infiniband Communications Manager
CEP  - wibcm.sys - CEP_INSTANCE_TAG
CMSH - wibcm.sys - WIBCM_SHARED_TAG
CMWK - wibcm.sys - WIBCM_WORK_TAG
CMWT - wibcm.sys - WIBCM_TIMER_TAG

IBm* - wibms.sys - Windows Infiniband Management Server pool tags

IbPm - wibpm.sys - WIBPM_TAG Windows Infiniband Performance Manager
IbPS - wibpm.sys - WIBPM_SENT_TAG
IbPI - wibpm.sys - WIBPM_ITEM_TAG
IbPA - wibpm.sys - WIBPM_SAMPLE_TAG

IbW0 - wibwmi.sys - WIBWMI0_TAG Windows Infiniband WMI Manager
IbW1 - wibwmi.sys - WIBWMI1_TAG
IbW2 - wibwmi.sys - WIBWMI2_TAG

IPX  - <unknown>    - Nwlnkipx transport
Icp  - <unknown>    - I/O completion packets queue on a completion ports
IcpP - <unknown>    - NPAGED_LOOKASIDE_LIST I/O completion per processor lookaside pointers
IdeP - <unknown>    - atapi IDE
IdeX - <unknown>    - PCI IDE
idle - <unknown>    - Power Manager idle handler
Ifs  - <unknown>    - Default file system allocations (user's of ntifs.h)
Info - <unknown>    - general system information allocations
Io   - nt!io        - general IO allocations
IoCc - nt!io        - Io completion context
IoEa - nt!io        - Io extended attributes
IoEr - nt!io        - Io error log packets
IoFc - nt!io        - Io name transmogrify operation
IoFs - nt!io        - Io shutdown packet
IoFu - nt!pnp       - Io file utils
IoKB - <unknown>    - Registry basic key block (temp allocation)
IoNm - nt!io        - Io parsing names
IoRb - nt!io        - Io remote boot related
IoRN - nt!io        - Registry key name (temp allocation)
IoSe - nt!io        - Io security related
IoSh - nt!io        - Io shutdown packet
IoTi - nt!io        - Io timers
IoTt - nt!vf        - I/O verifier IRP tracking table
Ioin - <unknown>    - Io interrupts
IoOp - nt!io       - I/O subsystem open packet
IoRi - nt!io        - I/O SubSystem Driver Reinitialization Callback Packet
IoUs - nt!io       - I/O SubSystem completion Context Allocation
Irp  - <unknown>    - Io, IRP packets
Irp+ - nt!vf        - I/O verifier allocated IRP packets
IrpB - nt!vf        - I/O verifier direct I/O double buffer allocation
Irpd - nt!vf        - I/O verifier deferred completion context
Irps - nt!vf        - I/O verifier per-IRP session tracking data
Irpt - nt!vf        - I/O verifier per-IRP tracking data
IrpC - nt!vf        - I/O verifier stack contexts
Irpl - nt!io        - system large IRP lookaside list
Irps - nt!io        - system small IRP lookaside list
Isap - <unknown>    - Pnp Isa bus extender
II?? - <unknown>    - IP in IP tunneling
IIrf - <unknown>    - Free memory
IIdt - <unknown>    - Data
IITn - <unknown>    - Tunnel
IIhd - <unknown>    - Header
IIpk - <unknown>    - Packet
IIsc - <unknown>    - Send Context
IIts - <unknown>    - Transfer Context
IIwc - <unknown>    - Work Context

Im*  - <unknown>    - Imapi.sys from adaptec

IPm? - <unknown>    - IP Multicasting
IPmg - <unknown>    - Group
IPms - <unknown>    - Source
IPmo - <unknown>    - Outgoing Interface
IPmm - <unknown>    - Message
IPmf - <unknown>    - Free memory (only in checked builds)

Ip?? - ipsec.sys    - IP Security
IpSI - ipsec.sys    - initial allcoations
IpAT - ipsec.sys    -  AH headers in transport mode
IpAU - ipsec.sys    -  AH headers in tunnel mode
IpET - ipsec.sys    -  ESP headers in transport mode
IpEU - ipsec.sys    -  ESP headers in tunnel mode
IpHT - ipsec.sys    -  HUGHES headers in transport mode
IpHU - ipsec.sys    -  HUGHES headers in tunnel mode
IpAX - ipsec.sys    -  Key acquire contexts
IpFI - ipsec.sys    -  Filter blocks
IpSA - ipsec.sys    -  Security Associations
IpKE - ipsec.sys    -  keys
IpTI - ipsec.sys    -  timers
IpSQ - ipsec.sys    -  stall queues
IpLA - ipsec.sys    -  lookaside lists
IpBP - ipsec.sys    -  buffer pools
IpSC - ipsec.sys    -  send complete context
IpEQ - ipsec.sys    -  event queue
IpHW - ipsec.sys    -  hardware accleration items
IpCO - ipsec.sys    -  IP compression

IrD? - <unknown>    - IrDA TDI and RAS drivers

IU?? - <unknown>    - IIS Utility Driver
IUDl - <unknown>    -     Lookaside list allocations
IUcp - <unknown>    -     completion port minipackets

KAPC - nt!io              - I/O SubSystem HardError APC
KbdC - <unknown>    - Keyboard Class Driver
KCfe - <unknown>    - Kernel COM factory entry
Ke   - <unknown>    - Kernel data structures
KeIC - <unknown>    - Kernel Interrupt Object Chain
Key  - <unknown>    - Key objects
KMIX - <unknown>    - Kmixer (wdm audio)
KNMI - <unknown>    - Kernel NMI Callback object
KrbC - <unknown>    - Kerberos Client package
KSah - <unknown>    - Ks auxiliary stream headers
KSAI - <unknown>    -    allocator instance
KSai - <unknown>    -    default allocator instance header
KSbi - <unknown>    -    event buffered item
KSCI - <unknown>    -    clock instance
KSce - <unknown>    -    create item entry
KSch - <unknown>    -    create handler entry
KSci - <unknown>    -    default clock instance header
KScp - <unknown>    -    object creation parameters auxiliary copy
KSda - <unknown>    -    default allocator
KSdc - <unknown>    -    default clock
KSdh - <unknown>    -    device header
Ksec - <unknown>    -    security device driver
KSed - <unknown>    -    event dpc item
KSee - <unknown>    -    event entry
KSed - <unknown>    -    oneshot event deletion dpc
KSep - <unknown>    -    irp system buffer event parameter
KSew - <unknown>    -    oneshot event deletion workitem
KSqr - <unknown>    -    QM quality report
KSer - <unknown>    -    QM error report
KSfd - <unknown>    -    filter cache data (MSKSSRV)
KsFI - <unknown>    -    filter instance
KSns - <unknown>    -    null security object
KSnv - <unknown>    -    registry name value
KSoh - <unknown>    -    object header
KSop - <unknown>    -    object creation parameters
KSpc - <unknown>    -    port driver instance FsContext
KSPI - <unknown>    -    pin instance
KSpp - <unknown>    -    irp system buffer property/method/event parameter
KSpt - <unknown>    -    pin type list (MSKSSRV)
KSqf - <unknown>    -    query information file buffer
KSsc - <unknown>    -    port driver stream FsContext
KSsf - <unknown>    -    set information file buffer
KSsh - <unknown>    -    stream headers
KSsi - <unknown>    -    software bus interface
KSsl - <unknown>    -    symbolic link buffer (MSKSSRV)
KSsp - <unknown>    -    serialized property set
KsoO - <unknown>    -    WDM audio stuff
L2CA - bthport.sys  - Bluetooth port driver (L2CAP)
L2T0 - <unknown>    -    ndis\l2tp / MTAG_FREED
L2T1 - <unknown>    -    ndis\l2tp / MTAG_ADAPTERCB
L2T2 - <unknown>    -    ndis\l2tp / MTAG_TUNNELCB
L2T3 - <unknown>    -    ndis\l2tp / MTAG_VCCB
L2T4 - <unknown>    -    ndis\l2tp / MTAG_VCTABLE
L2T5 - <unknown>    -    ndis\l2tp / MTAG_WORKITEM
L2T6 - <unknown>    -    ndis\l2tp / MTAG_TIMERQ
L2T7 - <unknown>    -    ndis\l2tp / MTAG_TIMERQITEM
L2T8 - <unknown>    -    ndis\l2tp / MTAG_PACKETPOOL
L2T9 - <unknown>    -    ndis\l2tp / MTAG_FBUFPOOL
L2Ta - <unknown>    -    ndis\l2tp / MTAG_HBUFPOOL
L2Tb - <unknown>    -    ndis\l2tp / MTAG_TDIXRDG
L2Tc - <unknown>    -    ndis\l2tp / MTAG_TDIXSDG
L2Td - <unknown>    -    ndis\l2tp / MTAG_CTRLRECD
L2Te - <unknown>    -    ndis\l2tp / MTAG_CTRLSENT
L2Tf - <unknown>    -    ndis\l2tp / MTAG_PAYLRECD
L2Tg - <unknown>    -    ndis\l2tp / MTAG_PAYLSENT
L2Th - <unknown>    -    ndis\l2tp / MTAG_INCALL
L2Ti - <unknown>    -    ndis\l2tp / MTAG_UTIL
L2Tj - <unknown>    -    ndis\l2tp / MTAG_ROUTEQUERY
L2Tk - <unknown>    -    ndis\l2tp / MTAG_ROUTESET
L2Tl - <unknown>    -    ndis\l2tp / MTAG_L2TPPARAMS
L2Tm - <unknown>    -    ndis\l2tp / MTAG_TUNNELWORK
L2Tn - <unknown>    -    ndis\l2tp / MTAG_TDIXROUTE
LANE - atmlane.sys  - LAN Emulation Client for ATM
LB?? - <unknown>    - LM Datagram receiver allocations
LBan - <unknown>    -     Server announcement
LBvb - <unknown>    -     View buffer
LBma - <unknown>    -     Master announce context
LBxp - <unknown>    -     Transport
LBxn - <unknown>    -     TransportName
LBxm - <unknown>    -     Master name
LBtn - <unknown>    -     Transport name
LBea - <unknown>    -     Ea buffer
LBdi - <unknown>    -     POOL_DOMAIN_INFO
LBds - <unknown>    -     Send datagram context
LBci - <unknown>    -     Connection info
LBmh - <unknown>    -     Mailslot header
LBbl - <unknown>    -     Backup List
LBsl - <unknown>    -     Browser server list
LBbs - <unknown>    -     Browser server
LBgb - <unknown>    -     GetBackupList request
LBbr - <unknown>    -     GetBackupList response
LBmb - <unknown>    -     Mailslot Buffer
LBid - <unknown>    -     Illegal datagram context
LBbn - <unknown>    -     Name
LBnn - <unknown>    -     Name name
LBic - <unknown>    -     IRP context
LBwi - <unknown>    -     Work item
LBel - <unknown>    -     Election context
LBbb - <unknown>    -     Become backup context
LBbr - <unknown>    -     Become backup request
LBpn - <unknown>    -     Paged Name
LBpt - <unknown>    -     Paged transport
LBse - <unknown>    -     Browser security

Lbuf - <unknown>    - EXIFS Large Buffer

LeoC - <unknown>    -     Symantec/Norton AntiVirus filter driver
List - <unknown>    -     kernel utilities list allocation
LCam - <unknown>    - WDM mini video capture driver for Logitech camera
Lfs  - <unknown>    - Lfs allocations
LfsI - <unknown>    - Lfs allocations
LpcZ - <unknown>    - LPC Zone
LpcM - <unknown>    - Local procedure call message blocks
Lr?? - <unknown>    - LM redirector allocations
Lr   - <unknown>    -     Generic allocations
Lrcx - <unknown>    -     Context blocks of various types
Lrcl - <unknown>    -     ConnectListEntries
Lrsl - <unknown>    -     ServerListEntries
Lrse - <unknown>    -     Security entry
Lrsc - <unknown>    -     Search Control Blocks
Lrea - <unknown>    -     EA related allocations
Lric - <unknown>    -     Instance Control Blocks
Lrfc - <unknown>    -     File Control Blocks
Lrfl - <unknown>    -     Fcb Locks
Lrfp - <unknown>    -     Fcb Paging locks
Lrcn - <unknown>    -     Computer Name
Lrdn - <unknown>    -     Domain Name
Lr?? - <unknown>    -     Buffers used for FsControlFile APIs
Lrlc - <unknown>    -     Lock Control Blocks
Lrlb - <unknown>    -     Lock Control Block buffers
Lrnf - <unknown>    -     Non paged FCB
Lrnt - <unknown>    -     Non paged transport
Lrps - <unknown>    -     Paged security entry
Lrte - <unknown>    -     Transport event.
Lrxx - <unknown>    -     Transceive context blocks
Lr!! - <unknown>    -     Cancel request context blocks
Lrmt - <unknown>    -     MPX table
Lrme - <unknown>    -     MPX table entries
Lrsx - <unknown>    -     Send contexts
Lraw - <unknown>    -     Async write context
Lrwb - <unknown>    -     Write behind buffer header
Lrbb - <unknown>    -     Write behind buffer
Lrwq - <unknown>    -     Work queue item
Lrac - <unknown>    -     ACL for redirector
Lrds - <unknown>    -     Security Descriptor for redirector
Lrsm - <unknown>    -     SMB buffer
Lrds - <unknown>    -     Duplicated ansi string
Lrdu - <unknown>    -     Duplicated unicode string
Lxpt - <unknown>    -     Transport
Lrtc - <unknown>    -     Transport connection
Lrna - <unknown>    -     Netbios Addresses
Lrca - <unknown>    -     Temporary storage used in name canonicalization
Lr2x - <unknown>    -     Transact SMB context
Lrpt - <unknown>    -     Primary transport server list
Lrso - <unknown>    -     Operating system name
Lref - <unknown>    -     Reference history (debug only)
LS?? - <unknown>    - LM server allocations
LSac - <unknown>    -     BlockTypeAdminCheck
LSas - <unknown>    -     BlockTypeAdapterStatus
LSbf - <unknown>    -     buffer descriptor
LScd - <unknown>    -     comm device
LScn - <unknown>    -     connection
LSdb - <unknown>    -     data buffer
LSdc - <unknown>    -     BlockTypeDirCache
LSdi - <unknown>    -     BlockTypeDirectoryInfo
LSep - <unknown>    -     endpoint
LSfn - <unknown>    -     BlockTypeFSName
LSlf - <unknown>    -     LFCB
LSlr - <unknown>    -     BlockTypeLargeReadX
LSmf - <unknown>    -     MFCB
LSmi - <unknown>    -     BlockTypeMisc
LSnh - <unknown>    -     nonpaged block header
LSni - <unknown>    -     BlockTypeNameInfo
LSop - <unknown>    -     oplock break wait
LSpc - <unknown>    -     paged connection
LSpm - <unknown>    -     paged MFCB
LSpr - <unknown>    -     paged RFCB
LSrf - <unknown>    -     RFCB
LSsc - <unknown>    -     search(core)
LSsd - <unknown>    -     BlockTypeShareSecurityDescriptor
LSsf - <unknown>    -     BlockTypeDfs
LSsh - <unknown>    -     share
LSsp - <unknown>    -     search(core complete)
LSsr - <unknown>    -     search
LSss - <unknown>    -     session
LStb - <unknown>    -     table
LStc - <unknown>    -     tree connect
LSti - <unknown>    -     timer
LStr - <unknown>    -     transaction
LSvi - <unknown>    -     BlockTypeVolumeInformation
LSwi - <unknown>    -     initial work context
LSwn - <unknown>    -     normal work context
LSwq - <unknown>    -     BlockTypeWorkQueue
LSwr - <unknown>    -     raw work context
LSws - <unknown>    -     BlockTypeWorkContextSpecial
LSnd - <unknown>    - WDM mini sound driver for Logitech video camera
LXMK - <unknown>    - kernel mixer line driver (KMXL - looks like they got their tag backwards)

MXF  - <unknown>    - DirectMusic (MIDI Transform Filter)

MapP - <unknown>    - PNP map
Mapr - <unknown>    - arc firmware registry routines

McaC - hal.dll      - HAL MCA corrected Log
McaD - hal.dll      - HAL MCA Driver Log
McaK - hal.dll      - HAL MCA Kernel Log
McaP - hal.dll      - HAL MCA Log from previous boot
McaT - hal.dll      - HAL MCA temporary Log

MCAM - <unknown>    - WDM mini driver for Intel USB camera
MCDx - <unknown>    - OpenGL MCD server (mcdsrv32.dll) allocations
MCDd - <unknown>    - OpenGL MCD driver (embedded in a display driver like s3mvirge.dll)
Mdl  - <unknown>    - Io, Mdls
MdlP - <unknown>    - MDL per processor lookaside list pointers
MePr - <unknown>    - In-memory print buffer
Mm   - nt!mm        - general Mm Allocations
MmBk - nt!mm        - Mm banked sections
MmCa - nt!mm        - Mm control areas for mapped files
MmCd - nt!mm        - Mm fork clone descriptors
MmCh - nt!mm        - Mm fork clone headers
MmCi - nt!mm        - Mm control areas for images
MmCm - nt!mm        - Calls made to MmAllocateContiguousMemory
MmDb - nt!mm        - NtMapViewOfSection service
MmDT - nt!mm        - Mm debug
MmEx - nt!mm        - Mm events
MmHi - nt!mm        - Mm image entry - allocated per session
MmHn - nt!mm        - Mm sessionwide address name string entry
MmHv - nt!mm        - Mm sessionwide address entry
MmIn - nt!mm        - Mm inpaged io structures
MmLd - nt!mm        - Mm load module database
MmPg - nt!mm        - Mm page table pages at init time
MmRw - nt!mm        - Mm read write virtual memory buffers
MmSb - nt!mm        - Mm subsections
MmSe - nt!mm        - Mm secured VAD allocation
MmSg - nt!mm        - Mm segments
MmSt - nt!mm        - Mm section object prototype ptes
Mmdl - nt!mm        - Mm Mdls for flushes
Mmpp - nt!mm        - Mm prototype PTEs for pool
MmVd - nt!mm        - Mm virtual address descriptors for mapped views
MmVs - nt!mm        - Mm virtual address descriptors short form (private views)
Mmxx - nt!mm        - Mm temporary allocations
MmCx - nt!mm        - info for dynamic section extension
MmDm - nt!mm      - deferred MmUnlock entries
MmHt - nt!mm        -  session space PTE data
MmLn - nt!mm        -  temporary name buffer for driver loads
MmMl - nt!mm        -  physical memory range information
MmRl - nt!mm        -  temporary readlists for file prefetch
MmSP - nt!mm       -  SLIST entries for system PTE NB queues
MmSc - nt!mm       -   subsections used to map data files
MmSd - nt!mm       -  extended subsections used to map data files
MmSm - nt!mm       -  segments used to map data files
MmWe - nt!mm       - Work entries for writing out modified filesystem pages.
MmWw - nt!mm      - Write watch VAD info
Mmpv  - nt!mm      - Physical view VAD info
Mmww - nt!mm      - Write watch bitmap VAD info

MRXx - <unknown>    - Client side caching for SMB

Msdv - <unknown>    - WDM mini driver for IEEE 1394 DV Camera and VCR
MST? - <unknown>    - MSTEE (mstee.sys)
MSTa - <unknown>    -    associated stream header
MSTc - <unknown>    -    filer connection
MSTd - <unknown>    -    data format
MSTf - <unknown>    -    filter instance
MSTp - <unknown>    -    pin instance
MSTs - <unknown>    -    stream header

MuoC - <unknown>    - MOUSE_POOL_TAG Mouse Class Driver
MupI - <unknown>    - DFS file system driver IRP context allocation
MsFc - <unknown>    - Mailslot CCB, Client control block. Each client with an opened mailslot has one of these
MsFC - <unknown>    - Mailslot root CCB, A client control block for the top level mailslot directory
MsFd - <unknown>    - Mailslot data entry write buffer, This is writes buffered inside mailslots
MsFD - <unknown>    - Mailslot root DCB and its name buffer
MsFf - <unknown>    - Mailslot FCB, File control block, Service side block for each created mailslot.
MsFg - <unknown>    - Mailslot global resource
MsFn - <unknown>    - Mailslot temporary name buffer
MsFN - <unknown>    - Mailslot FCB name buffer, name for each created mailslot
MsFr - <unknown>    - Mailslot read buffer, buffer created for pended reads issued.
MsFt - <unknown>    - Mailslot query template, used for directory queries.
MsFw - <unknown>    - Mailslot work context block, blocks create when we need to timeout reads.

MsvC - <unknown>    - Msv/Ntlm client package
Mup  - <unknown>    - Multiple UNC provider allocations
Muta - <unknown>    - Mutant objects
NBF  - <unknown>    - general NBF allocations
NBFa - <unknown>    - NBF address object
NBFb - <unknown>    - NBF receive buffer
NBFc - <unknown>    - NBF connection object
NBFd - <unknown>    - NBF packet pool descriptor
NBFe - <unknown>    - NBF bind & export names
NBFf - <unknown>    - NBF address file object
NBFg - <unknown>    - NBF registry path name
NBFi - <unknown>    - NBF tdi connection info
NBFk - <unknown>    - NBF loopback buffer
NBFl - <unknown>    - NBF link object
NBFn - <unknown>    - NBF netbios name
NBFo - <unknown>    - NBF config data
NBFp - <unknown>    - NBF packet
NBFq - <unknown>    - NBF query buffer
NBFr - <unknown>    - NBF request
NBFs - <unknown>    - NBF provider stats
NBFt - <unknown>    - NBF connection table
NBFu - <unknown>    - NBF UI frame
NBFw - <unknown>    - NBF work item
NBI  - <unknown>    - NwlnkNb transport
NBS  - <unknown>    - general NetBIOS allocations
NBSa - <unknown>    -     address block
NBSc - <unknown>    -     connection block
NBSe - <unknown>    -     EA buffer
NBSf - <unknown>    -     FCB
NBSl - <unknown>    -     LANA block
NBSn - <unknown>    -     copy of user NCB
NBSr - <unknown>    -     registry allocations
NBSx - <unknown>    -     XNS NETONE address (connect block)
NBSy - <unknown>    -     NetBIOS address (connect block)
NBSz - <unknown>    -     NetBIOS address (listen block)
NBqh - <unknown>    -     Non-blocking queue entries used to carry the real data in the queue.

NCSt - <unknown>    - EXIFS NC

ND   - ndis.sys     - general NDIS allocations
NDDl - ndis.sys     - NDIS_TAG_DBG_LOG
NDMb - ndis.sys     - NDIS_TAG_MAC_BLOCK
NDPX - ndis.sys     -     NDIS Proxy allocations
NDPa - ndis.sys     - Apple Talk
NDPb - ndis.sys     - NBF
NDPi - ndis.sys     - NWLNKIPX
NDPn - ndis.sys     - NWLNKNB
NDPp - ndis.sys     - Packet Scheduler.
NDPs - ndis.sys     - NWLNKSPX
NDPt - ndis.sys     - TCPIP
NDPw - ndis.sys     - WAN_PACKET_TAG
NDam - ndis.sys     -     NdisAllocateMemory
NDan - ndis.sys     -     adapter name
NDco - ndis.sys     - NDIS_TAG_CO
NDd  - ndis.sys     - NDIS_TAG_DBG
NDdb - ndis.sys     -     DMA block
NDdl - ndis.sys     - NDIS_TAG_DBG_L
NDdp - ndis.sys     - NDIS_TAG_DBG_P
NDds - ndis.sys     - NDIS_TAG_DBG_S
NDdt - ndis.sys     - NDIS_TAG_DFRD_TMR
NDfa - ndis.sys     - NDIS_TAG_FILTER_ADDR
NDlb - ndis.sys     -     lookahead buffer
NDlp - ndis.sys     - NDIS_TAG_LOOP_PKT
NDmb - ndis.sys     -     MAC block
NDmr - ndis.sys     -     map register entry array
NDoa - ndis.sys     - NDIS_TAG_OID_ARRAY
NDob - ndis.sys     -     open block
NDpb - ndis.sys     -     protocol block
NDpf - ndis.sys     - NDIS_TAG_FILTER
NDpk - ndis.sys     - NDIS_TAG_PKT_PATTERN
NDpp - ndis.sys     -     packet pool
NDrl - ndis.sys     -     resource list
NDrp - ndis.sys     - NDIS_TAG_REGISTRY_PATH
NDrq - ndis.sys     - NDIS_TAG_Q_REQ
NDsi - ndis.sys     -     EISA slot information
NDsm - ndis.sys     -     Cached shared memory descriptor
NDst - ndis.sys     - NDIS_TAG_STRING
NDw0 - ndis.sys     - NDIS_TAG_WMI_REG_INFO
NDw1 - ndis.sys     - NDIS_TAG_WMI_GUID_TO_OID
NDw2 - ndis.sys     - NDIS_TAG_WMI_OID_SUPPORTED_LIST
NDw3 - ndis.sys     - NDIS_TAG_WMI_EVENT_ITEM
NDwi - ndis.sys     - NDIS_TAG_WORK_ITEM

Nb?? - <unknown>    -  NetBT allocations
Nls  - <unknown>    - Nls strings
Nmdd - <unknown>    - NetMeeting display driver miniport 1 MB block
None - <unknown>    - call to ExAllocatePool

Npf* - npfs.sys     - Npfs Allocations
NpFc - npfs.sys     - CCB, client control block
NpFC - npfs.sys     - ROOT_DCB CCB
NpFD - npfs.sys     - DCB, directory block
NpFg - npfs.sys     - Global storage
NpFi - npfs.sys     - NPFS client info buffer.
NpFn - npfs.sys     - Name block
NpFq - npfs.sys     - Query template buffer used for directory query
NpFr - npfs.sys     - DATA_ENTRY records (read/write buffers)
NpFs - npfs.sys     - Client security context
NpFw - npfs.sys     - Write block
NpFW - npfs.sys     - Write block


NS?? - <unknown>    - Netware server allocations

Ntf? - ntfs.sys     - NTFS Specific allocation tags
Ntf0 - ntfs.sys     -     general pool allocation
Ntf9 - ntfs.sys     -     Large Temporary Buffer
NtfC - ntfs.sys     -     CCB
Ntfc - ntfs.sys     -     CCB_DATA
NtfD - ntfs.sys     -     DEALLOCATED_RECORDS
NtfE - ntfs.sys     -     INDEX_CONTEXT
NtfF - ntfs.sys     -     FCB_INDEX
Ntff - ntfs.sys     -     FCB_DATA
NtfI - ntfs.sys     -     NTFS_IO_CONTEXT
Ntfi - ntfs.sys     -     IRP_CONTEXT
NtfK - ntfs.sys     -     KEVENT
Ntfk - ntfs.sys     -     FILE_LOCK
Ntfl - ntfs.sys     -     LCB
NtfM - ntfs.sys     -     NTFS_MCB_ENTRY
Ntfm - ntfs.sys     -     NTFS_MCB_ARRAY
NtfN - ntfs.sys     -     NUKEM
Ntfn - ntfs.sys     -     SCB_NONPAGED
Ntfo - ntfs.sys     -     SCB_INDEX normalized named buffer
NtfQ - ntfs.sys     -     QUOTA_CONTROL_BLOCK
Ntfq - ntfs.sys     -     General Allocation with Quota
NtfR - ntfs.sys     -     READ_AHEAD_THREAD
Ntfr - ntfs.sys     -     ERESOURCE
NtfS - ntfs.sys     -     SCB_INDEX
Ntfs - ntfs.sys     -     SCB_DATA
NtfT - ntfs.sys     -     SCB_SNAPSHOT
Ntft - ntfs.sys     -     SCB (Prerestart)
NtfV - ntfs.sys     -     VPB
Ntfv - ntfs.sys     -     COMPRESSION_SYNC
Ntfw - ntfs.sys     -     Workspace
Ntfx - ntfs.sys     -     General Allocation
NtF? - ntfs.sys     - NTFS tags based on source module
NtFa - ntfs.sys     -     AllocSup.c
NtFA - ntfs.sys     -     AttrSup.c
NtFB - ntfs.sys     -     BitmpSup.c
NtFC - ntfs.sys     -     Create.c
NtFD - ntfs.sys     -     DevioSup.c
NtFd - ntfs.sys     -     DirCtrl.c
NtFE - ntfs.sys     -     Ea.c
NtFF - ntfs.sys     -     FileInfo.c
NtFf - ntfs.sys     -     FsCtrl.c
NtFI - ntfs.sys     -     IndexSup.c
NtFL - ntfs.sys     -     LogSup.c
NtFM - ntfs.sys     -     McbSup.c
NtFN - ntfs.sys     -     NtfsData.c
NtFO - ntfs.sys     -     ObjIdSup.c
NtFQ - ntfs.sys     -     QuotaSup.c
NtFR - ntfs.sys     -     RestrSup.c
NtFS - ntfs.sys     -     SecurSup.c
NtFs - ntfs.sys     -     StrucSup.c
NtFU - ntfs.sys     -     usnsup.c
NtFV - ntfs.sys     -     VerfySup.c
NtFv - ntfs.sys     -     ViewSup.c
NtFW - ntfs.sys     -     Write.c

Nwcs - <unknown>    - Client Services for NetWare
NwFw - <unknown>    - ntos\tdi\fwd


ObSq - nt!ob        - object security descriptors (query)
ObCi - nt!ob        - captured information for ObCreateObject
ObCI - nt!ob        - object creation lookaside list
ObDi - nt!ob        - object directory
ObHd - nt!ob        - object handle count data base
ObNm - nt!ob        - object names
ObNM - nt!ob        - name buffer per processor lookaside pointers
ObZn - nt!ob        - object zone
ObjT - nt!ob        - object type objects
Obtb - nt!ob        - object tables via EX handle.c
ObTR - nt!ob        - object table ERESOURCEs
Obeb - nt!ob        - object tables extra bit tables via EX handle.c
ObDm - nt!ob        - object device map
ObSc - nt!ob        - Object security descriptor cache block
ObSt - nt!ob        - Object Manager temporary storage

OHCI - <unknown>    - Open Host Controller Interface for USB
ohci - <unknown>    - 1394 OHCI host controller driver

Ovfl - <unknown>    - The internal pool tag table has overflowed - usually this is a result of nontagged allocations being made

OvfL - <unknown>    - EXIFS FCBOVF List

PaeD - <unknown>    - PAE top level directory allocation blocks

ParC - <unknown>    - Parallel class driver
ParL - <unknown>    - Parallel link driver
ParP - <unknown>    - Parallel port driver
ParV - <unknown>    - ParVdm driver for vdm<->parallel port communciation

PciB - pci.sys      - PnP pci bus enumerator

PcCi - <unknown>    - WDM audio port class adapter device object stuff
PcCr - <unknown>    - WDM audio stuff
PcDi - <unknown>    - WDM audio stuff
PcDm - <unknown>    - DirectMusic MXF objects (WDM audio)
PcFM - <unknown>    - WDM audio FM synthesizer
PcFp - <unknown>    - WDM audio stuff
PcIc - <unknown>    - WDM audio stuff
PcIl - <unknown>    - WDM audio stuff
PcNw - <unknown>    - WDM audio stuff
PcPc - <unknown>    - WDM audio stuff
PcPr - <unknown>    - WDM audio stuff
PcSX - <unknown>    - WDM audio stuff
PcSl - <unknown>    - WDM audio stuff
PcSt - <unknown>    - WDM audio stuff
PcSx - <unknown>    - WDM audio stuff
PcUs - <unknown>    - WDM audio stuff

Pcmc - pcmcia.sys   - Pcmcia bus enumerator, general structures
Pcic - <unknown>    - Pcmcia bus enumerator, PCIC/Cardbus controller specific structures
Pcdb - <unknown>    - Pcmcia bus enumerator, Databook controller specific structures

Pgm? - <unknown>    - Pgm (Pragmatic General Multicast) protocol: RMCast.sys

Plcp - <unknown>    - Cache aware pushlock list (array of puchlock addresses)
Plcl - <unknown>    - Cache aware pushlock entry. One per processor
PNCH - <unknown>    - Power Notify Channel
PNCL - <unknown>    - Power Notify channel list
PNDP - <unknown>    - Power Abort Dpc Routine
PNI  - <unknown>    - Power Notify Instance

Pool - <unknown>    - Pool tables, etc.
PooL - <unknown>    - Phase 0 initialization of the executive component, paged and nonpaged small pool lookaside structures
Port - <unknown>    - Port objects

PoSL - <unknown>    - Power shutdown event list

Prof - <unknown>    - Profile objects

POWI - nt!po        - Power Work Item (executive worker thread work item entry)
PSwt - nt!po        - Power switch structure
PSTA - nt!po        - Po registered system state
PDss - nt!po        - Po device system state

Prcr - processr.sys - Processr hal driver allocations
Proc - <unknown>    - Process objects
Ps   - <unknown>    - general ps allocations
Psap - <unknown>    - Block used to hold a user mode APC while its queued to a thread
PsAp - <unknown>    - Process APC queued by user mode process
PsCa - <unknown>    - APC queued at thread create time.
PsCr - <unknown>    - Working set change record, should be a temporary allocation
PsEx - <unknown>    - Process exit APC
PsIm - <unknown>    - Thread impersonation (PS_IMPERSONATE_INFORMATION)
Psjb - <unknown>    - Job set array, should be a temporary allocation
PsLd - <unknown>    - Process LDT information blocks
PsQb - <unknown>    - Process quota block
PsTf - <unknown>    - Job object token filter
Pstb - <unknown>    - Process tables via EX handle.c
Psta - <unknown>    - Power management system state
PsTp - <unknown>    - Thread termination port block
PsWs - <unknown>    - Process working set watch array

PSE3 - pse36.sys    - Physical Size Extension driver

PnPb - nt!pnp       - PnP BIOS resource manipulation
PpEB - nt!pnp       - PNP_POOL_EVENT_BUFFER
PpEE - nt!pnp       - PNP_DEVICE_EVENT_ENTRY_TAG
PpEL - nt!pnp       - PNP_DEVICE_EVENT_LIST_TAG
PpLg - nt!pnp       - PnP last good.
PpUB - nt!pnp       - PNP_USER_BLOCK_TAG
PpWI - nt!pnp       - PNP_DEVICE_WORK_ITEM_TAG
Ppcd - nt!pnp       - PnP critical device database
Ppcr - nt!pnp       - plug-and-play critical allocations
Ppdd - nt!pnp       - new Plug-And-Play driver entries and IRPs
Ppde - nt!pnp       - routines to perform device removal
Ppei - nt!pnp       - Eisa related code
Ppen - nt!pnp       - routines to perform device enumeration
Ppin - nt!pnp       - plug-and-play initialization
Ppio - nt!pnp       - plug-and-play IO system APIs
Ppre - nt!pnp       - resource allocation and translation
Pprl - nt!pnp       - routines to manipulate relations list
Ppsu - nt!pnp       - plug-and-play subroutines for the I/O system

PPTP - <unknown>    - PPTP_MEMORYPOOL_TAG
PPT0 - <unknown>    - PPTP_TDIADDR_TAG
PPT1 - <unknown>    - PPTP_TDICONN_TAG
PPT2 - <unknown>    - PPTP_CONNINFO_TAG
PPT3 - <unknown>    - PPTP_ADDRINFO_TAG
PPT4 - <unknown>    - PPTP_TIMEOUT_TAG
PPT5 - <unknown>    - PPTP_TIMER_TAG
PPT6 - <unknown>    - PPTP_TDICOTS_TAG
PPT7 - <unknown>    - PPTP_WRKQUEUE_TAG
PPT8 - <unknown>    - PPTP_SEND_CTRLDATA_TAG
PPT9 - <unknown>    - PPTP_SEND_ACKDATA_TAG
PPTa - <unknown>    - PPTP_SEND_DGRAMDESC_TAG
PPTb - <unknown>    - PPTP_TDICLTS_TAG
PPTc - <unknown>    - PPTP_RECV_CTRLDESC_TAG
PPTd - <unknown>    - PPTP_RECV_CTRLDATA_TAG
PPTe - <unknown>    - PPTP_RECV_DGRAMDESC_TAG
PPTf - <unknown>    - PPTP_RECV_DGRAMDATA_TAG
PPTg - <unknown>    - PPTP_RECVDESC_TAG
PPTh - <unknown>    - PPTP_ENGINE_TAG
PPTi - <unknown>    - PPTP_RECVDATA_TAG

PSC? - <unknown>    - Packet Scheduler (PSCHED) Tags

PSC0 - <unknown>    - NDIS Request
PSC1 - <unknown>    - GPC Client Vc
PSC2 - <unknown>    - WanLink
PSC3 - <unknown>    - Miscellaneous allocations
PSC4 - <unknown>    - WMI
PSCa - <unknown>    - Adapter
PSCb - <unknown>    - CallParameters
PSCc - <unknown>    - PipeContext
PSCd - <unknown>    - FlowContext
PSCe - <unknown>    - ClassMapContext
PSCf - <unknown>    - Adapter Profile
PSCg - <unknown>    - Component

PX1  - <unknown>    - ndis ProviderEventLookaside

p2?? - perm2dll.dll - Permedia2 display driver
p2d3 - perm2dll.dll - Permedia2 display driver - d3d.c
p2d6 - perm2dll.dll - Permedia2 display driver - d3ddx6.c
p2de - perm2dll.dll - Permedia2 display driver - debug.c
p2ds - perm2dll.dll - Permedia2 display driver - d3dstate.c
p2dt - perm2dll.dll - Permedia2 display driver - d3dtxman.c
p2su - perm2dll.dll - Permedia2 display driver - ddsurf.c
p2en - perm2dll.dll - Permedia2 display driver - enable.c
p2fi - perm2dll.dll - Permedia2 display driver - fillpath.c
p2he - perm2dll.dll - Permedia2 display driver - heap.c
p2hw - perm2dll.dll - Permedia2 display driver - hwinit.c
p2cx - perm2dll.dll - Permedia2 display driver - p2ctxt.c
p2pa - perm2dll.dll - Permedia2 display driver - palette.c
p2pe - perm2dll.dll - Permedia2 display driver - permedia.c
p2tx - perm2dll.dll - Permedia2 display driver - textout.c

P3D? - perm3dd.dll  - Permedia3 display driver - DirectDraw/3D
P3G? - perm3dd.dll  - Permedia3 display driver

Qnam - <unknown>    - EXIFS Query Name

Qp?? - <unknown>    - Generic Packet Classifier (MSGPC)
Qppn - <unknown>    -      Queued Notifications
Qppi - <unknown>    -      Pending Irp structures
Qpci - <unknown>    -      CfInfo
Qpct - <unknown>    -      Client blocks
Qppa - <unknown>    -      Pattern blocks
Qphf - <unknown>    -      HandleFactory
Qpph - <unknown>    -      PathHash
Qprz - <unknown>    -      Rhizome
Qppd - <unknown>    -      GenPatternDb
Qpfd - <unknown>    -      FragmentDb
Qpcf - <unknown>    -      ClassificationFamily
Qpcd - <unknown>    -      CfInfoData
Qpcb - <unknown>    -      ClassificationBlock
Qppt - <unknown>    -      Protocol
Qpdg - <unknown>    -      Debug

RB?? - <unknown>    - RedBook Filter Driver, static allocations
RBEv - <unknown>    - RedBook - Thread Events
RBRl - <unknown>    - RedBook - Remove lock
RBRg - <unknown>    - RedBook - driverExtension->RegistryPath
RBSe - <unknown>    - RedBook - Serialization tracking for checked builds
RBWa - <unknown>    - RedBook - Wait block for system thread

rb?? - <unknown>    - RedBook Filter Driver, dynamic allocations
rbBu - <unknown>    - RedBook - Buffer for read/stream
rbIr - <unknown>    - RedBook - Irp for read/stream
rbIp - <unknown>    - RedBook - Irp pointer block
rbMd - <unknown>    - RedBook - Mdl for read/stream
rbMp - <unknown>    - RedBook - Mdl pointer block
rbRc - <unknown>    - RedBook - Read completion context
rbRx - <unknown>    - RedBook - Read Xtra info
rbSc - <unknown>    - RedBook - Stream completion context
rbSx - <unknown>    - RedBook - Stream Xtra info
rbTo - <unknown>    - RedBook - Cached table of contents

Rcp? - sacdrv.sys - SAC Driver (Headless)
RcpA - sacdrv.sys -     Internal memory mgr alloc block
RcpI - sacdrv.sys -     Internal memory mgr initial heap block
RcpS - sacdrv.sys -     Security related block

ReEv - <unknown>    - Resource Event
ReSe - <unknown>    - Resource Semaphore
ReTa - <unknown>    - Resource Extended Table

RefT - <unknown>    - Bluetooth reference tracking
Rf?? - <unknown>    - Bluetooth RFCOMM TDI driver
RfAD - rfcomm.sys   -   RFCOMM Address
RfBB - rfcomm.sys   -   RFCOMM BRB
RfBT - rfcomm.sys   -   RFCOMM (bthport)
RfCB - rfcomm.sys   -   RCOMMM
RfCH - rfcomm.sys   -   RFCOMM channel
RfCN - rfcomm.sys   -   RFCOMM connect
RfDA - rfcomm.sys   -   RFCOMM data
RfFR - rfcomm.sys   -   RFCOMM frame
RfRX - rfcomm.sys   -   RFCOMM receive
RfPP - rfcomm.sys   -   RFCOMM pnp
RfWR - rfcomm.sys   -   RFCOMM worker

RRle - <unknown>    - RTL_RANGE_LIST_ENTRY_TAG
RRlm - <unknown>    - RTL_RANGE_LIST_MISC_TAG

RtPi - <unknown>    - Temp allocation for product type key

RPrt - rstorprt.sys - Remote Storage Port Driver

Rqrv - <unknown>    - Registry query buffer
RS?? - <unknown>    - Remote Storage
RSFS - <unknown>    -      Recall Queue
RSFN - <unknown>    -      File Name
RSSE - <unknown>    -      Security info
RSWQ - <unknown>    -      Work Queue
RSQI - <unknown>    -      Queue info
RSLT - <unknown>    -      Long term data
RSIO - <unknown>    -      Ioctl Queue
RSFO - <unknown>    -      File Obj queue
RSVO - <unknown>    -      Validate Queue
RSER - <unknown>    -      Error log data
RWan - rawwan.sys   - Raw WAN driver

Rx?? - <unknown>    - Rdr2 rdbss.sys stuff
RxSc - <unknown>    -         SrvCall
RxNr - <unknown>    -         NetRoot
RxVn - <unknown>    -         VNetRoot
RxFc - <unknown>    -         FCB
RxSo - <unknown>    -         SrvOpen
RxFx - <unknown>    -         Fobx
RxNf - <unknown>    -         nonpaged FCB
RxWq - <unknown>    -         work queue
RxBm - <unknown>    -         buffering manager
RxMs - <unknown>    -         misc.
RxIr - <unknown>    -         RxContext (IrpContext)
RxMx - <unknown>    -         minirdr dispatch table
RxNc - <unknown>    -         NameCache (not currently used)

RxCt - <unknown>    -         connection engine transport
RxCa - <unknown>    -         connection engine address
RxCc - <unknown>    -         connection engine connection
RxCv - <unknown>    -         connection engine VC
RxCd - <unknown>    -         connection engine TDI

RxSp - <unknown>    -         srvcall calldown params (special build only)
RxVp - <unknown>    -         vnetroot calldown params (special build only)
RxTm - <unknown>    -         timer context (special build only)
RxDc - <unknown>    -         querydirectory pattern (special build only)
RxCx - <unknown>    -         connection engine misc. (special build only)

S3   - <unknown>    - S3 video driver
SBad - <unknown>    - bad block simulator - simbad.c

sbp2 - <unknown>    - Sbp2 1394 storage port driver

SC?? - <unknown>    - Smart card driver tags
SCLb - <unknown>    -  Smart card driver library
SCB8 - <unknown>    -  Bull CP8 Transac serial reader
SCB3 - <unknown>    -  Bull SmarTlp PnP
SCS4 - <unknown>    -  SCM Microsystems pcmcia reader
SCl0 - <unknown>    -  Litronic 220

Sc?? - <unknown>    - Mass storage driver tags

ScB? - classpnp.sys - ClassPnP misc allocations
ScC? - classpnp.sys - ClassPnP misc allocations

ScC? - <unknown>    -   CdRom
ScCE - <unknown>    -      device control synch event
ScCG - <unknown>    -      disk geometry buffer
ScCH - <unknown>    -      hitachi error buffer
ScCI - <unknown>    -      sense info buffers
ScCS - <unknown>    -      srb allocation
ScCM - <unknown>    -      mode data buffer
ScCP - <unknown>    -      read capacity buffer
ScCp - <unknown>    -      play active checks
ScCQ - <unknown>    -      read sub q buffer
ScCR - <unknown>    -      raw mode read buffer
ScCT - <unknown>    -      read toc buffer
ScCt - <unknown>    -      toshiba error buffer
ScCU - <unknown>    -      update capacity path
ScCV - <unknown>    -      volume control buffer
ScCv - <unknown>    -      volume control buffer

ScD? - <unknown>    -   Disk
ScD  - <unknown>    -      generic tag
ScDa - <unknown>    -      SMART
ScDA - <unknown>    -      Info Exceptions
ScDC - <unknown>    -      disable cache paths
ScDb - classpnp.sys -      ClassPnP debug globals buffer
ScDc - <unknown>    -      disk allocated completion c
ScDG - <unknown>    -      disk geometry buffer
ScDg - <unknown>    -      update disk geometry paths
ScDI - <unknown>    -      sense info buffers
ScDp - <unknown>    -      pnp ids
ScDM - <unknown>    -      mode data buffer
ScDM - <unknown>    -      mbr checksum code
ScDN - <unknown>    -      disk name code
ScDP - <unknown>    -      read capacity buffer
ScDp - <unknown>    -      disk partition lists
ScDS - <unknown>    -      srb allocation
ScDs - <unknown>    -      start device paths
ScDU - <unknown>    -      update capacity path
ScDW - <unknown>    -      work-item context
ScMC - <unknown>    -      medium changer allocations

ScF? - <unknown>    -   FtDisk
ScFt - <unknown>    -      FtDisk allocations

ScIO - classpnp.sys - ClassPnP device control
ScL? - classpnp.sys -   Classpnp
ScLA - classpnp.sys -      allocation to check for autorun disable
ScLF - classpnp.sys -      File Object Extension
ScLc - classpnp.sys -      Cache filters
ScLf - classpnp.sys -      Fault prediction
ScLm - classpnp.sys -      Mount
ScLM - classpnp.sys -      Media Change Detection
ScLq - classpnp.sys -      Release queue
ScLw - classpnp.sys -      WMI
ScLW - classpnp.sys -      Power

ScNo - classpnp.sys - ClassPnP notification

ScP? - <unknown>    -   Scsiport
ScPa - <unknown>    -      Hold registry data
ScPA - <unknown>    -      Access Ranges
ScPb - <unknown>    -      Get Bus Dat Holder
ScPB - <unknown>    -      Queuetag BitMap
ScPc - <unknown>    -      Fake common buffer
ScPC - <unknown>    -      reset bus code
ScPd - <unknown>    -      Pnp id strings
ScPD - <unknown>    -      SRB_DATA allocations
ScPE - <unknown>    -      Scatter gather lists
ScPG - <unknown>    -      Global memory
ScPh - <unknown>    -      HwDevice Ext
ScPi - <unknown>    -      Sense Info
ScPI - <unknown>    -      Init data chain
ScPl - <unknown>    -      remove lock tracking
ScPL - <unknown>    -      scatter gather lists
ScPm - <unknown>    -      address mapping lists
ScPM - <unknown>    -      scatter gather lists
ScPp - <unknown>    -      device & adapter enable
ScpP - <unknown>    -      scsi PortConfig copies
ScPq - <unknown>    -      inquiry data
ScPQ - <unknown>    -      request sense
ScPr - <unknown>    -      resource list copy
ScPS - <unknown>    -      registry allocations
ScPt - <unknown>    -      legacy request rerouting
ScPT - <unknown>    -      interface mapping
ScPu - <unknown>    -      device relation structs
ScPv - <unknown>    -      KEVENT
ScPV - <unknown>    -      Device map allocations
ScPw - <unknown>    -      Wmi Events
ScPW - <unknown>    -      Wmi Requests
ScPx - <unknown>    -      Report Luns
ScPY - <unknown>    -      Report Targets
ScPZ - <unknown>    -      Device name buffer

ScR? - <unknown>    -   Partition Manager
ScRi - <unknown>    -      IOCTL buffer
ScRp - <unknown>    -      Partition entry
ScRr - <unknown>    -      Remove lock
ScRt - <unknown>    -      Table entry
ScRv - <unknown>    -      Dependant volume relations lists
ScRV - <unknown>    -      Volume entry
ScRw - <unknown>    -      Power mgmt private work item

ScsC - <unknown>    - non-pnp SCSI CdRom
ScsD - <unknown>    - non-pnp SCSI Disk
ScsH - <unknown>    - non-pnp SCSI from class.h (class2)
ScsI - <unknown>    - non-pnp SCSI port internal
ScsL - <unknown>    - non-pnp SCSI class.c driver allocations
ScsP - <unknown>    - non-pnp SCSI port.c
Scs$ - <unknown>    - Tag for pnp class driver's SRB lookaside list

ScUn - <unknown>    - Default Tag for pnp class driver allocations

ScV? - <unknown>    -  Dvd functionality in cdrom.sys
ScVk - <unknown>    -      read buffer for DVD keys
ScVK - <unknown>    -      write buffer for DVD keys
ScVS - <unknown>    -      buffer for reads of DVD on-disk structures

SdCc - <unknown>    - ObsSecurityDescriptorCache / SECURITY_DESCRIPTOR_CACHE_ENTRIES

Sdba - <unknown>    - Application compatibility Sdb* allocations

Sdp? - <unknown>    - Bluetooth SDP functionality in BTHPORT.sys
SdpC - bthport.sys  -     Bluetooth SDP client connection
SdpD - bthport.sys  -     Bluetooth SDP database
SdpI - bthport.sys  -     Bluetooth port driver (SDP)

Se   - nt!se        - General security allocations
SeAc - nt!se        - Security ACL
SeAi - nt!se        - Security Audit Work Item
SeAp - nt!se        - Security Audit Parameter Record
SeCL - nt!se        - Security CONTEXT_TAG
SeDb - nt!se        - Temp directory query buffer to delete logon session symbolic links. 
SeFS - nt!se        - SEP_FILE_SYSTEM_NOTIFY_CONTEXT
SeHa - nt!se        - Temp directory query buffer to delete logon session symbolic links. 
SeLs - nt!se        - Security Logon Session
SeLu - nt!se        - Security LUID and Attributes array
SeLS - nt!se        - Security Logon Session tracking array
SeLw - nt!se        - Security LSA Work Item
SeOn - nt!se        - Security Captured Object Name information
SeOt - nt!se        - Captured object type array. Used by access check code
SePa - nt!se        - Process audit image names and captured polity structures
SePr - nt!se        - Security Privilege Set
SeSa - nt!se        - Security SID and Attributes
SeSc - nt!se        - Captured Security Descriptor
SeSd - nt!se        - Security Descriptor
SeSi - nt!se        - Security SID
SeTa - nt!se        - Security Temporary Array
SeTd - nt!se        - Security Token dynamic part
SeTn - nt!se        - Security Captured Type Name information
SeUs - nt!se        - Security Captured Unicode string

Sect - <unknown>    - Section objects
Sema - <unknown>    - Semaphore objects
Senm - <unknown>    - Serenum (RS-232 serial bus enumerator)
SimB - <unknown>    - Simbad (bad sector simulation driver) allocations
SIfs - <unknown>    - Default tag for user's of ntsrv.h
sidg - <unknown>    - GDI spooler events
Sis  - <unknown>    - Single Instance Store (dd\sis\filter)
SisB - <unknown>    -         SIS per file object break event
SisC - <unknown>    -         SIS common store file object
SisF - <unknown>    -         SIS per file object
SisL - <unknown>    -         SIS per link object
SisS - <unknown>    -         SIS SCB
Setp - <unknown>    - SETUPDD SpMemAlloc calls

SRdm - scsirdma.sys - Infiniband SRP driver

SW?? - <unknown>    - Software Bus Enumerator
SWbi - <unknown>    -         bus ID
SWbr - <unknown>    -         bus reference
SWda - <unknown>    -         POOLTAG_DEVICE_ASSOCIATION
SWdn - <unknown>    -         device name
SWdr - <unknown>    -         device reference
SWdr - <unknown>    -         POOLTAG_DEVICE_DRIVER_REGISTRY
SWfd - <unknown>    -         POOLTAG_DEVICE_FDOEXTENSION
SWid - <unknown>    -         device ID
SWii - <unknown>    -         instance ID
SWip - <unknown>    -         POOLTAG_DEVICE_INTERFACEPATH
SWki - <unknown>    -         key information
SWpd - <unknown>    -         POOLTAG_DEVICE_PDOEXTENSION
SWre - <unknown>    -         relations
SWrp - <unknown>    -         reparse string
SWrs - <unknown>    -         reference string

Sm?? - <unknown>    - rdr2 smb mini
SmCe - <unknown>    -         smbmini connection engine
SmMm - <unknown>    -         smbmini managed datastructures
SmAd - <unknown>    -         session setup/admin exchange
SmRw - <unknown>    -         read/write path
SmTr - <unknown>    -         transact exchange
SmMs - <unknown>    -         misc.
SmTp - <unknown>    -         server transport tag
SmRb - <unknown>    -         remote boot

SmFc - <unknown>    -         fsctl structures  (special build only)
SmDc - <unknown>    -         dir query buffer (special build only)
SmPi - <unknown>    -         pipeinfo buffer (special build only)
SmDO - <unknown>    -         deferred open context  (special build only)
SmQP - <unknown>    -         params for directory query transact  (special build only)
SmRx - <unknown>    -         minirdr generated RxContext  (special build only)

SmVr - <unknown>    -         VNetroot  (special build only)
SmSr - <unknown>    -         Server  (special build only)
SmSe - <unknown>    -         Session  (special build only)
SmNr - <unknown>    -         NetRoot  (special build only)
SmMa - <unknown>    -         mid atlas  (special build only)
SmMt - <unknown>    -         mailslot buffer  (special build only)
SmVc - <unknown>    -         smbmini connectionengine VC  (special build only)
SmEc - <unknown>    -         echo buffer  (special build only)
SmKs - <unknown>    -         Kerberos blob  (special build only)

SPX  - <unknown>    - Nwlnkspx transport
SQOS - <unknown>    - Security quality of service in IO

Sr?? - sr.sys       - System Restore file system filter driver
SrCo - <unknown>    -         SR's control object
SrSC - <unknown>    -         Stream contexts
SrDB - <unknown>    -         Debug information for lookup blob
SrDL - <unknown>    -         Device list
SrFE - <unknown>    -         File information buffer
SrFN - <unknown>    -         File name
SrHB - <unknown>    -         Hash bucket
SrHH - <unknown>    -         Hash header
SrHK - <unknown>    -         Hash key
SrLB - <unknown>    -         Log buffer
SrLC - <unknown>    -         Logging context
SrLE - <unknown>    -         Log entry
SrLT - <unknown>    -         Lookup blob
SrMP - <unknown>    -         Mount point information
SrOI - <unknown>    -         Overwrite information
SrPC - <unknown>    -         Persistant configuration information
SrRB - <unknown>    -         Rename buffer
SrRG - <unknown>    -         Logger context
SrRH - <unknown>    -         Reparse data size
SrRR - <unknown>    -         Registry information
SrSD - <unknown>    -         Security data information
SrST - <unknown>    -         Stream data information
SrTI - <unknown>    -         Directory delete information
SrWI - <unknown>    -         Work queue item

ST*  - <unknown>    - New MMC compliant storage drivers

Stac - <unknown>    - Stack Trace Database - i386 checked and built with NTNOFPO=1 only
Strg - <unknown>    - Dynamic Translated strings
Strm - <unknown>    - Streams and streams transports allocations
SwMi - <unknown>    - SWMidi KS filter (WDM Audio)
Symb - <unknown>    - Symbolic link objects
Symt - <unknown>    - Symbolic link target strings

SYSA - <unknown>    - Sysaudio (wdm audio)

TAPI - <unknown>    - ntos\ndis\ndistapi

TC?? - TCP          - TCP/IP network protocol
TCh? - <unknown>    -     TCP/IP header pools
TCPC - <unknown>    -     TCP connection pool
TCPT - <unknown>    -     TCB pool
TCPY - <unknown>    -     SYN-TCB pool
TCPr - <unknown>    -     TCP request pool

TDIc - <unknown>    - TDI resource
TDId - <unknown>    - TDI resource
TDIe - <unknown>    - TDI resource
TDIf - <unknown>    - TDI resource
TDIg - <unknown>    - TDI resource
TDIk - <unknown>    - TDI resource
TDIu - <unknown>    - TDI resource
TDIv - <unknown>    - TDI resource

thdd - <unknown>    - DirectDraw/3D handle manager table

Thre - nt!ps        - Thread objects
Time - nt!ke        - Timer objects
Toke - nt!se        - Token objects

Tran - <unknown>    - EXIFS Translate

TSBV - <unknown>    - WDM mini driver for Toshiba 750 capture
TSdd - <unknown>    - RDPDD - Hydra Display Driver
TSmc - <unknown>    - PDMCS - Hydra MCS Protocol Driver
TSwd - <unknown>    - RDPWD - Hydra Winstation Driver

TTFC - <unknown>    - Font file cache
Ttfd - <unknown>    - TrueType Font driver

Thrm - <unknown>    - Thermal zone structure
Tun4  - <unknown>   - Tunnel cache allocation for long file name
TunL - <unknown>    - Tunnel cache lookaside-allocated elements
TunP - <unknown>    - Tunnel cache oddsized pool-allocated elements
TunK - <unknown>    - Tunnel cache temporary key value

Type - <unknown>    - Type objects

UCAM - <unknown>    - USB digital camera library
UDFV - <unknown>    - Udfs volume descriptor sequence descriptor buffer

Udf1 - <unknown>    - Udfs file set descriptor buffer
Udf2 - <unknown>    - Udfs volmume recognition sequence descriptor buffer
Udf2 - <unknown>    - Udfs volume descriptor sequence descriptor buffer
UdfC - <unknown>    - Udfs CRC table
UdfD - <unknown>    - Udfs FID buffer for view spanning
UdfF - <unknown>    - Udfs nonpaged Fcb errata
UdfI - <unknown>    - Udfs IO context
UdfL - <unknown>    - Udfs IRP context lite (delayed close)
UdfS - <unknown>    - Udfs short file name
UdfT - <unknown>    - Udfs generic table entry
Udfb - <unknown>    - Udfs IO buffer
Udfc - <unknown>    - Udfs IRP context
Udfd - <unknown>    - Udfs file Fcb
Udfe - <unknown>    - Udfs enumeration match expression
Udff - <unknown>    - Udfs normalized full filename
Udfi - <unknown>    - Udfs directory Fcb
Udfl - <unknown>    - Udfs Lcb
Udfp - <unknown>    - Udfs Pcb
Udfs - <unknown>    - Udfs Sparing Mcb
Udft - <unknown>    - Udfs CDROM TOC
Udfv - <unknown>    - Udfs Vpb
Udfx - <unknown>    - Udfs Ccb

Udp  - <unknown>    - Udp protocol (TCP/IP driver)
Ufsc - <unknown>    - User FULLSCREEN
UHCD - <unknown>    - Universal Host Controller (USB - Intel Controller)
UHUB - <unknown>    - Universal Serial Bus Hub

Ul?? - http.sys     - tags. Note: In-use tags are of the form "Ul??" or "Uc??".and   Free tags are of the form "uL??" or "uC??";
Uc?? - http.sys     - i.e., the case of the leading "Ul" or "Uc" has been reversed.

Ucac - http.sys     - Auth Cache Pool
UcCO - http.sys     - Client Connection
Ucmp - http.sys     - Multipart String Buffer
Ucre - http.sys     - Receive Response
Ucrp - http.sys     - Response App Buffer
Ucrq - http.sys     - Request Pool
UcSc - http.sys     - Common Server Information
UcSN - http.sys     - Server name
UcSP - http.sys     - Process Server Information
UcSp - http.sys     - Sspi Pool
UcST - http.sys     - Server info table
Uctd - http.sys     - Response Tdi Buffer
Ucte - http.sys     - Entity Pool
Ucto - http.sys     - Tdi Objects Pool
UlAB - http.sys     - Auxiliary Buffer
UlAO - http.sys     - App Pool Object
UlAP - http.sys     - App Pool Process
UlBL - http.sys     - Binary Log File Entry
UlBO - http.sys     - Outstanding i/o
UlCC - http.sys     - Control Channel
UlCE - http.sys     - Config Group Tree Entry
UlCH - http.sys     - Config Group Tree Header
UlCI - http.sys     - Config Group URL Info
UlCJ - http.sys     - Config Group Object Pool
UlCK - http.sys     - Chunk Tracker
UlCL - http.sys     - Config Group LogDir
UlCl - http.sys     - Connection RefTraceLog
UlCO - http.sys     - Connection
UlCT - http.sys     - Config Group Timestamp
UlCY - http.sys     - Connection Count Entry
UlDB - http.sys     - Debug
UlDC - http.sys     - Data Chunks array
UlDO - http.sys     - Disconnect Object
UlDR - http.sys     - Deferred Remove Item
UlDT - http.sys     - Debug Thread Pool
UlEP - http.sys     - Endpoint
UlFA - http.sys     - Force Abort Work Item
UlFC - http.sys     - File Cache Entry
Ulfc - http.sys     - Uri Filter Context
UlFD - http.sys     - Noncached File Data
UlFP - http.sys     - Filter Process
UlFR - http.sys     - Dummy Filter Receive Buffer
UlFT - http.sys     - Filter Channel
UlFU - http.sys     - Full Tracker
UlFW - http.sys     - Filter Write Tracker
UlHC - http.sys     - Http Connection
UlHc - http.sys     - Http Connection RefTraceLog
UlHL - http.sys     - Internal Request RefTraceLog
UlHR - http.sys     - Internal Request
UlHT - http.sys     - Hash Table
UlHV - http.sys     - Header Value
UlIC - http.sys     - Irp Context
UlID - http.sys     - Conn ID Table
UlIR - http.sys     - Internal Response
UlLD - http.sys     - Log Field
UlLF - http.sys     - Log File Entry
UlLG - http.sys     - Log Generic
UlLH - http.sys     - Log File Handle
UlLL - http.sys     - Log File Buffer
UlLS - http.sys     - Ansi Log Data Buffer
UlLT - http.sys     - Binary Log Data Buffer
UlNO - http.sys     - NSGO Pool
UlNP - http.sys     - Non-Paged Data
UlOE - http.sys     - Endpoint OwnerRefTraceLog PoolTag
UlOR - http.sys     - Owner RefTrace Signature
UlOT - http.sys     - Opaque ID Table
UlPB - http.sys     - APool Proc Binding
UlPL - http.sys     - Pipeline
UlQF - http.sys     - TCI Filter
UlQG - http.sys     - TCI Generic
UlQI - http.sys     - TCI Interface
UlQL - http.sys     - TCI Flow
UlQT - http.sys     - TCI Tracker
UlQW - http.sys     - TCI WMI
UlRB - http.sys     - Receive Buffer
UlRD - http.sys     - Registry Data
UlRE - http.sys     - Request Body Buffer
UlRP - http.sys     - Request Buffer
UlRR - http.sys     - Request Buffer References
UlRS - http.sys     - Non-Paged Resource
UlRT - http.sys     - RefTraceLog PoolTag
UlSC - http.sys     - SSL Cert Data
UlSD - http.sys     - Security Data
UlSl - http.sys     - StringLog Buffer PoolTag
UlSL - http.sys     - StringLog PoolTag
UlSO - http.sys     - Site Counter Entry
UlSS - http.sys     - Simple Status Item
UlTA - http.sys     - Address Pool
UlTD - http.sys     - UL_TRANSPORT_ADDRESS
UlTT - http.sys     - Thread Tracker
UlUB - http.sys     - URL Buffer
UlUC - http.sys     - Uri Cache Entry
UlUH - http.sys     - HTTP Unknown Header
UlUL - http.sys     - URL
UlUM - http.sys     - URL Map
UlVH - http.sys     - Virtual Host
UlWC - http.sys     - Work Context
UlWI - http.sys     - Work Item

UndP - <unknown>    - EXIFS Underlying Path

Urdr - win32k!SetRedirectionBitmap          - REDIRECT
Usac - win32k!_CreateAcceleratorTable       - ACCEL
Usai - win32k!zzzAttachThreadInput          - ATTACHINFO
Usal - win32k!InitSwitchWndInfo             - ALTTAB
USBB - bthusb.sys                           - Bluetooth USB minidriver
USBD - <unknown>                            - Universal Serial Bus Class Driver
Usbg - win32k!xxxLogClipData                - DEBUG
Uscb - win32k!_ConvertMemHandle             - CLIPBOARD
Uscc - win32k!AllocCallbackMessage          - CALLBACK
Usci - win32k!InitSystemThread              - CLIENTTHREADINFO
Uscl - win32k!ClassAlloc                    - CLASS
Uscm - win32k!InitScancodeMap               - SCANCODEMAP
Uscp - win32k!CreateDIBPalette              - CLIPBOARDPALETTE
Uscr - win32k!NtUserSetSysColors            - COLORS
Usct - win32k!CkptRestore                   - CHECKPT
Uscu - win32k!_CreateEmptyCursorObject      - CURSOR
Uscv - win32k!NtUserSetSysColors            - COLORVALUES
Usd1 - win32k!FreeListAdd                   - DDE1
Usd2 - win32k!_DdeSetQualityOfService       - DDE2
Usd4 - win32k!AddPublicObject               - DDE4
Usd5 - win32k!xxxCsEvent                    - DDE5
Usd6 - win32k!xxxCsEvent                    - DDE6
Usd7 - win32k!xxxCsEvent                    - DDE7
Usd8 - win32k!xxxMessageEvent               - DDE8
Usd9 - win32k!xxxCsDdeInitialize            - DDE9
UsdA - win32k!NewConversation               - DDEa
UsdB - win32k!Createpxs                     - DDEb
UsdD - win32k!NtUserfnDDEINIT               - DDEd
UsdE - win32k!xxxClientCopyDDEIn1           - DDE
Usdc - win32k!CreateCacheDC                 - DCE
Usdi - win32k!CreateMonitor             - DISPLAYINFO
UsDI - win32k!CreateDeviceInfo              - DEVICEINFO
Usds - win32k!xxxDragObject                 - DRAGDROP
Usdv - win32k!NtUserfnINDEVICECHANGE        - DEVICECHANGE
User - win32k!InitCreateUserCrit            - ERESOURCE
Usev - win32k!xxxPollAndWaitForSingleObject - EVENT
Usgh - win32k!NtUserUserHandleGrantAccess   - GRANTEDHANDLES
Usgl - win32k!CreateShadowTL                - GLOBALTHREADLOCK
Usgt - win32k!AddGhost                      - GHOST
Usha - win32k!AllocateHidData               - HIDDATA
UshD - Win32k!AllocateHidDesc               - HIDDESC
Ushk - win32k!_RegisterHotKey               - HOTKEY
Usih - win32k!SetImeHotKey                  - IMEHOTKEY
Usim - win32k!CreateInputContext            - IME
Usjb - win32k!CreateW32Job                  - W32JOB
Usjx - win32k!JobCalloutAddProcess          - W32JOBEXTRA
Uskb - win32k!xxxLoadKeyboardLayoutEx       - KBDLAYOUT
Uske - win32k!GetKbdExId                    - KBDEXID
Uskf - win32k!LoadKeyboardLayoutFile        - KBDFILE
Usks - win32k!PostUpdateKeyStateEvent       - KBDSTATE
Uskt - win32k!ReadLayoutFile                - KBDTABLE
Usla - win32k!InitLockRecordLookaside       - LOOKASIDE
Usld - win32k!GrowLogIfNecessary            - LOGDESKTOP
Uslr - win32k!InitLockRecordLookaside       - LOCKRECORD
Usmi - win32k!MirrorRegion                  - MIRROR
Usmr - win32k!SnapshotMonitorRects          - MONITORRECTS
Usms - win32k!xxxMoveSize                   - MOVESIZE
Usmt - win32k!xxxMNAllocMenuState           - MENUSTATE
Usno - win32k!zzz                           - NONE - don't use
Usny - win32k!CreateNotify                  - NOTIFY
Uspi - win32k!MapDesktop                    - PROCESSINFO
Uspm - win32k!MNAllocPopup                  - POPUPMENU
Uspn - win32k!CreateProfileUserName         - PROFILEUSERNAME
Uspo - win32k!QueuePowerRequest             - POWER
Uspp - win32k!AllocateAndLinkHidTLCInfo     - PNP
Uspr - win32k!GetPrivateProfileStruct       - PROFILE
Usqm - win32k!InitQEntryLookaside           - QMSG
Usqu - win32k!InitQEntryLookaside           - Q
Usrt - win32k!xxxDrawMenuItemText           - RTL
Ussa - win32k!xxxBroadcastMessage           - SMS_ASYNC
Ussb - win32k!CreateSpb                     - SPB
Ussd - win32k!xxxAddShadow                  - SHADOW
Ussc - win32k!xxxInterSendMsgEx             - SMS_CAPTURE
Usse - win32k!SetDisconnectDesktopSecurity  - SECURITY
Ussi - win32k!NtUserSendInput               - SENDINPUT
Ussm - win32k!InitSMSLookaside              - SMS
Usss - win32k!xxxBroadcastMessage           - SMS_STRING
Usst - win32k!xxxSBTrackInit                - SCROLLTRACK
Ussw - win32k!_BeginDeferWindowPos          - SWP
Ussy - win32k!xxxDesktopThread              - SYSTEM
Ustd - win32k!TrackAddDesktop               - TRACKDESKTOP
Usti - win32k!AllocateW32Thread             - THREADINFO
Ustk - win32k!HeavyAllocPool                - STACK
Ustm - win32k!InternalSetTimer              - TIMER
Usto - win32k!xxxConnectService             - TOKEN
Usts - win32k!_Win32CreateSection           - SECTION
Ustx - win32k!NtUserDrawCaptionTemp         - TEXT
Usty - win32k!NtUserResolveDesktopForWOW    - TEXT2
Usub - win32k!NtUserToUnicodeEx             - UNICODEBUFFER
Usvi - win32k!ResizeVisExcludeMemory        - VISRGN
Usvl - win32k!VWPLAdd                       - VWPL
Uswd - win32k!xxxCreateWindowEx             - WINDOW
Uswe - win32k!_SetWinEventHook              - WINEVENT
Uswl - win32k!BuildHwndList                 - WINDOWLIST
Uswo - win32k!zzzInitTask                   - WOWTDB
Uswp - win32k!xxxRegisterUserHungAppHandlers - WOWPROCESSINFO
Uswt - win32k!xxxUserNotifyProcessCreate    - WOWTHREADINFO

Vad  - nt!mm        - Mm virtual address descriptors
VadF - nt!mm        - VADs created by a FreeVM splitting
Vadl - nt!mm        - Mm virtual address descriptors (long)
VadS - nt!mm        - Mm virtual address descriptors (short)
VidL - videoprt.sys - VideoPort Allocation List (FDO_EXTENSION)
VidR - videoprt.sys - VideoPort Allocation on behalf of Miniport
VDM  - nt!vdm       - ntos\vdm

VoS? - volsnap.sys  -  VolSnap (Volume Snapshot Driver)
VoSa - volsnap.sys  -      Application information allocations
VoSb - volsnap.sys  -      Buffer allocations
VoSc - volsnap.sys  -      Snapshot context allocations
VoSd - volsnap.sys  -      Diff area volume allocations
VoSf - volsnap.sys  -      Diff area file allocations
VoSh - volsnap.sys  -      Bit history allocations
VoSi - volsnap.sys  -      Io status block allocations
VoSm - volsnap.sys  -      Bitmap allocations
VoSo - volsnap.sys  -      Old heap entry allocations
VoSp - volsnap.sys  -      Pnp id allocations
VoSr - volsnap.sys  -      Device relations allocations
VoSs - volsnap.sys  -      Short term allocations
VoSt - volsnap.sys  -      Temp table allocations
VoSw - volsnap.sys  -      Work queue allocations
VoSx - volsnap.sys  -      Dispatch context allocations

Vpb  - <unknown>    - Io, vpb's

Vprt - <unknown>    - ntos\video\port
VraP - <unknown>    - parallel class driver
Vtfd - <unknown>    - Font file/context

Wait - <unknown>    - NtWaitForMultipleObjects

Wan? - <unknown>    - NdisWan Tags (PPP Framing module for Remote Access)
WanA - <unknown>    - BundleCB
WanB - <unknown>    - ProtocolCB/LinkCB
WanC - <unknown>    - DataDesc
WanD - <unknown>    - WanRequest
WanE - <unknown>    - LoopbackDesc
WanG - <unknown>    - MiniportCB
WanH - <unknown>    - OpenCB
WanI - <unknown>    - IoPacket
WanJ - <unknown>    - LineUpInfo
WanK - <unknown>    - Unicode String Buffer
WanL - <unknown>    - Protocol Table
WanM - <unknown>    - Connection Table
WanN - <unknown>    - NdisPacketPool Desc
WanQ - <unknown>    - DataBuffer
WanR - <unknown>    - WanPacket
WanS - <unknown>    - AfCB/SapCB/VcCB
WanT - <unknown>    - Transform Driver
WanV - <unknown>    - RC4 Encryption Context
WanW - <unknown>    - SHA Encryption
WanX - <unknown>    - Send Compression Context
WanY - <unknown>    - Recv Compression Context
WanZ - <unknown>    - Protocol Specific Info

Wdm  - <unknown>    - WDM
WDMA - <unknown>    - WDM Audio

Wdog - watchdog.sys - Watchdog object/context allocation

Wmii - <unknown>    - Wmi InstId chunks
Wmij - <unknown>    - Wmi GuidMaps
Wmim - <unknown>    - Wmi KM to UM Notification Buffers
Wmin - <unknown>    - Wmi Notification Slot Chunks
Wmip - <unknown>    - Wmi General purpose allocation
Wmis - <unknown>    - Wmi SysId allocations
Wmit - <unknown>    - Wmi Trace
Wmiw - <unknown>    - Wmi Notification Waiting Buffers, in paged queue waiting for IOCTL
Wmiz - <unknown>    - Wmi MCA Insertions debug code

WmiA - <unknown>    - Wmi ACPI mapper
WmiC - <unknown>    - Wmi Create Pump Thread Work Item
WmiD - <unknown>    - Wmi Registration DataSouce
WmiG - <unknown>    - Allocation of WMIGUID
WmiI - <unknown>    - Wmi Instance Names
WmiL - <unknown>    - WmiLIb
WmiN - <unknown>    - Wmi Notification Notification Packet
WmiR - <unknown>    - Wmi Registration info blocks

WmDS - <unknown>    - Wmi DataSource chunks
WmGE - <unknown>    - Wmi GuidEntry chunks
WmIS - <unknown>    - Wmi InstanceSet chunks
WmMR - <unknown>    - Wmi MofResouce chunks

WMca - <unknown>    - WMI MCA Handling

Wrp? - <unknown>    - WanArp Tags (ARP module for Remote Access)

Wrpa - <unknown>    - WAN_ADAPTER_TAG
Wrpi - <unknown>    - WAN_INTERFACE_TAG
Wrpr - <unknown>    - WAN_REQUEST_TAG
Wrps - <unknown>    - WAN_STRING_TAG
Wrpc - <unknown>    - WAN_CONN_TAG
Wrpw - <unknown>    - WAN_PACKET_TAG
Wrpf - <unknown>    - FREE_TAG (checked builds only)
Wrpd - <unknown>    - WAN_DATA_TAG
Wrph - <unknown>    - WAN_HEADER_TAG
Wrpn - <unknown>    - WAN_NOTIFICATION_TAG

WvCy - <unknown>    - WDM Audio WaveCyc port
WvPc - <unknown>    - WDM Audio WavePCI port

xCVD - mrxdav.sys - AsyncEngineContext Tag
xSMB - smbmrx.sys - IFSKIT sample SMB mini-redirector

XWan - <unknown>    - ndis\usrwan allocations

Xtra - <unknown>    - EXIFS Extra Create

ysd8 - win32k.sys   - User DDE8
ysd9 - win32k.sys   - User DDE9
ysda - win32k.sys   - User DDEa
ysdb - win32k.sys   - User DDEb
ysdc - win32k.sys   - User DDEc
ysdd - win32k.sys   - User DDEd

ZsaB - <unknown>    - EXIFS ZeroBlock

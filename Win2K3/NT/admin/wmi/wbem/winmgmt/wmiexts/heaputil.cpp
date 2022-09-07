/*++

Copyright (c) 2000-2001  Microsoft Corporation

Module Name:

    heaputil.cpp
    
Revision History:

    ivanbrug     oct 2000 created

--*/

#include <wmiexts.h>
#include <utilfun.h>
#include <malloc.h>

/*
#if defined(_WIN64)
#define HEAP_GRANULARITY_SHIFT      4   // Log2( HEAP_GRANULARITY )
#else
#define HEAP_GRANULARITY_SHIFT      3   // Log2( HEAP_GRANULARITY )
#endif

#define HEAP_ENTRY_BUSY             0x01
#define HEAP_ENTRY_EXTRA_PRESENT    0x02
#define HEAP_ENTRY_FILL_PATTERN     0x04
#define HEAP_ENTRY_VIRTUAL_ALLOC    0x08
#define HEAP_ENTRY_LAST_ENTRY       0x10
#define HEAP_ENTRY_SETTABLE_FLAG1   0x20
#define HEAP_ENTRY_SETTABLE_FLAG2   0x40
#define HEAP_ENTRY_SETTABLE_FLAG3   0x80
#define HEAP_ENTRY_SETTABLE_FLAGS   0xE0


typedef struct _HEAP_ENTRY {

    //
    //  This field gives the size of the current block in allocation
    //  granularity units.  (i.e. Size << HEAP_GRANULARITY_SHIFT
    //  equals the size in bytes).
    //
    //  Except if this is part of a virtual alloc block then this
    //  value is the difference between the commit size in the virtual
    //  alloc entry and the what the user asked for.
    //

    USHORT Size;

    //
    // This field gives the size of the previous block in allocation
    // granularity units. (i.e. PreviousSize << HEAP_GRANULARITY_SHIFT
    // equals the size of the previous block in bytes).
    //

    USHORT PreviousSize;

    //
    // This field contains the index into the segment that controls
    // the memory for this block.
    //

    UCHAR SegmentIndex;

    //
    // This field contains various flag bits associated with this block.
    // Currently these are:
    //
    //  0x01 - HEAP_ENTRY_BUSY
    //  0x02 - HEAP_ENTRY_EXTRA_PRESENT
    //  0x04 - HEAP_ENTRY_FILL_PATTERN
    //  0x08 - HEAP_ENTRY_VIRTUAL_ALLOC
    //  0x10 - HEAP_ENTRY_LAST_ENTRY
    //  0x20 - HEAP_ENTRY_SETTABLE_FLAG1
    //  0x40 - HEAP_ENTRY_SETTABLE_FLAG2
    //  0x80 - HEAP_ENTRY_SETTABLE_FLAG3
    //

    UCHAR Flags;

    //
    // This field contains the number of unused bytes at the end of this
    // block that were not actually allocated.  Used to compute exact
    // size requested prior to rounding requested size to allocation
    // granularity.  Also used for tail checking purposes.
    //

    UCHAR UnusedBytes;

    //
    // Small (8 bit) tag indexes can go here.
    //

    UCHAR SmallTagIndex;

#if defined(_WIN64)
    ULONGLONG Reserved1;
#endif

} HEAP_ENTRY, *PHEAP_ENTRY;

typedef struct _HEAP_UNCOMMMTTED_RANGE {
    struct _HEAP_UNCOMMMTTED_RANGE *Next;
    ULONG_PTR Address;
    SIZE_T Size;
    ULONG filler;
} HEAP_UNCOMMMTTED_RANGE, *PHEAP_UNCOMMMTTED_RANGE;


typedef struct _HEAP_SEGMENT {
    HEAP_ENTRY Entry;

    ULONG Signature;
    ULONG Flags;
    struct _HEAP *Heap;
    SIZE_T LargestUnCommittedRange;

    PVOID BaseAddress;
    ULONG NumberOfPages;
    PHEAP_ENTRY FirstEntry;
    PHEAP_ENTRY LastValidEntry;

    ULONG NumberOfUnCommittedPages;
    ULONG NumberOfUnCommittedRanges;
    PHEAP_UNCOMMMTTED_RANGE UnCommittedRanges;
    USHORT AllocatorBackTraceIndex;
    USHORT Reserved;
    PHEAP_ENTRY LastEntryInSegment;
} HEAP_SEGMENT, *PHEAP_SEGMENT;
*/


typedef ULONG_PTR ERESOURCE_THREAD;
typedef ERESOURCE_THREAD *PERESOURCE_THREAD;

typedef struct _OWNER_ENTRY {
    ERESOURCE_THREAD OwnerThread;
    union {
        LONG OwnerCount;
        ULONG TableSize;
    };

} OWNER_ENTRY, *POWNER_ENTRY;


typedef void * PKSEMAPHORE;
typedef void * PKEVENT;

typedef struct _ERESOURCE {
    LIST_ENTRY SystemResourcesList;
    POWNER_ENTRY OwnerTable;
    SHORT ActiveCount;
    USHORT Flag;
    PKSEMAPHORE SharedWaiters;
    PKEVENT ExclusiveWaiters;
    OWNER_ENTRY OwnerThreads[2];
    ULONG ContentionCount;
    USHORT NumberOfSharedWaiters;
    USHORT NumberOfExclusiveWaiters;
    union {
        PVOID Address;
        ULONG_PTR CreatorBackTraceIndex;
    };

    KSPIN_LOCK SpinLock;
} ERESOURCE, *PERESOURCE;

//typedef void * PRTL_TRACE_BLOCK;

#include "heap.h"
#include "heappagi.h"

#define HE_VERBOSITY_FLAGS   1
#define HE_VERBOSITY_NUMERIC 2
#define HE_VERBOSITY_VTABLE  4

#if defined(_X86_)
    #ifndef PAGE_SIZE
    #define PAGE_SIZE   0x1000
    #endif
    #define USER_ALIGNMENT 8

#elif defined(_IA64_)
    #ifndef PAGE_SIZE
    #define PAGE_SIZE   0x2000
    #endif
    #define USER_ALIGNMENT 16

#elif defined(_AMD64_)
    #ifndef PAGE_SIZE
    #define PAGE_SIZE   0x1000
    #endif
    #define USER_ALIGNMENT 16

#else
    #error  // platform not defined
#endif


//
//
//
//
//

typedef DWORD (__stdcall * fnCallBack)(ULONG_PTR pParam1,ULONG_PTR pParam2);

DWORD g_UsedInHeap = 0;

void
PrintHeapEntry(HEAP_ENTRY * pEntry,void * pAddr)
{

    BYTE varEntry[sizeof(HEAP_ENTRY)+2*sizeof(void *)];
    LIST_ENTRY * pListEntry = (LIST_ENTRY *)((HEAP_ENTRY *)varEntry+1);

    DWORD PrintSize = 0;
    BOOL bIsPossiblePageHeap = FALSE;
    if (pEntry->Flags & HEAP_ENTRY_BUSY)
    {
        // re-read the entry, to get if it's on the LookAside
        if (ReadMemory((MEMORY_ADDRESS)pAddr,varEntry,sizeof(varEntry),NULL))
        {
#ifdef WIN64            
            if (0xf0f0f0f0f0f0f0f0 == (ULONG_PTR)pListEntry->Blink )
#else
            if (0xf0f0f0f0 == (ULONG_PTR)pListEntry->Blink )
#endif          
            {
                PrintSize = 0xf7eef7ee;
            }
            else 
            {
                PrintSize = (pEntry->Size<<HEAP_GRANULARITY_SHIFT)-pEntry->UnusedBytes;
                g_UsedInHeap += PrintSize;
            }

            DWORD Sign = *((DWORD *)pListEntry);
            //dprintf("Sign %08x\n",Sign);
            if (0xabcdaaaa == Sign)
            {
                bIsPossiblePageHeap = TRUE;
            }            
        }
        else
        {
            PrintSize = (pEntry->Size<<HEAP_GRANULARITY_SHIFT)-pEntry->UnusedBytes;
            g_UsedInHeap += PrintSize;            
        }
    }
    else
    {
        PrintSize = 0xf7eef7ee;
    }

    dprintf("      %p: %04x . %04x [%02x] - (%x)\n",
            pAddr,pEntry->Size,pEntry->PreviousSize,pEntry->Flags,PrintSize);

    if (bIsPossiblePageHeap)
    {
        //dprintf("Possible %p\n",(MEMORY_ADDRESS)((BYTE*)pAddr+sizeof(HEAP_ENTRY)+sizeof(DPH_BLOCK_INFORMATION)));
        GetVTable((MEMORY_ADDRESS)((BYTE*)pAddr+sizeof(HEAP_ENTRY)+sizeof(DPH_BLOCK_INFORMATION)));
    }
    else
        GetVTable((MEMORY_ADDRESS)((BYTE*)pAddr+sizeof(HEAP_ENTRY)));    
};

//
//
// print the HEAP_ENTRY structure
//

DECLARE_API(he) {

    INIT_API();

    DEFINE_CPP_VAR( HEAP_ENTRY, varHEAP_ENTRY);
    HEAP_ENTRY * pEntry = GET_CPP_VAR_PTR( HEAP_ENTRY , varHEAP_ENTRY );
    memset(pEntry,0xfe,sizeof(HEAP_ENTRY));
    
    MEMORY_ADDRESS pByte = GetExpression(args);
    
    if (pByte)
    {
        if (ReadMemory((MEMORY_ADDRESS)pByte,pEntry ,sizeof(HEAP_ENTRY),NULL))
        {
            PrintHeapEntry(pEntry,(void *)pByte);
        }
        else
        {
            dprintf("RM %p\n",pByte);
        }
    } else {
        dprintf("invalid HEAP_ENTRY address %s\n",args);
    }
}

//
// HEAP_ENTRY list
// finds the beginning of the "list" of HEAP_ENTRYs
//

DECLARE_API(heb) {

    INIT_API();
    
    MEMORY_ADDRESS pEntry = GetExpression(args);
    
    if (pEntry){
       HEAP_ENTRY HeapEntry;
       ReadMemory((MEMORY_ADDRESS)pEntry,&HeapEntry,sizeof(HeapEntry),0);

       PrintHeapEntry(&HeapEntry,(void *)pEntry);

       while (HeapEntry.PreviousSize)
       {
           pEntry = (MEMORY_ADDRESS)((HEAP_ENTRY*)pEntry - HeapEntry.PreviousSize);
           if (ReadMemory((MEMORY_ADDRESS)pEntry,&HeapEntry,sizeof(HeapEntry),0))
           {
               PrintHeapEntry(&HeapEntry,(void *)pEntry);
           }
           else
           {
               dprintf("RM %p\n",pEntry);
               break;
           }

           if (CheckControlC())
               break;           
       }

       dprintf(" begin %08x\n",pEntry);
    } else {
        dprintf("invalid address %s\n",args);
    };

}

//
//
//  HEAP_ENTRY forward
//  
//////////////////////////////////////////////////////

DECLARE_API(hef) {

    INIT_API();

    DWORD BeginNum=0;

    MEMORY_ADDRESS pEntry = GetExpression(args);

    if (pEntry)
    {
       HEAP_ENTRY HeapEntry;
       ReadMemory((MEMORY_ADDRESS)pEntry,&HeapEntry,sizeof(HeapEntry),0);

       PrintHeapEntry(&HeapEntry,(void *)pEntry);

       while (!(HeapEntry.Flags & HEAP_ENTRY_LAST_ENTRY))
       {
           pEntry = (MEMORY_ADDRESS)((HEAP_ENTRY*)pEntry + HeapEntry.Size);
           if (ReadMemory((MEMORY_ADDRESS)pEntry,&HeapEntry,sizeof(HeapEntry),0))
           {
                 PrintHeapEntry(&HeapEntry,(void *)pEntry);
           }
           else
           {
               dprintf("RM %p\n",pEntry);
               break;
           }
                      
           if (CheckControlC())
               break;
           
       }

       dprintf(" end %08x\n",pEntry);
    } else {
        dprintf("invalid address %s\n",args);
    };

}

DWORD EnumEntries(HEAP_ENTRY * pEntry,DWORD * pSize,fnCallBack CallBack,ULONG_PTR Addr){

       DWORD i=0;
       HEAP_ENTRY HeapEntry;
       DWORD Size=0;
       ReadMemory((MEMORY_ADDRESS)pEntry,&HeapEntry,sizeof(HeapEntry),0);

       if (CallBack)
       {
           CallBack((ULONG_PTR)pEntry,Addr);
       }
       else
       {
           PrintHeapEntry(&HeapEntry,pEntry);
       };

       while (!(HeapEntry.Flags & HEAP_ENTRY_LAST_ENTRY))
       {
           if (0 == HeapEntry.Size)
           {
               dprintf("HEAP_ENTRY %p with zero Size\n",pEntry);
               break;
           }
           pEntry = (HEAP_ENTRY*)pEntry + HeapEntry.Size;
           if (ReadMemory((MEMORY_ADDRESS)pEntry,&HeapEntry,sizeof(HeapEntry),0))
           {
               if (CallBack)
               {
                   CallBack((ULONG_PTR)pEntry,Addr);
               }
               else
               {
                   PrintHeapEntry(&HeapEntry,pEntry);
               };
           }
           else
           {
               dprintf("RM %p\n",pEntry);
               break;
           }

           i++;
           Size += ((HeapEntry.Size<<HEAP_GRANULARITY_SHIFT)-HeapEntry.UnusedBytes);

           if (CheckControlC())
               break;
           
       }

    if (pSize){
        *pSize = Size;
    }

    return i;
}

void
PrintHEAP_SEGMENT(HEAP_SEGMENT * pSeg_OOP, 
                  fnCallBack CallBack, 
                  ULONG_PTR Addr)
{
    DEFINE_CPP_VAR( HEAP_SEGMENT, varHEAP_SEGMENT);
    HEAP_SEGMENT * pSeg = GET_CPP_VAR_PTR( HEAP_SEGMENT , varHEAP_SEGMENT );

    BOOL bRet = ReadMemory((MEMORY_ADDRESS)pSeg_OOP,pSeg ,sizeof(HEAP_SEGMENT),0);

    if (bRet)
    {
        if (!CallBack)
            dprintf("    Flags %08x HEAP %p\n",pSeg->Flags,pSeg->Heap);

        //SIZE_T LargestUnCommittedRange;

        //PVOID BaseAddress;
        //ULONG NumberOfPages;
        //PHEAP_ENTRY FirstEntry;
        //PHEAP_ENTRY LastValidEntry;

        //ULONG NumberOfUnCommittedPages;
            DWORD unComm = pSeg->NumberOfUnCommittedRanges;
        //PHEAP_UNCOMMMTTED_RANGE UnCommittedRanges;
        //USHORT AllocatorBackTraceIndex;
        //USHORT Reserved;
        //PHEAP_ENTRY LastEntryInSegment;

        HEAP_UNCOMMMTTED_RANGE UncRange;   
        HEAP_UNCOMMMTTED_RANGE * pUncRange = pSeg->UnCommittedRanges;
        HEAP_ENTRY ** pCommRange = (HEAP_ENTRY **)_alloca(sizeof(HEAP_ENTRY *)*(unComm+1));

        DWORD Size=0;
        DWORD Num=0;

        pCommRange[0] = (HEAP_ENTRY *)pSeg->FirstEntry;

        Num = EnumEntries(pCommRange[0],&Size,CallBack,Addr);

        if (!CallBack)
            dprintf("    - %p Size %p entries %d \n",pCommRange[0],Size,Num);
            
        for (DWORD i=0; i<unComm; i++)
        {

            
            bRet = ReadMemory((MEMORY_ADDRESS)pUncRange,&UncRange,sizeof(UncRange),NULL);
            if (bRet)
            {
                pUncRange = UncRange.Next;                
                pCommRange[1+i] = (HEAP_ENTRY *)(UncRange.Address + UncRange.Size);

                if (NULL == pUncRange)
                {
                    if ((ULONG_PTR)pCommRange[1+i] == (ULONG_PTR)pSeg->LastValidEntry)
                        break;
                }
               
                Num = EnumEntries(pCommRange[1+i],&Size,CallBack,Addr);

                if (!CallBack)
                    dprintf("    - %p Size %p entries %d\n",pCommRange[1+i],Size,Num);
            }
            else
            {
                dprintf("RM %p\n",pUncRange);
            }
        }
    } else {
        dprintf("RM %p\n",pSeg_OOP);
    }

}

//
//
// Dump the HEAP_SEGMENT
//

DECLARE_API(hs) {

    INIT_API();
        
    MEMORY_ADDRESS pByte = 0;
    pByte = GetExpression(args);
    
    if (pByte)
    {
        PrintHEAP_SEGMENT((HEAP_SEGMENT *)pByte,NULL,NULL);
    } 
    else 
    {
        dprintf("invalid address %s\n",args);
    }
}

//
// Define heap lookaside list allocation functions.
//

struct SLIST_HEADER_ 
{
    SLIST_HEADER_ * Next;
    ULONG_PTR       Align;
};

typedef struct _HEAP_LOOKASIDE {
    SLIST_HEADER_ ListHead;

    USHORT Depth;
    USHORT MaximumDepth;

    ULONG TotalAllocates;
    ULONG AllocateMisses;
    ULONG TotalFrees;
    ULONG FreeMisses;

    ULONG LastTotalAllocates;
    ULONG LastAllocateMisses;

    ULONG Counters[2];
#ifdef _IA64_
    DWORD Pad[3];
#else
    DWORD Pad;
#endif

} HEAP_LOOKASIDE, *PHEAP_LOOKASIDE;



void Dump_LookAside(ULONG_PTR pLookAside_OOP)
{
    DWORD dwSize = sizeof(HEAP_LOOKASIDE) * HEAP_MAXIMUM_FREELISTS ;

    BYTE * pData = new BYTE[dwSize];

    if (pData)
    {
        if (ReadMemory(pLookAside_OOP,pData,dwSize,NULL))
        {
            DWORD i;
            HEAP_LOOKASIDE * pLookasideArray = (HEAP_LOOKASIDE *)pData;

            
            BYTE varHEAP_ENTRY[sizeof(HEAP_ENTRY)+sizeof(SLIST_HEADER_)];
            HEAP_ENTRY * pEntry = GET_CPP_VAR_PTR( HEAP_ENTRY , varHEAP_ENTRY );
            SLIST_HEADER_ * pSListEntry = (SLIST_HEADER_ *)(pEntry+1);

            char Fill[8];
            memset(Fill,0xf0,8);
            
            for(i=0;i<HEAP_MAXIMUM_FREELISTS;i++)
            {                
                ULONG_PTR pTmp;                
                SLIST_HEADER_ * pHead_OOP = pLookasideArray[i].ListHead.Next;
#ifdef _IA64_                               
                pTmp = (ULONG_PTR)pHead_OOP;
                pTmp>>=25;
                pTmp<<=4;
                pHead_OOP = (SLIST_HEADER_ *)pTmp;
#endif
                
                dprintf("    LookAside[%x] - %p Depth %x Maximum %x\n",
                       i,
                       pHead_OOP, //pLookasideArray[i].ListHead.Next,
                       pLookasideArray[i].Depth,
                       pLookasideArray[i].MaximumDepth);
                
                //dprintf("size %x %p\n",sizeof(HEAP_LOOKASIDE),pHead_OOP);
                USHORT Depth = 0;
                while(pHead_OOP)
                {
                       HEAP_ENTRY * pEntry_OOP = (HEAP_ENTRY *)pHead_OOP-1;

                       if (ReadMemory((MEMORY_ADDRESS)pEntry_OOP,pEntry,sizeof(varHEAP_ENTRY),NULL))
                       {
                           ULONG_PTR pToWrite = sizeof(ULONG_PTR) + (ULONG_PTR)pHead_OOP;
                           WriteMemory(pToWrite,Fill,sizeof(ULONG_PTR),0);

                           pHead_OOP = pSListEntry->Next; 
                           PrintHeapEntry(pEntry,pEntry_OOP);
                       }
                       else
                       {
                           dprintf("RM %d\n",GetLastError());
                            break;
                       }
                       Depth++;
                       if (Depth > pLookasideArray[i].MaximumDepth)
                       {
                            dprintf("MaximumDepth exceeded\n");
                            break;
                       }
                };
            }
        }
        else
        {
            dprintf("RM %d\n",GetLastError());
        }
        delete [] pData;
    }

}

#define HEAP_FRONT_LOOKASIDE        1
#define HEAP_FRONT_LOWFRAGHEAP      2

//
// prepares the Lookaside list for dump
//
DECLARE_API(lhp) 
{
    INIT_API();

    DEFINE_CPP_VAR( HEAP, varHEAP);
    HEAP * pHeap = GET_CPP_VAR_PTR( HEAP , varHEAP );    
    MEMORY_ADDRESS pByte = GetExpression(args);
    DWORD i;

    if (pByte)
    {     
        if (ReadMemory(pByte,pHeap ,sizeof(HEAP),NULL))
        {

            //dprintf("-----  LookAside %p\n",pHeap->FrontEndHeap);
            if (1 == pHeap->FrontEndHeapType) 
                Dump_LookAside((ULONG_PTR)pHeap->FrontEndHeap);
            else
                dprintf("_HEAP FrontEndHeapType %d not recognized \n",pHeap->FrontEndHeapType);
        }
        else
        {
            dprintf("RM %p %d\n",pByte,GetLastError());
        }
    }
    else
    {
        dprintf("invalid heap address %s\n",args);
    }
}

//
//  Low fragmentation heap data structures
//

typedef struct _BLOCK_ENTRY : HEAP_ENTRY 
{
    USHORT LinkOffset;
    USHORT Reserved2;

} BLOCK_ENTRY, *PBLOCK_ENTRY;



typedef struct _INTERLOCK_SEQ {

    union {

        struct {
            
            union {

                struct {

                    USHORT Depth;
                    USHORT FreeEntryOffset;
                };
                volatile ULONG OffsetAndDepth;
            };
            volatile ULONG  Sequence;
        };

        volatile LONGLONG Exchg;
    };

} INTERLOCK_SEQ, *PINTERLOCK_SEQ;

struct _HEAP_USERDATA_HEADER;

typedef struct _HEAP_SUBSEGMENT {
    
    PVOID Bucket;
    
    volatile struct _HEAP_USERDATA_HEADER * UserBlocks;
    
    INTERLOCK_SEQ AggregateExchg;

    union {

        struct {
            USHORT BlockSize;
            USHORT FreeThreshold;
            USHORT BlockCount;
            UCHAR  SizeIndex;
            UCHAR  AffinityIndex;
        };

        ULONG Alignment[2];
    };
    
    SINGLE_LIST_ENTRY SFreeListEntry;
    volatile ULONG Lock;

} HEAP_SUBSEGMENT, *PHEAP_SUBSEGMENT;

typedef struct _HEAP_USERDATA_HEADER {

    union {
        
        SINGLE_LIST_ENTRY SFreeListEntry;
        PHEAP_SUBSEGMENT SubSegment;
    };

    PVOID HeapHandle;

    ULONG_PTR SizeIndex;
    ULONG_PTR Signature;

} HEAP_USERDATA_HEADER, *PHEAP_USERDATA_HEADER;


typedef union _HEAP_BUCKET_COUNTERS{

    struct {
        
        volatile ULONG  TotalBlocks;
        volatile ULONG  SubSegmentCounts;
    };

    volatile LONGLONG Aggregate64;

} HEAP_BUCKET_COUNTERS, *PHEAP_BUCKET_COUNTERS;

//
//  The HEAP_BUCKET structure handles same size allocations 
//

typedef struct _HEAP_BUCKET {

    HEAP_BUCKET_COUNTERS Counters;

    USHORT BlockUnits;
    UCHAR SizeIndex;
    UCHAR UseAffinity;
    
    LONG Conversions;

} HEAP_BUCKET, *PHEAP_BUCKET;

//
//  LFH heap uses zones to allocate sub-segment descriptors. This will preallocate 
//  a large block and then for each individual sub-segment request will move the 
//  water mark pointer with a non-blocking operation
//

typedef struct _LFH_BLOCK_ZONE {

    LIST_ENTRY ListEntry;
    PVOID      FreePointer;
    PVOID      Limit;

} LFH_BLOCK_ZONE, *PLFH_BLOCK_ZONE;

#define FREE_CACHE_SIZE  16

typedef struct _HEAP_LOCAL_SEGMENT_INFO {

    PHEAP_SUBSEGMENT Hint;
    PHEAP_SUBSEGMENT ActiveSubsegment;

    PHEAP_SUBSEGMENT CachedItems[ FREE_CACHE_SIZE ];
    SLIST_HEADER SListHeader;

    SIZE_T BusyEntries;
    SIZE_T LastUsed;

} HEAP_LOCAL_SEGMENT_INFO, *PHEAP_LOCAL_SEGMENT_INFO;

#define HEAP_BUCKETS_COUNT      128  

typedef struct _HEAP_LOCAL_DATA {
    
    //
    //  We reserve the 128 bytes below to avoid sharing memory
    //  into the same cacheline on MP machines
    //

    UCHAR Reserved[128];

    volatile PLFH_BLOCK_ZONE CrtZone;
    struct _LFH_HEAP * LowFragHeap;

    HEAP_LOCAL_SEGMENT_INFO SegmentInfo[HEAP_BUCKETS_COUNT];
    SLIST_HEADER DeletedSubSegments;

    ULONG Affinity;
    ULONG Reserved1;

} HEAP_LOCAL_DATA, *PHEAP_LOCAL_DATA;

//
//  Fixed size large block cache data structures & definitions
//  This holds in S-Lists the blocks that can be free, but it
//  delay the free until no other thread is doing a heap operation
//  This helps reducing the contention on the heap lock,
//  improve the scalability with a relatively low memory footprint
//

#define HEAP_LOWEST_USER_SIZE_INDEX 7
#define HEAP_HIGHEST_USER_SIZE_INDEX 18

#define HEAP_USER_ENTRIES (HEAP_HIGHEST_USER_SIZE_INDEX - HEAP_LOWEST_USER_SIZE_INDEX + 1)

typedef struct _USER_MEMORY_CACHE {

    SLIST_HEADER UserBlocks[ HEAP_USER_ENTRIES ];

    ULONG FreeBlocks;
    ULONG Sequence;

    ULONG MinDepth[ HEAP_USER_ENTRIES ];
    ULONG AvailableBlocks[ HEAP_USER_ENTRIES ];
    
} USER_MEMORY_CACHE, *PUSER_MEMORY_CACHE;

typedef struct _LFH_HEAP {
    
    RTL_CRITICAL_SECTION Lock;

    LIST_ENTRY SubSegmentZones;
    SIZE_T ZoneBlockSize;
    HANDLE Heap;
    LONG Conversions;
    LONG ConvertedSpace;

    ULONG SegmentChange;           //  
    ULONG SegmentCreate;           //  Various counters (optional)
    ULONG SegmentInsertInFree;     //   
    ULONG SegmentDelete;           //     

    USER_MEMORY_CACHE UserBlockCache;

    //
    //  Bucket data
    //

    HEAP_BUCKET Buckets[HEAP_BUCKETS_COUNT];

    //
    //  The LocalData array must be the last field in LFH structures
    //  The sizes of the array is choosen depending upon the
    //  number of processors.
    //

    HEAP_LOCAL_DATA LocalData[1];

} LFH_HEAP, *PLFH_HEAP;


/*
    MEMORY_ADDRESS Addr = GetExpression(args);
    if (NULL == Addr)
    {
        dprintf("unable to resolve %s\n",args);
        return;
    }
    HEAP_USERDATA_HEADER UserData;
    if (ReadMemory(Addr,&UserData,sizeof(UserData),NULL))
    {
        dprintf("    Next %p HeapHandle %p SizeIndex %p Signature %p\n",
                   UserData.SFreeListEntry.Next,
                   UserData.HeapHandle,     
                   UserData.SizeIndex,
                   UserData.Signature);
        BLOCK_ENTRY * pBlkEntry = (BLOCK_ENTRY *)((HEAP_USERDATA_HEADER*)Addr+1);
        while(pBlkEntry)
        {
            BLOCK_ENTRY BlkEntry;
            
            if (ReadMemory((ULONG_PTR)pBlkEntry,&BlkEntry,sizeof(BlkEntry),NULL))
            {                
                dprintf("        %p : %08x %02x %02x %02x %02x - %04x %04x\n",
                            pBlkEntry,
                            *(DWORD *)&BlkEntry.Size,
                            BlkEntry.SmallTagIndex,
                            BlkEntry.Flags,
                            BlkEntry.UnusedBytes,
                            BlkEntry.SegmentIndex,
                            BlkEntry.LinkOffset,
                            BlkEntry.Reserved2);
                
                GetVTable((MEMORY_ADDRESS)((BYTE*)pBlkEntry+sizeof(BLOCK_ENTRY)));
                
                if (1 == BlkEntry.SmallTagIndex)
                {
                    pBlkEntry = (BLOCK_ENTRY *)((HEAP_ENTRY *)pBlkEntry+BlkEntry.LinkOffset);
                }
                else
                {
                    pBlkEntry = 0;
                }
            }
            else
            {
                dprintf("RM %p\n",pBlkEntry);
                break;
            }
        }
    }
    else
    {
        dprintf("RM %p\n");
    }
*/

void Print_HEAP_SUBSEGMENT( HEAP_SUBSEGMENT * pSubSeg_OOP,BOOL bHere)
{
    HEAP_SUBSEGMENT HeapSubSeg;
    HEAP_SUBSEGMENT * pSubSegHERE = &HeapSubSeg;
    if (NULL == pSubSeg_OOP) return;

    if (bHere)
    {
        pSubSegHERE = pSubSeg_OOP;
    }
    else
    {
        if (!ReadMemory((ULONG_PTR)pSubSeg_OOP,&HeapSubSeg,sizeof(HeapSubSeg),0))
        {
            dprintf("RM %p\n",pSubSeg_OOP);
            return;
        }
    }
        dprintf("  - Bkt %p Blk %p D %04x F %04x S %08x\n",
                   pSubSegHERE->Bucket,
                   pSubSegHERE->UserBlocks,
                   pSubSegHERE->AggregateExchg.Depth,
                   pSubSegHERE->AggregateExchg.FreeEntryOffset,
                   pSubSegHERE->AggregateExchg.Sequence);
        dprintf("    Bs %04x FT %04x BC %04x S %02x A %02x Lock %08x\n",
                pSubSegHERE->BlockSize,
                pSubSegHERE->FreeThreshold,
                pSubSegHERE->BlockCount,
                pSubSegHERE->SizeIndex,
                pSubSegHERE->AffinityIndex,
                pSubSegHERE->Lock);

        if (0 == pSubSegHERE->AggregateExchg.Depth) return;
        
        struct EntryTable : HEAP_ENTRY 
        {
            ULONG_PTR vTable;
        } Entry;
        HEAP_ENTRY * pEntry_OOP = (HEAP_ENTRY *)((HEAP_USERDATA_HEADER *)pSubSegHERE->UserBlocks+1);
        DWORD Count = 0;
        do
        {
            if (ReadMemory((ULONG_PTR)pEntry_OOP,&Entry,sizeof(Entry),NULL))
            {
                dprintf("      %p: %04x . %04x [%02x] - (%x)\n",
                           pEntry_OOP,1+pSubSegHERE->SizeIndex,0,
                           Entry.Flags,
                           (sizeof(HEAP_ENTRY))*( pSubSegHERE->BlockSize)-Entry.UnusedBytes);
                GetVTable(Entry.vTable);
                pEntry_OOP += ( pSubSegHERE->BlockSize );
                Count++;
            }
            else
            {
                dprintf("RM %p\n",pEntry_OOP);
                break;
            }
        } while(1 == Entry.SmallTagIndex && Count < pSubSegHERE->AggregateExchg.Depth);

}

DWORD EnumListCrtZone(VOID * pZone_OOP,
                                      VOID * pBlockZone)
{
    LFH_BLOCK_ZONE CrtZone;
    LFH_BLOCK_ZONE * pBlockZone_OOP = (LFH_BLOCK_ZONE *)pZone_OOP;
    if (ReadMemory((ULONG_PTR)pBlockZone_OOP,&CrtZone,sizeof(CrtZone),NULL))
    {
        ULONG_PTR Size = (ULONG_PTR)CrtZone.FreePointer - (ULONG_PTR)pBlockZone_OOP;
        BYTE * pMem = (BYTE *)HeapAlloc(GetProcessHeap(),0,Size);
        if (pMem)
        {
            ULONG_PTR AddrCrtZone = (ULONG_PTR)pBlockZone_OOP;
            if (ReadMemory(AddrCrtZone,pMem,(ULONG)Size,NULL))
            {
                HEAP_SUBSEGMENT * pSubSeg = (HEAP_SUBSEGMENT *)((LFH_BLOCK_ZONE *)pMem + 1);
                HEAP_SUBSEGMENT * pSubSegEnd = (HEAP_SUBSEGMENT *)((BYTE *)pSubSeg+Size);

                HEAP_SUBSEGMENT * pSubSeg_OOP = (HEAP_SUBSEGMENT *)((LFH_BLOCK_ZONE *)AddrCrtZone+1);
                while (pSubSeg < pSubSegEnd )
                {
                    //dprintf("SS - %p\n",pSubSeg_OOP);
                    Print_HEAP_SUBSEGMENT(pSubSeg,TRUE);
                    pSubSeg++;
                    pSubSeg_OOP++;
                }
            }
            HeapFree(GetProcessHeap(),0,pMem);
        }
    }
    else
    {
        dprintf("RM %p\n",pBlockZone_OOP);
    }
    return 0;    
}

void Print_LFH(LFH_HEAP * pLFH_OOP)
{
    ULONG_PTR AddrAff = GetExpression("ntdll!RtlpHeapMaxAffinity");
    if (AddrAff)
    {
        ULONG Affinity = 0;
        if (ReadMemory(AddrAff,&Affinity,sizeof(Affinity),0))            
        {
            ULONG_PTR SizeRead = sizeof(LFH_HEAP) + Affinity*sizeof(HEAP_LOCAL_DATA);
            LFH_HEAP * pLFH = (LFH_HEAP *)HeapAlloc(GetProcessHeap(),0,SizeRead);
            if (NULL == pLFH) return;

            if (ReadMemory((ULONG_PTR)pLFH_OOP,pLFH,(ULONG)SizeRead,0))
            {
                dprintf("LFH_HEAP %p\n",pLFH_OOP);

               LIST_ENTRY  * pListHead_oop = &pLFH_OOP->SubSegmentZones;
               EnumLinkedListCB(pListHead_oop,
                                          sizeof(LFH_BLOCK_ZONE),
                                          FIELD_OFFSET(LFH_BLOCK_ZONE,ListEntry), 
                                         EnumListCrtZone);                 

/*
                for (DWORD i=0;i<HEAP_USER_ENTRIES;i++)
                {
                    dprintf("    HEAP_USERDATA_HEADER - %d\n",i);
                    HEAP_USERDATA_HEADER * pUsrDataHdr = (HEAP_USERDATA_HEADER *)pLFH->UserBlockCache.UserBlocks[i].Next.Next;

                    dprintf("      Next %p Available %x\n",pUsrDataHdr,pLFH->UserBlockCache.AvailableBlocks[i]);

                    HEAP_USERDATA_HEADER UsrDataHdr;
                    UsrDataHdr.SFreeListEntry.Next = NULL;
                    for (;pUsrDataHdr;pUsrDataHdr = (HEAP_USERDATA_HEADER *)UsrDataHdr.SFreeListEntry.Next)
                    {
                        if (ReadMemory((ULONG_PTR)pUsrDataHdr,&UsrDataHdr,sizeof(UsrDataHdr),NULL))
                        {
                            dprintf("        ----\n");
                            dprintf("        Next %p\n",UsrDataHdr.SFreeListEntry.Next);
                            dprintf("        HeapHandle %p\n",UsrDataHdr.HeapHandle);
                            dprintf("        SizeIndex %p\n",UsrDataHdr.SizeIndex);
                            dprintf("        Signature %p\n",UsrDataHdr.Signature);
                        }
                        else
                        {
                            dprintf("RM %p\n",pUsrDataHdr);
                        }
                    }
                }
*/

                //
                // Buckets
                //
/*
                dprintf("    Buckets\n");
                dprintf("    #    BUnt S A Conv TotBlk SubSegCnt\n");
                for (DWORD i = 0; i < HEAP_BUCKETS_COUNT; i++) 
                {
                      //+0x000 Counters : _HEAP_BUCKET
                      dprintf("    %02x - %04x %02x %02x %p %08x  %08x\n",
                                 i,
                                 pLFH->Buckets[i].BlockUnits,
                                 pLFH->Buckets[i].SizeIndex,
                                 pLFH->Buckets[i].UseAffinity,
                                 pLFH->Buckets[i].Conversions,
                                 pLFH->Buckets[i].Counters.TotalBlocks,
                                 pLFH->Buckets[i].Counters.SubSegmentCounts);
                }
*/

                for (DWORD i = 0; i <= Affinity; i++) 
                {
                    if (0 == i)
                        dprintf("  HEAP_LOCAL_DATA - NO AFFINITY\n");
                    else
                        dprintf("  HEAP_LOCAL_DATA - AFFINITY %d\n",i-1);
                    
                    dprintf("        @ %p CrtZone %p LFHeap %p  Affinity %x\n",
                               (ULONG_PTR)pLFH_OOP + (ULONG_PTR)&pLFH->LocalData[i] - (ULONG_PTR)pLFH,
                               pLFH->LocalData[i].CrtZone,
                               pLFH->LocalData[i].LowFragHeap,
                               pLFH->LocalData[i].Affinity);

                    //Print_BLOCK_ZONE((LFH_BLOCK_ZONE *)pLFH->LocalData[i].CrtZone);
                    
                    HEAP_LOCAL_DATA * pLocData = &pLFH->LocalData[i];

                    dprintf("        #   Hint     Active   Next   BusyEntries LastUsed\n");
                    for (DWORD j=0;j<HEAP_BUCKETS_COUNT;j++)
                    {
                        dprintf("        %02x  %p %p %p %08x %08x\n",
                                   j,
                                   pLocData->SegmentInfo[j].Hint,
                                   pLocData->SegmentInfo[j].ActiveSubsegment,
                                   pLocData->SegmentInfo[j].SListHeader, //.Next, //.next
                                   pLocData->SegmentInfo[j].BusyEntries,
                                   pLocData->SegmentInfo[j].LastUsed);
                        //Print_HEAP_SUBSEGMENT(pLocData->SegmentInfo[j].Hint,FALSE);
                        //Print_HEAP_SUBSEGMENT(pLocData->SegmentInfo[j].ActiveSubsegment,FALSE);
                    }
                    
                }                
            }
            else
            {
                dprintf("RM %p\n",pLFH_OOP);
            }
            HeapFree(GetProcessHeap(),0,pLFH);
        }
        else
        {
            dprintf("RM %p\n",AddrAff);
        }
    }
    else
    {
        dprintf("unable to resolve ntdll!RtlpHeapMaxAffinity\n");
    }
}

DECLARE_API(lfhp)
{
    INIT_API();
    
    MEMORY_ADDRESS Addr = GetExpression(args);
    if (NULL == Addr)
    {
        dprintf("unable to resolve %s\n",args);
        return;
    }

    DEFINE_CPP_VAR( HEAP, varHEAP);
    HEAP * pHeap = GET_CPP_VAR_PTR( HEAP , varHEAP );    

    if (ReadMemory(Addr,pHeap ,sizeof(HEAP),NULL))
    {

        //dprintf("-----  LookAside %p\n",pHeap->FrontEndHeap);
        //Dump_LookAside((ULONG_PTR)pHeap->FrontEndHeap);
        if (HEAP_FRONT_LOWFRAGHEAP == pHeap->FrontEndHeapType )
        {
            Print_LFH((LFH_HEAP*)pHeap->FrontEndHeap);
        }
        else
        {
            dprintf("Unrecognized FrontEndHeapType %d\n",pHeap->FrontEndHeapType);
        }
    }
    else
    {
        dprintf("RM %p\n",Addr);
    }    
    
}

//
//
// dump the HEAP, incomplete
//
//

DECLARE_API(hp) 
{
    INIT_API();

    g_UsedInHeap = 0;

    DEFINE_CPP_VAR( HEAP, varHEAP);
    HEAP * pHeap = GET_CPP_VAR_PTR( HEAP , varHEAP );    
    MEMORY_ADDRESS pByte = GetExpression(args);
    DWORD i;

    if (pByte)
    {     
        if (ReadMemory(pByte,pHeap ,sizeof(HEAP),NULL))
        {            
            for (i=0;i<HEAP_MAXIMUM_SEGMENTS;i++)
            {
                if (pHeap->Segments[i])
                    PrintHEAP_SEGMENT(pHeap->Segments[i],NULL,NULL);
                    //dprintf(" seg %i - %p\n",i,pHeap->Segments[i]);
                
            }
            dprintf("Used Bytes %p\n",g_UsedInHeap);
        }
        else
        {
            dprintf("RM %p %d\n",pByte,GetLastError());
        }
    }
    else
    {
        dprintf("invalid address %s\n",args);
    }
}

//
//
//
/////////////////////////////////////////////////////////////

DWORD       g_BlockSize;
ULONG_PTR * g_pBlockBlob;
ULONG g_NumMatch;

DWORD CallBackSearch(ULONG_PTR pHeapEntry_OOP,ULONG_PTR Addr)
{
    if (!g_pBlockBlob)
    {
        dprintf("no GLOBAL search block\n");
        return STATUS_NO_MEMORY;
    }
    HEAP_ENTRY Entry;
    if (ReadMemory(pHeapEntry_OOP,&Entry,sizeof(HEAP_ENTRY),NULL))
    {
        HEAP_ENTRY * pEntry = (HEAP_ENTRY *)&Entry;        
        DWORD Size = (pEntry->Flags & HEAP_ENTRY_BUSY)?(pEntry->Size<<HEAP_GRANULARITY_SHIFT)-pEntry->UnusedBytes:0;
        if (Size)
        {
            if (Size < g_BlockSize)
            {
                ULONG_PTR * pData = (ULONG_PTR *)g_pBlockBlob;
                ReadMemory(pHeapEntry_OOP+sizeof(HEAP_ENTRY),pData,Size,NULL);
                
                // here is the assumption that pointers are aligned
                DWORD nTimes = Size/sizeof(ULONG_PTR);
                DWORD i;
                for (i=0;i<nTimes;i++)
                {
                    if (Addr == pData[i])
                    {
                        dprintf("- %p off %p\n",pHeapEntry_OOP,sizeof(ULONG_PTR)*i);
                        PrintHeapEntry((HEAP_ENTRY *)pEntry,(void *)pHeapEntry_OOP);
                        g_NumMatch++;
                    }
                }
            }
            else
            {
                dprintf("        entry %p too big\n",pHeapEntry_OOP);
            }
        }         
    }
    else
    {
        dprintf("RM %p\n",pHeapEntry_OOP);
    }
    return 0;
}

//
//
// search the HEAP, incomplete
//
///////////////////////////////////////////////

DECLARE_API(shp) 
{
    INIT_API();

    DEFINE_CPP_VAR( HEAP, varHEAP);
    HEAP * pHeap = GET_CPP_VAR_PTR( HEAP , varHEAP );    
    

    char * pArgs = (char *)args;
    while(isspace(*pArgs)) pArgs++;

    MEMORY_ADDRESS pByte = GetExpression(pArgs);

    while(!isspace(*pArgs)) pArgs++;

    MEMORY_ADDRESS Addr = GetExpression(pArgs);
    
    DWORD i;

    if (pByte && Addr)
    {
        g_BlockSize = 0x10000*sizeof(HEAP_ENTRY);
        g_pBlockBlob = (ULONG_PTR *)VirtualAlloc(NULL,g_BlockSize,MEM_COMMIT,PAGE_READWRITE);

        if (!g_pBlockBlob)
        {
            dprintf("VirtualAlloc err %d\n",GetLastError());
            return;
        }        
             
        if (ReadMemory(pByte,pHeap ,sizeof(HEAP),NULL))
        {

            g_NumMatch = 0;
            for (i=0;i<HEAP_MAXIMUM_SEGMENTS;i++)
            {
                if (pHeap->Segments[i])
                    PrintHEAP_SEGMENT(pHeap->Segments[i],CallBackSearch,(ULONG_PTR)Addr);
                //    dprintf(" seg %i - %p\n",i,pHeap->Segments[i]);
                
            }
            dprintf("%d matches found\n",g_NumMatch);
        }
        else
        {
            dprintf("RM %p %d\n",pByte,GetLastError());
        }

        if (g_pBlockBlob)
        {
            VirtualFree(g_pBlockBlob,0,MEM_RELEASE);
            g_pBlockBlob = NULL;
            g_BlockSize = 0;
        }
    }
    else
    {
        dprintf("invalid heap address pair in%s\n",args);
    }
}

//
//  decode heap flags
//
/*
#define HEAP_NO_SERIALIZE               0x00000001      // winnt
#define HEAP_GROWABLE                   0x00000002      // winnt
#define HEAP_GENERATE_EXCEPTIONS        0x00000004      // winnt
#define HEAP_ZERO_MEMORY                0x00000008      // winnt
#define HEAP_REALLOC_IN_PLACE_ONLY      0x00000010      // winnt
#define HEAP_TAIL_CHECKING_ENABLED      0x00000020      // winnt
#define HEAP_FREE_CHECKING_ENABLED      0x00000040      // winnt
#define HEAP_DISABLE_COALESCE_ON_FREE   0x00000080      // winnt

#define HEAP_SETTABLE_USER_VALUE        0x00000100
#define HEAP_SETTABLE_USER_FLAG1        0x00000200
#define HEAP_SETTABLE_USER_FLAG2        0x00000400
#define HEAP_SETTABLE_USER_FLAG3        0x00000800

#define HEAP_CLASS_0                    0x00000000      // process heap
#define HEAP_CLASS_1                    0x00001000      // private heap
#define HEAP_CLASS_2                    0x00002000      // Kernel Heap
#define HEAP_CLASS_3                    0x00003000      // GDI heap
#define HEAP_CLASS_4                    0x00004000      // User heap
#define HEAP_CLASS_5                    0x00005000      // Console heap
#define HEAP_CLASS_6                    0x00006000      // User Desktop heap
#define HEAP_CLASS_7                    0x00007000      // Csrss Shared heap
#define HEAP_CLASS_8                    0x00008000      // Csr Port heap
*/
#define HEAP_LOCK_USER_ALLOCATED            (ULONG)0x80000000
#define HEAP_VALIDATE_PARAMETERS_ENABLED    (ULONG)0x40000000
#define HEAP_VALIDATE_ALL_ENABLED           (ULONG)0x20000000
#define HEAP_SKIP_VALIDATION_CHECKS         (ULONG)0x10000000
#define HEAP_CAPTURE_STACK_BACKTRACES       (ULONG)0x08000000

#define HEAP_FLAG_PAGE_ALLOCS       0x01000000

#define HEAP_PROTECTION_ENABLED     0x02000000
#define HEAP_BREAK_WHEN_OUT_OF_VM   0x04000000
#define HEAP_NO_ALIGNMENT           0x08000000



void DecodeFlags(ULONG Flags)
{
    dprintf("           Flags: %08x ",Flags);
    if (Flags & HEAP_NO_SERIALIZE)
           dprintf("HEAP_NO_SERIALIZE ");
    if (Flags & HEAP_GROWABLE)
        dprintf("HEAP_GROWABLE ");
    if (Flags & HEAP_GENERATE_EXCEPTIONS)
        dprintf("HEAP_GENERATE_EXCEPTIONS ");
    if (Flags & HEAP_ZERO_MEMORY)
        dprintf("HEAP_ZERO_MEMORY ");
    
    if (Flags & HEAP_REALLOC_IN_PLACE_ONLY)
        dprintf("HEAP_REALLOC_IN_PLACE_ONLY ");
    if (Flags & HEAP_TAIL_CHECKING_ENABLED)
        dprintf("HEAP_TAIL_CHECKING_ENABLED ");
    if (Flags & HEAP_FREE_CHECKING_ENABLED)
        dprintf("HEAP_FREE_CHECKING_ENABLED ");
    if (Flags & HEAP_DISABLE_COALESCE_ON_FREE)
        dprintf("HEAP_DISABLE_COALESCE_ON_FREE ");
    
    if (Flags & HEAP_SETTABLE_USER_VALUE)
        dprintf("HEAP_SETTABLE_USER_VALUE ");    
    if (Flags & HEAP_SETTABLE_USER_FLAG1)
        dprintf("HEAP_SETTABLE_USER_FLAG1 ");
    if (Flags & HEAP_SETTABLE_USER_FLAG2)
        dprintf("HEAP_SETTABLE_USER_FLAG2 ");
    if (Flags & HEAP_SETTABLE_USER_FLAG3)
        dprintf("HEAP_SETTABLE_USER_FLAG3 ");

    if (Flags & HEAP_CLASS_MASK)
        dprintf("HEAP_CLASS %d",(Flags&HEAP_CLASS_MASK)>>12);
/*    
    if (Flags & HEAP_CLASS_1)
        dprintf("HEAP_CLASS_1 ");
    if (Flags & HEAP_CLASS_2)
        dprintf("HEAP_CLASS_2 ");
    if (Flags & HEAP_CLASS_3)
        dprintf("HEAP_CLASS_3 ");
    if (Flags & HEAP_CLASS_4)
        dprintf("HEAP_CLASS_4 ");
    if (Flags & HEAP_CLASS_5)
        dprintf("HEAP_CLASS_5 ");
    if (Flags & HEAP_CLASS_6)
        dprintf("HEAP_CLASS_6 ");
    if (Flags & HEAP_CLASS_7)
        dprintf("HEAP_CLASS_7 ");
*/        

    //if (Flags & HEAP_CAPTURE_STACK_BACKTRACES)
    //    dprintf("HEAP_CAPTURE_STACK_BACKTRACES ");    
    if (Flags &HEAP_SKIP_VALIDATION_CHECKS)
        dprintf("HEAP_SKIP_VALIDATION_CHECKS ");
    if (Flags &HEAP_VALIDATE_ALL_ENABLED)
        dprintf("HEAP_VALIDATE_ALL_ENABLED ");
    if (Flags &HEAP_VALIDATE_PARAMETERS_ENABLED)
        dprintf("HEAP_VALIDATE_PARAMETERS_ENABLED ");
    if (Flags &HEAP_LOCK_USER_ALLOCATED)
        dprintf("HEAP_LOCK_USER_ALLOCATED ");

    if (Flags &HEAP_FLAG_PAGE_ALLOCS)
        dprintf("HEAP_FLAG_PAGE_ALLOCS "); 
    if (Flags &HEAP_PROTECTION_ENABLED)
        dprintf("HEAP_PROTECTION_ENABLED "); 
    if (Flags &HEAP_BREAK_WHEN_OUT_OF_VM)
        dprintf("HEAP_BREAK_WHEN_OUT_OF_VM "); 
    if (Flags &HEAP_NO_ALIGNMENT)
        dprintf("HEAP_NO_ALIGNMENT ");     
    
    //if (Flags &)
    //    dprintf(" ");    
    dprintf("\n");
}

//
//  Get all the heaps
//

DECLARE_API(hps)
{
    INIT_API();

    PEB * pPeb = NULL;
    PEB   ThisPeb;
    GetPeb(hCurrentProcess,&pPeb);

    if(!pPeb)
    {
#ifdef  _WIN64
        pPeb = (PEB *)0x6fbfffde000;
#else
        pPeb = (PEB *)0x7ffdf000;
#endif
    }
    
    if (pPeb)
    {
        ReadMemory((MEMORY_ADDRESS)pPeb,&ThisPeb,sizeof(PEB),0);
        void ** pHeaps = (void**)_alloca(ThisPeb.NumberOfHeaps*sizeof(void*));
        DWORD i,j;
        ULONG_PTR TotCommitSize = 0;
        ULONG_PTR TotVirtSize = 0;        

        if (ReadMemory((MEMORY_ADDRESS)ThisPeb.ProcessHeaps,pHeaps,ThisPeb.NumberOfHeaps*sizeof(void*),0))
        {
            for(i=0;i<ThisPeb.NumberOfHeaps;i++)
            {
                DEFINE_CPP_VAR( HEAP, varHEAP);
                HEAP * pHeap = GET_CPP_VAR_PTR( HEAP , varHEAP );
                ULONG_PTR TotHeapCommitSize = 0;
                ULONG_PTR TotHeapVirtSize = 0;

                if (ReadMemory((MEMORY_ADDRESS)pHeaps[i],pHeap ,sizeof(HEAP),0))
                {
                    for (j=0;j<HEAP_MAXIMUM_SEGMENTS;j++)
                    {
                        if (pHeap->Segments[j])
                        {
                            DEFINE_CPP_VAR( HEAP_SEGMENT, varHEAP_SEGMENT);
                            HEAP_SEGMENT * pHeapSeg = GET_CPP_VAR_PTR( HEAP_SEGMENT , varHEAP_SEGMENT ); 

                            if (ReadMemory((MEMORY_ADDRESS)pHeap->Segments[j],pHeapSeg,sizeof(HEAP_SEGMENT),0))
                            {
                                dprintf("       - %p (C %p - R %p)\n",
                                        pHeap->Segments[j],
                                        (pHeapSeg->NumberOfPages - pHeapSeg->NumberOfUnCommittedPages) * PAGE_SIZE,
                                        (pHeapSeg->NumberOfPages) * PAGE_SIZE);
                                
                                TotHeapCommitSize += ((pHeapSeg->NumberOfPages - pHeapSeg->NumberOfUnCommittedPages) * PAGE_SIZE);
                                TotHeapVirtSize += ((pHeapSeg->NumberOfPages) * PAGE_SIZE);
                                // now print the beggining of a committed range
                                dprintf("            CR %p\n",pHeapSeg->BaseAddress);
                                HEAP_UNCOMMMTTED_RANGE * pUncomm_OOP = pHeapSeg->UnCommittedRanges;
                                for (DWORD i=0;i<pHeapSeg->NumberOfUnCommittedRanges && pUncomm_OOP;i++)                                    
                                {
                                    HEAP_UNCOMMMTTED_RANGE UncommRange;
                                    if (ReadMemory((MEMORY_ADDRESS)pUncomm_OOP,&UncommRange,sizeof(HEAP_UNCOMMMTTED_RANGE),NULL))
                                    {
                                        if (UncommRange.Next)
                                        {
                                            pUncomm_OOP = UncommRange.Next;
                                        }
                                        ULONG_PTR RangeAddr = (ULONG_PTR)UncommRange.Address+UncommRange.Size;
                                        if (RangeAddr != (ULONG_PTR)pHeapSeg->LastEntryInSegment)
                                        dprintf("            CR %p\n",RangeAddr);
                                    }
                                    else
                                    {
                                        dprintf("RM %p\n",pHeapSeg->UnCommittedRanges);
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                dprintf("RM %p\n",pHeap->Segments[j]);
                            }
                        }
                    }
                }
                else
                {
                    dprintf("RM %p\n",pHeaps[i]);
                    pHeap = NULL;
                }                    
                dprintf("       HEAP %p - %p\n",pHeaps[i],TotHeapCommitSize);
                if (pHeap)
                {
                    DecodeFlags(pHeap->Flags|pHeap->ForceFlags);
                    dprintf("           FrontEndHeapType %d\n",pHeap->FrontEndHeapType);
                }
                TotCommitSize += TotHeapCommitSize;
                TotVirtSize += TotHeapVirtSize;
            }
            dprintf("      -- Tot C %p Tot R %p\n",TotCommitSize, TotVirtSize);
           }
        else
        {
            dprintf("RM %p\n",ThisPeb.ProcessHeaps);
        }
    }
    else
    {
        dprintf("unable to get PEB\n");
    }
}


//
//  reverse heap free list
//
//////////////////

DWORD
CallBackFreeList(VOID * pStructure_OOP,
                 VOID * pLocalCopy)
{
    HEAP_FREE_ENTRY * pFreeEntry = (HEAP_FREE_ENTRY *)pLocalCopy;

    dprintf("    %p (%p,%p): %04x - %04x [%02x] %02x %02x (%x)\n",
            pStructure_OOP,
            pFreeEntry->FreeList.Flink,
            pFreeEntry->FreeList.Blink,
            pFreeEntry->Size,
            pFreeEntry->PreviousSize,
            pFreeEntry->Flags,
            pFreeEntry->Index,
            pFreeEntry->Mask,
            pFreeEntry->Size*sizeof(HEAP_ENTRY));
            
    return 0;    
}


DECLARE_API( rllc )
{
    INIT_API();

    MEMORY_ADDRESS Addr = GetExpression(args);

    if (Addr)
    {
        EnumReverseLinkedListCB((LIST_ENTRY *)Addr,
                                sizeof(HEAP_FREE_ENTRY),
                                FIELD_OFFSET(HEAP_FREE_ENTRY,FreeList),
                                CallBackFreeList);
    }
    else
    {
        dprintf("cannot resolve %s\n",args);
    }
}

//
//
//  Print the Free Lists of the Heap
//
/////////////////////////////////////////////////

DWORD
CallBackFreeList2(VOID * pStructure_OOP,
                 VOID * pLocalCopy)
{
    HEAP_FREE_ENTRY * pFreeEntry = (HEAP_FREE_ENTRY *)pLocalCopy;

    dprintf("    %p (%p,%p): %04x - %04x [%02x] %02x %02x (%x)",
            pStructure_OOP,
            pFreeEntry->FreeList.Flink,
            pFreeEntry->FreeList.Blink,
            pFreeEntry->Size,
            pFreeEntry->PreviousSize,
            pFreeEntry->Flags,
            pFreeEntry->Index,
            pFreeEntry->Mask,
            pFreeEntry->Size*sizeof(HEAP_ENTRY));

    MEMORY_ADDRESS pEntry = (MEMORY_ADDRESS)pStructure_OOP;
    
    HEAP_ENTRY HeapEntry;
    ReadMemory((MEMORY_ADDRESS)pEntry,&HeapEntry,sizeof(HeapEntry),0);

    while (HeapEntry.PreviousSize)
    {
        pEntry = (MEMORY_ADDRESS)((HEAP_ENTRY*)pEntry - HeapEntry.PreviousSize);
        if (ReadMemory((MEMORY_ADDRESS)pEntry,&HeapEntry,sizeof(HeapEntry),0))
        {               
        }
        else
        {
            dprintf("RM %p\n",pEntry);
            break;
        }

        if (CheckControlC())
           break;           
    }
    dprintf(" -B %p\n",pEntry);            
    return 0;    
}

#define EmptyFull( expr ) (( expr )?'F':'-')

DECLARE_API( hpf )
{
    INIT_API();

    DEFINE_CPP_VAR( HEAP, varHEAP);
    HEAP * pHeap = GET_CPP_VAR_PTR( HEAP , varHEAP );    
    MEMORY_ADDRESS pByte = GetExpression(args);

    if (pByte)
    {     
        if (ReadMemory(pByte,pHeap ,sizeof(HEAP),NULL))
        {
                dprintf("        --   00 01 02 03 04 05 06 07\n");
            DWORD nBytes = HEAP_MAXIMUM_FREELISTS / 8 ;
            for (DWORD i=0;i<nBytes;i++)
            {
                BYTE Set = pHeap->u.FreeListsInUseBytes[i];
                dprintf("        %02x :  %c  %c  %c  %c  %c  %c  %c  %c\n",8*i,
                          EmptyFull(Set & 0x01),EmptyFull(Set & 0x02),EmptyFull(Set & 0x04),EmptyFull(Set & 0x08),
                          EmptyFull(Set & 0x10),EmptyFull(Set & 0x20),EmptyFull(Set & 0x40),EmptyFull(Set & 0x80));
            }
                dprintf("        ------------\n");           
            
            HEAP * pHeap_OOP = (HEAP *)pByte;        
            for (DWORD i=0;i<HEAP_MAXIMUM_FREELISTS;i++)
            {
                dprintf("    FreeList[%x] @ %p\n",i,&pHeap_OOP->FreeLists[i]);
                EnumReverseLinkedListCB((LIST_ENTRY *)&pHeap_OOP->FreeLists[i],
                                       sizeof(HEAP_FREE_ENTRY),
                                       FIELD_OFFSET(HEAP_FREE_ENTRY,FreeList),
                                       CallBackFreeList2);                
            }
        }
        else
        {
            dprintf("RM %p\n",pByte);
        }
    }
    else
    {
        dprintf("invalid address %s\n",args);
    }
    
}

//
// dumps the DPH_HEAP_ROOT
//
///////////////////////////////////////

DECLARE_API( php )
{
    INIT_API();

    char * pHeapAddr = (char *)args;
    while (isspace(*pHeapAddr)) pHeapAddr++;

    char * pNext = pHeapAddr;
    while (!isspace(*pNext)) pNext++; // skipt the Heap Addr
    if (*pNext)
    {
        *pNext = 0;
        pNext++;
    }        
    MEMORY_ADDRESS Addr = GetExpression(pHeapAddr);    
    while (isspace(*pNext)) pNext++; // skip the other spaces
    MEMORY_ADDRESS SearchAddr = 0;
    if (*pNext == 's' ||*pNext == 'S')
    {
        pNext++; // skip the 's'
        if (*pNext)
        {
            while(isspace(*pNext)) pNext++; // skip the spaces
            SearchAddr = GetExpression(pNext);
        } 
    }

    //dprintf("heap %p addr %p\n",Addr,SearchAddr);

    if (Addr)
    {

        g_BlockSize = 0x10000*sizeof(HEAP_ENTRY);
        g_pBlockBlob = NULL;
        if (SearchAddr)
            g_pBlockBlob = (ULONG_PTR *)VirtualAlloc(NULL,g_BlockSize,MEM_COMMIT,PAGE_READWRITE);

        if (SearchAddr && !g_pBlockBlob)
        {
            dprintf("VirtualAlloc err %d\n",GetLastError());
            return;
        }        

        HEAP Heap;
        DPH_HEAP_ROOT HeapPage;
        if (0 == SearchAddr)
            dprintf("  HEAP @ %p\n",Addr);
        if (ReadMemory((MEMORY_ADDRESS)Addr,&Heap,sizeof(HEAP),NULL))
        {
            if (Heap.ForceFlags & HEAP_FLAG_PAGE_ALLOCS)
            {
                Addr += PAGE_SIZE;
                dprintf("  DPH_HEAP_ROOT @ %p\n",Addr);
                if (ReadMemory((MEMORY_ADDRESS)Addr,&HeapPage,sizeof(DPH_HEAP_ROOT),NULL))
                {
                    DPH_HEAP_BLOCK HeapBlock;                
                    DPH_HEAP_BLOCK * pNextBlock;

                    if (0 == SearchAddr)
                    {
                        pNextBlock = HeapPage.pVirtualStorageListHead;
                        dprintf("    - pVirtualStorageListHead\n");                    
                        while(pNextBlock)
                        {

                            if (ReadMemory((MEMORY_ADDRESS)pNextBlock,&HeapBlock,sizeof(DPH_HEAP_BLOCK),NULL))
                            {
                                dprintf("    %p - (%p) B %p S %p \n",
                                       pNextBlock,
                                       HeapBlock.pNextAlloc,
                                       HeapBlock.pVirtualBlock,
                                       HeapBlock.nVirtualBlockSize);
                                pNextBlock = HeapBlock.pNextAlloc;
                            }
                            else
                            {
                                pNextBlock = NULL;
                            }
                        }
                    }
                
                    pNextBlock = HeapPage.pBusyAllocationListHead;
                    if (0 == SearchAddr)
                        dprintf("    - pBusyAllocationListHead\n");
                    while(pNextBlock)
                    {
                        if (ReadMemory((MEMORY_ADDRESS)pNextBlock,&HeapBlock,sizeof(DPH_HEAP_BLOCK),NULL))
                        {
                            if (0 == SearchAddr)
                            {
                                dprintf("    %p - (%p) %x %x %x U %p S %p\n",
                                       pNextBlock,
                                       HeapBlock.pNextAlloc,
                                       ULONG_PTR(HeapBlock.pVirtualBlock)/PAGE_SIZE,
                                       HeapBlock.nVirtualBlockSize/PAGE_SIZE,
                                       HeapBlock.nVirtualAccessSize/PAGE_SIZE,
                                       HeapBlock.pUserAllocation,
                                       HeapBlock.StackTrace_);
                                GetVTable((MEMORY_ADDRESS)HeapBlock.pUserAllocation+sizeof(DPH_BLOCK_INFORMATION));
                            }
                            else // do the real search
                            {
                                MEMORY_ADDRESS Size = (MEMORY_ADDRESS)HeapBlock.pVirtualBlock+HeapBlock.nVirtualAccessSize-(MEMORY_ADDRESS)HeapBlock.pUserAllocation;
                                if (ReadMemory((MEMORY_ADDRESS)HeapBlock.pUserAllocation,g_pBlockBlob,(ULONG)Size,NULL))
                                {
                                    Size /= sizeof(ULONG_PTR);
                                    BOOL bFound = FALSE;
                                    for (ULONG_PTR j =0;j<Size;j++)
                                    {
                                        if (SearchAddr == g_pBlockBlob[j])
                                        {
                                            bFound = TRUE;
                                            dprintf("    OFF %p\n",j*sizeof(ULONG_PTR));
                                        }
                                    }
                                    if (bFound)
                                    {
                                        dprintf("    B   %p\n",HeapBlock.pUserAllocation);
                                    }
                                }
                                else
                                {
                                    dprintf("RM %p\n",HeapBlock.pUserAllocation);
                                }
                            }
                            pNextBlock = HeapBlock.pNextAlloc;
                        }
                        else
                        {
                            pNextBlock = NULL;
                        }
                    }

                    if (0 == SearchAddr)
                    {                    
                        pNextBlock = HeapPage.pFreeAllocationListHead;
                        dprintf("    - pFreeAllocationListHead\n");
                        while(pNextBlock)
                        {
                            if (ReadMemory((MEMORY_ADDRESS)pNextBlock,&HeapBlock,sizeof(DPH_HEAP_BLOCK),NULL))
                            {
                                dprintf("    %p - (%p) %x %x %x U %p S %p\n",
                                       pNextBlock,
                                       HeapBlock.pNextAlloc,
                                       ULONG_PTR(HeapBlock.pVirtualBlock)/PAGE_SIZE,
                                       HeapBlock.nVirtualBlockSize/PAGE_SIZE,
                                       HeapBlock.nVirtualAccessSize/PAGE_SIZE,
                                       HeapBlock.pUserAllocation,
                                       HeapBlock.StackTrace_);
                                pNextBlock = HeapBlock.pNextAlloc;
                            }
                            else
                            {
                                pNextBlock = NULL;
                            }
                        }
                    }

                    if (0 == SearchAddr)
                    {                    
                        pNextBlock = HeapPage.pAvailableAllocationListHead;
                        dprintf("    - pAvailableAllocationListHead\n");
                        while(pNextBlock)
                        {

                            if (ReadMemory((MEMORY_ADDRESS)pNextBlock,&HeapBlock,sizeof(DPH_HEAP_BLOCK),NULL))
                            {
                                dprintf("    %p - (%p) B %p S %p \n",
                                       pNextBlock,
                                       HeapBlock.pNextAlloc,
                                       HeapBlock.pVirtualBlock,
                                       HeapBlock.nVirtualBlockSize);
                                pNextBlock = HeapBlock.pNextAlloc;
                            }
                            else
                            {
                                pNextBlock = NULL;
                            }
                        }
                    }

                    if (0 == SearchAddr)
                    {
                        pNextBlock = HeapPage.pNodePoolListHead;
                        dprintf("    - pNodePoolListHead\n");
                        while(pNextBlock)
                        {

                            if (ReadMemory((MEMORY_ADDRESS)pNextBlock,&HeapBlock,sizeof(DPH_HEAP_BLOCK),NULL))
                            {
                                dprintf("    %p - (%p) B %p S %p \n",
                                       pNextBlock,
                                       HeapBlock.pNextAlloc,
                                       HeapBlock.pVirtualBlock,
                                       HeapBlock.nVirtualBlockSize);
                                pNextBlock = HeapBlock.pNextAlloc;
                            }
                            else
                            {
                                pNextBlock = NULL;
                            }
                        }
                    }                    

                    dprintf("  NormalHeap @ %p\n",HeapPage.NormalHeap);
                    if (ReadMemory((ULONG_PTR)HeapPage.NormalHeap,&Heap ,sizeof(HEAP),NULL))
                    {
                        for (DWORD h=0;h<HEAP_MAXIMUM_SEGMENTS;h++)
                        {
                            if (Heap.Segments[h])
                            {
                                if (SearchAddr)
                                    PrintHEAP_SEGMENT(Heap.Segments[h],CallBackSearch,(ULONG_PTR)SearchAddr);
                                else
                                    PrintHEAP_SEGMENT(Heap.Segments[h],NULL,NULL);
                            }
                        }
                    }
                    else
                    {
                        dprintf("RM %p\n",HeapPage.NormalHeap);
                    }
                    
                }
                else
                {
                    dprintf("RM %p\n",Addr);
                }

            }
            else
            {
                DecodeFlags(Heap.ForceFlags|Heap.Flags);
            }
        }
        else
        {
            dprintf("RM %p\n",Addr);
        }

        if (g_pBlockBlob)
        {
            VirtualFree(g_pBlockBlob,g_BlockSize,MEM_RELEASE);
            g_pBlockBlob = NULL;
            g_BlockSize = 0;
        }
        
    }
    else
    {
        dprintf("unable to resolve %s\n",args);
    }
}

//
//
//  virtual_query helper
//
///////////////////////////////////////////////////////////////

char * GetState(DWORD State)
{
    switch(State)
    {
    case MEM_COMMIT:
        return "MEM_COMMIT";
    case MEM_RESERVE:
        return "MEM_RESERVE";
    case MEM_FREE:
        return "MEM_FREE";
    };
    return "";
}

char * GetType(DWORD Type)
{
    switch(Type)
    {
    case MEM_IMAGE:
        return "MEM_IMAGE";
    case MEM_MAPPED:
        return "MEM_MAPPED";
    case MEM_PRIVATE:
        return "MEM_PRIVATE";
    }
    return "";
}

char * GetProtect(DWORD Protect)
{
    switch(Protect)
    {
    case PAGE_NOACCESS:
        return "PAGE_NOACCESS";
    case PAGE_READONLY:
        return "PAGE_READONLY";
    case PAGE_READWRITE:
        return "PAGE_READWRITE";
    case PAGE_WRITECOPY:
        return "PAGE_WRITECOPY";
    case PAGE_EXECUTE:
        return "PAGE_EXECUTE";
    case PAGE_EXECUTE_READ:
        return "PAGE_EXECUTE_READ";
    case PAGE_EXECUTE_READWRITE:
        return "PAGE_EXECUTE_READWRITE";
    case PAGE_EXECUTE_WRITECOPY:
        return "PAGE_EXECUTE_WRITECOPY";
    case PAGE_GUARD:
        return "PAGE_GUARD";
    case PAGE_NOCACHE:
        return "PAGE_NOCACHE";
    case PAGE_WRITECOMBINE:
        return "PAGE_WRITECOMBINE";
    }
    return "<unk>";
}

//
//
//  VirtualQueryEx
//
//
//  vq -a address
//  vq -f filter <all address space>
//
///////////////////////////////////////////

DECLARE_API(vq)
{
    INIT_API();

    MEMORY_ADDRESS pVA = 0;
    MEMORY_ADDRESS Filter = (MEMORY_ADDRESS)-1;
    DWORD FilterSize = 0;
    BOOL bAll = TRUE;

    char * pCurrent = (char *)args;
    
    if(0 < strlen(pCurrent))
    {
        while (isspace(*pCurrent)) pCurrent++;
        if ('-' == *pCurrent  ||
            '/'  == *pCurrent)
        {
            pCurrent++;
            while (isspace(*pCurrent)) pCurrent++;
            if ('a' == *pCurrent)
            {
                pCurrent++;
                while (*pCurrent && isspace(*pCurrent)) pCurrent++;
                pVA = GetExpression(pCurrent);
                bAll = FALSE;
            } 
            else if ('f' == *pCurrent)
            {
                pCurrent++;
                while (*pCurrent && isspace(*pCurrent)) pCurrent++;
                Filter = GetExpression(pCurrent);   
                dprintf("FILTER %08x\n",Filter);
            }
            else if ('s' == *pCurrent)
            {
                pCurrent++;
                while (*pCurrent && isspace(*pCurrent)) pCurrent++;
                FilterSize = (DWORD)GetExpression(pCurrent);   
                dprintf("Size %08x\n",FilterSize);
            }            
            else
            {
                dprintf("usage: -a ADDR\n"
                        "usage: -F Filter <all address space>\n");
            }
        }
    }
    else
    {
        dprintf("no param\n");
    }


    ULONG_PTR Tot = 0; 
    MEMORY_BASIC_INFORMATION MemInfo;
    SIZE_T dwRet = 0;
    
    do
    {
        dwRet = VirtualQueryEx(hCurrentProcess,(LPCVOID)pVA,&MemInfo,sizeof(MemInfo));
    
        if (dwRet && 
            (MemInfo.AllocationProtect & Filter) && 
            (MemInfo.RegionSize > FilterSize))
        {
            dprintf("    Base %p Size %p Alloc %p Prot %s  %s %s %s\n",
                MemInfo.BaseAddress,
                MemInfo.RegionSize,                
                MemInfo.AllocationBase,
                GetProtect(MemInfo.AllocationProtect),
                GetState(MemInfo.State),
                GetProtect(MemInfo.Protect),
                GetType(MemInfo.Type));
            Tot += MemInfo.RegionSize;
        }
        
        pVA = (ULONG_PTR)MemInfo.BaseAddress + (ULONG_PTR)MemInfo.RegionSize;
        
        if (CheckControlC())
            break;
    } while (dwRet && bAll);

    dprintf("    Total %p\n",Tot);

}

//
//
//
//

#ifdef KDEXT_64BIT

struct _HEAP_ENTRY_64 
{
   WORD Size         ;
   WORD PreviousSize ;
   BYTE SegmentIndex ;
   BYTE Flags        ;
   BYTE UnusedBytes  ;
   BYTE SmallTagIndex;
   ULONG64 Pointer;
};

#endif /*KDEXT_64BIT*/

DECLARE_API(hef64)
{
    INIT_API();
#ifdef KDEXT_64BIT

    _HEAP_ENTRY_64 HeapEntry;
    ULONG64  MemAddr = GetExpression(args);
    ULONG64  pVTable = 0;

    if (MemAddr)
    {
        if (ReadMemory(MemAddr,&HeapEntry,sizeof(HeapEntry),NULL))
        {
            dprintf("    %p: %04x - %04x [%02x] (%x)\n",MemAddr,HeapEntry.Size,HeapEntry.PreviousSize,HeapEntry.Flags,HeapEntry.Size*sizeof(_HEAP_ENTRY_64)-HeapEntry.UnusedBytes);
            GetVTable(MemAddr + sizeof(_HEAP_ENTRY_64));
            MemAddr = MemAddr+HeapEntry.Size*sizeof(_HEAP_ENTRY_64);
                        
            // 0x10 is LAST_ENTRY
            while(!(HeapEntry.Flags & 0x10))
            {
                if (ReadMemory(MemAddr,&HeapEntry,sizeof(HeapEntry),NULL))
                {
                    dprintf("    %p: %04x - %04x [%02x] (%x)\n",MemAddr,HeapEntry.Size,HeapEntry.PreviousSize,HeapEntry.Flags,HeapEntry.Size*sizeof(_HEAP_ENTRY_64)-HeapEntry.UnusedBytes);
                    GetVTable(MemAddr + sizeof(_HEAP_ENTRY_64));                    
                    MemAddr = MemAddr+HeapEntry.Size*sizeof(_HEAP_ENTRY_64);
                }
                else
                {
                    dprintf("RM %p\n",MemAddr);
                    break;
                }
                if (CheckControlC())
                    break;
            }
            dprintf("last %p\n",MemAddr);
        }
        else
        {
            dprintf("RM %p\n",MemAddr);
        }
    }
    else
    {
        dprintf("unable to resolve %s\n",args);
    }
    
#endif /*KDEXT_64BIT*/    
}

DECLARE_API(heb64)
{
    INIT_API();
#ifdef KDEXT_64BIT

    _HEAP_ENTRY_64 HeapEntry;
    ULONG64  MemAddr = GetExpression(args);
    ULONG64  pVTable = 0;

    if (MemAddr)
    {
        if (ReadMemory(MemAddr,&HeapEntry,sizeof(HeapEntry),NULL))
        {
            dprintf("    %p: %04x - %04x [%02x] (%x)\n",MemAddr,HeapEntry.Size,HeapEntry.PreviousSize,HeapEntry.Flags,HeapEntry.Size*sizeof(_HEAP_ENTRY_64)-HeapEntry.UnusedBytes);
            GetVTable(MemAddr + sizeof(_HEAP_ENTRY_64));
            MemAddr = MemAddr - HeapEntry.PreviousSize*sizeof(_HEAP_ENTRY_64);
                        
            // 0x10 is LAST_ENTRY
            while(HeapEntry.PreviousSize)
            {
                if (ReadMemory(MemAddr,&HeapEntry,sizeof(HeapEntry),NULL))
                {
                    dprintf("    %p: %04x - %04x [%02x] (%x)\n",MemAddr,HeapEntry.Size,HeapEntry.PreviousSize,HeapEntry.Flags,HeapEntry.Size*sizeof(_HEAP_ENTRY_64)-HeapEntry.UnusedBytes);
                    GetVTable(MemAddr + sizeof(_HEAP_ENTRY_64));                    
                    MemAddr = MemAddr - HeapEntry.PreviousSize*sizeof(_HEAP_ENTRY_64);
                }
                else
                {
                    dprintf("RM %p\n",MemAddr);
                    break;
                }
                if (CheckControlC())
                    break;
            }
                                    
            dprintf("last %p\n",MemAddr);
        }
        else
        {
            dprintf("RM %p\n",MemAddr);
        }
    }
    else
    {
        dprintf("unable to resolve %s\n",args);
    }
    
#endif /*KDEXT_64BIT*/    
}


DECLARE_API(hps64)
{
    INIT_API();
#ifdef KDEXT_64BIT

    ULONG64 Peb = GetExpression(args);

    if (!Peb)
    {
        Peb = 0x6fbfffde000;
    }

    ULONG NumberOfHeapsOffset;
    ULONG HeapsOffset;
    
    ULONG SegmentsOffset;    
    
    if ( Peb &&
         (0 == GetFieldOffset("ntdll!_PEB","NumberOfHeaps",&NumberOfHeapsOffset)) &&
         (0 == GetFieldOffset("ntdll!_PEB","ProcessHeaps",&HeapsOffset)) &&
         (0 == GetFieldOffset("ntdll!_HEAP","Segments",&SegmentsOffset)))
    {
        //dprintf(" %x %x\n",NumberOfHeapsOffset,HeapsOffset);
        ULONG nHeaps;
        ULONG64 MemAddr;
        if (ReadMemory(Peb+NumberOfHeapsOffset,&nHeaps,sizeof(ULONG),NULL))
        {
            //dprintf("nHeaps %08x\n",nHeaps);
            ReadMemory(Peb+HeapsOffset,&MemAddr,sizeof(ULONG64),NULL);
            ULONG64 * pHeaps = (ULONG64 *)_alloca(sizeof(ULONG64)*(DWORD)nHeaps);
            ReadMemory(MemAddr,pHeaps,sizeof(ULONG64)*(DWORD)nHeaps,NULL);

            //  +0x0a0 Segments         : [64] 0x000006fb`f9fa0c50

            ULONG64 Segments[64];
                        
            for(ULONG i=0;i<nHeaps;i++)
            {
                if (ReadMemory(pHeaps[i]+SegmentsOffset,Segments,sizeof(Segments),NULL))
                {
                    for (DWORD j=0;j<64;j++)
                    {
                        if (Segments[j])
                        {
                            dprintf("        S %p\n",Segments[j]);
                        }
                        if (CheckControlC())
                            break;                        
                    }
                }
                dprintf("    %p\n",pHeaps[i]);
                
                if (CheckControlC())
                    break;                
            }
        }
        else
        {
            dprintf("RM %p\n",Peb+NumberOfHeapsOffset);
        }
    }
    else
    {
        dprintf("check symbols for ntdll.dll or validate %p as PEB\n",Peb);
    }
#endif /*KDEXT_64BIT*/
}

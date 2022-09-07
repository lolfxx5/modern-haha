// ==++==
// 
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// 
// ==--==
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XX                                                                           XX
XX                          GCInfo                                           XX
XX                                                                           XX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*/

#include "jitpch.h"
#pragma hdrstop

#include "GCInfo.h"
#include "emit.h"
#include "malloc.h"     // for alloca

/*****************************************************************************/
#if TRACK_GC_REFS
/*****************************************************************************/

/*****************************************************************************/
#define REGEN_SHORTCUTS 0
// To Regenerate the compressed info header shortcuts, define REGEN_SHORTCUTS
// and use the following UNIX command line pipe/filter to give you the 128
// most useful encodings.  Unix utilities available at www.cygnus.com
//
// jit -n:D- @all.lst | sort | uniq -c | sort -r | head -128

#define REGEN_CALLPAT 0
// To Regenerate the compressed info header shortcuts, define REGEN_CALLPAT
// and use the following UNIX command line pipe/filter to give you the 80
// most useful encodings.  Unix utilities available at www.cygnus.com
//
// jit -n:D- @all.lst | sort | uniq -c | sort -r | head -80

/*****************************************************************************/

#if GC_WRITE_BARRIER_CALL && defined(NOT_JITC)
extern int         JITGcBarrierCall;
#else
int                JITGcBarrierCall = 1;
#endif

/*****************************************************************************/

#if MEASURE_PTRTAB_SIZE
/* static */ unsigned       Compiler::s_gcRegPtrDscSize = 0;
/* static */ unsigned       Compiler::s_gcTotalPtrTabSize = 0;
#endif

#if GC_WRITE_BARRIER_CALL && defined(NOT_JITC)
/* static */ void *         Compiler::s_gcWriteBarrierPtr = NULL;
#else
/* static */ const void *   Compiler::s_GCptrTable[128];
/* static */ void *         Compiler::s_gcWriteBarrierPtr = Compiler::s_GCptrTable;
#endif

void                Compiler::gcInit()
{
}

/*****************************************************************************/
#ifndef OPT_IL_JIT
/*****************************************************************************
 *
 *  If the given tree value is sitting in a register, free it now.
 */

void                Compiler::gcMarkRegPtrVal(GenTreePtr tree)
{
    if  (varTypeIsGC(tree->TypeGet()))
    {
        if  (tree->gtOper == GT_LCL_VAR)
            genMarkLclVar(tree);

        if  (tree->gtFlags & GTF_REG_VAL)
            gcMarkRegSetNpt(genRegMask(tree->gtRegNum));
    }
}

/*****************************************************************************/

#ifdef  DEBUG

void                Compiler::gcRegPtrSetDisp(unsigned regMask, bool fixed)
{
    unsigned        regNum;

    assert(REG_STK+1 == REG_COUNT);

    for (regNum = 0; regNum < REG_STK; regNum++)
    {
        if  (regMask & genRegMask((regNumber)regNum))
        {
            char    reg[10];

            strcpy(reg, compRegVarName((regNumber)regNum));

#ifndef _WIN32_WCE
            _strlwr(reg+1);
#endif

            printf("%3s", reg);
        }
        else
        {
            if  (fixed)
                printf("   ");
        }
    }
}

#endif

/*****************************************************************************/
#endif // OPT_IL_JIT
/*****************************************************************************
 *
 *  Initialize the non-register pointer variable tracking logic.
 */

void                Compiler::gcVarPtrSetInit()
{
    gcVarPtrSetCur = 0;

    /* Initialize the list of lifetime entries */

    gcVarPtrList =
    gcVarPtrLast = (varPtrDsc *)compGetMem(sizeof(*gcVarPtrList));

    gcVarPtrList->vpdNext =
    gcVarPtrList->vpdPrev = 0;
}

/*****************************************************************************
 *
 *  Allocate a new pointer register set / pointer argument entry and append
 *  it to the list.
 */

Compiler::regPtrDsc  *        Compiler::gcRegPtrAllocDsc()
{
    regPtrDsc  *    regPtrNext;

    assert(genFullPtrRegMap);

    /* Allocate a new entry and initialize it */

    regPtrNext = (regPtrDsc *)compGetMem(sizeof(*regPtrNext));

    regPtrNext->rpdEpilog        = FALSE;
    regPtrNext->rpdIsThis        = FALSE;

    regPtrNext->rpdOffs          = 0;
//  regPtrNext->rpdNext          = 0;

    /* Append the entry to the end of the list */

    assert(gcRegPtrList);
    assert(gcRegPtrLast);

    /* Note that we don't set the 'next' link for the new entry */

    gcRegPtrLast->rpdNext  = regPtrNext;
    gcRegPtrLast           = regPtrNext;

#if MEASURE_PTRTAB_SIZE
    s_gcRegPtrDscSize += sizeof(*regPtrNext);
#endif

    return  regPtrNext;
}

/*****************************************************************************
 *
 *  Compute the various counts that get stored in the info block header.
 */

void                Compiler::gcCountForHeader(unsigned short* untrackedCount,
                                               unsigned short* varPtrTableSize)
{
    unsigned        varNum;
    LclVarDsc *     varDsc;
    varPtrDsc *     varTmp;

    assert(gcVarPtrList);
    assert(gcVarPtrLast);

    /* Terminate the linked list of variable lifetimes */

    gcVarPtrLast->vpdNext = 0;

    /* Skip over the initial fake lifetime entry */

    gcVarPtrList = gcVarPtrList->vpdNext;

    if  (genFullPtrRegMap)
    {
        assert(gcRegPtrList);
        assert(gcRegPtrLast);

        /* Terminate the linked list */

        gcRegPtrLast->rpdNext = 0;

        /* The first entry in the list is fake */

        gcRegPtrList = gcRegPtrList->rpdNext;
    }
    else
    {
        assert(gcCallDescList);
        assert(gcCallDescLast);

        /* Terminate the linked list of call descriptors */

        gcCallDescLast->cdNext = 0;

        /* Skip over the initial fake call entry */

        gcCallDescList = gcCallDescList->cdNext;
    }

    bool        thisIsInUntracked = false; // did we track "this" ?
    unsigned    count = 0;

    /* Count the untracked locals and non-enregistered args */

    for (varNum = 0, varDsc = lvaTable;
         varNum < lvaCount;
         varNum++  , varDsc++)
    {
        if  (varTypeIsGC(varDsc->TypeGet()))
        {
            /* Do we have an argument or local variable? */
            if  (!varDsc->lvIsParam)
            {
                if  (varDsc->lvTracked || !varDsc->lvOnFrame)
                    continue;
            }
            else
            {
                /* Stack-passed arguments which are not enregistered
                 * are always reported in this "untracked stack
                 * pointers" section of the GC info even if lvTracked==true
                 */

                /* Has this argument been enregistered? */
                if  (varDsc->lvRegister)
                {
                    /*
                       Special case: include the stack location of 'this'
                       for synchronized methods, so that runtime can find
                       'this' in case an exception goes by.
                     */
#if !USE_FASTCALL
                    if  (varNum != 0 || !(info.compFlags & FLG_SYNCH))
#endif
                        continue;
                }
                else
                {
                    if  (!varDsc->lvOnFrame)
                    {
                        /* If this non-enregistered pointer arg is never
                         * used, we dont need to report it
                         */
                        assert(varDsc->lvRefCnt == 0);
                        continue;
                    }
#if USE_FASTCALL
                    else  if (varDsc->lvIsRegArg && varDsc->lvTracked)
                    {
                        /* If this register-passed arg is tracked, then
                         * it has been allocated space near the other
                         * pointer variables and we have accurate life-
                         * time info. It will be reported with
                         * gcVarPtrList in the "tracked-pointer" section
                         */

                        continue;
                    }
#endif
                }
            }

            if (varDsc->lvIsThis)
            {
                // Encoding of untracked variables does not support reporting
                // "this". So report it as a tracked variable with a liveness
                // extending over the entire method.

                thisIsInUntracked = true;
                continue;
            }

#ifdef  DEBUG
            if  (verbose)
            {
                int         offs = varDsc->lvStkOffs;

                printf("GCINFO: untrckd %s lcl at [%s",
                        varTypeGCstring(varDsc->TypeGet()),
                        genFPused ? "EBP" : "ESP");

                if      (offs < 0)
                    printf("-%02XH", -offs);
                else if (offs > 0)
                    printf("+%02XH", +offs);

                printf("]\n");
            }
#endif

            count++;
        }
        else if  (varDsc->lvType == TYP_STRUCT && varDsc->lvOnFrame)
        {
            assert(!varDsc->lvTracked);

            CLASS_HANDLE cls = lvaLclClass(varNum);
            assert(cls != 0);
            if (cls == REFANY_CLASS_HANDLE)
            {
                count++;    // Any REFANY has one GC (interior) pointer
            }
            else
            {
                unsigned slots = roundUp(eeGetClassSize(cls), sizeof(void*)) / sizeof(void*);
                bool* gcPtrs = (bool*) _alloca(slots*sizeof(bool));
                eeGetClassGClayout(cls, gcPtrs);

                    // walk each member of the array
                for (unsigned i = 0; i < slots; i++)
                    if (gcPtrs[i])     // count only gc slots
                        count++;
            }
        }
    }

    /* Also count spill temps that hold pointers */

    for (TempDsc * tempThis = tmpListBeg();
         tempThis;
         tempThis = tmpListNxt(tempThis))
    {
        if  (varTypeIsGC(tempThis->tdTempType()) == false)
            continue;

#ifdef  DEBUG
        if  (verbose)
        {
            int         offs = tempThis->tdTempOffs();

            printf("GCINFO: untrck %s Temp at [%s",
                    varTypeGCstring(varDsc->TypeGet()),
                    genFPused ? "EBP" : "ESP");

            if      (offs < 0)
                printf("-%02XH", -offs);
            else if (offs > 0)
                printf("+%02XH", +offs);

            printf("]\n");
        }
#endif

        count++;
    }

#ifdef  DEBUG
    if (verbose) printf("GCINFO: untrckVars = %u\n", count);
#endif

    *untrackedCount = count;

    /* Count the number of entries in the table of non-register pointer
       variable lifetimes. */

    count = 0;

    if (thisIsInUntracked)
        count++;

    if  (gcVarPtrList)
    {
        /* We'll use a delta encoding for the lifetime offsets */

        for (varTmp = gcVarPtrList; varTmp; varTmp = varTmp->vpdNext)
        {
            /* Special case: skip any 0-length lifetimes */

            if  (varTmp->vpdBegOfs == varTmp->vpdEndOfs)
                continue;

            count++;
        }
    }

#ifdef  DEBUG
    if (verbose) printf("GCINFO: trackdLcls = %u\n", count);
#endif

    *varPtrTableSize = count;
}

/*****************************************************************************
 *
 *  Shutdown the 'pointer value' register tracking logic and save the necessary
 *  info (which will be used at runtime to locate all pointers) at the specified
 *  address. The number of bytes written to 'destPtr' must be identical to that
 *  returned from gcPtrTableSize().
 */

BYTE    *           Compiler::gcPtrTableSave(BYTE *          destPtr,
                                             const InfoHdr & header,
                                             unsigned        codeSize)
{
    /* Write the tables to the info block */

    return  destPtr + gcMakeRegPtrTable(destPtr, -1, header, codeSize);
}

/*****************************************************************************
 *
 *  Initialize the 'pointer value' register/argument tracking logic.
 */

void                Compiler::gcRegPtrSetInit()
{
    gcRegGCrefSetCur =
    gcRegByrefSetCur = 0;

    if  (genFullPtrRegMap)
    {
        gcRegPtrList =
        gcRegPtrLast = (regPtrDsc *)compGetMem(roundUp(sizeof(*gcRegPtrList)));

//      gcRegPtrList->rpdNext            = 0;
        gcRegPtrList->rpdOffs            = 0;
        gcRegPtrList->rpdCompiler.rpdAdd =
        gcRegPtrList->rpdCompiler.rpdDel = 0;
    }
    else
    {
        /* Initialize the 'call descriptor' list */

        gcCallDescList =
        gcCallDescLast = (CallDsc *)compGetMem(sizeof(*gcCallDescList));
    }
}

/*****************************************************************************
 *
 *  Helper passed to genEmitter.emitCodeEpilogLst() to generate
 *  the table of epilogs.
 */

/* static */ size_t Compiler::gcRecordEpilog(void * pCallBackData,
                                             unsigned offset)
{
    Compiler  *     pCompiler = (Compiler *)pCallBackData;

    ASSert(pCompiler);

    size_t result = encodeUDelta(pCompiler->gcEpilogTable,
                                 offset,
                                 pCompiler->gcEpilogPrevOffset);

    if (pCompiler->gcEpilogTable)
        pCompiler->gcEpilogTable += result;

    pCompiler->gcEpilogPrevOffset = offset;

    return result;
}

/*****************************************************************************/
#endif // TRACK_GC_REFS
/*****************************************************************************/

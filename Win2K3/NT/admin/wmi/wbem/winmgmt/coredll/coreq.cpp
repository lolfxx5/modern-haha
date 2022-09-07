/*++

Copyright (C) 1996-2001 Microsoft Corporation

Module Name:

    EXECQ.CPP

Abstract:

  Implements classes related to abstract execution queues.

  Classes implemeted:

      CCoreExecReq    An abstract request.
      CCoreQueue      A queue of requests with an associated thread

History:

      23-Jul-96   raymcc    Created.
      3/10/97     levn      Fully documented (heh, heh)
      14-Aug-99   raymcc    Changed timeouts
      30-Oct-99   raymcc    Critsec changes for NT Wksta Stress Oct 30 1999
--*/

#include "precomp.h"
#include <stdio.h>
#include <wbemcomn.h>
#include <cominit.h>
#include <sync.h>
#include "genutils.h"
#include "tls.h"
#include <wbemcore.h>
#include <wmiarbitrator.h>
#include <scopecheck.h>

#define IDLE_THREAD_TIMEOUT     12000
#define OVERLFLOW_TIMEOUT        5000




//***************************************************************************

long CCoreQueue::m_lEmergencyThreads = 0 ;
long CCoreQueue::m_lPeakThreadCount = 0 ;
long CCoreQueue::m_lPeakEmergencyThreadCount = 0;
    
CTLS g_QueueTlsIndex;

// This is TLS init for the purpose of hiding the __thisnamespace class from users.
// The idea is that we use this TLS slot to hold the flag so that
// the repository can then access this slot to determine to skip a check or not.
// The index is passed down to the repository at Logon time (cfgmgr.cpp)
CTLS g_SecFlagTlsIndex;

CCoreExecReq::CCoreExecReq(): 
    m_hWhenDone(NULL), 
    m_pNext(NULL),
    m_lPriority(0), 
    m_phTask(NULL),
    m_fOk(true)
{
}

CCoreExecReq::~CCoreExecReq()
{
    if (m_phTask)
    {
        m_phTask->Release();
        m_phTask = 0;
    }
}

HRESULT CCoreExecReq::SetTaskHandle(_IWmiCoreHandle *phTask)
{
    if (phTask)
    {
        phTask->AddRef();
        m_phTask = phTask;
    }
    return WBEM_S_NO_ERROR;
}

DWORD CCoreQueue::GetTlsIndex()
{
    return g_QueueTlsIndex.GetIndex();
}

//
// Get the security flag TLS index.
//
DWORD CCoreQueue::GetSecFlagTlsIndex ( )
{
    return g_SecFlagTlsIndex.GetIndex() ;
}

//
//
//
void CCoreQueue::SetArbitrator(_IWmiArbitrator* pArbitrator)
{
    if (!m_pArbitrator)
    {
           m_pArbitrator = pArbitrator;
           if (m_pArbitrator)
               m_pArbitrator->AddRef();
    }
}

//
// Be VERY CAREFUL when using this function.  It's here to support
// merger requests which actually execute a number of unqueued requests
// and in order to stay consistent, we want to ensure that the current
// request points to the true request, and not the dummy request we
// created as a starting point.
//

HRESULT CCoreQueue::ExecSubRequest( CCoreExecReq* pNewRequest )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

    CThreadRecord* pRecord = (CThreadRecord*)TlsGetValue(GetTlsIndex());
    if(pRecord)
    {
        if ( !pRecord->m_bExitNow )
        {
            CCoreExecReq* pCurrReq = pRecord->m_pCurrentRequest;

            pRecord->m_pCurrentRequest = pNewRequest;
            
            if (!pRecord->m_pQueue->Execute( pRecord ))
            {
                // here the request has gone already
            }
                
            // Restore the request
            pRecord->m_pCurrentRequest = pCurrReq;
        }
        else
        {
            hr = WBEM_E_SHUTTING_DOWN;
        }
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}


CCoreQueue::CThreadRecord::CThreadRecord(CCoreQueue* pQueue):
      m_pQueue(pQueue),
      m_pCurrentRequest(NULL),
      m_bReady(FALSE),
      m_bExitNow(FALSE)
{
    m_hAttention = CreateEvent(NULL, FALSE, FALSE, NULL);  
    if (NULL == m_hAttention) throw CX_MemoryException();
    m_pQueue->AddRef();
    m_pArb = m_pQueue->GetArbitrator();
}

CCoreQueue::CThreadRecord::~CThreadRecord()
{
    CloseHandle(m_hAttention);
    if (m_hThread) CloseHandle(m_hThread); 
    m_pQueue->Release();
    if (m_pArb) m_pArb->Release();
}

void CCoreQueue::CThreadRecord::Signal()
{
    SetEvent(m_hAttention);
}


//******************************************************************************
//
//  See execq.h for documentation
//
//******************************************************************************
CCoreQueue::CCoreQueue() :
    m_lNumThreads(0),
    m_lMaxThreads(1),
    m_lNumIdle(0),
    m_lNumRequests(0),
    m_pHead(NULL),
    m_pTail(NULL),
    m_dwTimeout(IDLE_THREAD_TIMEOUT),
    m_dwOverflowTimeout(OVERLFLOW_TIMEOUT),
    m_lHiPriBound(-1),
    m_lHiPriMaxThreads(1),
    m_lRef(0),
    m_bShutDownCalled(FALSE),
    m_pArbitrator(NULL)
{
    SetRequestLimits(4000);
}

//******************************************************************************
//
//  See execq.h for documentation
//
//******************************************************************************
CCoreQueue::~CCoreQueue()
{
    try
    {
        Shutdown(FALSE); // SystemShutDown is called Explicitly

        // Remove all outstanding requests
        // ===============================

        while(m_pHead)
        {
            CCoreExecReq* pReq = m_pHead;
            m_pHead = m_pHead->GetNext();
            delete pReq;
        }

        if ( m_pArbitrator )
        {
            m_pArbitrator->Release ( ) ;
            m_pArbitrator = NULL ;
        }

    }   // end try
    catch(...) // To protect svchost.exe; we know this isn't a good recovery for WMI
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::~CCoreQueue() exception\n"));
    }
}

void CCoreQueue::Shutdown(BOOL bIsSystemShutDown)
{
    try
    {
        CCritSecWrapper cs(&m_cs);

        if (!m_bShutDownCalled)
        {
            m_bShutDownCalled = TRUE;
        }
        else
        {
            return;
        }

        // Get all member thread handles
        // =============================

        cs.Enter();                                   
        int nNumHandles = m_aThreads.Size();
        HANDLE* ah = new HANDLE[nNumHandles];
        if (NULL == ah)
        {
            nNumHandles = 0;
        }
        DEBUGTRACE((LOG_WBEMCORE, "Queue is shutting down!\n"));

        int i;
        for(i = 0; i < nNumHandles; i++)
        {
            CThreadRecord* pRecord = (CThreadRecord*)m_aThreads[i];
            ah[i] = pRecord->m_hThread;

            // Inform the thread it should go away when ready
            // ==============================================

            pRecord->m_bExitNow = TRUE;

            // Wake it up if necessary
            // =======================
        
            pRecord->Signal();
        }
        cs.Leave();


        // Make sure all our threads are gone
        // ==================================

        if(nNumHandles > 0 && !bIsSystemShutDown)
        {
            WaitForMultipleObjects(nNumHandles, ah, TRUE, 10000);
        }

        for(i = 0; i < nNumHandles; i++)
            CloseHandle(ah[i]);

        delete [] ah;

    }   // end try
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::Shutdown() exception\n"));
    }
}


void CCoreQueue::Enter()
{
    m_cs.Enter();
}

void CCoreQueue::Leave()
{
    m_cs.Leave();
}

//******************************************************************************
//
//
//******************************************************************************
void CCoreQueue::Register(CThreadRecord* pRecord)
{
    g_QueueTlsIndex.Set((void*)pRecord);
}

void CCoreQueue::Unregister()
{
    g_QueueTlsIndex.Set(NULL);
}


BOOL CCoreQueue::IsSuitableThread(CThreadRecord* pRecord, CCoreExecReq* pReq)
{
    if(pRecord->m_pCurrentRequest == NULL)
        return TRUE;

    // This thread is in the middle of something. By default, ignore it
    // ================================================================

    return FALSE;
}

//******************************************************************************
//
//
//******************************************************************************
HRESULT CCoreQueue::Enqueue(CCoreExecReq* pRequest, HANDLE* phWhenDone)
{
    try
    {

        CCritSecWrapper cs(&m_cs);

        if (m_bShutDownCalled) return WBEM_E_SHUTTING_DOWN;
        if (!pRequest->IsOk()) return WBEM_E_OUT_OF_MEMORY;


        // Create an event handle to signal when request is finished, if required
        // ======================================================================

        if(phWhenDone)
        {
            *phWhenDone = CreateEvent(NULL, FALSE, FALSE, NULL);

            if ( NULL == *phWhenDone )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

            pRequest->SetWhenDoneHandle(*phWhenDone);
        }

        cs.Enter(); 
        
        // Search for a suitable thread
        // ============================

        for(int i = 0; i < m_aThreads.Size(); i++)
        {
            CThreadRecord* pRecord = (CThreadRecord*)m_aThreads[i];

            if(pRecord->m_bReady)
            {
                // Free. Check if suitable
                // =======================

                if(IsSuitableThread(pRecord, pRequest))
                {
                    pRecord->m_pCurrentRequest = pRequest;
                    pRecord->m_bReady = FALSE;
                    pRecord->Signal();
                    m_lNumIdle--;

                    // Done!
                    // =====

                    cs.Leave();
                    return WBEM_S_NO_ERROR;
                }
            }
        }

        BOOL bNeedsAttention = FALSE ;

        if ( IsDependentRequest ( pRequest ) || ( (CWbemRequest*) pRequest)->GetForceRun ( ) > 0 )
        {
            bNeedsAttention = TRUE ;
        }

        // No suitable thread found. Add to the queue
        // ==========================================

        if(m_lNumRequests >= m_lAbsoluteLimitCount && !bNeedsAttention )
        {
            cs.Leave();
            return WBEM_E_FAILED;
        }

        // Search for insert position based on priority
        // ============================================
        if ( bNeedsAttention )
        {
            // the requests that have called SetForceRun have already set their priority
            if (0 == ((CWbemRequest*)pRequest)->GetForceRun())
                pRequest->SetPriority(PriorityNeedsAttentionRequests);
        }
        else
            AdjustInitialPriority(pRequest);

        if ( bNeedsAttention )
        {
            if (!CreateNewThread ( TRUE ))
            {
                //
                // Always return a failure if we can not create a thread.
                // Avoiding very rare deadlocks when rest of system is
                // dependent on a request to finish.
                //
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
        else
        {
            if(DoesNeedNewThread(pRequest, true))
            {
                if (!CreateNewThread())
                {
                    //
                    // Always return a failure if we can not create a thread.
                    // Avoiding very rare deadlocks when rest of system is
                    // dependent on a request to finish.
                    //
                    return WBEM_E_OUT_OF_MEMORY;                    
                }    // IF !CreateNewThread

            }    // IF DoesNeedNewThread

        }    // ELSE !bNeedsAttention

        HRESULT hr = PlaceRequestInQueue( pRequest );

        long lIndex = m_lNumRequests;
        cs.Leave();

        if ( SUCCEEDED( hr ) )
        {
            SitOutPenalty(lIndex);
        }

        return hr;

    }   // end try
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::Enqueue() exception\n"));
        return WBEM_E_CRITICAL_ERROR;
    }
}


//******************************************************************************
//
//  See dbgalloc.h for documentation
//
//******************************************************************************
HRESULT CCoreQueue::EnqueueWithoutSleep(CCoreExecReq* pRequest, HANDLE* phWhenDone)
{
    try
    {

        CCritSecWrapper cs(&m_cs);

        if (m_bShutDownCalled) return WBEM_E_SHUTTING_DOWN;
        if (!pRequest->IsOk()) return WBEM_E_OUT_OF_MEMORY;

        // Create an event handle to signal when request is finished, if required
        // ======================================================================

        if(phWhenDone)
        {
            *phWhenDone = CreateEvent(NULL, FALSE, FALSE, NULL); 
            if (NULL == *phWhenDone)
                return WBEM_E_OUT_OF_MEMORY;
            pRequest->SetWhenDoneHandle(*phWhenDone);
        }

        cs.Enter();  

        // Search for a suitable thread
        // ============================

        for(int i = 0; i < m_aThreads.Size(); i++)
        {
            CThreadRecord* pRecord = (CThreadRecord*)m_aThreads[i];

            if(pRecord->m_bReady)
            {
                // Free. Check if suitable
                // =======================

                if(IsSuitableThread(pRecord, pRequest))
                {
                    pRecord->m_pCurrentRequest = pRequest;
                    pRecord->m_bReady = FALSE;
                    pRecord->Signal();
                    m_lNumIdle--;

                    // Done!
                    // =====

                    cs.Leave();
                    return WBEM_S_NO_ERROR;
                }
            }
        }

        // No suitable thread found. Add to the queue
        // ==========================================

        if(m_lNumRequests >= m_lAbsoluteLimitCount)
        {
            cs.Leave();
            return WBEM_E_FAILED;
        }

        // Search for insert position based on priority
        // ============================================

        AdjustInitialPriority(pRequest);

        // Create a new thread, if required
        // ================================
        if(DoesNeedNewThread(pRequest, true))
        {
            if (!CreateNewThread())
            {
                //
                // Always return a failure if we can not create a thread.
                // Avoiding very rare deadlocks when rest of system is
                // dependent on a request to finish.
                //
                return WBEM_E_OUT_OF_MEMORY;                
            }
        }

        HRESULT hr = PlaceRequestInQueue( pRequest );

        long lIndex = m_lNumRequests;
        cs.Leave();

        return hr;

    }   // end try
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::EnqueueWithoutSleep() exception\n"));
        return WBEM_E_CRITICAL_ERROR;
    }
}

HRESULT CCoreQueue::PlaceRequestInQueue( CCoreExecReq* pRequest )
{
    CCoreExecReq* pCurrent = m_pHead;
    CCoreExecReq* pLast = NULL;

    // Tracks whether or not we need to cleanup the queue
    bool        bQueued = false;

    try
    {
        // Find a spot in the current queue based on priority
        while(pCurrent && pCurrent->GetPriority() <= pRequest->GetPriority())
        {
            pLast = pCurrent;
            pCurrent = pCurrent->GetNext();
        }

        // Insert
        // ======

        // If we have a pCurrent pointer, then pRequest is higher in priority, so should
        // be inserted before it.  Otherwise, we are inserting at the end of the queue

        if(pCurrent)
        {
            pRequest->SetNext(pCurrent);
        }
        else
        {
            m_pTail = pRequest;
        }

        // If we have a pLast pointer, we need to point it at pRequest, otherwise, we
        // are inserting at the head of the queue.
        if(pLast)
        {
            pLast->SetNext(pRequest);
        }
        else
        {
            m_pHead = pRequest;
        }

        m_lNumRequests++;

        bQueued = true;

        // Adjust priorities of the loosers
        // ================================

        while(pCurrent)
        {
            AdjustPriorityForPassing(pCurrent);
            pCurrent = pCurrent->GetNext();
        }

        return WBEM_S_NO_ERROR;
    }
    catch( ... )
    {
        // Fixup the queue if necessary
        if ( bQueued )
        {
            // Fixup the tail to point to the last request
            if ( pRequest == m_pTail )
            {
                m_pTail = pLast;
            }

            // Fixup the head to point to the next request
            if ( pRequest == m_pHead )
            {
                m_pHead = pRequest->GetNext();
            }

            // Fixup pLast to skip over the current request
            if ( NULL != pLast )
            {
                pLast->SetNext( pRequest->GetNext() );
            }

            // Decrement this
            m_lNumRequests--;

        }

        return WBEM_E_CRITICAL_ERROR;
    }

}

DWORD CCoreQueue::CalcSitOutPenalty(long lRequestIndex)
{
    if(lRequestIndex <= m_lStartSlowdownCount)
        return 0; // no penalty

    if(lRequestIndex >= m_lAbsoluteLimitCount)
        lRequestIndex = m_lAbsoluteLimitCount;

    // Calculate the timeout
    // =====================

    double dblTimeout =
        m_dblAlpha / (m_lAbsoluteLimitCount - lRequestIndex) +
            m_dblBeta;

    // Return penalty
    // ===========

    return ((DWORD) dblTimeout);
}

void CCoreQueue::SitOutPenalty(long lRequestIndex)
{
    DWORD   dwSitOutPenalty = CalcSitOutPenalty( lRequestIndex );

    // Sleep on it
    // ===========

    if ( 0 != dwSitOutPenalty )
    {
        Sleep( dwSitOutPenalty );
    }
}


HRESULT CCoreQueue::EnqueueAndWait(CCoreExecReq* pRequest)
{
    if(IsAppropriateThread())
    {
        pRequest->Execute();
        delete pRequest;
        return WBEM_S_NO_ERROR;
    }

    HANDLE hWhenDone = NULL;
    HRESULT hr = Enqueue(pRequest, &hWhenDone);

    // Scoped closing of the handle
    CCloseMe    cmWhenDone( hWhenDone );

    if ( FAILED(hr) )
    {
        return hr;
    }

    DWORD dwRes = CCoreQueue::QueueWaitForSingleObject(hWhenDone, INFINITE);

    return ( dwRes == WAIT_OBJECT_0 ? WBEM_S_NO_ERROR : WBEM_E_FAILED );
}


BOOL CCoreQueue::DoesNeedNewThread(CCoreExecReq* pRequest, bool bIgnoreNumRequests )
{
    // We will ignore the number or requests ONLY if requested
    // Default is to check if there are any threads in the queue

    if(m_lNumIdle > 0 || ( !bIgnoreNumRequests && m_lNumRequests == 0 ) )
        return FALSE;

    if(m_lNumThreads < m_lMaxThreads)
        return TRUE;
    else if(pRequest->GetPriority() <= m_lHiPriBound &&
            m_lNumThreads < m_lHiPriMaxThreads)
        return TRUE;
    else
        return FALSE;
}

//
// takes ownership of the CCoreExecRequest
//  
///////////////////////////////////////////////////////////
BOOL CCoreQueue::pExecute(CThreadRecord* pRecord)
{
    wmilib::auto_ptr<CCoreExecReq> pReq( pRecord->m_pCurrentRequest);
    CAutoSignal SetMe(pReq->GetWhenDoneHandle());
    NullPointer NullMe((PVOID *)&pRecord->m_pCurrentRequest);

    HRESULT hres;
    {
#ifdef WMI_PRIVATE_DBG
#ifdef DBG
        CTestNullTokenOnScope Test(__LINE__,__FILE__);
#endif
#endif
        hres =  pReq->Execute();
    }

    if(FAILED(hres))
    {
        LogError(pReq.get(), hres);
        return FALSE;
    }
        
    return TRUE;
}

BOOL CCoreQueue::Execute(CThreadRecord* pRecord)
{
    __try
    {
        return pExecute(pRecord);
    }
    __except(ExceptFilter(GetExceptionInformation(),GetExceptionCode()))
    {
        return FALSE;
    }    
}

//******************************************************************************
//
//  See dbgalloc.h for documentation
//
//******************************************************************************
void CCoreQueue::LogError(CCoreExecReq* pRequest, int nRes)
{
    try
    {
        //DbgPrintfA(0,"Error %08x occured executing request for %S\n", nRes,pRequest->GetReqInfo());
        DEBUGTRACE((LOG_WBEMCORE,"Error %08x occured executing request for %S\n", nRes,pRequest->GetReqInfo()));
        pRequest->DumpError();
    }
    catch (CX_MemoryException &)
    {
        // we might be using Internal CWbemClass|Instance
        // interfaces that throws.
        // the caller thread is unprepared to handle exceptions ...
    }
}

HRESULT CCoreQueue::InitializeThread()
{
    return InitializeCom();
}

void CCoreQueue::UninitializeThread()
{
    CoUninitialize();
}


CCoreExecReq* CCoreQueue::SearchForSuitableRequest(CThreadRecord* pRecord)
{
    try
    {
        CWmiArbitrator* pArb = (CWmiArbitrator*)pRecord->m_pArb;

        // Assumes in critical section
        // ===========================

        CCoreExecReq* pCurrent = m_pHead;
        CCoreExecReq* pPrev = NULL;

        while(pCurrent)
        {
            if(IsSuitableThread(pRecord, pCurrent))
            {
                // Always except dependent requests, otherwise, we only accept requests if we are accepting
                // new tasks

                if ( IsDependentRequest( pCurrent ) || pArb->AcceptsNewTasks(pCurrent) )
                {
                    // Found one --- take it
                    // =====================

                    if(pPrev)
                        pPrev->SetNext(pCurrent->GetNext());
                    else
                        m_pHead = pCurrent->GetNext();

                    if(pCurrent == m_pTail)
                        m_pTail = pPrev;

                    m_lNumRequests--;
                    break;
                }
                else
                {
                    // This means we have a primary task *and* we are not accepting new tasks.  Since there
                    // should NEVER be dependent tasks following primary tasks, we'll give up now.

                    pCurrent = NULL;
                    break;
                }
            }

            pPrev = pCurrent;
            pCurrent = pCurrent->GetNext();
        }

        return pCurrent;

    }   // end try
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::SearchForSuitableRequest() exception\n"));
        return NULL;
    }
}

//******************************************************************************
//
//  See dbgalloc.h for documentation
//
//******************************************************************************
void CCoreQueue::ThreadMain(CThreadRecord* pRecord)
{
    if (FAILED(InitializeThread())) return;
    
    CCritSecWrapper cs(&m_cs);    

    // Register this queue with this thread, so any further wait would be
    // interruptable
    // ==================================================================

    Register(pRecord);
    OnDelete0<void(*)(void),&CCoreQueue::Unregister> UnRegMe;

    while (1)
    {
        // Returning from work. At this point, our event is not signaled,
        // our m_pCurrentRequest is NULL and our m_bReady is FALSE
        // ====================================================================

        // Search for work in the queue
        // ============================

        cs.Enter(); 

        CCoreExecReq* pCurrent = SearchForSuitableRequest(pRecord);
        if(pCurrent)
        {
            // Found some. Take it
            // ===================

            pRecord->m_pCurrentRequest = pCurrent;
        }
        else
        {
            // No work in the queue. Wait
            // ==========================

            pRecord->m_bReady = TRUE;
            m_lNumIdle++;
            DWORD dwTimeout = GetIdleTimeout(pRecord);
            cs.Leave();
            DWORD dwRes = WbemWaitForSingleObject(pRecord->m_hAttention,
                                        dwTimeout);
            cs.Enter(); 

            if(dwRes != WAIT_OBJECT_0)
            {
                // Check if someone managed to place a request in our record
                // after the timeout.
                // =========================================================

                if(WbemWaitForSingleObject(pRecord->m_hAttention, 0) ==
                    WAIT_OBJECT_0)
                {
                    DEBUGTRACE((LOG_WBEMCORE, "AMAZING: Thread %p received "
                        "request %p after timing out. Returning to the "
                        "queue\n", pRecord, pRecord->m_pCurrentRequest));

                    if(pRecord->m_bExitNow || pRecord->m_pCurrentRequest == NULL)
                    {
                        ShutdownThread(pRecord);
                        cs.Leave();
                        return;
                    }
                    pRecord->m_pQueue->Enqueue(pRecord->m_pCurrentRequest);
                    pRecord->m_pCurrentRequest = NULL;
                }

                // Timeout. See if it is time to quit
                // ==================================

                pRecord->m_bReady = FALSE;
                if(IsIdleTooLong(pRecord, dwTimeout))
                {
                    ShutdownThread(pRecord);
                    cs.Leave();
                    return;
                }

                // Go and wait a little more
                // =========================

                m_lNumIdle--;
                cs.Leave();
                continue;
            }
            else
            {
                // Check why we were awaken
                // ========================

                if(pRecord->m_bExitNow || pRecord->m_pCurrentRequest == NULL)
                {
                    ShutdownThread(pRecord);
                    cs.Leave();
                    return;
                }

                // We have a request. Enqueue already adjusted lNumIdle and
                // our m_bReady;
            }
        }

        // Execute the request
        // ===================

        cs.Leave();
        Execute(pRecord);

    }
}

DWORD CCoreQueue::GetIdleTimeout(CThreadRecord* pRecord)
{
    if(m_lNumThreads > m_lMaxThreads)
        return m_dwOverflowTimeout;
    else
        return m_dwTimeout;
}

BOOL CCoreQueue::IsIdleTooLong(CThreadRecord* pRecord, DWORD dwTimeout)
{
    if(m_lNumThreads > m_lMaxThreads)
        return TRUE;
    else if(dwTimeout < m_dwTimeout)
        return FALSE;
    else if ( m_lNumRequests > 0 && m_lNumThreads == 1 )
    {
        // If there are requests in the queue, and we're the only thread in the system, we shouldn't die.
        // The likelihood is that memory usage is causing the arbitrator to refuse tasks, and therefore
        // also disabling us from servicing requests (see SearchForSuitableRequest).
        return FALSE;
    }
    else
        return TRUE;
}

void CCoreQueue::ShutdownThread(CThreadRecord* pRecord)
{
    try
    {
        CCritSecWrapper cs(&m_cs);
        cs.Enter();      
        
        g_QueueTlsIndex.Set(NULL);
        for(int i = 0; i < m_aThreads.Size(); i++)
        {
            if(m_aThreads[i] == pRecord)
            {
                m_aThreads.RemoveAt(i);

                // Make sure we don't close the handle if the queue's Shutdown is
                // waiting on it
                // ==============================================================

                if(pRecord->m_bExitNow)
                    pRecord->m_hThread = NULL;
                delete pRecord;
                m_lNumIdle--;
                m_lNumThreads--;

                break;
            }
        }
        UninitializeThread();
        cs.Leave();

    }   // end try
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::ShutdownThread() exception\n"));
    }
}

//******************************************************************************
//
//  See dbgalloc.h for documentation
//
//******************************************************************************
// static
extern LONG g_lCoreThreads;

DWORD WINAPI CCoreQueue::_ThreadEntry(LPVOID pObj)
{

    try
    {
        InterlockedIncrement(&g_lCoreThreads);

        RecordPeakThreadCount ( );

        CThreadRecord* pRecord = (CThreadRecord*)pObj;
        CCoreQueue* pQueue = pRecord->m_pQueue;
        if(pQueue)
        {
            pQueue->AddRef();
            pQueue->ThreadMain(pRecord);
            pQueue->Release();
        }

        InterlockedDecrement(&g_lCoreThreads);

    }   // end try
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::_ThreadEntry() unhandled exception\n"));
    }

    return 0;
}

DWORD WINAPI CCoreQueue::_ThreadEntryRescue (LPVOID pObj)
{
    try
    {        
        InterlockedIncrement ( &m_lEmergencyThreads ) ;
        RecordPeakThreadCount ( );

        CThreadRecord* pRecord = (CThreadRecord*)pObj;
        CCoreQueue* pQueue = pRecord->m_pQueue;
        if(pQueue)
        {
            pQueue->AddRef();

            CWmiArbitrator* pArb = (CWmiArbitrator*)pRecord->m_pArb;

            if (pArb) pArb->DecUncheckedCount();

            pQueue->ThreadMain(pRecord);

            if (pArb) pArb->IncUncheckedCount();

            pQueue->Release();
        }

        InterlockedDecrement ( &m_lEmergencyThreads ) ;

        return 0;
    }   // end try
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::ThreadEntryRescue() exception\n"));
        return 0;
    }

}

//******************************************************************************
//
//  See dbgalloc.h for documentation
//
//******************************************************************************
BOOL CCoreQueue::CreateNewThread ( BOOL bNeedsAttention )
{
    try
    {
        CInCritSec ics(&m_cs);

        // Create new thread record
        // ========================

        wmilib::auto_ptr<CThreadRecord> pNewRecord( new CThreadRecord(this)); // throws
        if (NULL == pNewRecord.get()) return FALSE;
        if (CFlexArray::no_error != m_aThreads.Add(pNewRecord.get())) return FALSE;

        DWORD dwId;
        if ( !bNeedsAttention )
        {
            pNewRecord->m_hThread = CreateThread(0, 0, _ThreadEntry, pNewRecord.get(), 0,
                                                    &dwId);
        }
        else
        {
            pNewRecord->m_hThread = CreateThread(0, 0, _ThreadEntryRescue, pNewRecord.get(), 0,
                                                    &dwId);
        }

        if(NULL == pNewRecord->m_hThread)
        {
            m_aThreads.RemoveAt(m_aThreads.Size()-1);
            return FALSE;
        }

        pNewRecord.release(); // FlexArray took ownership
        m_lNumThreads++;
        return TRUE;
    }   // end try
    catch(CX_Exception &)
    {
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::CreateNewThread() exception\n"));
        return FALSE;
    }
}

DWORD CompensateForBug(DWORD dwOriginal, DWORD dwElapsed)
{
    if(dwOriginal == 0xFFFFFFFF)
        return 0xFFFFFFFF;

    DWORD dwLeft = dwOriginal - dwElapsed;
    if(dwLeft > 0x7FFFFFFF)
        dwLeft = 0x7FFFFFFF;

    return dwLeft;
}

DWORD CCoreQueue::WaitForSingleObjectWhileBusy(HANDLE hHandle, DWORD dwWait,
                                                CThreadRecord* pRecord)
{
    CCritSecWrapper cs(&m_cs);

    CCoreExecReq* pOld = pRecord->m_pCurrentRequest;
    DWORD dwStart = GetTickCount();
    while (dwWait > GetTickCount() - dwStart)
    {
        // Search for work in the queue
        // ============================

        cs.Enter();
        CCoreExecReq* pCurrent = SearchForSuitableRequest(pRecord);
        if(pCurrent != NULL)
        {
            pRecord->m_pCurrentRequest = pCurrent;

            if(pRecord->m_pCurrentRequest == pOld)
            {
                // Something is very wrong
                // =======================
            }
        }
        else
        {
            // No work in the queue. Wait
            // ==========================

            pRecord->m_bReady = TRUE;

            // Block until a request comes through.
            // ====================================

            HANDLE ahSems[2];
            ahSems[0] = hHandle;
            ahSems[1] = pRecord->m_hAttention;

            cs.Leave();
            DWORD dwLeft = CompensateForBug(dwWait, (GetTickCount() - dwStart));
            DWORD dwRes = WbemWaitForMultipleObjects(2, ahSems, dwLeft);

            cs.Enter();

            pRecord->m_bReady = FALSE;
            if(dwRes != WAIT_OBJECT_0 + 1)
            {
                // Either our target handle is ready or we timed out
                // =================================================

                // Check if anyone placed a request in our record
                // ==============================================

                if(pRecord->m_pCurrentRequest != pOld)
                {
                    // Re-issue it to the queue
                    // ========================

                    pRecord->m_pQueue->Enqueue(pRecord->m_pCurrentRequest);
                    pRecord->m_pCurrentRequest = pOld;

                    // Decrement our semaphore
                    // =======================

                    dwRes = WaitForSingleObject(pRecord->m_hAttention, 0);
                    if(dwRes != WAIT_OBJECT_0)
                    {
                        // Internal error --- whoever placed the request had
                        // to have upped the semaphore
                        // =================================================

                        ERRORTRACE((LOG_WBEMCORE, "Internal error: queue "
                            "semaphore is too low\n"));
                    }
                }

                cs.Leave();
                return dwRes;
            }
            else
            {
                // Check why we were awaken
                // ========================

                if(pRecord->m_bExitNow || pRecord->m_pCurrentRequest == NULL)
                {
                    // Can't exit in the middle of a request. Leave it for later
                    // =========================================================

                    pRecord->Signal();
                    cs.Leave();
                    DWORD dwLeft = CompensateForBug(dwWait,
                                        (GetTickCount() - dwStart));
                    return WbemWaitForSingleObject(hHandle, dwLeft);
                }

                // We've got work to do
                // ====================

                if(pRecord->m_pCurrentRequest == pOld)
                {
                    // Something is very wrong
                    // =======================
                }
            }
        }

        // Execute the request
        // ===================

        cs.Leave();
        Execute(pRecord);
        pRecord->m_pCurrentRequest = pOld;

    }
    return WAIT_TIMEOUT;
}

DWORD CCoreQueue::UnblockedWaitForSingleObject(HANDLE hHandle, DWORD dwWait,
                                                CThreadRecord* pRecord)
{
    CCritSecWrapper cs(&m_cs);

    // Silently bump the max threads count.  We will not allow the queue to reuse
    // this thread, so we need to account for this missing thread while we
    // are blocked.  Essentially, we are hijacking the code that was hijacking
    // the thread

    cs.Enter();
        m_lMaxThreads++;
        m_lHiPriMaxThreads++;
    cs.Leave();

    DWORD   dwRet = WbemWaitForSingleObject( hHandle, dwWait );

    // The thread is back, so bump down the max threads number.  If extra threads were in
    // fact created, they should eventually peter out and go away.
    cs.Enter(); 
        m_lMaxThreads--;
        m_lHiPriMaxThreads--;
    cs.Leave();

    return dwRet;
}


// static
DWORD CCoreQueue::ExceptFilter(LPEXCEPTION_POINTERS pExcptPtrs,DWORD Status)
{
    ExceptionCounter c;

    EXCEPTION_RECORD * pRec = pExcptPtrs->ExceptionRecord;

    ERRORTRACE((LOG_WBEMCORE, "CCoreQueue exception %08x\n",Status));
    switch(Status)
    {
    case STATUS_ACCESS_VIOLATION:
        ERRORTRACE((LOG_WBEMCORE," exr addr %p\n",pRec->ExceptionAddress));
        for (DWORD i=0;i<pRec->NumberParameters;i++)
        {
            ERRORTRACE((LOG_WBEMCORE," exr p[%d] %p\n",i,pRec->ExceptionInformation[i]));
        }
        break;
    default:
        break;
    }
    
    return EXCEPTION_EXECUTE_HANDLER;
};

//******************************************************************************
//
//  See dbgalloc.h for documentation
//
//******************************************************************************
// static
DWORD CCoreQueue::QueueWaitForSingleObject(HANDLE hHandle, DWORD dwWait)
{
    __try
    {

        // Get the queue that is registered for this thread, if any
        // ========================================================

        CThreadRecord* pRecord = (CThreadRecord*)g_QueueTlsIndex.Get(); 

        if(pRecord == NULL)
        {
            // No queue is registered with this thread. Just wait
            // ==================================================

            return WbemWaitForSingleObject(hHandle, dwWait);
        }

        CCoreQueue* pQueue = pRecord->m_pQueue;

        return pQueue->WaitForSingleObjectWhileBusy(hHandle, dwWait, pRecord);

    }
    __except(ExceptFilter(GetExceptionInformation(),GetExceptionCode()))
    {
        return WAIT_TIMEOUT;
    }
}

// static
DWORD CCoreQueue::QueueUnblockedWaitForSingleObject(HANDLE hHandle, DWORD dwWait)
{
    __try
    {
        // Get the queue that is registered for this thread, if any
        // ========================================================

        CThreadRecord* pRecord = (CThreadRecord*)g_QueueTlsIndex.Get();

        if(pRecord == NULL)
        {
            // No queue is registered with this thread. Just wait
            // ==================================================

            return WbemWaitForSingleObject(hHandle, dwWait);
        }

        CCoreQueue* pQueue = pRecord->m_pQueue;

        return pQueue->UnblockedWaitForSingleObject(hHandle, dwWait, pRecord);

    }   // end try
    __except(ExceptFilter(GetExceptionInformation(),GetExceptionCode()))
    {
        return WAIT_TIMEOUT;
    }    
}

BOOL CCoreQueue::SetThreadLimits(long lMaxThreads, long lHiPriMaxThreads,
                                    long lHiPriBound)
{
    m_lMaxThreads = lMaxThreads;
    if(lHiPriMaxThreads == -1)
        m_lHiPriMaxThreads = lMaxThreads * 1.1;
    else
        m_lHiPriMaxThreads = lHiPriMaxThreads;
    m_lHiPriBound = lHiPriBound;

    BOOL bRet = TRUE;

    while(DoesNeedNewThread(NULL) && bRet)
    {
        bRet = CreateNewThread();
    }

    return bRet;
}

BOOL CCoreQueue::IsAppropriateThread()
{
    // Get the queue that is registered for this thread, if any
    // ========================================================

    CThreadRecord* pRecord = (CThreadRecord*)g_QueueTlsIndex.Get();

    if(pRecord == NULL)
        return FALSE;

    CCoreQueue* pQueue = pRecord->m_pQueue;
    if(pQueue != this)
        return FALSE;

    return TRUE;
}

void CCoreQueue::SetRequestLimits(long lAbsoluteLimitCount,
                              long lStartSlowdownCount,
                              long lOneSecondDelayCount)
{
    CCritSecWrapper cs(&m_cs);

    cs.Enter();     // SEC:REVIEWED 2002-03-22 : Assumes success, needs EH

    m_lAbsoluteLimitCount = lAbsoluteLimitCount;

    m_lStartSlowdownCount = lStartSlowdownCount;
    if(m_lStartSlowdownCount < 0)
    {
        m_lStartSlowdownCount = m_lAbsoluteLimitCount / 2;
    }

    m_lOneSecondDelayCount = lOneSecondDelayCount;

    if(m_lOneSecondDelayCount < 0)
    {
        m_lOneSecondDelayCount =
            m_lAbsoluteLimitCount * 0.2 + m_lStartSlowdownCount * 0.8;
    }

    // Calculate coefficients
    // ======================

    m_dblBeta =
        1000 *
        ((double)m_lAbsoluteLimitCount - (double)m_lOneSecondDelayCount) /
        ((double)m_lStartSlowdownCount - (double)m_lOneSecondDelayCount);

    m_dblAlpha = m_dblBeta *
        ((double)m_lStartSlowdownCount - (double)m_lAbsoluteLimitCount);
    cs.Leave();
}


BOOL CCoreQueue::IsDependentRequest ( CCoreExecReq* pRequest )
{
    try
    {
        BOOL bRet = FALSE;
        
        if ( pRequest )
        {
            CWbemRequest* pWbemRequest = (CWbemRequest*) pRequest ;
            if ( pWbemRequest )
            {
                if ( !pWbemRequest->IsDependee ( ) )
                {
                    CWmiTask* pTask = (CWmiTask*) pRequest->m_phTask;
                    if ( pTask )
                    {
                        CWbemNamespace* pNamespace = (CWbemNamespace*) pTask->GetNamespace ( );
                        if ( pNamespace )
                        {
                            if ( pNamespace->GetIsProvider ( ) || pNamespace->GetIsESS ( ) )
                            {
                                bRet = TRUE;
                            }
                        }
                    }
                }
                else
                {
                    bRet = TRUE;
                }
            }
        }
        return bRet;

    }   // end try
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::IsDependentRequest() exception\n"));
        return FALSE;
    }
}



VOID CCoreQueue::RecordPeakThreadCount (  )
{
    if ( ( g_lCoreThreads + m_lEmergencyThreads ) > m_lPeakThreadCount )
    {
        m_lPeakThreadCount = g_lCoreThreads + m_lEmergencyThreads ;
    }
    if ( m_lEmergencyThreads > m_lPeakEmergencyThreadCount )
    {
        m_lPeakEmergencyThreadCount = m_lEmergencyThreads;
    }
}


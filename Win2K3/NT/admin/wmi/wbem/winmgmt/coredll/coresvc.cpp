/*++

Copyright (C) 1996-2001 Microsoft Corporation

Module Name:

    coresvc.cpp

Abstract:

    CCoreServices Class

History:

    raymcc      1-Mar-00        Created

--*/

#include "precomp.h"
#include "wbemcore.h"
#include "wmifinalizer.h"
#include "wmiarbitrator.h"

#pragma warning(disable:4355)

extern IClassFactory* g_pPathFac;
extern IClassFactory* g_pQueryFact;


_IWmiProvSS *CCoreServices::m_pProvSS = 0;
_IWbemFetchRefresherMgr* CCoreServices::m_pFetchRefrMgr = NULL;
CCoreServices* CCoreServices::g_pSvc = 0;
IWbemEventSubsystem_m4 *CCoreServices::m_pEssOld = 0;
_IWmiESS               *CCoreServices::m_pEssNew = 0;

CStaticCritSec CCoreServices::m_csHookAccess;

static BOOL g_bEventsEnabled = FALSE;
static CFlexArray g_aHooks;

LONG g_nSinkCount = 0;
LONG g_nStdSinkCount = 0;
LONG g_nSynchronousSinkCount = 0;
LONG g_nProviderSinkCount = 0;
LONG g_nNamespaceCount = 0;
LONG g_lCoreThreads = 0;

//***************************************************************************
//
//***************************************************************************

struct SHookElement
{
    _IWmiCoreWriteHook *m_pHook;
    ULONG               m_uFlags;
    HRESULT             m_hResPre; // The PreXXX return code may prevent the PostXXX code to be called

    SHookElement();
   ~SHookElement();
    SHookElement(SHookElement &Src);
    SHookElement& operator =(SHookElement &Src);
};



//***************************************************************************
//
//***************************************************************************

HRESULT CCoreServices::Initialize ()
{
    m_pEssOld = 0;
    m_pEssNew = 0;

    g_pSvc = CCoreServices::CreateInstance();
    if (NULL == g_pSvc) return WBEM_E_OUT_OF_MEMORY;
    HRESULT hr = CoCreateInstance( CLSID__WbemFetchRefresherMgr, 
                                NULL, 
                                CLSCTX_INPROC_SERVER,  
                                IID__IWbemFetchRefresherMgr, 
                                (void**) &m_pFetchRefrMgr );

    return hr;
}

//***************************************************************************
//
//***************************************************************************
HRESULT CCoreServices::SetEssPointers(
    IWbemEventSubsystem_m4 *pEssOld,
    _IWmiESS               *pEssNew
    )
{
    m_pEssOld = pEssOld;
    m_pEssOld->AddRef();
    m_pEssNew = pEssNew;
    m_pEssNew->AddRef();
    return 0;
}


//***************************************************************************
//
//***************************************************************************

HRESULT CCoreServices::UnInitialize ()
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    if (m_pProvSS)
    {
        m_pProvSS->Release () ;
        m_pProvSS = NULL ;
    }

    // Cleanup the refresher manager fetcher
    if ( NULL != m_pFetchRefrMgr )
    {
        // We should uninitialize as well - commented out for now
        // m_pFetchRefrMgr->Uninit();
        m_pFetchRefrMgr->Release ();
        m_pFetchRefrMgr = NULL;
    }

    // Free up the perflib

    ReleaseIfNotNULL(m_pEssOld);
    ReleaseIfNotNULL(m_pEssNew);

    ReleaseIfNotNULL(g_pSvc);

    return hRes;
}

//***************************************************************************
//
//***************************************************************************

CCoreServices::CCoreServices() : m_lRef(0)
{
    gClientCounter.AddClientPtr(&m_Entry);
}

//***************************************************************************
//
//***************************************************************************

CCoreServices::~CCoreServices()
{
    gClientCounter.RemoveClientPtr(&m_Entry);
}

ULONG CCoreServices::AddRef()
{
    InterlockedIncrement(&m_lRef);
    return ULONG(m_lRef);
}

//***************************************************************************
//
//***************************************************************************

ULONG CCoreServices::Release()
{
    ULONG uNewCount = (ULONG) InterlockedDecrement(&m_lRef);
    if (0 != uNewCount)
        return uNewCount;
    delete this;
    return 0;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown || riid == IID__IWmiCoreServices)
    {
        AddRef();
        *ppv = (void*)this;
        return S_OK;
    }
    else return E_NOINTERFACE;
}


//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::GetObjFactory(
            /* [in] */ long lFlags,
            /* [out] */ _IWmiObjectFactory __RPC_FAR *__RPC_FAR *pFact)
{
    return WBEM_E_NOT_AVAILABLE;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::GetServices(
            /* [in] */ LPCWSTR pszNamespace,
            /* [in] */ LPCWSTR pszUser,
            /* [in] */ LPCWSTR pszLocale,
            /* [in] */ long lFlags,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *pServices)
{
    BOOL bRepositOnly = false;
    HRESULT hRes;

    if (lFlags & WMICORE_FLAG_REPOSITORY)
        bRepositOnly = true;

    CWbemNamespace *pNs = CWbemNamespace::CreateInstance();
    if (NULL == pNs) return WBEM_E_OUT_OF_MEMORY;
    CReleaseMe rm((IWbemServices *)pNs);

    LPWSTR wszTempLocale = ( LPWSTR ) pszLocale ;
    if ( wszTempLocale == NULL )
    {
        wszTempLocale = GetDefaultLocale();
        if(wszTempLocale == NULL) return WBEM_E_OUT_OF_MEMORY;
    }

    CVectorDeleteMe<WCHAR> vdm(pszLocale ? NULL : wszTempLocale );

    hRes = pNs->Initialize(
        LPWSTR(pszNamespace),
        pszUser ? ( LPWSTR ) pszUser : ADMINISTRATIVE_USER,
        0,
        FULL_RIGHTS,
        FALSE,
        bRepositOnly,
        NULL,
        0XFFFFFFFF,
        FALSE,
        NULL);


    if (SUCCEEDED(hRes))
    {
        pNs->SetLocale (wszTempLocale);
        if ( lFlags & WMICORE_CLIENT_TYPE_PROVIDER )
        {
            pNs->SetIsProvider(TRUE) ;
        }
        if ( lFlags & WMICORE_CLIENT_TYPE_ESS )
        {
            pNs->SetIsESS ( TRUE );
        }

        *pServices = rm.dismiss();

    }

    return hRes;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::GetRepositoryDriver(
            /* [in] */ long lFlags,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *pDriver)
{
    return WBEM_E_NOT_AVAILABLE;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::GetCallSec(
            /* [in] */ long lFlags,
            /* [out] */ _IWmiCallSec __RPC_FAR *__RPC_FAR *pCallSec)
{
    return WBEM_E_NOT_AVAILABLE;
}

//***************************************************************************
//
//***************************************************************************
bool CCoreServices::IsProviderSubsystemEnabled()
{
    HKEY hKey;
    long lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
                    0, KEY_READ, &hKey);
    if(lRes)
        return false;

    DWORD dwEnableProviderSubSystemFlag;
    DWORD dwLen = sizeof(DWORD);
    lRes = RegQueryValueExW(hKey, L"Enable Provider Subsystem", NULL, NULL, 
                (LPBYTE)&dwEnableProviderSubSystemFlag, &dwLen);
    RegCloseKey(hKey);

    if(lRes == ERROR_SUCCESS && (dwEnableProviderSubSystemFlag == 0))
    {
        return false;
    }
    return true;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::GetProviderSubsystem(
            /* [in] */ long lFlags,
            /* [out] */ _IWmiProvSS __RPC_FAR *__RPC_FAR *pProvSS)
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    if (m_pProvSS == NULL)
    {
        if(!IsProviderSubsystemEnabled() || IsNtSetupRunning())
        {
            //Provider subsystem is currently disabled because of it's registry key, or
            //because NT Setup is currently running.
            *pProvSS = NULL;
            return S_FALSE;
        }

        hRes = CoCreateInstance(    
            CLSID_WmiProvSS,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID__IWmiProvSS,
            (LPVOID *) &m_pProvSS
            );

        if (SUCCEEDED(hRes))
        {
            CCoreServices *pSvc = CCoreServices::CreateInstance();
            CReleaseMe _(pSvc);

            IWbemContext *pCtx = 0;

            hRes = m_pProvSS->Initialize(
                lFlags,
                pCtx,
                pSvc
                );

            if (FAILED(hRes))
            {
                ERRORTRACE((LOG_WBEMCORE, "IWmiProvSS::Initialize returned failure <0x%X>!\n", hRes));
                m_pProvSS->Release() ;
                m_pProvSS = NULL ;
            }
            else
            {
               m_pProvSS->AddRef();
               *pProvSS = m_pProvSS;
            }
        }
        else // FAILED to CoCreate
        {
            ERRORTRACE((LOG_WBEMCORE, "ProviderSubsystem CoCreateInstance returned failure <0x%X>!\n", hRes));
        }
    }
    else // Already created
    {
        m_pProvSS->AddRef();
        *pProvSS = m_pProvSS;
        hRes = WBEM_S_NO_ERROR;
    }

    return hRes;
}



//***************************************************************************
//
//***************************************************************************

HRESULT CCoreServices::DeliverIntrinsicEvent(
            /* [in] */ LPCWSTR pszNamespace,
            /* [in] */ ULONG uType,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [in] */ LPCWSTR pszParam,
            /* [in] */ LPCWSTR pszTransGuid,
            /* [in] */ ULONG uObjectCount,
            /* [in] */ _IWmiObject __RPC_FAR *__RPC_FAR *ppObjList
            )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    if (g_bEventsEnabled == FALSE)
        return hRes;

    if (m_pEssOld)
    {
        hRes = m_pEssOld->ProcessInternalEvent(
            uType,
            LPWSTR(pszNamespace),
            LPWSTR(pszParam),
            LPWSTR(pszTransGuid),
            0,
            0,
            uObjectCount,
            ppObjList,
            pCtx
            );
    }

    return hRes;
}

//***************************************************************************
//
//***************************************************************************

HRESULT CCoreServices::DeliverExtrinsicEvent(
            /* [in] */ LPCWSTR pszNamespace,
            /* [in] */ ULONG uFlags,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [in] */ _IWmiObject __RPC_FAR *pEvt
            )
{
    return E_NOTIMPL;
}


//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::StopEventDelivery( void)
{
    g_bEventsEnabled = FALSE;
    return 0;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::StartEventDelivery( void)
{
    g_bEventsEnabled = TRUE;
    return 0;
}



//***************************************************************************
//
//***************************************************************************

HRESULT CCoreServices::IncrementCounter(
            /* [in] */ ULONG uID,
            /* [in] */ ULONG uParam
            )
{
    if (uID == WMICORE_SELFINST_SINK_COUNT)
    {
        InterlockedIncrement(&g_nSinkCount);
    }
    else if (uID == WMICORE_SELFINST_STD_SINK_COUNT)
    {
        InterlockedIncrement(&g_nStdSinkCount);
    }
    else if ( uID == WMICORE_SELFINST_CONNECTIONS )
    {
        InterlockedIncrement(&g_nNamespaceCount);
    }
    return S_OK;
}

//***************************************************************************
//
//***************************************************************************

HRESULT CCoreServices::DecrementCounter(
            /* [in] */ ULONG uID,
            /* [in] */ ULONG uParam
            )
{
    if (uID == WMICORE_SELFINST_SINK_COUNT)
    {
        InterlockedDecrement(&g_nSinkCount);
    }
    else if (uID == WMICORE_SELFINST_STD_SINK_COUNT)
    {
        InterlockedDecrement(&g_nStdSinkCount);
    }
    else if ( uID == WMICORE_SELFINST_CONNECTIONS )
    {
        InterlockedDecrement(&g_nNamespaceCount);
    }

    return S_OK;
}

//***************************************************************************
//
//***************************************************************************

HRESULT CCoreServices::SetCounter(
            /* [in] */ ULONG uID,
            /* [in] */ ULONG uParam
            )
{
    return S_OK;
}

//***************************************************************************
//
//***************************************************************************

HRESULT CCoreServices::GetSelfInstInstances(
            /* [in] */ LPCWSTR pszClass,
            /* [in] */ IWbemObjectSink __RPC_FAR *pSink
            )
{
    return WBEM_S_NO_ERROR;
}

//***************************************************************************
//
//***************************************************************************

typedef _IWmiObject * PWMIOBJ;

STDMETHODIMP CCoreServices::GetSystemObjects(
            /* [in] */ ULONG lFlags,
            /* [out] */ ULONG __RPC_FAR *puArraySize,
            /* [out] */ _IWmiObject __RPC_FAR *__RPC_FAR *pObjects)
{
    CFlexArray Results;
    HRESULT hr;
    if(puArraySize == NULL)
        return WBEM_E_INVALID_PARAMETER;
    try
    {
        switch(lFlags)
        {
        case GET_SYSTEM_STD_OBJECTS:
            hr = GetSystemStdObjects(&Results);
            break;
        case GET_SYSTEM_SECURITY_OBJECTS:
            hr = GetSystemSecurityObjects(&Results);
            break;
        case GET_SYSTEM_ROOT_OBJECTS:
            hr = GetSystemRootObjects(&Results);
            break;
        case GET_SYSTEM_STD_INSTANCES:
            hr = GetStandardInstances(&Results);
            break;
        default:
            return WBEM_E_INVALID_PARAMETER;
        }
    }
    catch(...)
    {
        ExceptionCounter c;
        //  The class init stuff uses exceptions to handle out of memory conditions.
        hr = WBEM_E_OUT_OF_MEMORY;
    }


    // See if the buffer is large enough

    if(SUCCEEDED(hr))
    {
        DWORD dwBuff = *puArraySize;
        *puArraySize = Results.Size();
        if(dwBuff < Results.Size() || pObjects == NULL)
            hr = WBEM_E_BUFFER_TOO_SMALL;
    }

    // If failure in lower level, free up any elements in the flex array

    if(FAILED(hr))
    {
        for(DWORD dwCnt = 0; dwCnt < Results.Size(); dwCnt++)
        {
            PWMIOBJ pObj = (PWMIOBJ)Results.GetAt(dwCnt);
            if(pObj)
                delete pObj;
        }
        return hr;
    }

    // for success, transfer the results from the flex array to the return array.

    for(DWORD dwCnt = 0; dwCnt < Results.Size(); dwCnt++)
    {
        pObjects[dwCnt] = (PWMIOBJ)Results.GetAt(dwCnt);
    }
    return hr;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::GetSystemClass(
            /* [in] */ LPCWSTR pszClassName,
            /* [out] */ _IWmiObject __RPC_FAR *__RPC_FAR *pClassDef)
{
    return WBEM_E_NOT_AVAILABLE;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::GetConfigObject(
            ULONG uID,
            /* [out] */ _IWmiObject __RPC_FAR *__RPC_FAR *pCfgObject)
{
    return WBEM_E_NOT_AVAILABLE;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::RegisterWriteHook(
            /* [in] */ ULONG uFlags,
            /* [in] */ _IWmiCoreWriteHook __RPC_FAR *pHook
            )
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    if (pHook == 0)
        return WBEM_E_INVALID_PARAMETER;

    SHookElement *pHookEl = new SHookElement;
    if (!pHookEl)
        return WBEM_E_OUT_OF_MEMORY;

    pHookEl->m_pHook = pHook;
    pHookEl->m_pHook->AddRef();
    pHookEl->m_uFlags = uFlags;

    CInCritSec ics(&m_csHookAccess); 

    int nRes = g_aHooks.Add(pHookEl);

    if (nRes)
    {
        delete pHookEl;
        hRes = WBEM_E_FAILED;
    }

    return hRes;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::UnregisterWriteHook(
      _IWmiCoreWriteHook __RPC_FAR *pTargetHook
      )
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    if (pTargetHook == 0)
        return WBEM_E_INVALID_PARAMETER;

    BOOL bFound = FALSE;
    CInCritSec ics(&m_csHookAccess);
    for (int i = 0; i < g_aHooks.Size(); i++)
    {
        SHookElement *pEl = (SHookElement *) g_aHooks.GetAt(i);
        if (pEl == 0)
            continue;

        _IWmiCoreWriteHook *pHook = pEl->m_pHook;
        if (pHook == pTargetHook)
        {
            g_aHooks.RemoveAt(i);
            delete pEl;
            hRes = WBEM_S_NO_ERROR;
            bFound = TRUE;
            break;
        }
    }

    if (!bFound)
        hRes = WBEM_E_NOT_FOUND;

    return hRes;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::CreateCache(
            /* [in] */ ULONG uFlags,
            /* [out] */  _IWmiCache __RPC_FAR *__RPC_FAR *pCache
            )
{
    return WBEM_E_NOT_AVAILABLE;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::CreateFinalizer(
            /* [in] */ ULONG uFlags,
            /* [out] */  _IWmiFinalizer __RPC_FAR *__RPC_FAR *ppFnz
            )
{
    CWmiFinalizer *pFnz;
    try
    {
        pFnz = new CWmiFinalizer(this); // throw
        if (!pFnz)
            return WBEM_E_OUT_OF_MEMORY;
    }
    catch (CX_Exception &)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

    pFnz->AddRef();
    *ppFnz = pFnz;

    return WBEM_S_NO_ERROR;
}


//***************************************************************************
//
//***************************************************************************

HRESULT CCoreServices::CreatePathParser(
                ULONG uFlags,
                IWbemPath **pParser
                )
{
    if (NULL == g_pPathFac) return WBEM_E_INITIALIZATION_FAILURE;
    return g_pPathFac->CreateInstance(0, IID_IWbemPath, (LPVOID *) pParser);
}

//***************************************************************************
//
//***************************************************************************

HRESULT CCoreServices::CreateQueryParser(
                ULONG uFlags,
                _IWmiQuery **pResultQueryObj
                )
{
    if (NULL == g_pQueryFact) return WBEM_E_INITIALIZATION_FAILURE;
    return g_pQueryFact->CreateInstance(0, IID__IWmiQuery, (LPVOID *)pResultQueryObj);
}

//***************************************************************************
//
//***************************************************************************
HRESULT CCoreServices::GetDecorator(
                ULONG uFlags,
                IWbemDecorator **pDest
                )
{
    CDecorator *pDec = new CDecorator;
    if (!pDec)
        return WBEM_E_OUT_OF_MEMORY;
    pDec->AddRef();
    *pDest = pDec;
    return WBEM_S_NO_ERROR;
}


//***************************************************************************
//
//***************************************************************************

HRESULT CCoreServices::GetServices2(
            /* [in] */ LPCWSTR pszPath,
            /* [in] */ LPCWSTR pszUser,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [in] */ ULONG uClientFlags,
            /* [in] */ DWORD dwSecFlags,
            /* [in] */ DWORD dwPermissions,
            /* [in] */ ULONG uInternalFlags,
            /* [in] */ LPCWSTR pszClientMachine,
            /* [in] */ DWORD dwClientProcessID,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *pServices
            )
{
    HRESULT hRes = CWbemNamespace::PathBasedConnect(
            pszPath,
            pszUser,
            pCtx,
            uClientFlags,
            dwSecFlags,
            dwPermissions,
            uInternalFlags,
            pszClientMachine,
            dwClientProcessID,
            riid,
            pServices
            );

    return hRes;
}



//***************************************************************************
//
//***************************************************************************
//
HRESULT CCoreServices::NewPerTaskHook(
            /* [out] */ _IWmiCoreWriteHook __RPC_FAR *__RPC_FAR *pHook
            )
{
    CPerTaskHook *pNewHook = 0;
    HRESULT hRes = CPerTaskHook::CreatePerTaskHook(&pNewHook);
    if (FAILED(hRes))
        return hRes;

    *pHook = (_IWmiCoreWriteHook *) pNewHook;
    return hRes;
}

//***************************************************************************
//
//***************************************************************************

HRESULT CCoreServices::GetArbitrator(
            /* [out] */ _IWmiArbitrator **pReturnedArb
            )
{
    return E_NOTIMPL;
}

HRESULT CCoreServices::DumpCounters(FILE *f)
{
    if (!f)
        return E_FAIL;

    fprintf(f, "Total sinks active     = %d\n", g_nSinkCount);     // SEC:REVIEWED 2002-03-22 : ok, no path to here
    fprintf(f, "Total std sink objects = %d\n", g_nStdSinkCount);  // SEC:REVIEWED 2002-03-22 : ok, no path to here

    return 0;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CCoreServices::InitRefresherMgr(
            /* [in] */ long lFlags )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    // Get a refresher manager from the provider subsystem.  We initialize this once,
    // then retrieve from the fetcher.  If this succeeds, initialize the refreshing
    // services pointer with this guy so we can pawn off all the refresher
    // hi-perf provider fixup stuff.

    _IWmiProvSS *pProvSS = 0;
    GetProviderSubsystem(0, &pProvSS);
    CReleaseMe _2(pProvSS);

    // We'll need a Services pointer to create the manager
    IWbemServices*  pService = NULL;
    hr = GetServices( L"root", NULL,NULL,WMICORE_FLAG_REPOSITORY, IID_IWbemServices, (void**) &pService );
    CReleaseMe  rm1(pService);

    if ( NULL != m_pFetchRefrMgr )
    {
        hr = m_pFetchRefrMgr->Init( pProvSS, pService );
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

//***************************************************************************
//
//***************************************************************************

HRESULT CCoreServices::InternalSetCounter(
            DWORD dwCounter, DWORD dwValue)
{
    return S_OK;
}
//***************************************************************************
//
//***************************************************************************

ULONG CPerTaskHook::AddRef()
{
    InterlockedIncrement((LONG *) &m_uRefCount);
    return m_uRefCount;
}

//***************************************************************************
//
//***************************************************************************

ULONG CPerTaskHook::Release()
{
    ULONG uNewCount = InterlockedDecrement((LONG *) &m_uRefCount);
    if (0 != uNewCount)
        return uNewCount;
    delete this;
    return 0;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CPerTaskHook::QueryInterface(
    IN REFIID riid,
    OUT LPVOID *ppvObj
    )
{
    if (IID_IUnknown==riid || IID__IWmiCoreWriteHook==riid)
    {
        *ppvObj = this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}



//***************************************************************************
//
//***************************************************************************

CPerTaskHook::CPerTaskHook()
{
    m_uRefCount = 0;
}

//***************************************************************************
//
//***************************************************************************

CPerTaskHook::~CPerTaskHook()
{
    for (int i = 0; i < m_HookList.Size(); i++)
    {
        SHookElement *pHE = (SHookElement *) m_HookList.GetAt(i);
        delete pHE;
    }
}

//***************************************************************************
//
//***************************************************************************

HRESULT CPerTaskHook::CreatePerTaskHook(
    OUT CPerTaskHook **pDestNew
    )
{
    int nRes;

    *pDestNew = 0;

    wmilib::auto_ptr<CPerTaskHook> pNew(new CPerTaskHook);
    
    if (NULL == pNew.get()) return WBEM_E_OUT_OF_MEMORY;

    {
        CInCritSec ics(&CCoreServices::m_csHookAccess);

        for (int i = 0; i < g_aHooks.Size(); i++)
        {
            SHookElement *pEl = (SHookElement *) g_aHooks.GetAt(i);
            if (pEl == 0)
                continue;

            wmilib::auto_ptr<SHookElement> pNewEl( new SHookElement);

            if (NULL == pNewEl.get()) return WBEM_E_OUT_OF_MEMORY;

            *pNewEl.get() = *pEl;

            if (CFlexArray::no_error != pNew->m_HookList.Add(pNewEl.get()))
                return WBEM_E_OUT_OF_MEMORY;
            pNewEl.release();

        }

    }

    if (pNew->m_HookList.Size() != 0)
    {
        pNew->AddRef();                
        *pDestNew = pNew.release();
    }
    else
    {
        *pDestNew = 0;                  // No sense in returning an empty list
    }

    return WBEM_S_NO_ERROR;
}


//***************************************************************************
//
//***************************************************************************

HRESULT CPerTaskHook::PrePut(
            /* [in] */ long lFlags,
            /* [in] */ long lUserFlags,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [in] */ IWbemPath __RPC_FAR *pPath,
            /* [in] */ LPCWSTR pszNamespace,
            /* [in] */ LPCWSTR pszClass,
            /* [in] */ _IWmiObject __RPC_FAR *pCopy
            )
{
    HRESULT hRes = 0;
    HRESULT hResBoth = 0;

    for (int i = 0; i < m_HookList.Size(); i++)
    {
        SHookElement *pEl = (SHookElement *) m_HookList.GetAt(i);
        if (pEl == 0)
            continue;
        _IWmiCoreWriteHook *pHook = pEl->m_pHook;

        //
        // One can register for both CLASS and INSTANCE
        // but there will be alwys ONE bit set in the lFlags
        //
        if (lFlags & pEl->m_uFlags)
        {
            if ((WBEM_FLAG_DISABLE_WHEN_OWNER_UPDATE & pEl->m_uFlags) &&
                (lUserFlags & WBEM_FLAG_NO_EVENTS))
            {
                // this HOOK is disabled becasue OWNER_UPDATE is set
                hRes = S_OK;
            }
            else
            {
                hRes = pHook->PrePut(lFlags, lUserFlags, pCtx, pPath, pszNamespace, pszClass, pCopy);
            }
            pEl->m_hResPre = hRes;
        }
        else
        {
            continue;
        }

        if (FAILED(hRes))
            return hRes;

        if (hRes == WBEM_S_POSTHOOK_WITH_BOTH)
            hResBoth = WBEM_S_POSTHOOK_WITH_BOTH;
    }

    return hResBoth;
}

//***************************************************************************
//
//***************************************************************************

HRESULT CPerTaskHook::PostPut(
            /* [in] */ long lFlags,
            /* [in] */ HRESULT hApiResult,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [in] */ IWbemPath __RPC_FAR *pPath,
            /* [in] */ LPCWSTR pszNamespace,
            /* [in] */ LPCWSTR pszClass,
            /* [in] */ _IWmiObject __RPC_FAR *pNew,
            /* [in] */ _IWmiObject __RPC_FAR *pOld
            )
{
    HRESULT hRes = 0;

    for (int i = 0; i < m_HookList.Size(); i++)
    {
        SHookElement *pEl = (SHookElement *) m_HookList.GetAt(i);
        if (pEl == 0)
            continue;
        if (pEl->m_hResPre == WBEM_S_NO_POSTHOOK)
            continue;
        _IWmiCoreWriteHook *pHook = pEl->m_pHook;

        //
        // One can register for both CLASS and INSTANCE
        // but there will be alwys ONE bit set in the lFlags
        //
        if (lFlags & pEl->m_uFlags)
        {
            hRes |= pHook->PostPut(lFlags, hApiResult, pCtx, pPath, pszNamespace, pszClass, pNew, pOld);
            pEl->m_hResPre = 0;
        }
        else
        {
            continue;
        }
    }

    return hRes;
}

//***************************************************************************
//
//***************************************************************************

HRESULT CPerTaskHook::PreDelete(
            /* [in] */ long lFlags,
            /* [in] */ long lUserFlags,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [in] */ IWbemPath __RPC_FAR *pPath,
            /* [in] */ LPCWSTR pszNamespace,
            /* [in] */ LPCWSTR pszClass
            )
{
    HRESULT hRes = 0, hResWithOld = 0;

    for (int i = 0; i < m_HookList.Size(); i++)
    {
        SHookElement *pEl = (SHookElement *) m_HookList.GetAt(i);
        if (pEl == 0)
            continue;
        _IWmiCoreWriteHook *pHook = pEl->m_pHook;

        if (lFlags & pEl->m_uFlags)
        {
            hRes = pHook->PreDelete(lFlags, lUserFlags, pCtx, pPath, pszNamespace, pszClass);
            pEl->m_hResPre = ULONG(hRes);
        }
        else
        {
            continue;
        }


        if (FAILED(hRes))
            return hRes;

        if (hRes == WBEM_S_POSTHOOK_WITH_OLD)
            hResWithOld = WBEM_S_POSTHOOK_WITH_OLD;
    }

    return hResWithOld;
}

//***************************************************************************
//
//***************************************************************************

HRESULT CPerTaskHook::PostDelete(
            /* [in] */ long lFlags,
            /* [in] */ HRESULT hApiResult,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [in] */ IWbemPath __RPC_FAR *pPath,
            /* [in] */ LPCWSTR pszNamespace,
            /* [in] */ LPCWSTR pszClass,
            /* [in] */ _IWmiObject __RPC_FAR *pOld
            )
{
    HRESULT hRes = 0;

    for (int i = 0; i < m_HookList.Size(); i++)
    {
        SHookElement *pEl = (SHookElement *) m_HookList.GetAt(i);
        if (pEl == 0)
            continue;
        if (pEl->m_hResPre == WBEM_S_NO_POSTHOOK)
            continue;

        if (lFlags & pEl->m_uFlags)
        {
            _IWmiCoreWriteHook *pHook = pEl->m_pHook;
            hRes |= pHook->PostDelete(lFlags, hApiResult, pCtx, pPath, pszNamespace, pszClass, pOld);
            pEl->m_hResPre = 0;
        }
        else
        {
            continue;
        }
    }

    return hRes;
}

SHookElement::SHookElement()
{
    m_pHook = 0;
    m_uFlags = 0;
    m_hResPre = 0;
}

SHookElement::~SHookElement()
{
    ReleaseIfNotNULL(m_pHook);
}

SHookElement::SHookElement(SHookElement &Src)
{
    m_pHook = 0;
    *this = Src;
}

SHookElement & SHookElement::operator =(SHookElement &Src)
{
    m_uFlags = Src.m_uFlags;
    ReleaseIfNotNULL(m_pHook);
    m_pHook = Src.m_pHook;
    if (m_pHook)
        m_pHook->AddRef();
    return *this;
}


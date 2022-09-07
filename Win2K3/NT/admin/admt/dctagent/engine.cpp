/*---------------------------------------------------------------------------
  File: EADCTAgent.cpp

  Comments: Implementation of DCT Agent COM server, mostly generated by ATL.

  (c) Copyright 1999, Mission Critical Software, Inc., All Rights Reserved
  Proprietary and confidential to Mission Critical Software, Inc.

  REVISION LOG ENTRY
  Revision By: Christy Boles
  Revised on 02/18/99 11:34:16

 ---------------------------------------------------------------------------
*/


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f EADCTAgentps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include <objidl.h>
//#include "McsEaDctAgent.h"
#include "Engine.h"

//#include "McsEaDctAgent_i.c"
#include "Engine_i.c"


#include <stdio.h>
#include "DCTAgent.h"
#include <objbase.h>
#include <LM.h>
#include <MigrationMutex.h>
#include "sdhelper.h"

CExeModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_DCTAgent, CDCTAgent)
END_OBJECT_MAP()


const DWORD dwTimeOut = 5000; // time for EXE to be idle before shutting down
const DWORD dwPause = 1000; // time to wait for threads to finish up

// function prototypes for DCOM initialization functions we'll try to load dynamically

/*#ifndef OFA
// here are some definitions we need, but that are only defined when _WIN32_WINNT is 0x400 and up
typedef struct  tagSOLE_AUTHENTICATION_SERVICE
    {
    DWORD dwAuthnSvc;
    DWORD dwAuthzSvc;
    OLECHAR __RPC_FAR *pPrincipalName;
    HRESULT hr;
    }	SOLE_AUTHENTICATION_SERVICE;

typedef SOLE_AUTHENTICATION_SERVICE __RPC_FAR *PSOLE_AUTHENTICATION_SERVICE;
typedef 
enum tagEOLE_AUTHENTICATION_CAPABILITIES
    {	EOAC_NONE	= 0,
	EOAC_MUTUAL_AUTH	= 0x1,
	EOAC_CLOAKING	= 0x10,
	EOAC_SECURE_REFS	= 0x2,
	EOAC_ACCESS_CONTROL	= 0x4,
	EOAC_APPID	= 0x8
    }	EOLE_AUTHENTICATION_CAPABILITIES;



#endif*/
typedef HRESULT STDAPICALLTYPE  COINITIALIZEEX (LPVOID,DWORD);
typedef HRESULT STDAPICALLTYPE  COINITIALIZESECURITY (PSECURITY_DESCRIPTOR,LONG,SOLE_AUTHENTICATION_SERVICE *,
                        void*,DWORD,DWORD,void*,DWORD,void*);

// Passed to CreateThread to monitor the shutdown event
static DWORD WINAPI MonitorProc(void* pv)
{
    CExeModule* p = (CExeModule*)pv;
    p->MonitorShutdown();
    return 0;
}

LONG CExeModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if (l == 0)
    {
        bActivity = true;
        SetEvent(hEventShutdown); // tell monitor that we transitioned to zero
    }
    return l;
}

//Monitors the shutdown event
void CExeModule::MonitorShutdown()
{
    while (1)
    {
        WaitForSingleObject(hEventShutdown, INFINITE);
        DWORD dwWait=0;
        do
        {
            bActivity = false;
            dwWait = WaitForSingleObject(hEventShutdown, dwTimeOut);
        } while (dwWait == WAIT_OBJECT_0);
        // timed out
        if (!bActivity && m_nLockCnt == 0) // if no activity let's really bail
        {
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
            CoSuspendClassObjects();
            if (!bActivity && m_nLockCnt == 0)
#endif
                break;
        }
    }
    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
}

bool CExeModule::StartMonitor()
{
	bool bCreated = false;
    hEventShutdown = CreateEvent(NULL, false, false, NULL);
    if (hEventShutdown == NULL)
        return false;
    DWORD dwThreadID;
    HANDLE h = CreateThread(NULL, 0, MonitorProc, this, 0, &dwThreadID);
    if (h != NULL)
	   bCreated = true;
    CloseHandle(h);
	return bCreated;
}

LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
    while (p1 != NULL && *p1 != NULL)
    {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL)
        {
            if (*p1 == *p)
                return CharNext(p1);
            p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////
//
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, 
    HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int /*nShowCmd*/)
{
    ATLTRACE(_T("E {ADMTAgnt.exe}_tWinMain(hInstance=0x%08lX,...)\n"), hInstance);

    // obtain agent mutex
    // the migration driver uses this mutex to determine
    // if the agent process is currently running
    //
    // Note that named kernel objects with namespace prefixes are only supported
    // on Windows 2000 or later and therefore must check OS version before
    // specifying mutex name and only use Global\ prefix on Windows 2000 or later.

    bool bW2KOrLater = false;

    PWKSTA_INFO_100 pInfo;

    DWORD dwError = NetWkstaGetInfo(NULL, 100, (LPBYTE*)&pInfo);

    if (dwError == ERROR_SUCCESS)
    {
        if (pInfo->wki100_ver_major >= 5)
        {
            bW2KOrLater = true;
        }

        NetApiBufferFree(pInfo);
    }  

    CMigrationMutex mutex(bW2KOrLater ? AGENT_MUTEX : AGENT_MUTEX_NT4, true);

    // set debug flags to check memory allocations and leaks
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);

    lpCmdLine = GetCommandLine(); //this line necessary for _ATL_MIN_CRT

    HRESULT hRes;

#if defined(_ATL_FREE_THREADED)
    hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
    hRes = CoInitialize(NULL);
#endif

    BOOL bSecurityInitialized = FALSE;

    _ASSERTE(SUCCEEDED(hRes));

    if ( SUCCEEDED(hRes) )
    {
        TSD*  adminsAndSystemSD = BuildAdminsAndSystemSDForCOM();
        if (adminsAndSystemSD)
        {
            hRes = CoInitializeSecurity(
                const_cast<SECURITY_DESCRIPTOR*>(adminsAndSystemSD->GetSD()), //Points to security descriptor
                -1,                          //Count of entries in asAuthSvc
                NULL,                        //Array of names to register
                NULL,                        //Reserved for future use
                RPC_C_AUTHN_LEVEL_PKT_PRIVACY,      //The default authentication
                //level for proxies
                RPC_C_IMP_LEVEL_IMPERSONATE, //The default impersonation
                //level for proxies
                NULL,                        //Reserved; must be set to NULL
                EOAC_NONE,                   //Additional client or
                //server-side capabilities
                NULL                         //Reserved for future use
                );
            if (SUCCEEDED(hRes))
                bSecurityInitialized = TRUE;
            if (adminsAndSystemSD)
                delete adminsAndSystemSD;
        }
    }
    else
    {
        // if CoInitialize fails, returns an error
        return 1;
    }

    if (!bSecurityInitialized)
    {
        // if CoInitializeSecurity fails, returns an error
        CoUninitialize();
        return 1;
    }

    _Module.Init(ObjectMap, hInstance, &LIBID_MCSEADCTAGENTLib);
    _Module.dwThreadID = GetCurrentThreadId();
    TCHAR szTokens[] = _T("-/");

    int nRet = 0;
    BOOL bRun = TRUE;
    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_EADCTAgent, FALSE);
            nRet = _Module.UnregisterServer(TRUE);            
            bRun = FALSE;
            break;
        }
        if (lstrcmpi(lpszToken, _T("RegServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_EADCTAgent, TRUE);
            nRet = _Module.RegisterServer(TRUE);
            bRun = FALSE;
            break;
        }
        lpszToken = FindOneOf(lpszToken, szTokens);
    }

    if (bRun)
    {
        _Module.StartMonitor();
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
        hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, 
            REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED);
        _ASSERTE(SUCCEEDED(hRes));
        hRes = CoResumeClassObjects();
#else
        hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, 
            REGCLS_MULTIPLEUSE);
#endif
        _ASSERTE(SUCCEEDED(hRes));

        MSG msg;
        while (GetMessage(&msg, 0, 0, 0))
            DispatchMessage(&msg);

        _Module.RevokeClassObjects();
        Sleep(dwPause); //wait for any threads to finish
    }

    ATLTRACE(_T("L {ADMTAgnt.exe}_tWinMain(hInstance=0x%08lX,...)\n"), hInstance);
    //	_CrtDbgBreak();
    _Module.Term();
    CoUninitialize();
    return nRet;
}

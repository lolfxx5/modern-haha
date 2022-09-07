//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 2000.
//
//  File:       svchost.c
//
//  Contents:   Generic Host Process for Win32 Services
//
//  Classes:
//
//  Functions:
//
//  History:    3-30-98   RichardW   Created
//              3-31-98   ShaunCo    Took ownership.
//                                   Finished off basic implementation.
//              1-24-00   JSchwart   Took ownership.
//                                   Adapted to run NT intrinsic services.
//              April 2002 JayKrell  added simple support for ServiceManifest registry setting
//                                    did minimal cleanup..much room for improvement..
//----------------------------------------------------------------------------

#include "pch.h"
#pragma hdrstop
#include "globals.h"
#include "registry.h"
#include "security.h"

//
//  Generic Service Process:
//
//  This process will grovel the service portion of the registry, looking
//  for instances of itself (details below), and constructing a list of services
//  to submit to the service controller.  As an individual service is started,
//  the DLL is loaded and the entry point called.  Services in these DLLs are
//  expected to play nicely with others, that is, use the common thread pool,
//  not stomp memory, etc.
//
//
//  Loading.
//
//  Each service that will be resident in this process must have svchost.exe as
//  the ImagePath, with the same parameters.  Additionally, the service must
//  have under its Parameters key, these values:
//
//  ServiceDll      = REG_EXPAND_SZ <path to DLL>
//  ServiceMain     = REG_SZ        <pszFunctionName>  OPTIONAL
//
//  If ServiceMain is not present, then it defaults to "ServiceMain".
//
//
//  Multiple Service Groups
//
//  Multiple service groups can be accomplished by supplying parameters to the
//  svchost.exe on the ImagePath.
//
//      svchost.exe -k "Key"
//
//  will grovel the services and only load those with matching ImagePath.
//

#define REGSTR_PATH_SVCHOST     TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Svchost")

typedef struct _COMMAND_OPTIONS
{
    PTSTR   CommandLineBuffer;
    PTSTR   ImageName;

    BOOL    fServiceGroup;
    PTSTR   ServiceGroupName;

    //
    // dwCoInitializeSecurityParam is a DWORD read from the registry for the
    // service group we were instantiated for.  If non-zero, we will
    // call CoInitializeSecurity in a way based on the value.
    //

    DWORD   dwCoInitializeSecurityParam;
    DWORD   dwAuthLevel;
    DWORD   dwImpersonationLevel;
    DWORD   dwAuthCapabilities;

    //
    // Default stack size for RPC threads (to prevent stack overflow)
    //

    DWORD   dwDefaultRpcStackSize;

    //
    // Should this svchost instance mark itself as system-critical?
    //

    BOOL    fSystemCritical;
}
COMMAND_OPTIONS, * PCOMMAND_OPTIONS;


typedef struct _SERVICE_DLL
{
    LIST_ENTRY      List;
    HMODULE         hmod;
    PTSTR           pszDllPath;
    PTSTR           pszManifestPath;
    HANDLE          hActCtx;
} SERVICE_DLL, * PSERVICE_DLL;


typedef struct _SERVICE
{
    PTSTR           pszName;
    PSERVICE_DLL    pDll;
    PSTR            pszEntryPoint;
} SERVICE, * PSERVICE;


//+---------------------------------------------------------------------------
//
// Global variables.
//

// ListLock protects access to the Dll list and Service array.
//
CRITICAL_SECTION    ListLock;

// DllList is a list of SERVICE_DLL structures representing the DLL's
// which host entry points for the services hosted by this process.
//
LIST_ENTRY          DllList;

// ServiceArray is an array of SERVICE structures representing the services
// hosted by this process.
//
PSERVICE            ServiceArray;

// ServiceCount is the count of SERVICE entries in ServiceList.
//
UINT                ServiceCount;

// ServiceNames is the multi-sz read from the registry for the
// service group we were instantiated for.
//
PTSTR               ServiceNames;


//+---------------------------------------------------------------------------
//
// Local function prototypes
//

VOID
SvchostCharLowerW(
    LPWSTR  pszString
    );


//+---------------------------------------------------------------------------
//

VOID
DummySvchostCtrlHandler(
    DWORD   Opcode
    )
{
    return;
}


VOID
AbortSvchostService(               // used if cant find Service DLL or entrypoint
    LPWSTR  ServiceName,
    DWORD   Error
    )
{
    SERVICE_STATUS_HANDLE   GenericServiceStatusHandle;
    SERVICE_STATUS          GenericServiceStatus;

    GenericServiceStatus.dwServiceType        = SERVICE_WIN32;
    GenericServiceStatus.dwCurrentState       = SERVICE_STOPPED;
    GenericServiceStatus.dwControlsAccepted   = SERVICE_CONTROL_STOP;
    GenericServiceStatus.dwCheckPoint         = 0;
    GenericServiceStatus.dwWaitHint           = 0;
    GenericServiceStatus.dwWin32ExitCode      = Error;
    GenericServiceStatus.dwServiceSpecificExitCode = 0;

    GenericServiceStatusHandle = RegisterServiceCtrlHandler(ServiceName,
                                                            DummySvchostCtrlHandler);

    if (GenericServiceStatusHandle == NULL)
    {
        SVCHOST_LOG1(ERROR,
                     "AbortSvchostService: RegisterServiceCtrlHandler failed %d\n",
                     GetLastError());
    }
    else if (!SetServiceStatus (GenericServiceStatusHandle,
                                &GenericServiceStatus))
    {
        SVCHOST_LOG1(ERROR,
                     "AbortSvchostService: SetServiceStatus error %ld\n",
                     GetLastError());
    }

    return;
}

FARPROC
GetServiceDllFunction (
    PSERVICE_DLL    pDll,
    PCSTR           pszFunctionName,
    LPDWORD         lpdwError        OPTIONAL
    )
{
    FARPROC pfn = NULL;
    HMODULE hmod;
    ULONG_PTR ulpActCtxStackCookie = 0;
    BOOL      fActivateSuccess = FALSE;

    //
    // GetProcAddress can lead to .dlls being loaded, at least
    // in the presence of forwarders. So we are sure to activate the activationcontext
    // even if we are not doing a LoadLibrary here.
    //
    fActivateSuccess = ActivateActCtx(pDll->hActCtx, &ulpActCtxStackCookie);
    if (!fActivateSuccess)
    {
        if (lpdwError)
        {
            *lpdwError = GetLastError();
        }

        SVCHOST_LOG2(ERROR,
                     "ActivateActCtx for %ws failed.  Error %d.\n",
                     pDll->pszDllPath,
                     GetLastError());
        goto Exit;
    }

    // Load the module if neccessary.
    //
    hmod = pDll->hmod;
    if (!hmod)
    {
        hmod = LoadLibraryEx (
                    pDll->pszDllPath,
                    NULL,
                    LOAD_WITH_ALTERED_SEARCH_PATH);

        if (hmod)
        {
            pDll->hmod = hmod;
        }
        else
        {
            if (lpdwError)
            {
                *lpdwError = GetLastError();
            }

            SVCHOST_LOG2(ERROR,
                         "LoadLibrary (%ws) failed.  Error %d.\n",
                         pDll->pszDllPath,
                         GetLastError());
            goto Exit;
        }
    }

    ASSERT (hmod);

    pfn = GetProcAddress(hmod, pszFunctionName);

    if (!pfn)
    {
        if (lpdwError)
        {
            *lpdwError = GetLastError();
        }

        SVCHOST_LOG3(TRACE,
                     "GetProcAddress (%s) failed on DLL %ws.  Error = %d.\n",
                     pszFunctionName,
                     pDll->pszDllPath,
                     GetLastError());
    }
Exit:
    if (fActivateSuccess)
        DeactivateActCtx(0, ulpActCtxStackCookie);
    return pfn;
}

PSERVICE_DLL
FindDll(
    IN LPCTSTR pszManifestPath,
    IN LPCTSTR pszDllPath
    )
{
    PLIST_ENTRY     pNode;
    PSERVICE_DLL    pDll = NULL;

    ASSERT (pszDllPath);

    EnterCriticalSection (&ListLock);

    pNode = DllList.Flink;

    while (pNode != &DllList)
    {
        pDll = CONTAINING_RECORD (pNode, SERVICE_DLL, List);

        if (0 == lstrcmp (pDll->pszDllPath, pszDllPath)
            && 0 == lstrcmp (pDll->pszManifestPath, pszManifestPath)
            )
        {
            break;
        }

        pDll = NULL;

        pNode = pNode->Flink;
    }

    LeaveCriticalSection (&ListLock);

    return pDll;
}

PSERVICE_DLL
AddDll(
    IN LPCTSTR pszManifestPath,
    IN LPCTSTR pszDllPath,
    OUT LPDWORD lpdwError
    )
{
    PSERVICE_DLL    pDll;
    SIZE_T          nDllPathLength;
    SIZE_T          nManifestPathLength;

    ASSERT (pszDllPath);
    ASSERT (*pszDllPath);

    nDllPathLength = lstrlenW (pszDllPath);
    nManifestPathLength = lstrlenW (pszManifestPath);

    pDll = (PSERVICE_DLL)MemAlloc (HEAP_ZERO_MEMORY,
                sizeof (SERVICE_DLL)
                + ((nDllPathLength + 1) * sizeof(WCHAR))
                + ((nManifestPathLength + 1) * sizeof(WCHAR))
                );
    if (pDll)
    {
        // Set the structure members.
        //
        pDll->pszDllPath = (PTSTR) (pDll + 1);
        pDll->pszManifestPath = pDll->pszDllPath + nDllPathLength + 1;
        CopyMemory(pDll->pszDllPath, pszDllPath, nDllPathLength * sizeof(WCHAR));
        CopyMemory(pDll->pszManifestPath, pszManifestPath, nManifestPathLength * sizeof(WCHAR));

        ASSERT(pDll->hActCtx == NULL);
        ASSERT(pDll->pszDllPath[nDllPathLength] == 0);
        ASSERT(pDll->pszManifestPath[nManifestPathLength] == 0);

        // Add the entry to the list.
        //
        EnterCriticalSection (&ListLock);

        InsertTailList (&DllList, &pDll->List);

        LeaveCriticalSection (&ListLock);
    }
    else
    {
        *lpdwError = ERROR_NOT_ENOUGH_MEMORY;
    }

    return pDll;
}

LONG
OpenServiceParametersKey (
    LPCTSTR pszServiceName,
    HKEY*   phkey
    )
{
    LONG lr;
    HKEY hkeyServices = NULL;
    HKEY hkeySvc = NULL;

    ASSERT (phkey);

    // Open the Services key.
    //
    lr = RegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            REGSTR_PATH_SERVICES,
            0,
            KEY_READ,
            &hkeyServices);
    if (lr != ERROR_SUCCESS)
        goto Exit;

    // Open the service key.
    //
    lr = RegOpenKeyEx (
            hkeyServices,
            pszServiceName,
            0,
            KEY_READ,
            &hkeySvc);

    if (lr != ERROR_SUCCESS)
        goto Exit;

    // Open the Parameters key.
    //
    lr = RegOpenKeyEx (
            hkeySvc,
            TEXT("Parameters"),
            0,
            KEY_READ,
            phkey);
Exit:
    if (hkeyServices != NULL)
        RegCloseKey(hkeyServices);
    if (hkeySvc != NULL)
        RegCloseKey(hkeySvc);

    return lr;
}

#if DBG
BOOL
FDebugBreakForService (
    LPCWSTR pszwService
    )
{
    BOOL    fAttach = FALSE;
    LONG    lr;
    HKEY    hkeySvchost;

    // Open the Svchost key.
    //
    lr = RegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            REGSTR_PATH_SVCHOST,
            0,
            KEY_READ,
            &hkeySvchost);

    if (!lr)
    {
        HKEY  hkeyServiceOptions;

        // Look for the key with the same name as the service.
        //
        lr = RegOpenKeyExW (
                hkeySvchost,
                pszwService,
                0,
                KEY_READ,
                &hkeyServiceOptions);

        if (!lr)
        {
            DWORD dwValue;

            lr = RegQueryDword (
                    hkeyServiceOptions,
                    TEXT("DebugBreak"),
                    &dwValue);

            if (!lr)
            {
                fAttach = !!dwValue;
            }

            RegCloseKey (hkeyServiceOptions);
        }

        RegCloseKey (hkeySvchost);
    }

    return fAttach;
}
#endif

VOID
GetServiceMainFunctions (
    PSERVICE                       pService,
    LPSERVICE_MAIN_FUNCTION        *ppfnServiceMain,
    LPSVCHOST_PUSH_GLOBAL_FUNCTION *ppfnPushGlobals,
    LPDWORD                        lpdwError
    )
{
    LPCSTR pszEntryPoint;
    ACTCTXW ActCtxW = { sizeof(ActCtxW) };
    HANDLE hActCtx = NULL;
    HKEY hkeyParams = NULL;
    PSERVICE_DLL pDll = NULL;
    WCHAR pszExpandedDllName [MAX_PATH + 1];
    PWSTR pszDllName = pszExpandedDllName; // This sometimes get bumped forward to be the leaf name.
    WCHAR pszExpandedManifestName [MAX_PATH + 1];
    WCHAR * Temp = NULL;
    const DWORD TempSize = sizeof(pszExpandedDllName);

    *lpdwError = NO_ERROR;

    pszExpandedDllName[0] = 0;
    pszExpandedManifestName[0] = 0;

    // Get the dll and entrypoint for this service if we don't have it yet.
    //
    if (!pService->pDll)
    {
        LONG lr;

        lr = OpenServiceParametersKey (pService->pszName, &hkeyParams);
        if (!lr)
        {
            DWORD dwType;
            DWORD dwSize;
            Temp = (WCHAR*)MemAlloc(0, TempSize);
            if (Temp == NULL)
            {
                *lpdwError = ERROR_NOT_ENOUGH_MEMORY;
                goto Exit;
            }
            Temp[0] = 0;
            // Look for the service dll path and expand it.
            //
            dwSize = TempSize;
            lr = RegQueryValueEx (
                    hkeyParams,
                    TEXT("ServiceDll"),
                    NULL,
                    &dwType,
                    (LPBYTE)Temp,
                    &dwSize);
            if (lr != ERROR_SUCCESS)
            {
                *lpdwError = lr;

                SVCHOST_LOG2(ERROR,
                             "RegQueryValueEx for the ServiceDll parameter of the "
                             "%ws service returned %u\n",
                             pService->pszName,
                             lr);
                goto Exit;
            }
            if (dwType != REG_EXPAND_SZ)
            {
                *lpdwError = ERROR_FILE_NOT_FOUND;

                SVCHOST_LOG1(ERROR,
                             "The ServiceDll parameter for the %ws service is not "
                             "of type REG_EXPAND_SZ\n",
                             pService->pszName);
                goto Exit;
            }
            if (Temp[0] == 0)
            {
                *lpdwError = ERROR_FILE_NOT_FOUND;
                goto Exit;
            }

            // Expand the dll name and lower case it for comparison
            // when we try to find an existing dll record.
            //
            ExpandEnvironmentStrings (
                Temp,
                pszDllName,
                MAX_PATH);

            SvchostCharLowerW (pszDllName);

            dwSize = TempSize;
            lr = RegQueryValueExW (
                    hkeyParams,
                    L"ServiceManifest",
                    NULL,
                    &dwType,
                    (LPBYTE)Temp,
                    &dwSize);
            switch (lr)
            {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
                // ok
                pszExpandedManifestName[0] = 0;
                MemFree(Temp);
                Temp = NULL;
                goto NoManifest;
            default:
                *lpdwError = lr;

                SVCHOST_LOG2(ERROR,
                             "RegQueryValueEx for the ServiceManifest parameter of the "
                             "%ws service returned %u\n",
                             pService->pszName,
                             lr);
                goto Exit;
            case ERROR_SUCCESS:
                if (REG_EXPAND_SZ != dwType)
                {
                    // invalid parameter is probably better here, but just do
                    // as it does for ServiceDll
                    *lpdwError = ERROR_FILE_NOT_FOUND;

                    SVCHOST_LOG1(ERROR,
                                 "The ServiceManifest parameter for the %ws service is not "
                                 "of type REG_EXPAND_SZ\n",
                                 pService->pszName);
                    goto Exit;
                }
                if (Temp[0] == 0)
                {
                    // invalid parameter is probably better here, but just do
                    // as it does for ServiceDll
                    *lpdwError = ERROR_FILE_NOT_FOUND;

                    SVCHOST_LOG1(ERROR,
                                 "The ServiceManifest parameter for the %ws service is not "
                                 "of type REG_EXPAND_SZ\n",
                                 pService->pszName);
                    goto Exit;
                }
            }
            // Expand the manifest name and lower case it for comparison
            // when we try to find an existing dll record.
            //
            ExpandEnvironmentStringsW (
                Temp,
                pszExpandedManifestName,
                MAX_PATH);

            MemFree(Temp);
            Temp = NULL;

            SvchostCharLowerW (pszExpandedManifestName);

            //
            // Now only use the leaf dll name.
            // This way people can setup/register the same for downlevel and sidebyside.
            //
            {
                SIZE_T i = lstrlenW(pszDllName);
                while (i != 0)
                {
                    i -= 1;
                    if (pszDllName[i] == L'\\' || pszDllName[i] == L'/')
                    {
                        pszDllName = pszDllName + i + 1;
                        break;
                    }
                }
            }
NoManifest:
            // Try to find an existing dll record that we might have and
            // if we don't, add this as a new record.
            //
            pDll = FindDll (pszExpandedManifestName, pszDllName);
            if (!pDll)
            {
                if (pszExpandedManifestName[0] != 0)
                {
                    ActCtxW.lpSource = pszExpandedManifestName;
                    hActCtx = CreateActCtxW(&ActCtxW);
                    if (hActCtx == INVALID_HANDLE_VALUE)
                    {
                        *lpdwError = GetLastError();

                        SVCHOST_LOG3(ERROR,
                                 "CreateActCtxW(%ws) for the "
                                 "%ws service returned %u\n",
                                 pszExpandedManifestName,
                                 pService->pszName,
                                 *lpdwError);

                        goto Exit;
                    }
                }

                // Remember this dll for this service for next time.
                //

                pDll = AddDll (pszExpandedManifestName, pszDllName, lpdwError);

                if (pDll == NULL)
                {
                    //
                    // Don't set *lpdwError here as AddDll already set it
                    //

                    goto Exit;
                }

                pDll->hActCtx = hActCtx;
                hActCtx = NULL;
            }

            ASSERT (!pService->pDll);
            pService->pDll = pDll;
            pDll = NULL;

            // Look for an explicit entrypoint name for this service.
            // (Optional)
            //
            RegQueryStringA (
                hkeyParams,
                TEXT("ServiceMain"),
                REG_SZ,
                &pService->pszEntryPoint);
        }
        else
        {
            *lpdwError = lr;
        }

        // If we don't have the service dll record by now, we're through.
        //
        if (!pService->pDll)
        {
            ASSERT(*lpdwError != NO_ERROR);
            goto Exit;
        }
    }

    // We should have it the dll by now, so proceed to load the entry point.
    //
    ASSERT (pService->pDll);

    // Default the entry point if we don't have one specified.
    //
    if (pService->pszEntryPoint)
    {
        pszEntryPoint = pService->pszEntryPoint;
    }
    else
    {
        pszEntryPoint = "ServiceMain";
    }

    // Get the address for the service's ServiceMain
    //
    *ppfnServiceMain = (LPSERVICE_MAIN_FUNCTION) GetServiceDllFunction(
                                                     pService->pDll,
                                                     pszEntryPoint,
                                                     lpdwError);

    // Get the address for the "push the globals" function (optional)
    //
    *ppfnPushGlobals = (LPSVCHOST_PUSH_GLOBAL_FUNCTION) GetServiceDllFunction(
                                                            pService->pDll,
                                                            "SvchostPushServiceGlobals",
                                                            NULL);

Exit:
    if (hkeyParams != NULL)
        RegCloseKey (hkeyParams);
    if (Temp != NULL)
        MemFree(Temp);
    if (hActCtx != NULL && hActCtx != INVALID_HANDLE_VALUE)
        ReleaseActCtx(hActCtx);
}

LONG
ReadPerInstanceRegistryParameters(
    IN     HKEY             hkeySvchost,
    IN OUT PCOMMAND_OPTIONS pOptions
    )
{
    HKEY   hkeySvchostGroup;
    LONG   lr;

    // Read the value corresponding to this service group.
    //
    ASSERT (pOptions->ServiceGroupName);

    lr = RegQueryString (
            hkeySvchost,
            pOptions->ServiceGroupName,
            REG_MULTI_SZ,
            &ServiceNames);

    if (!lr && (!ServiceNames || !*ServiceNames))
    {
        lr = ERROR_INVALID_DATA;
    }

    // Read any per-instance parameters from the service group subkey
    // if it exists.
    //
    if (!RegOpenKeyEx (
            hkeySvchost,
            pOptions->ServiceGroupName,
            0, KEY_READ,
            &hkeySvchostGroup))
    {
        DWORD dwValue;

        if (!RegQueryDword (
                hkeySvchostGroup,
                TEXT("CoInitializeSecurityParam"),
                &dwValue))
        {
            pOptions->dwCoInitializeSecurityParam = dwValue;
        }

        if (pOptions->dwCoInitializeSecurityParam)
        {
            if (!RegQueryDword (
                    hkeySvchostGroup,
                    TEXT("AuthenticationLevel"),
                    &dwValue))
            {
                pOptions->dwAuthLevel = dwValue;
            }
            else
            {
                pOptions->dwAuthLevel = RPC_C_AUTHN_LEVEL_PKT;
            }

            if (!RegQueryDword (
                    hkeySvchostGroup,
                    TEXT("ImpersonationLevel"),
                    &dwValue))
            {
                pOptions->dwImpersonationLevel = dwValue;
            }
            else
            {
                pOptions->dwImpersonationLevel = RPC_C_IMP_LEVEL_IDENTIFY;
            }

            if (!RegQueryDword (
                    hkeySvchostGroup,
                    TEXT("AuthenticationCapabilities"),
                    &dwValue))
            {
                pOptions->dwAuthCapabilities = dwValue;
            }
            else
            {
                pOptions->dwAuthCapabilities = EOAC_NO_CUSTOM_MARSHAL |
                                                 EOAC_DISABLE_AAA;
            }
        }

        if (!RegQueryDword (
                hkeySvchostGroup,
                TEXT("DefaultRpcStackSize"),
                &dwValue))
        {
            pOptions->dwDefaultRpcStackSize = dwValue;
        }

        if (!RegQueryDword (
                hkeySvchostGroup,
                TEXT("SystemCritical"),
                &dwValue))
        {
            pOptions->fSystemCritical = dwValue;
        }

        RegCloseKey (hkeySvchostGroup);
    }

    return lr;
}


BOOL
CallPerInstanceInitFunctions(
    IN OUT PCOMMAND_OPTIONS pOptions
    )
{
    if (pOptions->dwCoInitializeSecurityParam)
    {
        if (!InitializeSecurity(pOptions->dwCoInitializeSecurityParam,
                                pOptions->dwAuthLevel,
                                pOptions->dwImpersonationLevel,
                                pOptions->dwAuthCapabilities))
        {
            return FALSE;
        }
    }

    if (pOptions->dwDefaultRpcStackSize)
    {
        RpcMgmtSetServerStackSize(pOptions->dwDefaultRpcStackSize * 1024);
    }
    else
    {
        //
        // Make sure the default RPC stack size will be at least as
        // large as the default thread stack size for the process so
        // a random service calling RpcMgmtSetServerStackSize can't
        // set it to a value that's too low, causing overflows.
        //

        PIMAGE_NT_HEADERS NtHeaders = RtlImageNtHeader(NtCurrentPeb()->ImageBaseAddress);

        if (NtHeaders != NULL)
        {
            RpcMgmtSetServerStackSize((ULONG) NtHeaders->OptionalHeader.SizeOfStackCommit);
        }
    }

    if (pOptions->fSystemCritical)
    {
        //
        // Ignore the return value
        //

        RtlSetProcessIsCritical(TRUE, NULL, TRUE);
    }

    return TRUE;
}


VOID
BuildServiceArray (
    IN OUT PCOMMAND_OPTIONS pOptions
    )
{
    LONG    lr;
    HKEY    hkeySvchost;

    // Open the Svchost key.
    //
    lr = RegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            REGSTR_PATH_SVCHOST,
            0, KEY_READ,
            &hkeySvchost);

    if (!lr)
    {
        lr = ReadPerInstanceRegistryParameters(hkeySvchost, pOptions);

        RegCloseKey (hkeySvchost);
    }

    if (!lr)
    {
        PTSTR pszServiceName;

        EnterCriticalSection (&ListLock);

        // Count the number of service names read.
        //
        ServiceCount = 0;
        for (pszServiceName = ServiceNames;
             *pszServiceName;
             pszServiceName += lstrlen(pszServiceName) + 1)
        {
            ServiceCount++;
        }
        ASSERT (ServiceCount);

        // Allocate memory for the service array.
        //
        ServiceArray = MemAlloc (HEAP_ZERO_MEMORY,
                            sizeof (SERVICE) * ServiceCount);
        if (ServiceArray)
        {
            PSERVICE pService;

            // Initialize the service array.
            //
            pService = ServiceArray;

            for (pszServiceName = ServiceNames;
                 *pszServiceName;
                 pszServiceName += lstrlen(pszServiceName) + 1)
            {
                pService->pszName = pszServiceName;

                pService++;
            }

            ASSERT (pService == ServiceArray + ServiceCount);
        }
        LeaveCriticalSection (&ListLock);
    }
}


// type of LPSERVICE_MAIN_FUNCTIONW
//
VOID
WINAPI
ServiceStarter(
    DWORD   argc,
    PWSTR   argv[]
    )
{
    LPSERVICE_MAIN_FUNCTION        pfnServiceMain = NULL;
    LPSVCHOST_PUSH_GLOBAL_FUNCTION pfnPushGlobals = NULL;
    LPCWSTR pszwService = argv[0];
    LPWSTR pszwAbort = NULL;
    DWORD  dwError = ERROR_FILE_NOT_FOUND;

    EnterCriticalSection (&ListLock);
    {
        UINT i;

        for (i = 0; i < ServiceCount; i++)
        {
            if (0 == lstrcmpi (pszwService, ServiceArray[i].pszName))
            {
#if DBG
                if (FDebugBreakForService (pszwService))
                {
                    SVCHOST_LOG1(TRACE,
                                "Attaching debugger before getting ServiceMain for %ws...",
                                pszwService);

                    DebugBreak ();
                }
#endif
                GetServiceMainFunctions(&ServiceArray[i],
                                        &pfnServiceMain,
                                        &pfnPushGlobals,
                                        &dwError);

                if (pfnServiceMain && pfnPushGlobals && !g_pSvchostSharedGlobals)
                {
                    SvchostBuildSharedGlobals();
                }

                pszwAbort = argv[0];
                break;
            }
        }
    }
    LeaveCriticalSection (&ListLock);

    if (pfnPushGlobals && g_pSvchostSharedGlobals)
    {
        pfnPushGlobals (g_pSvchostSharedGlobals);

        if (pfnServiceMain)
        {
            SVCHOST_LOG1(TRACE,
                         "Calling ServiceMain for %ws...\n",
                         pszwService);

            pfnServiceMain (argc, argv);
        }
        else if (pszwAbort)
        {
            AbortSvchostService(pszwAbort,
                                dwError);
        }
    }
    else if (pfnServiceMain && !pfnPushGlobals)
    {
        SVCHOST_LOG1(TRACE,
                     "Calling ServiceMain for %ws...\n",
                     pszwService);

        pfnServiceMain (argc, argv);
    }
    else if (pszwAbort)
    {
        AbortSvchostService(pszwAbort,
                            dwError);
    }
}


LPSERVICE_TABLE_ENTRY
BuildServiceTable(
    VOID
    )
{
    LPSERVICE_TABLE_ENTRY   pServiceTable;

    EnterCriticalSection (&ListLock);

    // Allocate one extra entry and zero the entire range.  The extra entry
    // is the table terminator required by StartServiceCtrlDispatcher.
    //
    pServiceTable = MemAlloc (HEAP_ZERO_MEMORY,
                        sizeof (SERVICE_TABLE_ENTRY) * (ServiceCount + 1));

    if (pServiceTable)
    {
        UINT i;

        for (i = 0; i < ServiceCount; i++)
        {
            pServiceTable[i].lpServiceName = ServiceArray[i].pszName;
            pServiceTable[i].lpServiceProc = ServiceStarter;

            SVCHOST_LOG1(TRACE,
                         "Added service table entry for %ws\n",
                         pServiceTable[i].lpServiceName);
        }
    }

    LeaveCriticalSection (&ListLock);

    return pServiceTable;
}


PCOMMAND_OPTIONS
BuildCommandOptions (
    LPCTSTR  pszCommandLine
    )
{
    PCOMMAND_OPTIONS    pOptions;
    ULONG               cbCommandLine;

    if (pszCommandLine == NULL)
    {
        return NULL;
    }

    cbCommandLine = (lstrlen(pszCommandLine) + 1) * sizeof (TCHAR);

    pOptions = MemAlloc (HEAP_ZERO_MEMORY,
                sizeof (COMMAND_OPTIONS) + cbCommandLine);
    if (pOptions)
    {
        TCHAR*  pch;
        TCHAR*  pArgumentStart;
        PTSTR* ppNextArgument = NULL;

        pOptions->CommandLineBuffer = (PTSTR) (pOptions + 1);
        RtlCopyMemory (
            pOptions->CommandLineBuffer,
            pszCommandLine,
            cbCommandLine);

        pch = pOptions->CommandLineBuffer;
        ASSERT (pch);

        // Skip the name of the executable.
        //
        pOptions->ImageName = pch;
        while (*pch && (L' ' != *pch) && (L'\t' != *pch))
        {
            pch++;
        }
        if (*pch)
        {
            *pch++ = 0;
        }

        SvchostCharLowerW (pOptions->ImageName);

        while (1)
        {
            // Skip whitespace.
            //
            while (*pch && ((L' ' == *pch) || (L'\t' == *pch)))
            {
                pch++;
            }

            // End of string?
            //
            if (!*pch)
            {
                break;
            }

            // Is it a '-' or '/' argument?
            //
            if (((L'-' == *pch) || (L'/' == *pch)) && *(++pch))
            {
                if ((L'k' == *pch) || (L'K' == *pch))
                {
                    pOptions->fServiceGroup = TRUE;
                    ppNextArgument = &pOptions->ServiceGroupName;
                }

                pch++;
                continue;
            }

            // This is the start of an argument.
            //
            pArgumentStart = pch;

            // If the argument starts with a quote, skip it and scan to the
            // next quote to terminate it.
            //
            if ((L'\"' == *pch) && *(++pch))
            {
                pArgumentStart = pch;

                while (*pch && (L'\"' != *pch))
                {
                    pch++;
                }
            }

            // otherwise, skip to the next whitespace and this will be
            // our argument.
            //
            else
            {
                while (*pch && (L' ' != *pch) && (L'\t' != *pch))
                {
                    pch++;
                }
            }

            if (*pch)
            {
                // terminate the newly found argument string.
                //
                *pch++ = 0;
            }

            if (ppNextArgument)
            {
                *ppNextArgument = pArgumentStart;
                ppNextArgument = NULL;
            }
        }

        pOptions->fServiceGroup = !!pOptions->ServiceGroupName;

        SVCHOST_LOG1(TRACE,
                     "Command line     : %ws\n",
                     pszCommandLine);

        SVCHOST_LOG1(TRACE,
                     "Service Group    : %ws\n",
                     (pOptions->fServiceGroup) ? pOptions->ServiceGroupName : L"No");

        // Validate the options.
        //
        if (!pOptions->fServiceGroup)
        {
            SVCHOST_LOG2(TRACE,
                         "Generic Service Host\n\n"
                         "%ws [-k <key>] | [-r] | <service>\n\n"
                         "   -k <key>   Host all services whose ImagePath matches\n"
                         "              %ws -k <key>.\n\n",
                         pOptions->CommandLineBuffer,
                         pOptions->CommandLineBuffer);

            MemFree (pOptions);
            pOptions = NULL;
        }
    }

    return pOptions;
}


VOID
SvchostCharLowerW(
    LPWSTR  pszString
    )
{
    //
    // LocalVersion of CharLower to avoid pulling in user32.dll
    //

    int   cwchT;
    DWORD cwch;

    if (pszString == NULL)
    {
        return;
    }

    cwch = (DWORD) wcslen(pszString) + 1;

    cwchT = LCMapStringW(LOCALE_USER_DEFAULT,
                         LCMAP_LOWERCASE,
                         pszString,
                         cwch,
                         pszString,
                         cwch);

    if (cwchT == 0)
    {
        SVCHOST_LOG1(ERROR,
                     "SvchostCharLowerW failed for %ws\n",
                     pszString);
    }

    return;
}


LONG
WINAPI
SvchostUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    return RtlUnhandledExceptionFilter(ExceptionInfo);
}


VOID
wmainCRTStartup (
    VOID
    )
{
    LPSERVICE_TABLE_ENTRY   pServiceTable = NULL;
    PCOMMAND_OPTIONS        pOptions;
    PCWSTR                  pszwCommandLine;

    SetUnhandledExceptionFilter(&SvchostUnhandledExceptionFilter);

    // Prevent critical errors from raising hard error popups and
    // halting svchost.exe.  The flag below will have the system send
    // the errors to the process instead.
    //
    SetErrorMode(SEM_FAILCRITICALERRORS);

    // Initialize our HeapAlloc wrapper to use the process heap.
    //
    MemInit (GetProcessHeap());

    // Initialize our global DLL list, Service array, and the critical
    // section that protects them.  InitializeCriticalSection can throw a
    // STATUS_NO_MEMORY exception.  We want the process to exit if that
    // happens, so the default exception handler is fine.
    //
    InitializeListHead (&DllList);
    InitializeCriticalSection (&ListLock);

    // Build a COMMAND_OPTIONS structure and use it to grovel the registry
    // and create the service entry table.
    //
    pszwCommandLine = GetCommandLine ();

    pOptions = BuildCommandOptions (pszwCommandLine);
    if (pOptions)
    {
        BuildServiceArray (pOptions);

        pServiceTable = BuildServiceTable ();

        if (pServiceTable)
        {
            if (!CallPerInstanceInitFunctions(pOptions))
            {
                SVCHOST_LOG0(ERROR,
                             "CallPerInstanceInitFunctions failed -- exiting!\n");

                ExitProcess(1);
            }
        }

        MemFree (pOptions);
    }

    // If we have a valid service entry table, use it to transfer control
    // to the service controller.  StartServiceCtrlDispatcher won't return
    // until all services are stopped.
    //
    if (pServiceTable)
    {
        StartServiceCtrlDispatcher (pServiceTable);
    }

    SVCHOST_LOG1(TRACE,
                 "Calling ExitProcess for %ws\n",
                 pszwCommandLine);

    ExitProcess (0);
}

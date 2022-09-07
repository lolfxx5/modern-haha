/******************************************************************
   Copyright (c) 1999 Microsoft Corporation

   SystemRestore.CPP -- WMI provider class implementation

   Generated by Microsoft WMI Code Generation Engine
  
   TO DO: - See individual function headers
          - When linking, make sure you link to framedyd.lib & 
            msvcrtd.lib (debug) or framedyn.lib & msvcrt.lib (retail).

   Description: 
   
  
  
******************************************************************/

#include <fwcommon.h>  // This must be the first include.

#include "SystemRestore.h"
#include "srdefs.h"
#include "srrestoreptapi.h"
#include "srrpcapi.h"
#include "enumlogs.h"
#include "utils.h"
#include "srshell.h"


// TO DO: Replace "NameSpace" with the appropriate namespace for your
//        provider instance.  For instance:  "root\\default or "root\\cimv2".
//===================================================================
CSystemRestore MySystemRestoreSet (PROVIDER_NAME_SYSTEMRESTORE, L"root\\default") ;

// Property names
//===============
const static WCHAR* pName = L"Description" ;
const static WCHAR* pNumber = L"SequenceNumber" ;
const static WCHAR* pType = L"RestorePointType" ;
const static WCHAR* pEventType = L"EventType" ;
const static WCHAR* pTime = L"CreationTime" ;


/*****************************************************************************
 *
 *  FUNCTION    :   CSystemRestore::CSystemRestore
 *
 *  DESCRIPTION :   Constructor
 *
 *  INPUTS      :   none
 *
 *  RETURNS     :   nothing
 *
 *  COMMENTS    :   Calls the Provider constructor.
 *
 *****************************************************************************/
CSystemRestore::CSystemRestore (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) :
    Provider(lpwszName, lpwszNameSpace)
{
}

/*****************************************************************************
 *
 *  FUNCTION    :   CSystemRestore::~CSystemRestore
 *
 *  DESCRIPTION :   Destructor
 *
 *  INPUTS      :   none
 *
 *  RETURNS     :   nothing
 *
 *  COMMENTS    : 
 *
 *****************************************************************************/
CSystemRestore::~CSystemRestore ()
{
}

/*****************************************************************************
*
*  FUNCTION    :    CSystemRestore::EnumerateInstances
*
*  DESCRIPTION :    Returns all the instances of this class.
*
*  INPUTS      :    A pointer to the MethodContext for communication with WinMgmt.
*                   A long that contains the flags described in 
*                   IWbemServices::CreateInstanceEnumAsync.  Note that the following
*                   flags are handled by (and filtered out by) WinMgmt:
*                       WBEM_FLAG_DEEP
*                       WBEM_FLAG_SHALLOW
*                       WBEM_FLAG_RETURN_IMMEDIATELY
*                       WBEM_FLAG_FORWARD_ONLY
*                       WBEM_FLAG_BIDIRECTIONAL
*
*  RETURNS     :    WBEM_S_NO_ERROR if successful
*
*  COMMENTS    : TO DO: All instances on the machine should be returned here and
*                       all properties that this class knows how to populate must 
*                       be filled in.  If there are no instances, return 
*                       WBEM_S_NO_ERROR.  It is not an error to have no instances.
*                       If you are implementing a 'method only' provider, you
*                       should remove this method.
*
*****************************************************************************/
HRESULT CSystemRestore::EnumerateInstances ( MethodContext* pMethodContext, long lFlags )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

// TO DO: The following commented lines contain the 'set' methods for the
//        properties entered for this class.  They are commented because they
//        will NOT compile in their current form.  Each <Property Value> should be
//        replaced with the appropriate value.  Also, consider creating a new
//        method and moving these set statements and the ones from GetObject 
//        into that routine.  See the framework sample (ReindeerProv.cpp) for 
//        an example of how this might be done.
//
//        If the expectation is that there is more than one instance on the machine
//        EnumerateInstances should loop through the instances and fill them accordingly.
//
//        Note that you must ALWAYS set ALL the key properties.  See the docs for
//        further details.
///////////////////////////////////////////////////////////////////////////////
    WCHAR               szDrive[MAX_PATH]=L""; 

    GetSystemDrive(szDrive);
    
    CRestorePointEnum   RPEnum(szDrive, TRUE, FALSE);
    CRestorePoint       RP;
    DWORD               dwRc;
    FILETIME            *pft = NULL;

    if (!IsAdminOrSystem())
    {
        return (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED));
    }
            
    dwRc = RPEnum.FindFirstRestorePoint(RP);

    while (dwRc == ERROR_SUCCESS)
    {
        CInstance* pInstance = CreateNewInstance(pMethodContext);
        if (! pInstance)
        {
            hRes = WBEM_E_OUT_OF_MEMORY;
            break;
        }
        
        pInstance->SetCHString(pName, RP.GetName());
        pInstance->SetDWORD(pNumber, RP.GetNum());
        pInstance->SetDWORD(pType, RP.GetType());
        pInstance->SetDWORD(pEventType, RP.GetEventType());
        
        if (pft = RP.GetTime())
        {
            WBEMTime wbt(*pft);    
            BSTR     bstrTime;
            if (bstrTime = wbt.GetBSTR())
            {
                pInstance->SetCHString(pTime, bstrTime);
                SysFreeString(bstrTime);
            }
        }
        
        hRes = pInstance->Commit();
        pInstance->Release();
                            
        dwRc = RPEnum.FindNextRestorePoint(RP);                
    }   

    RPEnum.FindClose();
    
    return hRes ;
}

/*****************************************************************************
*
*  FUNCTION    :    CSystemRestore::GetObject
*
*  DESCRIPTION :    Find a single instance based on the key properties for the
*                   class. 
*
*  INPUTS      :    A pointer to a CInstance object containing the key properties. 
*                   A long that contains the flags described in 
*                   IWbemServices::GetObjectAsync.  
*
*  RETURNS     :    WBEM_S_NO_ERROR if the instance can be found
*                   WBEM_E_NOT_FOUND if the instance described by the key properties 
*                   could not be found
*                   WBEM_E_FAILED if the instance could be found but another error 
*                   occurred. 
*
*  COMMENTS    :    If you are implementing a 'method only' provider, you should
*                   remove this method.
*
*****************************************************************************/
HRESULT CSystemRestore::GetObject ( CInstance* pInstance, long lFlags )
{
    // TO DO: The GetObject function is used to search for an instance of this
    //        class on the machine based on the key properties.  Unlike
    //        EnumerateInstances which finds all instances on the machine, GetObject
    //        uses the key properties to find the matching single instance and 
    //        returns that instance.
    //
    //        Use the CInstance Get functions (for example, call 
    //        GetCHString(L"Name", sTemp)) against pInstance to see the key values 
    //        the client requested.
    HRESULT hr = WBEM_E_NOT_FOUND;

//    if (<InstanceExists>)
//    {
// TO DO: The following commented lines contain the 'set' methods for the
//        properties entered for this class.  They are commented because they
//        will NOT compile in their current form.  Each <Property Value> should be
//        replaced with the appropriate value.
//
//        pInstance->SetCHString(pName, <Property Value>);
//        pInstance->SetVariant(pNumber, <Property Value>);
//        pInstance->SetVariant(pType, <Property Value>);
//
//        hr = WBEM_S_NO_ERROR;
//    }

    return hr ;
}

/*****************************************************************************
*
*  FUNCTION    :    CSystemRestore::ExecQuery
*
*  DESCRIPTION :    You are passed a method context to use in the creation of 
*                   instances that satisfy the query, and a CFrameworkQuery 
*                   which describes the query.  Create and populate all 
*                   instances which satisfy the query.  You may return more 
*                   instances or more properties than are requested and WinMgmt 
*                   will post filter out any that do not apply.
*
*  INPUTS      :    A pointer to the MethodContext for communication with WinMgmt.
*                   A query object describing the query to satisfy.
*                   A long that contains the flags described in 
*                   IWbemServices::CreateInstanceEnumAsync.  Note that the following
*                   flags are handled by (and filtered out by) WinMgmt:
*                       WBEM_FLAG_FORWARD_ONLY
*                       WBEM_FLAG_BIDIRECTIONAL
*                       WBEM_FLAG_ENSURE_LOCATABLE
*
*  RETURNS     :    WBEM_E_PROVIDER_NOT_CAPABLE if queries not supported for 
*                       this class or if the query is too complex for this class
*                       to interpret.  The framework will call the EnumerateInstances
*                       function instead and let Winmgmt post filter.
*                   WBEM_E_FAILED if the query failed
*                   WBEM_S_NO_ERROR if query was successful 
*
*  COMMENTS    : TO DO: Most providers will not need to implement this method.  If you don't, WinMgmt 
*                       will call your enumerate function to get all the instances and perform the 
*                       filtering for you.  Unless you expect SIGNIFICANT savings from implementing 
*                       queries, you should remove this method.  You should also remove this method
*                       if you are implementing a 'method only' provider.
*
*****************************************************************************/
HRESULT CSystemRestore::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{
     return (WBEM_E_PROVIDER_NOT_CAPABLE);
}

/*****************************************************************************
*
*  FUNCTION    : CSystemRestore::PutInstance
*
*  DESCRIPTION :    PutInstance should be used in provider classes that can 
*                   write instance information back to the hardware or 
*                   software.  For example: Win32_Environment will allow a 
*                   PutInstance to create or update an environment variable.  
*                   However, a class like MotherboardDevice will not allow 
*                   editing of the number of slots, since it is difficult for 
*                   a provider to affect that number.
*
*  INPUTS      :    A pointer to a CInstance object containing the key properties. 
*                   A long that contains the flags described in 
*                   IWbemServices::PutInstanceAsync.  
*
*  RETURNS     :    WBEM_E_PROVIDER_NOT_CAPABLE if PutInstance is not available
*                   WBEM_E_FAILED if there is an error delivering the instance
*                   WBEM_E_INVALID_PARAMETER if any of the instance properties 
*                   are incorrect.
*                   WBEM_S_NO_ERROR if instance is properly delivered
*
*  COMMENTS    : TO DO: If you don't intend to support writing to your provider, 
*                       or are creating a 'method only' provider, remove this 
*                       method.
*
*****************************************************************************/
HRESULT CSystemRestore::PutInstance ( const CInstance &Instance, long lFlags)
{
    // Use the CInstance Get functions (for example, call 
    // GetCHString(L"Name", sTemp)) against Instance to see the key values 
    // the client requested.

    return (WBEM_E_PROVIDER_NOT_CAPABLE);
}

/*****************************************************************************
*
*  FUNCTION    :    CSystemRestore::DeleteInstance
*
*  DESCRIPTION :    DeleteInstance, like PutInstance, actually writes information
*                   to the software or hardware.  For most hardware devices, 
*                   DeleteInstance should not be implemented, but for software
*                   configuration, DeleteInstance implementation is plausible.
*
*  INPUTS      :    A pointer to a CInstance object containing the key properties. 
*                   A long that contains the flags described in 
*                   IWbemServices::DeleteInstanceAsync.  
*
*  RETURNS     :    WBEM_E_PROVIDER_NOT_CAPABLE if DeleteInstance is not available.
*                   WBEM_E_FAILED if there is an error deleting the instance.
*                   WBEM_E_INVALID_PARAMETER if any of the instance properties 
*                   are incorrect.
*                   WBEM_S_NO_ERROR if instance is properly deleted.
*
*  COMMENTS    : TO DO: If you don't intend to support deleting instances or are
*                       creating a 'method only' provider, remove this method.
*
*****************************************************************************/
HRESULT CSystemRestore::DeleteInstance ( const CInstance &Instance, long lFlags )
{
    // Use the CInstance Get functions (for example, call 
    // GetCHString(L"Name", sTemp)) against Instance to see the key values 
    // the client requested.

    return (WBEM_E_PROVIDER_NOT_CAPABLE);
}

/*****************************************************************************
*
*  FUNCTION    :    CSystemRestore::ExecMethod
*
*  DESCRIPTION :    Override this function to provide support for methods.  
*                   A method is an entry point for the user of your provider 
*                   to request your class perform some function above and 
*                   beyond a change of state.  (A change of state should be 
*                   handled by PutInstance() )
*
*  INPUTS      :    A pointer to a CInstance containing the instance the method was executed against.
*                   A string containing the method name
*                   A pointer to the CInstance which contains the IN parameters.
*                   A pointer to the CInstance to contain the OUT parameters.
*                   A set of specialized method flags
*
*  RETURNS     :    WBEM_E_PROVIDER_NOT_CAPABLE if not implemented for this class
*                   WBEM_S_NO_ERROR if method executes successfully
*                   WBEM_E_FAILED if error occurs executing method 
*
*  COMMENTS    : TO DO: If you don't intend to support Methods, remove this method.
*
*****************************************************************************/
HRESULT CSystemRestore::ExecMethod ( const CInstance& Instance,
                        const BSTR bstrMethodName,
                        CInstance *pInParams,
                        CInstance *pOutParams,
                        long lFlags)
{
    // For non-static methods, use the CInstance Get functions (for example, 
    // call GetCHString(L"Name", sTemp)) against Instance to see the key 
    // values the client requested.

    HRESULT hresult = WBEM_E_PROVIDER_NOT_CAPABLE;

    if (lstrcmpi(bstrMethodName, L"CreateRestorePoint") == 0)
    {
        hresult = CreateRestorePoint(pInParams, pOutParams);
    }        
    else if (lstrcmpi(bstrMethodName, L"Enable") == 0)  
    {
        hresult = Enable(pInParams, pOutParams);
    }
    else if (lstrcmpi(bstrMethodName, L"Disable") == 0)  
    {
        hresult = Disable(pInParams, pOutParams);
    }
    else if (lstrcmpi(bstrMethodName, L"Restore") == 0)  
    {
        hresult = Restore(pInParams, pOutParams);
    }
    else if (lstrcmpi(bstrMethodName, L"GetLastRestoreStatus") == 0)  
    {
        hresult = GetLastRestoreStatus(pInParams, pOutParams);
    }
    
    return hresult;
}



HRESULT CSystemRestore::CreateRestorePoint(
                        CInstance *pInParams,
                        CInstance *pOutParams)
{
    LPWSTR              pwszName = NULL;
    HRESULT             hr = WBEM_S_NO_ERROR;
    RESTOREPOINTINFOW   rpi;
    STATEMGRSTATUS      ss;

    ss.nStatus = ERROR_INVALID_PARAMETER;
    pInParams->GetWCHAR(L"Description", &pwszName);
    if (pwszName)
    {        
        pInParams->GetDWORD(L"RestorePointType", rpi.dwRestorePtType);
        pInParams->GetDWORD(L"EventType", rpi.dwEventType);    
        lstrcpy(rpi.szDescription, pwszName);

        // cannot create a RESTORE type restore point from WMI
        if (rpi.dwRestorePtType == RESTORE)
        {
            goto done;
        }

        ::SRSetRestorePoint(&rpi, &ss);        

        free(pwszName);   
    }
    
done:
	pOutParams->SetDWORD(L"ReturnValue", ss.nStatus);
    return hr;
}


HRESULT CSystemRestore::Enable(
                        CInstance *pInParams,
                        CInstance *pOutParams)
{
    LPWSTR              pwszDrive = NULL;
    HRESULT             hr = WBEM_E_INVALID_PARAMETER;
    bool                fWait = 0;
    DWORD               dwRc;
    
    pInParams->GetWCHAR(L"Drive", &pwszDrive);
    pInParams->Getbool(L"WaitTillEnabled", fWait);    
    if (pwszDrive)
    {
        if (0 == lstrcmpi(pwszDrive, L""))
            dwRc = ::EnableSREx(NULL, (BOOL) fWait);
        else        
            dwRc = ::EnableSREx(pwszDrive, (BOOL) fWait);
        
        pOutParams->SetDWORD(L"ReturnValue", dwRc);

        if (pwszDrive) free(pwszDrive);   

        hr = WBEM_S_NO_ERROR;
    }
    
    return hr;
}


HRESULT CSystemRestore::Disable(
                        CInstance *pInParams,
                        CInstance *pOutParams)
{
    LPWSTR              pwszDrive = NULL;
    HRESULT             hr = WBEM_E_INVALID_PARAMETER;
    DWORD               dwRc;
    
    pInParams->GetWCHAR(L"Drive", &pwszDrive);
    if (pwszDrive)
    {        
        if (0 == lstrcmpi(pwszDrive, L""))
            dwRc = ::DisableSR(NULL);
        else
            dwRc = ::DisableSR(pwszDrive);
            
        pOutParams->SetDWORD(L"ReturnValue", dwRc);

        if (pwszDrive) free(pwszDrive); 
        
        hr = WBEM_S_NO_ERROR;        
    }
    
    return hr;
}


HRESULT CSystemRestore::Restore(
                        CInstance *pInParams,
                        CInstance *pOutParams)
{
    HRESULT             hr = WBEM_E_INVALID_PARAMETER;
    DWORD               dwRp = 0, dwRpNew;
    IRestoreContext     *pCtx = NULL;
    DWORD               dwErr = ERROR_INTERNAL_ERROR;
    HMODULE             hModule = NULL;
    CRestorePoint       rp;
    WCHAR               szRp[MAX_RP_PATH];
    
    pInParams->GetDWORD(L"SequenceNumber", dwRp);

    if (dwRp == 0) 
        goto Err;

    hr = WBEM_S_NO_ERROR;

    // validate the restore point first
    
    wsprintf( szRp, L"%s%ld", s_cszRPDir, dwRp );
    rp.SetDir(szRp);
    dwErr = rp.ReadLog();
    if (dwErr != ERROR_SUCCESS)   // it doesn't exist
    {
        goto Err;
    }

    if (rp.GetType() == CANCELLED_OPERATION)    // cannot restore to it
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto Err;
    }
    
    hModule = ::LoadLibraryW (L"srrstr.dll");

    if (hModule != NULL)
    {
        INITFUNC pInit = NULL;
        PREPFUNC pPrep = (PREPFUNC) GetProcAddress (hModule, "PrepareRestore");

        if (pPrep != NULL)
        {
            if (FALSE == (*pPrep) ((int) dwRp, &pCtx))
            {
                goto Err;
            }
        }
        else
        {
            dwErr = GetLastError();
            goto Err;
        }
        
        // 
        // make this a silent restore - no result page
        //
        
        pCtx->SetSilent();
        
        pInit = (INITFUNC) GetProcAddress (hModule, "InitiateRestore");
        if (pInit != NULL)
        {
            if (FALSE == (*pInit) (pCtx, &dwRpNew))
            {
                goto Err;
            }
            else
            {
                dwErr = ERROR_SUCCESS;
            }
        }
        else
        {
            dwErr = GetLastError();
            goto Err;
        }        
    }
    else dwErr = GetLastError();

Err:
    pOutParams->SetDWORD(L"ReturnValue", dwErr);     

    if (hModule != NULL)
        ::FreeLibrary (hModule);

    return hr;
}


HRESULT 
CSystemRestore::GetLastRestoreStatus(
    CInstance *pInParams,
    CInstance *pOutParams)
{
    HRESULT     hr = WBEM_S_NO_ERROR;
    DWORD       dwStatus = 0;
    HKEY        hkey = NULL;
    
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, s_cszSRRegKey, &hkey))
    {
        RegReadDWORD(hkey, s_cszRestoreStatus, &dwStatus);
        RegCloseKey(hkey);
    }
    else
    {
        dwStatus = 0xFFFFFFFF;
    }
    
    pOutParams->SetDWORD(L"ReturnValue", dwStatus);
    return hr;
}

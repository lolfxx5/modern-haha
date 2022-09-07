//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1997-2002.
//
//  File:       CertTmpl.cpp
//
//  Contents:   Implementation of DLL Exports
//
//----------------------------------------------------------------------------


#include "stdafx.h"
#define INITGUID
#pragma warning(push,3)
#include <initguid.h>
#pragma warning(pop)
#include "CertTmpl_i.c"
#include "about.h"      // CCertTemplatesAbout
#include "compdata.h" // CCertTmplSnapin
#include "uuids.h"

#pragma warning(push,3)
#include <ntverp.h>     // VER_PRODUCTVERSION_STR
#include <typeinfo.h>

#define INCL_WINSOCK_API_TYPEDEFS 1
#include <winsock2.h>
#include <svcguid.h>
#include <winldap.h>



#pragma warning(pop)
#include "chooser.cpp"

#include "ShellExt.h"
#include "PolicyOID.h"

extern POLICY_OID_LIST      g_policyOIDList;
USE_HANDLE_MACROS ("CERTTMPL (CertTmpl.cpp)")


//
// This is used by the nodetype utility routines in stdutils.cpp
//

const struct NODETYPE_GUID_ARRAYSTRUCT g_NodetypeGuids[CERTTMPL_NUMTYPES] =
{
    { // CERTTMPL_SNAPIN
        structuuidNodetypeSnapin,
        lstruuidNodetypeSnapin    },
    {  // CERT_TEMPLATE
        structuuidNodetypeCertTemplate,
        lstruuidNodetypeCertTemplate  }
};

const struct NODETYPE_GUID_ARRAYSTRUCT* g_aNodetypeGuids = g_NodetypeGuids;

const int g_cNumNodetypeGuids = CERTTMPL_NUMTYPES;

const CLSID CLSID_CertTemplateShellExt = /* {11BDCE06-D55C-44e9-BC0B-8655F89E8CC5} */
{ 0x11bdce06, 0xd55c, 0x44e9, { 0xbc, 0xb, 0x86, 0x55, 0xf8, 0x9e, 0x8c, 0xc5 } };


HINSTANCE   g_hInstance = 0;
CComModule  _Module;

BEGIN_OBJECT_MAP (ObjectMap)
    OBJECT_ENTRY (CLSID_CertTemplatesSnapin, CCertTmplSnapin)
    OBJECT_ENTRY (CLSID_CertTemplatesAbout, CCertTemplatesAbout)
    OBJECT_ENTRY(CLSID_CertTemplateShellExt, CCertTemplateShellExt)
END_OBJECT_MAP ()

class CCertTmplApp : public CWinApp
{
public:
    CCertTmplApp ();
    virtual ~CCertTmplApp ();
    virtual BOOL InitInstance ();
    virtual int ExitInstance ();
private:
};

CCertTmplApp theApp;

CCertTmplApp::CCertTmplApp ()
{
}

CCertTmplApp::~CCertTmplApp ()
{
}

BOOL CCertTmplApp::InitInstance ()
{
#ifdef _MERGE_PROXYSTUB
    hProxyDll = m_hInstance;

#endif
    g_hInstance = m_hInstance;
    AfxSetResourceHandle (m_hInstance);
    _Module.Init (ObjectMap, m_hInstance);

    AfxInitRichEdit();

#if DBG
    CheckDebugOutputLevel ();
#endif

    SHFusionInitializeFromModuleID (m_hInstance, 2);

    return CWinApp::InitInstance ();
}

int CCertTmplApp::ExitInstance ()
{
    SHFusionUninitialize();

    while ( !g_policyOIDList.IsEmpty () )
    {
        CPolicyOID* pPolicyOID = g_policyOIDList.RemoveHead ();
        if ( pPolicyOID )
            delete pPolicyOID;
    }

    _Module.Term ();
    return CWinApp::ExitInstance ();
}



/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow (void)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    return (AfxDllCanUnloadNow ()==S_OK && _Module.GetLockCount ()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject (rclsid, riid, ppv);
}





/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer (void)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());

    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _Module.RegisterServer (TRUE);
    ASSERT (SUCCEEDED (hr));
    if ( E_ACCESSDENIED == hr )
    {
        CString caption;
        CString text;
        CThemeContextActivator activator;

        VERIFY (caption.LoadString (IDS_REGISTER_CERTTMPL));
        VERIFY (text.LoadString (IDS_INSUFFICIENT_RIGHTS_TO_REGISTER_CERTTMPL));

        MessageBox (NULL, text, caption, MB_OK);
        return hr;
    }
    try
    {
        CString         strGUID;
        CString         snapinName;
        AMC::CRegKey    rkSnapins;
        BOOL            fFound = rkSnapins.OpenKeyEx (HKEY_LOCAL_MACHINE, SNAPINS_KEY);
        ASSERT (fFound);
        if ( fFound )
        {
            {
                AMC::CRegKey    rkCertTmplSnapin;
                hr = GuidToCString (&strGUID, CLSID_CertTemplatesSnapin);
                if ( FAILED (hr) )
                {
                    ASSERT (FALSE);
                    return SELFREG_E_CLASS;
                }
                rkCertTmplSnapin.CreateKeyEx (rkSnapins, strGUID);
                ASSERT (rkCertTmplSnapin.GetLastError () == ERROR_SUCCESS);
                rkCertTmplSnapin.SetString (g_szNodeType, g_aNodetypeGuids[CERTTMPL_SNAPIN].bstr);
                VERIFY (snapinName.LoadString (IDS_CERTTMPL_REGISTRY));
                rkCertTmplSnapin.SetString (g_szNameString, (PCWSTR) snapinName);
                hr = GuidToCString (&strGUID, CLSID_CertTemplatesAbout);
                if ( FAILED (hr) )
                {
                    ASSERT (FALSE);
                    return SELFREG_E_CLASS;
                }
                rkCertTmplSnapin.SetString (L"About", strGUID);
                rkCertTmplSnapin.SetString (L"Provider", L"Microsoft");
                // security review 2/20/2002 BryanWal ok
                size_t  len = strlen (VER_PRODUCTVERSION_STR);
                PWSTR   pszVer = new WCHAR[len+1];
                if ( pszVer )
                {
                    // security review BryanWal 02/20/2002 ok
                    ::ZeroMemory (pszVer, (len+1) * sizeof (WCHAR));
                    // security review BryanWal 02/20/2002 cnt should be len+1 
                    // to include conversion of NULL char
                    len = ::mbstowcs (pszVer, VER_PRODUCTVERSION_STR, len+1);

                    rkCertTmplSnapin.SetString (L"Version", pszVer);
                    delete [] pszVer;
                }


                AMC::CRegKey rkCertTmplMgrStandalone;
                rkCertTmplMgrStandalone.CreateKeyEx (rkCertTmplSnapin, g_szStandAlone);
                ASSERT (rkCertTmplMgrStandalone.GetLastError () == ERROR_SUCCESS);


                AMC::CRegKey rkMyNodeTypes;
                rkMyNodeTypes.CreateKeyEx (rkCertTmplSnapin, g_szNodeTypes);
                ASSERT (rkMyNodeTypes.GetLastError () == ERROR_SUCCESS);
                AMC::CRegKey rkMyNodeType;

                for (int i = CERTTMPL_SNAPIN; i < CERTTMPL_NUMTYPES; i++)
                {
                    rkMyNodeType.CreateKeyEx (rkMyNodeTypes, g_aNodetypeGuids[i].bstr);
                    ASSERT (rkMyNodeType.GetLastError () == ERROR_SUCCESS);
                    rkMyNodeType.CloseKey ();
                }

                //
                // BryanWal 5/18/00
                // 94793: MUI: MMC: Certificates snap-in stores its display 
                //              information in the registry
                //
                // MMC now supports NameStringIndirect
                //
                // NTRAID# Bug9 611500 prefast: certtmpl: certtmpl.cpp(247) : 
                // warning 53: Call to 'GetModuleFileNameW' may not 
                // zero-terminate string 'achModuleFileName'.
                // Fix by zero'ing out the buffer and passing in 1 less than
                // the buffer size to GetModuleFileName
                WCHAR achModuleFileName[MAX_PATH+20];
                ::ZeroMemory (achModuleFileName, sizeof(achModuleFileName)/sizeof(WCHAR));
                if (0 != ::GetModuleFileName(
                             AfxGetInstanceHandle(),
                             achModuleFileName,
                             sizeof(achModuleFileName)/sizeof(WCHAR) - 1))
                {
                    CString strNameIndirect;
                    strNameIndirect.Format (L"@%s,-%d",
                                            achModuleFileName,
                                            IDS_CERTTMPL_REGISTRY);
                    rkCertTmplSnapin.SetString (L"NameStringIndirect",
                                            strNameIndirect );
                }

                rkCertTmplSnapin.CloseKey ();
            }

            AMC::CRegKey rkNodeTypes;
            fFound = rkNodeTypes.OpenKeyEx (HKEY_LOCAL_MACHINE, NODE_TYPES_KEY);
            ASSERT (fFound);
            if ( fFound )
            {
                AMC::CRegKey rkNodeType;

                for (int i = CERTTMPL_SNAPIN; i < CERTTMPL_NUMTYPES; i++)
                {
                    rkNodeType.CreateKeyEx (rkNodeTypes, g_aNodetypeGuids[i].bstr);
                    ASSERT (rkNodeType.GetLastError () == ERROR_SUCCESS);
                    rkNodeType.CloseKey ();
                }


                rkNodeTypes.CloseKey ();
            }
            else
                return SELFREG_E_CLASS;
        }
        else
            return SELFREG_E_CLASS;
    }
    catch (COleException* e)
    {
        ASSERT (FALSE);
        e->Delete ();
        return SELFREG_E_CLASS;
    }

    ASSERT (SUCCEEDED (hr));
    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer (void)
{
    LRESULT         lResult = 0;

    try
    {
        AMC::CRegKey    rkSnapins;
        BOOL            fFound = FALSE;

        do
        {
            CString         strGUID;
            CString         snapinName;
            
            fFound = rkSnapins.OpenKeyEx (HKEY_LOCAL_MACHINE, SNAPINS_KEY);
            ASSERT (fFound);
            if ( fFound )
            {
                {
                    AMC::CRegKey    rkCertTmplSnapin;
                    HRESULT         hr = GuidToCString (&strGUID, CLSID_CertTemplatesSnapin);
                    if ( FAILED (hr) )
                    {
                        ASSERT (FALSE);
                        lResult = SELFREG_E_CLASS;
                        break;
                    }

                    lResult = RegDelnode (rkSnapins, (PCWSTR) strGUID);
                }

                AMC::CRegKey rkNodeTypes;
                fFound = rkNodeTypes.OpenKeyEx (HKEY_LOCAL_MACHINE, NODE_TYPES_KEY);
                ASSERT (fFound);
                if ( fFound )
                {
                    for (int i = CERTTMPL_SNAPIN; i < CERTTMPL_NUMTYPES; i++)
                    {
                        lResult = RegDelnode (rkNodeTypes, g_aNodetypeGuids[i].bstr);
                    }


                    rkNodeTypes.CloseKey ();
                }
                else
                {
                    lResult = SELFREG_E_CLASS;
                    break;
                }
            }
            else
            {
                lResult = SELFREG_E_CLASS;
                break;
            }
        } while (0);

        if ( fFound )
            rkSnapins.CloseKey ();
    }
    catch (COleException* e)
    {
        ASSERT (FALSE);
        e->Delete ();
        lResult = SELFREG_E_CLASS;
    }

    if ( SELFREG_E_CLASS != lResult )
        _Module.UnregisterServer ();

    return HRESULT_FROM_WIN32 (lResult);
}

STDAPI DllInstall(BOOL /*bInstall*/, PCWSTR /*pszCmdLine*/)
{
    return S_OK;
}





///////////////////////////////////////////////////////////////////////////////
//  FormatDate ()
//
//  utcDateTime (IN)    -   A FILETIME in UTC format.
//  pszDateTime (OUT)   -   A string containing the local date and time
//                          formatted by locale and user preference
//
///////////////////////////////////////////////////////////////////////////////
HRESULT FormatDate (FILETIME utcDateTime, CString & pszDateTime)
{
    //  Time is returned as UTC, will be displayed as local.
    //  Use FileTimeToLocalFileTime () to make it local,
    //  then call FileTimeToSystemTime () to convert to system time, then
    //  format with GetDateFormat () and GetTimeFormat () to display
    //  according to user and locale preferences
    HRESULT     hr = S_OK;
    FILETIME    localDateTime;

    BOOL bResult = FileTimeToLocalFileTime (&utcDateTime, // pointer to UTC file time to convert
            &localDateTime); // pointer to converted file time
    ASSERT (bResult);
    if ( bResult )
    {
        SYSTEMTIME  sysTime;

        bResult = FileTimeToSystemTime (
                &localDateTime, // pointer to file time to convert
                &sysTime); // pointer to structure to receive system time
        if ( bResult )
        {
            CString date;
            CString time;

            // Get date
            // Get length to allocate buffer of sufficient size
            int iLen = GetDateFormat (
                    LOCALE_USER_DEFAULT, // locale for which date is to be formatted
                    0, // flags specifying function options
                    &sysTime, // date to be formatted
                    0, // date format string
                    0, // buffer for storing formatted string
                    0); // size of buffer
            ASSERT (iLen > 0);
            if ( iLen > 0 )
            {
                int iResult = GetDateFormat (
                        LOCALE_USER_DEFAULT, // locale for which date is to be formatted
                        0, // flags specifying function options
                        &sysTime, // date to be formatted
                        0, // date format string
                        date.GetBufferSetLength (iLen), // buffer for storing formatted string
                        iLen); // size of buffer
                ASSERT (iResult);
                date.ReleaseBuffer ();
                if ( iResult )
                    pszDateTime = date;
                else
                    hr = HRESULT_FROM_WIN32 (GetLastError ());

                if ( iResult )
                {
                    // Get time
                    // Get length to allocate buffer of sufficient size
                    iLen = GetTimeFormat (
                            LOCALE_USER_DEFAULT, // locale for which date is to be formatted
                            0, // flags specifying function options
                            &sysTime, // date to be formatted
                            0, // date format string
                            0, // buffer for storing formatted string
                            0); // size of buffer
                    ASSERT (iLen > 0);
                    if ( iLen > 0 )
                    {
                        iResult = GetTimeFormat (
                                LOCALE_USER_DEFAULT, // locale for which date is to be formatted
                                0, // flags specifying function options
                                &sysTime, // date to be formatted
                                0, // date format string
                                time.GetBufferSetLength (iLen), // buffer for storing formatted string
                                iLen); // size of buffer
                        ASSERT (iResult);
                        time.ReleaseBuffer ();
                        if ( iResult )
                        {
                            pszDateTime = date + L"  " + time;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                else
                    hr = E_UNEXPECTED;
            }
            else
            {
                hr = HRESULT_FROM_WIN32 (GetLastError ());
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32 (GetLastError ());
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32 (GetLastError ());
    }

    return hr;
}


void DisplaySystemError (HWND hParent, DWORD dwErr)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    LPVOID  lpMsgBuf;

    // security review BryanWal 2/20/2002 ok because message is from system
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwErr,
            MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
             (PWSTR) &lpMsgBuf,    0,    NULL);

    // Display the string.
    CThemeContextActivator activator;
    CString caption;
    VERIFY (caption.LoadString (IDS_CERTTMPL));
    ::MessageBox (hParent, (PWSTR) lpMsgBuf, (PCWSTR) caption, MB_OK);
    // Free the buffer.
    LocalFree (lpMsgBuf);
}


bool IsWindowsNT()
{
    OSVERSIONINFO   versionInfo;

    // security review BryanWal 2/20/2002 ok 
    ::ZeroMemory (&versionInfo, sizeof (versionInfo));
    versionInfo.dwOSVersionInfoSize = sizeof (versionInfo);
    BOOL    bResult = ::GetVersionEx (&versionInfo);
    ASSERT (bResult);
    if ( bResult )
    {
        if ( VER_PLATFORM_WIN32_NT == versionInfo.dwPlatformId )
            bResult = TRUE;
    }

    return bResult ? true : false;
}





////// This stuff was stolen from windows\gina\snapins\gpedit (eric flo's stuff) //////

//*************************************************************
//
//  RegDelnodeRecurse()
//
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//              Called by RegDelnode
//
//  Parameters: hKeyRoot    -   Root key
//              pwszSubKey    -   SubKey to delete
//
//  Return:     ERROR_SUCCESS if successful
//              something else if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              10/3/95     ericflo    Created
//              5/13/98     BryanWal   Modified to return LRESULT
//
//*************************************************************

LRESULT RegDelnodeRecurse (HKEY hKeyRoot, CString szSubKey)
{
    ASSERT (hKeyRoot && !szSubKey.IsEmpty ());
    if ( !hKeyRoot || szSubKey.IsEmpty () )
        return ERROR_INVALID_PARAMETER;

    //
    // First, see if we can delete the key without having
    // to recurse.
    //


    LONG    lResult = ::RegDeleteKey(hKeyRoot, szSubKey);
    if (lResult == ERROR_SUCCESS) 
    {
        return lResult;
    }


    HKEY    hKey = 0;
    lResult = ::RegOpenKeyEx (hKeyRoot, szSubKey, 0, KEY_READ, &hKey);
    if (lResult == ERROR_SUCCESS) 
    {
        // ensure szSubKey ends with a slash
        if ( L'\\' != szSubKey.GetAt (szSubKey.GetLength () - 1) )
        {
            szSubKey += L"\\";
        }

        //
        // Enumerate the keys
        //

        DWORD       dwSize = MAX_PATH;
        FILETIME    ftWrite;
        WCHAR       szName[MAX_PATH];
        lResult = ::RegEnumKeyEx(hKey, 0, 
                    szName, 
                    &dwSize,    // size in TCHARS of szName, including terminating NULL (on input)
                    NULL,
                    NULL, NULL, &ftWrite);
        if (lResult == ERROR_SUCCESS) 
        {
            do {
                if ( ERROR_SUCCESS != RegDelnodeRecurse (hKeyRoot, szSubKey + szName) ) 
                {
                    break;
                }

                //
                // Enumerate again
                //

                dwSize = MAX_PATH;

                lResult = ::RegEnumKeyEx(hKey, 0, 
                            szName, 
                            &dwSize,     // size in TCHARS of szName, including terminating NULL (on input)
                            NULL,
                            NULL, NULL, &ftWrite);


            } while (lResult == ERROR_SUCCESS);
        }


        ::RegCloseKey (hKey);
    }

    // remove slash from szSubKey
    szSubKey.Delete (szSubKey.GetLength () - 1, 1);

    //
    // Try again to delete the key
    //

    lResult = ::RegDeleteKey(hKeyRoot, szSubKey);
    if (lResult == ERROR_SUCCESS) 
    {
        return lResult;
    }

    return lResult;
}

//*************************************************************
//
//  RegDelnode()
//
//  Purpose:    Deletes a registry key and all it's subkeys / values
//
//  Parameters: hKeyRoot    -   Root key
//              pwszSubKey    -   SubKey to delete
//
//  Return:     ERROR_SUCCESS if successful
//              something else if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              10/3/95     ericflo    Created
//              5/13/98     BryanWal   Modified to return LRESULT
//
//*************************************************************

LRESULT RegDelnode (HKEY hKeyRoot, CString szSubKey)
{
    ASSERT (hKeyRoot && !szSubKey.IsEmpty ());
    if ( !hKeyRoot || szSubKey.IsEmpty () )
        return ERROR_INVALID_PARAMETER;

    return RegDelnodeRecurse (hKeyRoot, szSubKey);
}


//+--------------------------------------------------------------------------
//
//  Function:   InitObjectPickerForDomainComputers
//
//  Synopsis:   Call IDsObjectPicker::Initialize with arguments that will
//              set it to allow the user to pick a single computer object.
//
//  Arguments:  [pDsObjectPicker] - object picker interface instance
//
//  Returns:    Result of calling IDsObjectPicker::Initialize.
//
//  History:    10-14-1998   DavidMun   Created
//
//---------------------------------------------------------------------------

HRESULT InitObjectPickerForDomainComputers(IDsObjectPicker *pDsObjectPicker)
{
    //
    // Prepare to initialize the object picker.
    // Set up the array of scope initializer structures.
    //

    static const int     SCOPE_INIT_COUNT = 1;
    DSOP_SCOPE_INIT_INFO aScopeInit[SCOPE_INIT_COUNT];


    // security review BryanWal 02/02/2002 ok
    ::ZeroMemory(aScopeInit, sizeof(aScopeInit));

    //
    // Since we just want computer objects from every scope, combine them
    // all in a single scope initializer.
    //

    aScopeInit[0].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[0].flType = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN
                           | DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;
    aScopeInit[0].FilterFlags.Uplevel.flBothModes =
            DSOP_FILTER_COMPUTERS;
    aScopeInit[0].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

    //
    // Put the scope init array into the object picker init array
    //

    DSOP_INIT_INFO  InitInfo;
    // security review BryanWal 02/02/2002 ok
    ::ZeroMemory(&InitInfo, sizeof(InitInfo));

    InitInfo.cbSize = sizeof(InitInfo);
    InitInfo.pwzTargetComputer = NULL;  // NULL == local machine
    InitInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
    InitInfo.aDsScopeInfos = aScopeInit;

    //
    // Note object picker makes its own copy of InitInfo.  Also note
    // that Initialize may be called multiple times, last call wins.
    //

    return pDsObjectPicker->Initialize(&InitInfo);
}


CString GetSystemMessage (DWORD dwErr)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    CString message;

    if ( HRESULT_FROM_WIN32 (ERROR_NO_SUCH_DOMAIN) == dwErr )
    {
        VERIFY (message.LoadString (IDS_ERROR_NO_SUCH_DOMAIN));
    }
    else
    {
        LPVOID lpMsgBuf = 0;

        // security review BryanWal 02/02/2002 ok because message is from system
        FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwErr,
                MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                 (PWSTR) &lpMsgBuf,    0,    NULL );
        message = (PWSTR) lpMsgBuf;

        // Remove white space (including new line characters)
        message.TrimRight ();

        // Free the buffer.
        LocalFree (lpMsgBuf);
    }

    return message;
}

//+---------------------------------------------------------------------------
//
//  Function:   LocaleStrCmp
//
//  Synopsis:   Do a case insensitive string compare that is safe for any
//              locale.
//
//  Arguments:  [ptsz1] - strings to compare
//              [ptsz2]
//
//  Returns:    -1, 0, or 1 just like lstrcmpi
//
//  History:    10-28-96   DavidMun   Created
//
//  Notes:      This is slower than lstrcmpi, but will work when sorting
//              strings even in Japanese.
//
//----------------------------------------------------------------------------

int LocaleStrCmp(LPCWSTR ptsz1, LPCWSTR ptsz2)
{
    ASSERT (ptsz1 && ptsz2);
    if ( !ptsz1 || !ptsz2 )
        return 0;

    int iRet = 0;

    iRet = CompareString(LOCALE_USER_DEFAULT,
                         NORM_IGNORECASE        |
                           NORM_IGNOREKANATYPE  |
                           NORM_IGNOREWIDTH,
                         ptsz1,
                         -1,
                         ptsz2,
                         -1);

    if (iRet)
    {
        iRet -= 2;  // convert to lstrcmpi-style return -1, 0, or 1

        if ( 0 == iRet )
        {
            UNICODE_STRING unistr1;
            UNICODE_STRING unistr2;

            // security review 2/20/2002 BryanWal ok - Length is length in BYTES
            ::RtlInitUnicodeString (&unistr1, ptsz1);

            // security review 2/20/2002 BryanWal ok - Length is length in BYTES
            ::RtlInitUnicodeString (&unistr2, ptsz2);
            
            iRet = ::RtlCompareUnicodeString(
                &unistr1,
                &unistr2,
                FALSE );
        }
    }
    else
    {
        DWORD   dwErr = GetLastError ();
        _TRACE (0, L"CompareString (%s, %s) failed: 0x%x\n", ptsz1, ptsz2, dwErr);
    }
    return iRet;
}


void FreeStringArray (PWSTR* rgpszStrings, DWORD dwAddCount)
{
    if ( rgpszStrings )
    {
        for (DWORD dwIndex = 0; dwIndex < dwAddCount; dwIndex++)
        {
            if ( rgpszStrings[dwIndex] )
                CoTaskMemFree (rgpszStrings[dwIndex]);
        }

        CoTaskMemFree (rgpszStrings);
    }
}

HRESULT DisplayRootNodeStatusBarText (LPCONSOLE pConsole)
{
    if ( !pConsole )
        return E_POINTER;

    _TRACE (1, L"Entering DisplayRootNodeStatusBarText\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ( ));
    CComPtr<IConsole2>  spConsole2;
    HRESULT     hr = pConsole->QueryInterface (IID_PPV_ARG (IConsole2, &spConsole2));
    if (SUCCEEDED (hr))
    {
        CString statusText;
        VERIFY (statusText.LoadString (IDS_ROOTNODE_STATUSBAR_TEXT));
        hr = spConsole2->SetStatusText ((PWSTR)(PCWSTR) statusText);
    }

    _TRACE (-1, L"Leaving DisplayRootNodeStatusBarText: 0x%x\n", hr);
    return hr;
}

HRESULT DisplayObjectCountInStatusBar (LPCONSOLE pConsole, DWORD dwCnt)
{
    if ( !pConsole )
        return E_POINTER;

    _TRACE (1, L"Entering DisplayObjectCountInStatusBar- %d, %s\n",
            dwCnt, L"Certificate Templates");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ( ));
    CComPtr<IConsole2>  spConsole2;
    HRESULT     hr = pConsole->QueryInterface (IID_PPV_ARG (IConsole2, &spConsole2));
    if (SUCCEEDED (hr))
    {
        CString statusText;
        UINT    formatID = 0;

        switch (dwCnt)
        {
        case -1:
            statusText = L"";
            break;

        case 1:
            VERIFY (statusText.LoadString (IDS_CERT_TEMPLATE_COUNT_SINGLE));
            break;

        default:
            formatID = IDS_CERT_TEMPLATE_COUNT;
            break;
        }

        if ( formatID )
        {
            // security review BryanWal 02/02/2002 ok
            statusText.FormatMessage (formatID, dwCnt);
        }

        hr = spConsole2->SetStatusText ((PWSTR)(PCWSTR) statusText);
    }

    _TRACE (-1, L"Leaving DisplayObjectCountInStatusBar: 0x%x\n", hr);
    return hr;
}


PCWSTR GetContextHelpFile ()
{
    static CString strHelpTopic;

    if ( strHelpTopic.IsEmpty () )
    {
        UINT nLen = ::GetSystemWindowsDirectory (strHelpTopic.GetBufferSetLength(2 * MAX_PATH), 2 * MAX_PATH);
        strHelpTopic.ReleaseBuffer();
        if (0 == nLen)
        {
            ASSERT(FALSE);
            return 0;
        }

        strHelpTopic += CERTTMPL_HELP_PATH; 
        strHelpTopic += CERTTMPL_CONTEXT_HELP_FILE;
    }

    return (PCWSTR) strHelpTopic;
}

bool MyGetOIDInfoA (CString & string, LPCSTR pszObjId)
{   
    ASSERT (pszObjId);
    if ( !pszObjId )
        return false;

    PCCRYPT_OID_INFO    pOIDInfo;  // This points to a constant data structure and must not be freed.
    bool                bResult = false;

    // NTRAID# 479067 Certtmpl UI: Title bar and descriptive text incorrect
    if ( !strcmp (szOID_CERT_POLICIES, pszObjId) )
    {
        VERIFY (string.LoadString (IDS_ISSUANCE_POLICIES));
        bResult = true;
    }
    else
    {
        string = L"";
        pOIDInfo = ::CryptFindOIDInfo (CRYPT_OID_INFO_OID_KEY, (void *) pszObjId, 0);

        if ( pOIDInfo )
        {
            string = pOIDInfo->pwszName;
            bResult = true;
        }
        else
        {
            for (POSITION nextPos = g_policyOIDList.GetHeadPosition (); nextPos; )
            {
                CPolicyOID* pPolicyOID = g_policyOIDList.GetNext (nextPos);
                if ( pPolicyOID )
                {
                    if ( !strcmp (pPolicyOID->GetOIDA (), pszObjId) )
                    {
                        string = pPolicyOID->GetDisplayName ();
                        bResult = true;
                        break;
                    }
                }
            }

            if ( !bResult )
            {
                // security review BryanWal 02/02/2002 ok
                int nLen = ::MultiByteToWideChar (CP_ACP, 0, pszObjId, -1, NULL, 0);
                ASSERT (nLen > 0);
                if ( nLen > 0)
                {
                    // security review BryanWal 02/02/2002 ok
                    // NOTICE: GetBufferSetLength () takes len not including trailing NULL,
                    // returns NULL on failure. MultiByteToWideChar () handles NULL arg.
                    nLen = ::MultiByteToWideChar (CP_ACP, 0, pszObjId, -1, 
                            string.GetBufferSetLength (nLen), nLen);
                    ASSERT (nLen > 0);
                    string.ReleaseBuffer ();
                }
                bResult = (nLen > 0) ? true : false;
            }
        }
    }

    return bResult;
}

#define REGSZ_ENABLE_CERTTYPE_EDITING L"EnableCertTypeEditing"

bool IsCerttypeEditingAllowed()
{
    DWORD   lResult;
    HKEY    hKey = NULL;
    DWORD   dwType;
    DWORD   dwEnabled = 0;
    DWORD   cbEnabled = sizeof(dwEnabled);
    lResult = RegOpenKeyEx (HKEY_CURRENT_USER, 
                            L"Software\\Microsoft\\Cryptography\\CertificateTemplateCache", 
                            0, 
                            KEY_READ,
                            &hKey);

    if (lResult == ERROR_SUCCESS) 
    {
        // security review BryanWal 02/02/2002 ok
        lResult = RegQueryValueEx(hKey, 
                  REGSZ_ENABLE_CERTTYPE_EDITING,  
                  NULL,
                  &dwType,
                  (PBYTE)&dwEnabled,
                  &cbEnabled);
        if(lResult == ERROR_SUCCESS)
        {
            if(dwType != REG_DWORD)
            {
                dwEnabled = 0;
            }
        }
        RegCloseKey (hKey);
    }


    return (dwEnabled != 0);
}

BOOL EnumOIDInfo (PCCRYPT_OID_INFO pInfo, void* /*pvArg*/)
{
    BOOL    bRVal = TRUE;

    if ( pInfo && pInfo->pszOID )
    {
        // NTRAID# 463344 Certtmpl.msc: Remove "All Application Policies" from 
        // Extensions selection list in Certtmpl.msc -- break Enrollment
        if ( !strcmp (szOID_ANY_APPLICATION_POLICY, pInfo->pszOID) )
            return TRUE;


        for (POSITION nextPos = g_policyOIDList.GetHeadPosition (); nextPos; )
        {
            CPolicyOID* pPolicyOID = g_policyOIDList.GetNext (nextPos);
            if ( pPolicyOID )
            {
                if ( !strcmp (pPolicyOID->GetOIDA (), pInfo->pszOID) )
                    return TRUE; // duplicate found, get next
            }
        }

        int flags = 0;
        if ( CRYPT_ENHKEY_USAGE_OID_GROUP_ID == pInfo->dwGroupId )
            flags = CERT_OID_TYPE_APPLICATION_POLICY;
        else if ( CRYPT_POLICY_OID_GROUP_ID == pInfo->dwGroupId )
            flags = CERT_OID_TYPE_ISSUER_POLICY;
        else
        {
            ASSERT (0);
            return TRUE;
        }

        CPolicyOID* pPolicyOID = new CPolicyOID (pInfo->pszOID, pInfo->pwszName,
                flags, false);
        if ( pPolicyOID )
        {
            g_policyOIDList.AddTail (pPolicyOID);
        }
        else
            bRVal = FALSE;
    }
    else
        bRVal = FALSE;

    return bRVal;
}
 

HRESULT GetBuiltInOIDs ()
{
    HRESULT hr = S_OK;

    CryptEnumOIDInfo (
            CRYPT_ENHKEY_USAGE_OID_GROUP_ID,
            0,
            0,
            EnumOIDInfo);

    CryptEnumOIDInfo (
            CRYPT_POLICY_OID_GROUP_ID,
            0,
            0,
            EnumOIDInfo);

    return hr;
}

HRESULT EnumerateOIDs (
        IDirectoryObject* pOIDContObj)
{
    _TRACE (1, L"Entering EnumerateOIDs\n");

    CComPtr<IDirectorySearch>   spDsSearch;
    HRESULT hr = pOIDContObj->QueryInterface (IID_PPV_ARG(IDirectorySearch, &spDsSearch));
    if ( SUCCEEDED (hr) )
    {
        ASSERT (!!spDsSearch);
        ADS_SEARCHPREF_INFO pSearchPref[1];
        DWORD dwNumPref = 1;

        pSearchPref[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
        pSearchPref[0].vValue.dwType = ADSTYPE_INTEGER;
        pSearchPref[0].vValue.Integer = ADS_SCOPE_ONELEVEL;

        hr = spDsSearch->SetSearchPreference(
                 pSearchPref,
                 dwNumPref
                 );
        if ( SUCCEEDED (hr) )
        {
            static const DWORD  cAttrs = 3;
            static PWSTR        rgszAttrList[cAttrs] = {OID_PROP_DISPLAY_NAME, OID_PROP_OID, OID_PROP_TYPE};
            ADS_SEARCH_HANDLE   hSearchHandle = 0;
            wstring             strQuery;
            ADS_SEARCH_COLUMN   Column;

            Column.pszAttrName = 0;
            strQuery = L"objectClass=msPKI-Enterprise-Oid";

            hr = spDsSearch->ExecuteSearch(
                                 const_cast <PWSTR>(strQuery.c_str ()),
                                 rgszAttrList,
                                 cAttrs,
                                 &hSearchHandle
                                 );
            if ( SUCCEEDED (hr) )
            {
                while ((hr = spDsSearch->GetNextRow (hSearchHandle)) != S_ADS_NOMORE_ROWS )
                {
                    if (FAILED(hr))
                        continue;

                    //
                    // Getting current row's information
                    //
                    hr = spDsSearch->GetColumn(
                             hSearchHandle,
                             rgszAttrList[0],
                             &Column
                             );
                    if ( SUCCEEDED (hr) )
                    {
                        CString strDisplayName = Column.pADsValues->CaseIgnoreString;

                        spDsSearch->FreeColumn (&Column);
                        Column.pszAttrName = NULL;

                        hr = spDsSearch->GetColumn(
                                 hSearchHandle,
                                 rgszAttrList[1],
                                 &Column
                                 );
                        if ( SUCCEEDED (hr) )
                        {
                            bool    bOIDFound = false;
                            CString strOID = Column.pADsValues->CaseIgnoreString;
                            spDsSearch->FreeColumn (&Column);

                            for (POSITION nextPos = g_policyOIDList.GetHeadPosition (); nextPos; )
                            {
                                CPolicyOID* pPolicyOID = g_policyOIDList.GetNext (nextPos);
                                if ( pPolicyOID )
                                {
                                    if ( pPolicyOID->GetOIDW () == strOID )
                                    {
                                        bOIDFound = true;
                                        break;
                                    }
                                }
                            }

                            if ( !bOIDFound )
                            {
                                Column.pszAttrName = NULL;

                                hr = spDsSearch->GetColumn(
                                         hSearchHandle,
                                         rgszAttrList[2],
                                         &Column
                                         );
                                if ( SUCCEEDED (hr) )
                                {
                                    ADS_INTEGER flags = Column.pADsValues->Integer;
                                    spDsSearch->FreeColumn (&Column);
                                    Column.pszAttrName = NULL;

                                    // Only add issuance and application OIDs to the list
                                    if ( CERT_OID_TYPE_ISSUER_POLICY == flags || 
                                        CERT_OID_TYPE_APPLICATION_POLICY == flags )
                                    {
                                        CPolicyOID* pPolicyOID = new CPolicyOID (strOID, strDisplayName, flags);
                                        if ( pPolicyOID )
                                        {
                                            g_policyOIDList.AddTail (pPolicyOID);
                                        }
                                        else
                                            break;
                                    }
                                }
                            }
                        }
                    }
                    else if ( hr != E_ADS_COLUMN_NOT_SET )
                    {
                        break;
                    }
                    else
                    {
                        _TRACE (0, L"IDirectorySearch::GetColumn () failed: 0x%x\n", hr);
                    }
                }
            }
            else
            {
                _TRACE (0, L"IDirectorySearch::ExecuteSearch () failed: 0x%x\n", hr);
            }

            spDsSearch->CloseSearchHandle(hSearchHandle);
        }
        else
        {
            _TRACE (0, L"IDirectorySearch::SetSearchPreference () failed: 0x%x\n", hr);
        }
    }
    else
    {
        _TRACE (0, L"IDirectoryObject::QueryInterface (IDirectorySearch) failed: 0x%x\n", hr);
    }

    _TRACE (-1, L"Leaving EnumerateOIDs: 0x%x\n", hr);
    return hr;
}


HRESULT GetEnterpriseOIDs ()
{
    _TRACE (1, L"Entering GetEnterpriseOIDs\n");
    AFX_MANAGE_STATE(AfxGetStaticModuleState());    
    HRESULT hr = S_OK;

    // Empty the list first
    while ( !g_policyOIDList.IsEmpty () )
    {
        CPolicyOID* pPolicyOID = g_policyOIDList.RemoveHead ();
        if ( pPolicyOID )
            delete pPolicyOID;
    }

    hr = GetBuiltInOIDs ();
    
    if ( SUCCEEDED (hr) )
    {
        CComPtr<IADsPathname> spPathname;
        //
        // Constructing the directory paths
        //
        // security review BryanWal 02/02/2002 ok
        hr = CoCreateInstance(
                    CLSID_Pathname,
                    NULL,
                    CLSCTX_ALL,
                    IID_PPV_ARG (IADsPathname, &spPathname));
        if ( SUCCEEDED (hr) )
        {
            CComBSTR    bstrPathElement;
            ASSERT (!!spPathname);

            bstrPathElement = CERTTMPL_LDAP;
            hr = spPathname->Set(bstrPathElement, ADS_SETTYPE_PROVIDER);
            if ( SUCCEEDED (hr) )
            {
                //
                // Open the root DSE object
                //
                bstrPathElement = CERTTMPL_ROOTDSE;
                hr = spPathname->AddLeafElement(bstrPathElement);
                if ( SUCCEEDED (hr) )
                {
                    BSTR bstrFullPath = 0;
                    hr = spPathname->Retrieve(ADS_FORMAT_X500, &bstrFullPath);
                    if ( SUCCEEDED (hr) )
                    {
                        CComPtr<IADs> spRootDSEObject;
                        VARIANT varNamingContext;


                        hr = ADsGetObject (
                              bstrFullPath,
                              IID_PPV_ARG (IADs, &spRootDSEObject));
                        if ( SUCCEEDED (hr) )
                        {
                            ASSERT (!!spRootDSEObject);
                            //
                            // Get the configuration naming context from the root DSE
                            //
                            bstrPathElement = CERTTMPL_CONFIG_NAMING_CONTEXT;
                            hr = spRootDSEObject->Get(bstrPathElement,
                                                 &varNamingContext);
                            if ( SUCCEEDED (hr) )
                            {
                                hr = spPathname->Set(V_BSTR(&varNamingContext),
                                                    ADS_SETTYPE_DN);
                                if ( SUCCEEDED (hr) )
                                {
                                    bstrPathElement = L"CN=Services";
                                    hr = spPathname->AddLeafElement (bstrPathElement);
                                    if ( SUCCEEDED (hr) )
                                    {
                                        bstrPathElement = L"CN=Public Key Services";
                                        hr = spPathname->AddLeafElement (bstrPathElement);
                                        if ( SUCCEEDED (hr) )
                                        {
                                            bstrPathElement = L"CN=OID";
                                            hr = spPathname->AddLeafElement (bstrPathElement);
                                            if ( SUCCEEDED (hr) )
                                            {
                                                BSTR bstrOIDPath = 0;
                                                hr = spPathname->Retrieve(ADS_FORMAT_X500, &bstrOIDPath);
                                                if ( SUCCEEDED (hr) )
                                                {
                                                    CComPtr<IDirectoryObject> spOIDContObj;

                                                    hr = ADsGetObject (
                                                          bstrOIDPath,
                                                          IID_PPV_ARG (IDirectoryObject, &spOIDContObj));
                                                    if ( SUCCEEDED (hr) )
                                                    {
                                                        hr = EnumerateOIDs (spOIDContObj);
                                                    }
                                                    else
                                                    {
                                                        _TRACE (0, L"ADsGetObject (%s) failed: 0x%x\n", bstrOIDPath, hr);
                                                    }

                                                    SysFreeString (bstrOIDPath);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                _TRACE (0, L"IADs::Get (%s) failed: 0x%x\n", CERTTMPL_CONFIG_NAMING_CONTEXT, hr);
                            }
                        }
                        else
                        {
                            _TRACE (0, L"ADsGetObject (%s) failed: 0x%x\n", bstrFullPath, hr);
                        }
                    }
                }
            }
        }
        else
            hr = E_POINTER;
    }

    _TRACE (-1, L"Leaving GetEnterpriseOIDs: 0x%x\n", hr);
    return hr;
}


bool OIDHasValidFormat (PCWSTR pszOidValue, int& rErrorTypeStrID)
{
    ASSERT (pszOidValue);
    if ( !pszOidValue )
        return false;

    _TRACE (1, L"Entering OIDHasValidFormat (%s)\n", pszOidValue);
    rErrorTypeStrID = 0;

    bool    bFormatIsValid = false;
    int nLen = WideCharToMultiByte(
          CP_ACP,                   // code page
          0,                        // performance and mapping flags
          pszOidValue,              // wide-character string
          -1,                       // -1 - calculate length of null-terminated string automatically
          0,                        // buffer for new string
          0,                        // size of buffer - 0 causes API to return length including null terminator
          0,                        // default for unmappable chars
          0);                       // set when default char used
    if ( nLen > 0 )
    {
        PSTR    pszAnsiBuf = new char[nLen];
        if ( pszAnsiBuf )
        {
            // security review BryanWal 02/02/2002 ok
            ZeroMemory (pszAnsiBuf, nLen*sizeof(char));
            // security review BryanWal 02/02/2002 ok
            nLen = WideCharToMultiByte(
                    CP_ACP,                 // code page
                    0,                      // performance and mapping flags
                    pszOidValue,            // wide-character string
                    -1,                     // -1 - calculate length of null-terminated string automatically
                    pszAnsiBuf,             // buffer for new string
                    nLen,                   // size of buffer
                    0,                      // default for unmappable chars
                    0);                     // set when default char used
            if ( nLen )
            {
                // According to PhilH:
                // The first number is limited to 
                // 0,1 or 2. The second number is 
                // limited to 0 - 39 when the first 
                // number is 0 or 1. Otherwise, any 
                // number.
                // Also, according to X.208, there 
                // must be at least 2 numbers.
                bFormatIsValid = true;
                // security review 2/20/2002 BryanWal ok
                size_t cbAnsiBufLen = strlen (pszAnsiBuf);

                // check for only digits and "."
                size_t nIdx = strspn (pszAnsiBuf, "0123456789.\0");
                if ( nIdx > 0 && nIdx < cbAnsiBufLen )
                {
                    bFormatIsValid = false;
                    rErrorTypeStrID = IDS_OID_CONTAINS_NON_DIGITS;
                }

                // check for consecutive "."s - string not valid if present
                if ( bFormatIsValid && strstr (pszAnsiBuf, "..") )
                {
                    bFormatIsValid = false;
                    rErrorTypeStrID = IDS_OID_CONTAINS_CONSECUTIVE_DOTS;
                }
                

                // must begin with "0." or "1." or "2."
                bool bFirstNumberIs0 = false;
                bool bFirstNumberIs1 = false;
                bool bFirstNumberIs2 = false;
                if ( bFormatIsValid )
                {
                    if ( !strncmp (pszAnsiBuf, "0.", 2) )
                        bFirstNumberIs0 = true;
                    else if ( !strncmp (pszAnsiBuf, "1.", 2) )
                        bFirstNumberIs1 = true;
                    else if ( !strncmp (pszAnsiBuf, "2.", 2) )
                        bFirstNumberIs2 = true;
                    
                    if ( !bFirstNumberIs0 && !bFirstNumberIs1 && !bFirstNumberIs2 )
                    {
                        bFormatIsValid = false;
                        rErrorTypeStrID = IDS_OID_MUST_START_WITH_0_1_2;
                    }
                }

                if ( bFormatIsValid && ( bFirstNumberIs0 || bFirstNumberIs1 ) )
                {
                    PSTR pszBuf = pszAnsiBuf;
                    pszBuf += 2;

                    // there must be a number after the dot
                    // security review 2/20/2002 BryanWal ok
                    if ( strlen (pszBuf) )
                    {
                        // truncate the string at the next dot, if any
                        PSTR pszDot = strstr (pszBuf, ".");
                        if ( pszDot )
                            pszDot[0] = 0;

                        // convert the string to a number and check for range 0-39
                        int nValue = atoi (pszBuf);
                        if ( nValue < 0 || nValue > 39 )
                        {
                            bFormatIsValid = false;
                            rErrorTypeStrID = IDS_OID_0_1_MUST_BE_0_TO_39;
                        }
                    }
                    else
                    {
                        bFormatIsValid = false;
                        rErrorTypeStrID = IDS_OID_MUST_HAVE_TWO_NUMBERS;
                    }
                }

                // ensure no trailing "."
                if ( bFormatIsValid )
                {
                    if ( '.' == pszAnsiBuf[cbAnsiBufLen - 1] )
                    {
                        bFormatIsValid = false;
                        rErrorTypeStrID = IDS_OID_CANNOT_END_WITH_DOT;
                    }
                }

                if ( bFormatIsValid )
                {
                    bFormatIsValid = false;
                    CRYPT_ATTRIBUTE cryptAttr;
                    // security review BryanWal 02/02/2002 ok
                    ::ZeroMemory (&cryptAttr, sizeof (cryptAttr));

                    cryptAttr.cValue = 0;
                    cryptAttr.pszObjId = pszAnsiBuf;
                    cryptAttr.rgValue = 0;

                    DWORD   cbEncoded = 0;
                    BOOL bResult = CryptEncodeObject (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                            PKCS_ATTRIBUTE,
                            &cryptAttr,
                            NULL,
                            &cbEncoded);
                    if ( cbEncoded > 0 )
                    {
                        BYTE* pBuffer = new BYTE[cbEncoded];
                        if ( pBuffer )
                        {
                            bResult = CryptEncodeObject (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                    PKCS_ATTRIBUTE,
                                    &cryptAttr,
                                    pBuffer,
                                    &cbEncoded);
                            if ( bResult )
                            {   
                                DWORD   cbStructInfo = 0;
                                bResult = CryptDecodeObject (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                        PKCS_ATTRIBUTE,
                                        pBuffer,
                                        cbEncoded,
                                        0,
                                        0,
                                        &cbStructInfo);
                                if ( cbStructInfo > 0 )
                                {
                                    BYTE* pStructBuf = new BYTE[cbStructInfo];
                                    if ( pStructBuf )
                                    {
                                        bResult = CryptDecodeObject (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                                PKCS_ATTRIBUTE,
                                                pBuffer,
                                                cbEncoded,
                                                0,
                                                pStructBuf,
                                                &cbStructInfo);
                                        if ( bResult )
                                        {
                                            CRYPT_ATTRIBUTE* pCryptAttr = (CRYPT_ATTRIBUTE*) pStructBuf;
                                            if ( !strcmp (pszAnsiBuf, pCryptAttr->pszObjId) )
                                            {
                                                bFormatIsValid = true;
                                            }
                                        }
                                        delete [] pStructBuf;
                                    }
                                }
                            }
                            delete [] pBuffer;
                        }
                    }
                }
            }
            else
            {
                _TRACE (0, L"WideCharToMultiByte (%s) failed: 0x%x\n", pszOidValue, 
                        GetLastError ());
            }

            delete [] pszAnsiBuf;
        }
    }
    else
    {
        _TRACE (0, L"WideCharToMultiByte (%s) failed: 0x%x\n", pszOidValue, 
                GetLastError ());
    }

    _TRACE (-1, L"Leaving EnumerateOIDs: %s\n", bFormatIsValid ? L"true" : L"false");
    return bFormatIsValid;
}

HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* psp)
{
    ASSERT (psp);
    if ( !psp )
        return 0;

    PROPSHEETPAGE_V3 sp_v3 = {0};
    // security review 2/20/2002 BryanWal ok
    ASSERT (sizeof (sp_v3) >= psp->dwSize);
    if ( sizeof (sp_v3) >= psp->dwSize )
    {
        CopyMemory (&sp_v3, psp, psp->dwSize);
        sp_v3.dwSize = sizeof(sp_v3);
    }
    else
        return 0;

    return (::CreatePropertySheetPage (&sp_v3));
}

/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002 Microsoft Corporation
//
//  Module Name:
//      VSSTaskEx.cpp
//
//  Description:
//      Implementation of the CVSSTaskApp class and DLL initialization
//      routines.
//
//  Author:
//      <name> (<e-mail name>) Mmmm DD, 2002
//
//  Revision History:
//
//  Notes:
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <initguid.h>
#include <CluAdmEx.h>
#include "VSSTaskEx.h"
#include "ExtObj.h"
#include "BasePage.h"
#include "RegExt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IID_DEFINED
#include "ExtObjID_i.c"

CComModule _Module;

#pragma warning( disable : 4701 ) // local variable may be used without having been initialized
#include <atlimpl.cpp>
#pragma warning( default : 4701 )

BEGIN_OBJECT_MAP( ObjectMap )
    OBJECT_ENTRY( CLSID_CoVSSTaskEx, CExtObject )
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// Global Function Prototypes
/////////////////////////////////////////////////////////////////////////////

STDAPI DllCanUnloadNow( void );
STDAPI DllGetClassObject( REFCLSID rclsid, REFIID riid, LPVOID * ppv );
STDAPI DllRegisterServer( void );
STDAPI DllUnregisterServer( void );
STDAPI DllRegisterCluAdminExtension( IN HCLUSTER hcluster );
STDAPI DllUnregisterCluAdminExtension( IN HCLUSTER hcluster );

/////////////////////////////////////////////////////////////////////////////
// class CVSSTaskApp
/////////////////////////////////////////////////////////////////////////////

class CVSSTaskApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();

}; //*** class CVSSTaskApp

/////////////////////////////////////////////////////////////////////////////
// The one and only CVSSTaskApp object

CVSSTaskApp theApp;

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CVSSTaskApp::InitInstance
//
//  Description:
//      Initialize this instance of the application.
//
//  Arguments:
//      None.
//
//  Return Value:
//      Any return codes from CWinApp::InitInstance().
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL CVSSTaskApp::InitInstance( void )
{
    _Module.Init( ObjectMap, m_hInstance );

    // Construct the help path.
    {
        TCHAR   szPath[_MAX_PATH];
        TCHAR   szDrive[_MAX_PATH];
        TCHAR   szDir[_MAX_DIR];
        size_t  cchPath;
        BOOL    fEnable;
        wchar_t *copyReturn;

        VERIFY(::GetSystemWindowsDirectory(szPath, _MAX_PATH));
        cchPath = _tcslen(szPath);
        if (szPath[cchPath - 1] != _T('\\'))
        {
            szPath[cchPath++] = _T('\\');
            szPath[cchPath] = _T('\0');
        } // if: no backslash on the end of the path
        copyReturn = wcsncpy( &szPath[ cchPath ], _T("Help\\"), RTL_NUMBER_OF( szPath ) - cchPath );
        ASSERT( copyReturn == &szPath[ cchPath ] );
        _tsplitpath(szPath, szDrive, szDir, NULL, NULL);
        _tmakepath(szPath, szDrive, szDir, _T("cluadmin"), _T(".hlp"));
        free((void *) m_pszHelpFilePath);
        fEnable = AfxEnableMemoryTracking(FALSE);
        m_pszHelpFilePath = _tcsdup(szPath);
        AfxEnableMemoryTracking(fEnable);
    }  // Construct the help path

    return CWinApp::InitInstance();

}  //*** CVSSTaskApp::InitInstance()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CVSSTaskApp::ExitInstance
//
//  Description:
//      Deinitialize this instance of the application.
//
//  Arguments:
//      None.
//
//  Return Value:
//      Any return codes from CWinApp::ExitInstance().
//
//--
/////////////////////////////////////////////////////////////////////////////
int CVSSTaskApp::ExitInstance( void )
{
    _Module.Term();
    return CWinApp::ExitInstance();

}  //*** CVSSTaskApp::ExitInstance()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  FormatError
//
//  Description:
//      Format an error.
//
//  Arguments:
//      rstrError   [OUT] String in which to return the error message.
//      dwError     [IN] Error code to format.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void FormatError( CString & rstrError, DWORD dwError )
{
    DWORD   _cch;
    TCHAR   _szError[512];

    // Format the NT status code from CLUSAPI.  This is necessary
    // for the cases where cluster messages haven't been added to
    // the system message file yet.
    _cch = FormatMessage(
                    FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                    ::GetModuleHandle(_T("CLUSAPI.DLL")),
                    dwError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                    _szError,
                    sizeof(_szError) / sizeof(TCHAR),
                    0
                    );
    if (_cch == 0)
    {
        _cch = FormatMessage(
                        FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        dwError,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                        _szError,
                        sizeof(_szError) / sizeof(TCHAR),
                        0
                        );
        if (_cch == 0)
        {
            // Format the NT status code from NTDLL since this hasn't been
            // integrated into the system yet.
            _cch = FormatMessage(
                            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                            ::GetModuleHandle(_T("NTDLL.DLL")),
                            dwError,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                            _szError,
                            sizeof(_szError) / sizeof(TCHAR),
                            0
                            );
        }  // if:  error formatting status code from system
    }  // if:  error formatting status code from ClusApi

    if (_cch > 0)
    {
        rstrError = _szError;
    }  // if:  no error
    else
    {
        TRACE(_T("FormatError() - Error 0x%08.8x formatting string for error code 0x%08.8x\n"), GetLastError(), dwError);
        rstrError.Format(_T("Error 0x%08.8x"), dwError);
    }  // else:  error formatting the message

}  //*** FormatError()

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );
    return ( AfxDllCanUnloadNow() && _Module.GetLockCount() == 0 ) ? S_OK : S_FALSE;

}  //*** DllCanUnloadNow()

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject( REFCLSID rclsid, REFIID riid, LPVOID * ppv )
{
    return _Module.GetClassObject( rclsid, riid, ppv );

}  //*** DllGetClassObject()

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer( void )
{
    HRESULT hRes = S_OK;
    // registers object, typelib and all interfaces in typelib
    hRes = _Module.RegisterServer( FALSE /*bRegTypeLib*/ );
    return hRes;

}  //*** DllRegisterServer()

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer( void )
{
    HRESULT hRes = S_OK;
    _Module.UnregisterServer();
    return hRes;

}  //*** DllUnregisterServer()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  DllRegisterCluAdminExtension
//
//  Description:
//      Register the extension with the cluster database.
//
//  Arguments:
//      hCluster        [IN] Handle to the cluster to modify.
//
//  Return Value:
//      S_OK            Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
STDAPI DllRegisterCluAdminExtension( IN HCLUSTER hCluster )
{
    HRESULT     hr;
    HRESULT     hrReturn = S_OK;
    LPCWSTR     pwszResTypes = g_wszResourceTypeNames;

    while ( *pwszResTypes != L'\0' )
    {
        wprintf( L"  %s\n", pwszResTypes );
        hr = RegisterCluAdminResourceTypeExtension(
                    hCluster,
                    pwszResTypes,
                    &CLSID_CoVSSTaskEx
                    );
        if ( hr != S_OK )
        {
            hrReturn = hr;
        } // if:  error registering the extension
        pwszResTypes += lstrlenW( pwszResTypes ) + 1;
    }  // while:  more resource types

    return hrReturn;

}  //*** DllRegisterCluAdminExtension()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  DllUnregisterCluAdminExtension
//
//  Description:
//      Unregister the extension with the cluster database.
//
//  Arguments:
//      hCluster        [IN] Handle to the cluster to modify.
//
//  Return Value:
//      S_OK            Extension unregistered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
STDAPI DllUnregisterCluAdminExtension( IN HCLUSTER hCluster )
{
    HRESULT     hr;
    HRESULT     hrReturn = S_OK;
    LPCWSTR     pwszResTypes = g_wszResourceTypeNames;

    while ( *pwszResTypes != L'\0' )
    {
        wprintf( L"  %s\n", pwszResTypes );
        hr = UnregisterCluAdminResourceTypeExtension(
                    hCluster,
                    pwszResTypes,
                    &CLSID_CoVSSTaskEx
                    );
        if ( hr != S_OK )
        {
            hrReturn = hr;
        } // if:  error unregistering the extension
        pwszResTypes += lstrlenW( pwszResTypes ) + 1;
    }  // while:  more resource types

    return hrReturn;

}  //*** DllUnregisterCluAdminExtension()

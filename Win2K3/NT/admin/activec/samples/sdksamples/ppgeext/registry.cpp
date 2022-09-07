//==============================================================;
//
//  This source code is only intended as a supplement to existing Microsoft documentation.
//
//
//
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (C) 1999 Microsoft Corporation.  All Rights Reserved.
//
//
//
//==============================================================;

#include <objbase.h>
#include <assert.h>
#include "Registry.h"
#include "Extend.h"
#include "GUIDs.h"
#include "globals.h"

// if not standalone comment out next line
//#define STANDALONE

// list all nodes that are extendable here
// List the GUID and then the description
// terminate with a NULL, NULL set.
EXTENSION_NODE _ExtendableNodes[] = {
    {NULL, NULL}
};

// list all of the nodes that we extend
EXTENDER_NODE _NodeExtensions[] = {
    {PropertySheetExtension,
    {0x29743811, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28}},
    {0xcfcdc9f3, 0xc50e, 0x11d2, {0x95, 0x2b, 0x0, 0xc0, 0x4f, 0xb9, 0x2e, 0xc2}},
    _T("Extension to the Rocket Node")},

    {DummyExtension,
    NULL,
    NULL,
    NULL}
};

////////////////////////////////////////////////////////
//
// Internal helper functions prototypes
//

// Set the given key and its value.
BOOL setKeyAndValue(const _TCHAR* pszPath,
                    const _TCHAR* szSubkey,
                    const _TCHAR* szValue) ;

// Set the given key and its value in the MMC Snapin location
BOOL setSnapInKeyAndValue(const _TCHAR* szKey,
                          const _TCHAR* szSubkey,
                          const _TCHAR* szName,
                          const _TCHAR* szValue);

// Set the given valuename under the key to value
BOOL setValue(const _TCHAR* szKey,
              const _TCHAR* szValueName,
              const _TCHAR* szValue);

BOOL setSnapInExtensionNode(const _TCHAR* szSnapID,
                            const _TCHAR* szNodeID,
                            const _TCHAR* szDescription);

// Delete szKeyChild and all of its descendents.
LONG recursiveDeleteKey(HKEY hKeyParent, const _TCHAR* szKeyChild) ;

////////////////////////////////////////////////////////
//
// Constants
//

// Size of a CLSID as a string
//const int CLSID_STRING_SIZE = 39 ;

/////////////////////////////////////////////////////////
//
// Public function implementation
//

//
// Register the component in the registry.
//
HRESULT RegisterServer(HMODULE hModule,            // DLL module handle
                       const CLSID& clsid,         // Class ID
                       const _TCHAR* szFriendlyName)       //   IDs
{
    // Get server location.
    _TCHAR szModule[512] ;
    DWORD dwResult =
        ::GetModuleFileName(hModule,
        szModule,
        sizeof(szModule)/sizeof(_TCHAR)) ;

    assert(dwResult != 0) ;

    // Get CLSID
    LPOLESTR wszCLSID = NULL ;
    HRESULT hr = StringFromCLSID(clsid, &wszCLSID) ;

    assert(SUCCEEDED(hr)) ;

    MAKE_TSTRPTR_FROMWIDE(pszCLSID, wszCLSID);

    // Build the key CLSID\\{...}
    _TCHAR szKey[64] ;
    _tcscpy(szKey, _T("CLSID\\")) ;
        _tcscat(szKey, pszCLSID) ;

    // Add the CLSID to the registry.
    setKeyAndValue(szKey, NULL, szFriendlyName) ;

    // Add the server filename subkey under the CLSID key.
    setKeyAndValue(szKey, _T("InprocServer32"), szModule) ;

    // set the threading model
    _tcscat(szKey, _T("\\InprocServer32"));
    setValue(szKey, _T("ThreadingModel"), _T("Apartment"));

    // Free memory.
    CoTaskMemFree(wszCLSID) ;

    return S_OK ;
}

//
// Remove the component from the registry.
//
LONG UnregisterServer(const CLSID& clsid)       //   IDs
{
    // Get CLSID
    LPOLESTR wszCLSID = NULL ;
    HRESULT hr = StringFromCLSID(clsid, &wszCLSID) ;


    // Build the key CLSID\\{...}
    _TCHAR szKey[64] ;
    _tcscpy(szKey, _T("CLSID\\")) ;

    MAKE_TSTRPTR_FROMWIDE(pszT, wszCLSID);
    _tcscat(szKey, pszT) ;

    // Delete the CLSID Key - CLSID\{...}
    LONG lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szKey) ;
    assert((lResult == ERROR_SUCCESS) ||
               (lResult == ERROR_FILE_NOT_FOUND)) ; // Subkey may not exist.

    // Free memory.
    CoTaskMemFree(wszCLSID) ;

    return S_OK ;
}

//
// Register the snap-in in the registry.
//
HRESULT RegisterSnapin(const CLSID& clsid,         // Class ID
                       const _TCHAR* szNameString,   // NameString
                       const CLSID& clsidAbout)         // Class Id for About Class

{
    // Get CLSID
    LPOLESTR wszCLSID = NULL ;
    LPOLESTR wszAboutCLSID = NULL;
    LPOLESTR wszExtendCLSID = NULL;
    LPOLESTR wszNodeCLSID = NULL;
    EXTENSION_NODE *pExtensionNode;
    EXTENDER_NODE *pNodeExtension;
    _TCHAR szKeyBuf[1024] ;
    HKEY hKey;

    HRESULT hr = StringFromCLSID(clsid, &wszCLSID) ;

    if (IID_NULL != clsidAbout)
        hr = StringFromCLSID(clsidAbout, &wszAboutCLSID);

    MAKE_TSTRPTR_FROMWIDE(pszCLSID, wszCLSID);
    MAKE_TSTRPTR_FROMWIDE(pszAboutCLSID, wszAboutCLSID);


    // Add the CLSID to the registry.
    setSnapInKeyAndValue(pszCLSID, NULL, _T("NameString"), szNameString) ;

#ifdef STANDALONE
    setSnapInKeyAndValue(pszCLSID, _T("StandAlone"), NULL, NULL);
#endif

    if (IID_NULL != clsidAbout)
        setSnapInKeyAndValue(pszCLSID, NULL, _T("About"), pszAboutCLSID);

    // register each of the node types in _ExtendableNodes as an extendable node
    for (pExtensionNode = &(_ExtendableNodes[0]);*pExtensionNode->szDescription;pExtensionNode++)
    {
        hr = StringFromCLSID(pExtensionNode->GUID, &wszExtendCLSID);
        MAKE_TSTRPTR_FROMWIDE(pszExtendCLSID, wszExtendCLSID);
        setSnapInExtensionNode(pszCLSID, pszExtendCLSID, pExtensionNode->szDescription);
        CoTaskMemFree(wszExtendCLSID);
    }

    // register each of the node extensions
    for (pNodeExtension = &(_NodeExtensions[0]);*pNodeExtension->szDescription;pNodeExtension++)
    {
        hr = StringFromCLSID(pNodeExtension->guidNode, &wszExtendCLSID);
        MAKE_TSTRPTR_FROMWIDE(pszExtendCLSID, wszExtendCLSID);
        _tcscpy(szKeyBuf, _T("SOFTWARE\\Microsoft\\MMC\\NodeTypes\\"));
        _tcscat(szKeyBuf, pszExtendCLSID);

        switch (pNodeExtension->eType) {
        case NameSpaceExtension:
            _tcscat(szKeyBuf, _T("\\Extensions\\NameSpace"));
            break;
        case ContextMenuExtension:
            _tcscat(szKeyBuf, _T("\\Extensions\\ContextMenu"));
            break;
        case ToolBarExtension:
            _tcscat(szKeyBuf, _T("\\Extensions\\ToolBar"));
            break;
        case PropertySheetExtension:
            _tcscat(szKeyBuf, _T("\\Extensions\\PropertySheet"));
            break;
        case TaskExtension:
            _tcscat(szKeyBuf, _T("\\Extensions\\Task"));
            break;
        case DynamicExtension:
            _tcscat(szKeyBuf, _T("\\Dynamic Extensions"));
        default:
            break;
        }

        // Create and open key and subkey.
        long lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE ,
            szKeyBuf,
            0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL,
            &hKey, NULL) ;

        if (lResult != ERROR_SUCCESS)
        {
            return FALSE ;
        }

        hr = StringFromCLSID(pNodeExtension->guidExtension, &wszNodeCLSID);
        assert(SUCCEEDED(hr));

        MAKE_TSTRPTR_FROMWIDE(pszNodeCLSID, wszNodeCLSID);
        // Set the Value.
        if (pNodeExtension->szDescription != NULL)
        {
            RegSetValueEx(hKey, pszNodeCLSID, 0, REG_SZ,
                (BYTE *)pNodeExtension->szDescription,
                (_tcslen(pNodeExtension->szDescription)+1)*sizeof(_TCHAR)) ;
        }

        RegCloseKey(hKey) ;

        CoTaskMemFree(wszExtendCLSID);
        CoTaskMemFree(wszNodeCLSID);
    }


    // Free memory.
    CoTaskMemFree(wszCLSID) ;
    if (IID_NULL != clsidAbout)
        CoTaskMemFree(wszAboutCLSID);

    return S_OK ;
}

//
// Unregister the snap-in in the registry.
//
HRESULT UnregisterSnapin(const CLSID& clsid)         // Class ID
{
    _TCHAR szKeyBuf[1024];
    LPOLESTR wszCLSID = NULL;

    // Get CLSID
    HRESULT hr = StringFromCLSID(clsid, &wszCLSID);
        MAKE_TSTRPTR_FROMWIDE(pszCLSID, wszCLSID);
    // Load the buffer with the Snap-In Location
    _tcscpy(szKeyBuf, _T("SOFTWARE\\Microsoft\\MMC\\SnapIns"));

    // Copy keyname into buffer.
    _tcscat(szKeyBuf, _T("\\"));
    _tcscat(szKeyBuf, pszCLSID);

    // Delete the CLSID Key - CLSID\{...}
    LONG lResult = recursiveDeleteKey(HKEY_LOCAL_MACHINE, szKeyBuf);
    assert((lResult == ERROR_SUCCESS) ||
               (lResult == ERROR_FILE_NOT_FOUND)) ; // Subkey may not exist.

	//Uncomment following for loop to unregister all extendable node types
	//Note that if a snap-in's extendable node types are unregistered,
	//any extension snap-ins for these node types will have to be re-registered
	//in order to rebuild their entries under the SOFTWARE\Microsoft\MMC\NodeTypes key

/*
    // Unregister each of the node types in _ExtendableNodes as an extendable node
	// Note that this snap-in does not register any extendable node types

    EXTENSION_NODE *pNode;
	LPOLESTR wszExtendCLSID = NULL;

    for (pNode = &(_ExtendableNodes[0]);*pNode->szDescription;pNode++)
    {
        hr = StringFromCLSID(pNode->GUID, &wszExtendCLSID);
        MAKE_TSTRPTR_FROMWIDE(pszExtendCLSID, wszExtendCLSID);

        // Load the buffer with the Snap-In Location
        _tcscpy(szKeyBuf, _T("SOFTWARE\\Microsoft\\MMC\\NodeTypes\\"));
        // Copy keyname into buffer.
        _tcscat(szKeyBuf, pszExtendCLSID);
        recursiveDeleteKey(HKEY_LOCAL_MACHINE, szKeyBuf);
        CoTaskMemFree(wszExtendCLSID);
    }

*/

    // free the memory
    CoTaskMemFree(wszCLSID);

    return S_OK;
}

//
// Delete a key and all of its descendents.
//
LONG recursiveDeleteKey(HKEY hKeyParent,           // Parent of key to delete
                        const _TCHAR* lpszKeyChild)  // Key to delete
{
    // Open the child.
    HKEY hKeyChild ;
    LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyChild, 0,
        KEY_ALL_ACCESS, &hKeyChild) ;
    if (lRes != ERROR_SUCCESS)
    {
        return lRes ;
    }

    // Enumerate all of the decendents of this child.
    FILETIME time ;
    _TCHAR szBuffer[256] ;
    DWORD dwSize = 256 ;
    while (RegEnumKeyEx(hKeyChild, 0, szBuffer, &dwSize, NULL,
        NULL, NULL, &time) == S_OK)
    {
        // Delete the decendents of this child.
        lRes = recursiveDeleteKey(hKeyChild, szBuffer) ;
        if (lRes != ERROR_SUCCESS)
        {
            // Cleanup before exiting.
            RegCloseKey(hKeyChild) ;
            return lRes;
        }
        dwSize = 256 ;
    }

    // Close the child.
    RegCloseKey(hKeyChild) ;

    // Delete this child.
    return RegDeleteKey(hKeyParent, lpszKeyChild) ;
}

//
// Create a key and set its value.
//   - This helper function was borrowed and modifed from
//     Kraig Brockschmidt's book Inside OLE.
//
BOOL setKeyAndValue(const _TCHAR* szKey,
                    const _TCHAR* szSubkey,
                    const _TCHAR* szValue)
{
    HKEY hKey;
    _TCHAR szKeyBuf[1024] ;

    // Copy keyname into buffer.
    _tcscpy(szKeyBuf, szKey) ;

    // Add subkey name to buffer.
    if (szSubkey != NULL)
    {
        _tcscat(szKeyBuf, _T("\\")) ;
        _tcscat(szKeyBuf, szSubkey ) ;
    }

    // Create and open key and subkey.
    long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT ,
        szKeyBuf,
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL,
        &hKey, NULL) ;
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE ;
    }

    // Set the Value.
    if (szValue != NULL)
    {
        RegSetValueEx(hKey, NULL, 0, REG_SZ,
            (BYTE *)szValue,
            (_tcslen(szValue)+1)*sizeof(_TCHAR)) ;
    }

    RegCloseKey(hKey) ;
    return TRUE ;
}

//
// Open a key value and set it
//
BOOL setValue(const _TCHAR* szKey,
              const _TCHAR* szValueName,
              const _TCHAR* szValue)
{
    HKEY hKey;
    _TCHAR szKeyBuf[1024] ;

    // Copy keyname into buffer.
    _tcscpy(szKeyBuf, szKey) ;

    // Create and open key and subkey.
    long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT ,
        szKeyBuf,
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL,
        &hKey, NULL) ;
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE ;
    }

    // Set the Value.
    if (szValue != NULL)
    {
        RegSetValueEx(hKey, szValueName, 0, REG_SZ,
            (BYTE *)szValue,
            (_tcslen(szValue)+1)*sizeof(_TCHAR)) ;
    }

    RegCloseKey(hKey) ;
    return TRUE ;
}

//
// Create a key and set its value.
//   - This helper function was borrowed and modifed from
//     Kraig Brockschmidt's book Inside OLE.
//
BOOL setSnapInKeyAndValue(const _TCHAR* szKey,
                          const _TCHAR* szSubkey,
                          const _TCHAR* szName,
                          const _TCHAR* szValue)
{
    HKEY hKey;
    _TCHAR szKeyBuf[1024] ;

    // Load the buffer with the Snap-In Location
    _tcscpy(szKeyBuf, _T("SOFTWARE\\Microsoft\\MMC\\SnapIns"));

    // Copy keyname into buffer.
    _tcscat(szKeyBuf, _T("\\")) ;
    _tcscat(szKeyBuf, szKey) ;

    // Add subkey name to buffer.
    if (szSubkey != NULL)
    {
        _tcscat(szKeyBuf, _T("\\")) ;
        _tcscat(szKeyBuf, szSubkey ) ;
    }

    // Create and open key and subkey.
    long lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE ,
        szKeyBuf,
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL,
        &hKey, NULL) ;
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE ;
    }

    // Set the Value.
    if (szValue != NULL)
    {
        RegSetValueEx(hKey, szName, 0, REG_SZ,
            (BYTE *)szValue,
            (_tcslen(szValue)+1)*sizeof(_TCHAR)) ;
    }

    RegCloseKey(hKey) ;
    return TRUE ;
}

BOOL setSnapInExtensionNode(const _TCHAR* szSnapID,
                            const _TCHAR* szNodeID,
                            const _TCHAR* szDescription)
{
    HKEY hKey;
    _TCHAR szSnapNodeKeyBuf[1024] ;
    _TCHAR szMMCNodeKeyBuf[1024];

    // Load the buffer with the Snap-In Location
    _tcscpy(szSnapNodeKeyBuf, _T("SOFTWARE\\Microsoft\\MMC\\SnapIns\\"));
    // add in the clisid into buffer.
    _tcscat(szSnapNodeKeyBuf, szSnapID) ;
    _tcscat(szSnapNodeKeyBuf, _T("\\NodeTypes\\"));
    _tcscat(szSnapNodeKeyBuf, szNodeID) ;

    // Load the buffer with the NodeTypes Location
    _tcscpy(szMMCNodeKeyBuf, _T("SOFTWARE\\Microsoft\\MMC\\NodeTypes\\"));
    _tcscat(szMMCNodeKeyBuf, szNodeID) ;

    // Create and open the Snapin Key.
    long lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE ,
        szSnapNodeKeyBuf,
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL,
        &hKey, NULL) ;
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE ;
    }

    // Set the Value.
    if (szDescription != NULL)
    {
        RegSetValueEx(hKey, NULL, 0, REG_SZ,
            (BYTE *)szDescription,
            (_tcslen(szDescription)+1)*sizeof(_TCHAR)) ;
    }

    RegCloseKey(hKey) ;

    // Create and open the NodeTypes Key.
    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE ,
        szMMCNodeKeyBuf,
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL,
        &hKey, NULL) ;
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE ;
    }

    // Set the Value.
    if (szDescription != NULL)
    {
        RegSetValueEx(hKey, NULL, 0, REG_SZ,
            (BYTE *)szDescription,
            (_tcslen(szDescription)+1)*sizeof(_TCHAR)) ;
    }

    RegCloseKey(hKey) ;

    return TRUE ;
}

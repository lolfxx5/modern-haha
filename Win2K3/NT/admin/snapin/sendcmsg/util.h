//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1997-2002.
//
//  File:       util.h
//
//  Contents:   
//
//----------------------------------------------------------------------------
//  Util.h

extern HINSTANCE g_hInstance;

#ifndef APIERR
    typedef DWORD APIERR;       // Error code typically returned by ::GetLastError()
#endif

/////////////////////////////////////////////////////////////////////
//
// Dummy macros
//
#define INOUT       // Parameter is both input and output
#define IGNORED     // Output parameter is ignored

/////////////////////////////////////////////////////////////////////
//
// Handy macros
//
#define LENGTH(x)           (sizeof(x)/sizeof(x[0]))


/////////////////////////////////////////////////////////////////////
int ListView_FindString(HWND hwndListview, PCWSTR pszTextSearch);
int ListView_GetSelectedItem(HWND hwndListview);
void ListView_SelectItem(HWND hwndListview, int iItem);
void ListView_UnselectItem(HWND hwndListview, int iItem);
void ListView_UnselectAllItems(HWND hwndListview);
void ListView_SetItemImage(HWND hwndListview, int iItem, int iImage);

BOOL FTrimString(INOUT WCHAR szString[]);

/////////////////////////////////////////////////////////////////////
INT_PTR DoDialogBox(
    UINT wIdDialog,
    HWND hwndParent,
    DLGPROC dlgproc,
    LPARAM lParam = 0);

int DoMessageBox (HWND hwndParent, UINT uStringId, UINT uFlags = MB_OK | MB_ICONINFORMATION);

/////////////////////////////////////////////////////////////////////
HKEY RegOpenOrCreateKey(HKEY hkeyRoot, const WCHAR szSubkey[]);
BOOL RegWriteString(HKEY hkey, const WCHAR szKey[], const WCHAR szValue[]);
BOOL RegWriteString(HKEY hkey, const WCHAR szKey[], UINT uStringId);

/////////////////////////////////////////////////////////////////////
HRESULT HrExtractDataAlloc(
    IN IDataObject * pDataObject,
    IN UINT cfClipboardFormat,
    OUT PVOID * ppavData,
    OUT UINT * pcbData = NULL);


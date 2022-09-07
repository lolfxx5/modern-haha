//+----------------------------------------------------------------------------
//
//  Windows NT Directory Service Property Pages
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 2001
//
//  File:      ForestVersion.cxx
//
//  Contents:  Dialogs and supporting code for displaying and raising the
//             forest version.
//
//  History:   14-April-01 EricB created
//
//
//-----------------------------------------------------------------------------

#include "pch.h"
#pragma hdrstop
#include <ntdsadef.h>
#include "proppage.h"
#include "qrybase.h"
#include "BehaviorVersion.h"

//+----------------------------------------------------------------------------
//
//  Function:  DSPROP_ForestVersionDlg
//
//  Synopsis:  Puts up a dialog that allows the user to view the forest version
//             levels available and if not at the highest level, to raise the
//             forest version.
//
//  Arguments: [pwzConfigPath] - The full ADSI path to the configuration container.
//             [pwzPartitionsPath] - The full ADSI path to the partitions container.
//             [pwzSchemaPath] - The full ADSI path to the schema container.
//             [pwzRootDnsName] - The DNS name of the enterprise root domain.
//             [hWndParent] - The caller's top level window handle.
//
//  Notes:     This function is called only from Domains & Trusts. If the
//             snapin is running on a child domain, the DC named in the ADSI
//             path will be for the child domain. In that case will need to
//             get the PDC (if possible) for the root domain.
//
//-----------------------------------------------------------------------------
void
DSPROP_ForestVersionDlg(PCWSTR pwzConfigPath,
                        PCWSTR pwzPartitionsPath,
                        PCWSTR pwzSchemaPath,
                        PCWSTR pwzRootDnsName,
                        HWND hWndParent)
{
   dspDebugOut((DEB_ITRACE, "DSPROP_ForestVersionDlg, config: %ws\n", pwzPartitionsPath));
   if (!pwzConfigPath || !pwzPartitionsPath || !pwzSchemaPath ||
       !pwzRootDnsName || !hWndParent || !IsWindow(hWndParent))
   {
      dspAssert(FALSE);
      return;
   }

   CForestVersion ForestVer;

   HRESULT hr = ForestVer.Init(pwzConfigPath, pwzPartitionsPath, pwzSchemaPath);

   if (HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN) == hr)
   {
      ErrMsgParam(IDS_ERR_NO_DC, (LPARAM)pwzRootDnsName, hWndParent);
      return;
   }
   if (HRESULT_FROM_WIN32(ERROR_DS_SERVER_DOWN) == hr)
   {
      ErrMsgParam(IDS_ERR_DC_NOT_WORKING, (LPARAM)ForestVer.GetDC(), hWndParent);
      return;
   }
   if (S_FALSE == hr)
   {
      // Error already reported.
      return;
   }

   CHECK_HRESULT_REPORT(hr, hWndParent, return);

   hr = ForestVer.CheckHighestPossible();

   CHECK_HRESULT_REPORT(hr, hWndParent, return);

   int nTemplateID = IDD_RAISE_FOREST_VERSION;

   if (!ForestVer.IsFsmoDcFound())
   {
      nTemplateID = IDD_CANT_RAISE_FOREST;
   }
   else if (!ForestVer.CanRaise())
   {
      nTemplateID = (ForestVer.IsHighest()) ?
                         IDD_HIGHEST_FOREST_VERSION : IDD_CANT_RAISE_FOREST;
   }

   CForestVersionDlg dialog(hWndParent, pwzRootDnsName, ForestVer, nTemplateID);

   dialog.DoModal();
}

//+----------------------------------------------------------------------------
//
//  Class:     CForestVersionDlg
//
//  Purpose:   Raise the forest behavior version.
//
//-----------------------------------------------------------------------------
CForestVersionDlg::CForestVersionDlg(HWND hParent,
                                     PCWSTR pwzRootDNS,
                                     CForestVersion & ForestVer,
                                     int nTemplateID) :
   _strRootDnsName(pwzRootDNS),
   _ForestVer(ForestVer),
   _nTemplateID(nTemplateID),
   CModalDialog(hParent, nTemplateID)
{
    TRACE(CForestVersionDlg,CForestVersionDlg);
#ifdef _DEBUG
    // NOTICE-2002/02/12-ericb - SecurityPush: szClass is 32 chars in size so this is safe.
    strcpy(szClass, "CForestVersionDlg");
#endif
}

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersionDlg::OnInitDialog
//
//  Synopsis:  Set the initial control values.
//
//-----------------------------------------------------------------------------
LRESULT
CForestVersionDlg::OnInitDialog(LPARAM lParam)
{
   TRACE(CForestVersionDlg,OnInitDialog);

   SetDlgItemText(_hDlg, IDC_VER_NAME_STATIC, _strRootDnsName);

   CStrW strVersion;
   _ForestVer.GetString(_ForestVer.GetVer(), strVersion);

   SetDlgItemText(_hDlg, IDC_CUR_VER_STATIC, strVersion);

   _ForestVer.SetDlgHwnd(_hDlg);

   if (!_ForestVer.IsFsmoDcFound())
   {
      ShowWindow(GetDlgItem(_hDlg, IDC_FOR_CANT_RAISE_DETAILS), SW_HIDE);
      ShowWindow(GetDlgItem(_hDlg, IDC_SAVE_LOG), SW_HIDE);
      CStrW strFormat, strMsg;
      // NOTICE-2002/02/12-ericb - SecurityPush: CStrW::LoadString sets the
      // string to an empty string on failure.
      strFormat.LoadString(g_hInstance, IDS_FOREST_CANT_RAISE_FSMO);
      strMsg.Format(strFormat, _ForestVer.GetFsmoDcName());
      SetDlgItemText(_hDlg, IDC_FOR_CANT_RAISE_STATIC, strMsg);
   }
   else if (_ForestVer.CanRaise())
   {
      dspAssert(IDD_RAISE_FOREST_VERSION == _nTemplateID);
      InitCombobox();

      if (_ForestVer.IsReadOnly())
      {
         EnableWindow(GetDlgItem(_hDlg, IDOK), FALSE);
         CStrW strMsg;
         // NOTICE-2002/02/12-ericb - SecurityPush: see LoadString note above.
         strMsg.LoadString(g_hInstance, IDS_FOREST_CANT_RAISE_ACCESS);
         SetDlgItemText(_hDlg, IDC_CANT_RAISE_STATIC, strMsg);
      }
   }
   else if (IDD_CANT_RAISE_FOREST == _nTemplateID)
   {
      CStrW strCantRaise;
      // NOTICE-2002/02/12-ericb - SecurityPush: see LoadString note above.
      strCantRaise.LoadString(g_hInstance, IDS_FOR_CANT_RAISE_STATIC);
      SetDlgItemText(_hDlg, IDC_FOR_CANT_RAISE_STATIC, strCantRaise);
   }

   return true;
}

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersionDlg::OnCommand
//
//  Synopsis:  Handle control notifications.
//
//-----------------------------------------------------------------------------
LRESULT
CForestVersionDlg::OnCommand(int id, HWND hwndCtl, UINT codeNotify)
{
   switch (codeNotify)
   {
   case BN_CLICKED:
      switch (id)
      {
      case IDOK:
         OnOK();
         break;

      case IDCANCEL:
         EndDialog(_hDlg, IDCANCEL);
         break;

      case IDC_HELP_BTN:
         ShowHelp(L"ADConcepts.chm::/sag_levels.htm");
         break;

      case IDC_SAVE_LOG:
         OnSaveLog();
         break;
      }
   }

   return 0;
}

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersionDlg::OnOK
//
//  Synopsis:   
//
//-----------------------------------------------------------------------------
void
CForestVersionDlg::OnOK(void)
{
   UINT nVer = ReadComboSel();

   dspDebugOut((DEB_ITRACE, "Combobox selection was %d\n", nVer));

   if (FOREST_VER_ERROR == nVer)
   {
      // No user selection. Set one and return.
      SendDlgItemMessage(_hDlg, IDC_VER_COMBO, CB_SETCURSEL, 0, 0);
      return;
   }

   int iRet = IDCANCEL;
   CStrW strTitle, strMsg;
   // NOTICE-2002/02/12-ericb - SecurityPush: see LoadString note above.
   strTitle.LoadString(g_hInstance, IDS_RAISE_FOR_VER_TITLE);
   strMsg.LoadString(g_hInstance, IDS_CONFIRM_FOR_RAISE);

   iRet = MessageBox(_hDlg, strMsg, strTitle, MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2);

   if (iRet == IDOK)
   {
      HRESULT hr = _ForestVer.RaiseVersion(nVer);

      if (SUCCEEDED(hr))
      {
         // NOTICE-2002/02/12-ericb - SecurityPush: see LoadString note above.
         strMsg.LoadString(g_hInstance, IDS_FOREST_MODE_CHANGED);
         MessageBox(_hDlg, strMsg, strTitle, MB_OK | MB_ICONINFORMATION);
      }
      else
      {
         if (HRESULT_FROM_WIN32(ERROR_DS_UNWILLING_TO_PERFORM) == hr)
         {
            ErrMsg(IDS_VERSION_UNWILLING_TO_RAISE, _hDlg);
         }
         else
         {
            REPORT_ERROR_FORMAT(hr, IDS_VERSION_ERROR_FORMAT, _hDlg);
         }
      }

      EndDialog(_hDlg, IDOK);
   }
}

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersionDlg::OnHelp
//
//  Synopsis:  Put up popup help for the control.
//
//-----------------------------------------------------------------------------
/* LRESULT
CForestVersionDlg::OnHelp(LPHELPINFO pHelpInfo)
{
   if (!pHelpInfo)
   {
      dspAssert(FALSE);
      return 0;
   }
   dspDebugOut((DEB_ITRACE, "WM_HELP: CtrlId = %d, ContextId = 0x%x\n",
                pHelpInfo->iCtrlId, pHelpInfo->dwContextId));

   if (pHelpInfo->iCtrlId < 1 || IDH_NO_HELP == pHelpInfo->dwContextId)
   {
      return 0;
   }
   WinHelp(_hDlg, DSPROP_HELP_FILE_NAME, HELP_CONTEXTPOPUP, pHelpInfo->dwContextId);

   return 0;
} */

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersionDlg::OnSaveLog
//
//  Synopsis:  Prompts the user for a file name to save the DC log.
//
//-----------------------------------------------------------------------------
void
CForestVersionDlg::OnSaveLog(void)
{
   TRACE(CForestVersionDlg,OnSaveLog);
   HRESULT hr = S_OK;
   OPENFILENAME ofn = {0};
   WCHAR wzFilter[MAX_PATH + 1] = {0}, wzFileName[MAX_PATH + MAX_PATH + 1] = {0};
   CStrW strFilter, strExt, strMsg;
   CWaitCursor Wait;

   // NOTICE-2002/02/12-ericb - SecurityPush: wzFileName is initialized to be all
   // nulls and is one char longer than the length passed to wcsncpy so that the
   // result is always null terminated even if _strDomainDnsName were longer than
   // MAX_PATH + MAX_PATH. The calls to wcsncat are correct because they pass
   // in the remaining buffer capacity, not the total buffer length.
   //
   wcsncpy(wzFileName, _strRootDnsName, MAX_PATH + MAX_PATH);
   strExt.LoadString(g_hInstance, IDS_FTFILE_SUFFIX);
   wcsncat(wzFileName, strExt, MAX_PATH + MAX_PATH - wcslen(wzFileName));
   wcsncat(wzFileName, L".", MAX_PATH + MAX_PATH - wcslen(wzFileName));
   strExt.LoadString(g_hInstance, IDS_FTFILE_CSV_EXT);
   wcsncat(wzFileName, strExt, MAX_PATH + MAX_PATH - wcslen(wzFileName));

   // NOTICE-2002/02/12-ericb - SecurityPush: wzFilter is initialized to all
   // zeros and is one char longer than the length passed to LoadString.
   //
   LoadString(g_hInstance, IDS_FTFILE_FILTER, wzFilter, MAX_PATH);

   ofn.lStructSize = sizeof(ofn);
   ofn.hwndOwner = _hDlg;
   ofn.lpstrFile = wzFileName;
   ofn.nMaxFile = MAX_PATH + MAX_PATH + 1;
   ofn.Flags = OFN_OVERWRITEPROMPT;
   ofn.lpstrDefExt = strExt;
   ofn.lpstrFilter = wzFilter;

   if (GetSaveFileName(&ofn))
   {
      dspDebugOut((DEB_ITRACE, "Saving forest DC version info to %ws\n", ofn.lpstrFile));
      PWSTR pwzErr = NULL;
      BOOL fSucceeded = TRUE;

      HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0,
                                NULL, CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL, NULL);

      if (INVALID_HANDLE_VALUE != hFile)
      {
         CStrW str, strFormat;
         strMsg = g_wzBOM; // start with ByteOrderMark
         // NOTICE-2002/02/12-ericb - SecurityPush: see LoadString note above.
         strFormat.LoadString(g_hInstance, IDS_FOR_VER_LOG_PREFIX);
         str.Format(strFormat, _strRootDnsName);
         strMsg += str;
         _ForestVer.GetString(_ForestVer.GetVer(), str);
         strMsg += str;
         strMsg += g_wzCRLF;
         strMsg += g_wzCRLF;
         // NOTICE-2002/02/12-ericb - SecurityPush: see LoadString note above.
         str.LoadString(g_hInstance, IDS_FOR_VER_LOG_HDR);
         strMsg += str;
         strMsg += g_wzCRLF;

         hr = _ForestVer.BuildDcListString(strMsg);

         if (SUCCEEDED(hr))
         {
            strMsg += g_wzCRLF;
            strMsg += g_wzCRLF;

            // BuildDcListString returns S_FALSE if there are no downlevel DCs.
            // Don't show the mixed mode domain list if there are downlevel DCs
            // because the AD won't replicate the nTMixedDomain attrib to the
            // cross-refs if there are downlevel DCs. Thus, if there are
            // downlevel DCs then the mixed domain info on the cross-refs will
            // not be accurate so don't show it. The presence of downlevel DCs
            // is sufficient information to show what needs to be upgraded to
            // unblock raising the forest version.
            //
            if (S_FALSE == hr)
            {
               // NOTICE-2002/02/12-ericb - SecurityPush: see LoadString note above.
               str.LoadString(g_hInstance, IDS_FOR_VER_LOG_MODE_HDR);
               strMsg += str;
               strMsg += g_wzCRLF;

               hr = _ForestVer.BuildMixedModeList(strMsg);
            }

            if (SUCCEEDED(hr))
            {
               strMsg += g_wzCRLF;

               DWORD dwWritten;

               // NOTICE-2002/02/12-ericb - SecurityPush: see the note for
               // CDomainVersionDlg::OnSaveLog in DomainVersion.cxx.
               //
               fSucceeded = WriteFile(hFile, strMsg.GetBuffer(0),
                                      strMsg.GetLength() * sizeof(WCHAR),
                                      &dwWritten, NULL);
            }
         }
         CloseHandle(hFile);
      }
      else
      {
         fSucceeded = FALSE;
      }


      if (!fSucceeded || FAILED(hr))
      {
         CStrW strTitle;
         // NOTICE-2002/02/12-ericb - SecurityPush: see LoadString note above.
         // string on failure.
         strTitle.LoadString(g_hInstance, IDS_DNT_MSG_TITLE);
         LoadErrorMessage(FAILED(hr) ? hr : GetLastError(), 0, &pwzErr);
         if (pwzErr)
         {
            // NOTICE-2002/02/12-ericb - SecurityPush: CStrW::FormatMessage sets
            // the string to an empty string on failure.
            strMsg.FormatMessage(g_hInstance, IDS_LOGFILE_CREATE_FAILED, pwzErr);
            delete [] pwzErr;
         }
         MessageBox(_hDlg, strMsg, strTitle, MB_ICONERROR);
      }
   }
}

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersionDlg::InitCombobox
//
//  Synopsis:  Fills the combobox with levels appropriate for the current
//             state.
//
//-----------------------------------------------------------------------------
void
CForestVersionDlg::InitCombobox(void)
{
   CStrW strVersion;

   switch (_ForestVer.GetVer())
   {
   case FOREST_VER_WIN2K:
   case FOREST_VER_XP_BETA:
      //
      // Can go to Whistler.
      //
      _ForestVer.GetString(FOREST_VER_XP, strVersion);
      SendDlgItemMessage(_hDlg, IDC_VER_COMBO, CB_INSERTSTRING, 0,
                         (LPARAM)(PCWSTR)strVersion);
      break;

   case FOREST_VER_XP:
      break;

   default:
      dspAssert(FALSE);
      return;
   }

   SendDlgItemMessage(_hDlg, IDC_VER_COMBO, CB_SETCURSEL, 0, 0);
}

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersionDlg::ReadComboSel
//
//  Synopsis:  Read the current selection of the combobox.
//
//  Notes:     Currently only one choice for the new version which is why the
//             value of lRet is not used.
//
//-----------------------------------------------------------------------------
UINT
CForestVersionDlg::ReadComboSel(void)
{
   LONG_PTR lRet = SendDlgItemMessage(_hDlg, IDC_VER_COMBO, CB_GETCURSEL, 0, 0);

   if (CB_ERR == lRet)
   {
      return FOREST_VER_ERROR;
   }

   dspAssert(0 == lRet);

   return FOREST_VER_XP;
}

//+----------------------------------------------------------------------------
//
//  Class:     CForestVersion
//
//  Purpose:   Manages the interpretation of the forest version value.
//
//-----------------------------------------------------------------------------

CForestVersion::~CForestVersion(void)
{
   TRACE(CForestVersion,~CForestVersion);

   for (DOMAIN_LIST::iterator i = _DomainLogList.begin();
        i != _DomainLogList.end();
        ++i)
   {
      delete *i;
   }
}

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersion::Init
//
//  Synopsis:  Called from the Domain General property page. Use the local DC
//             to read the partitiona and schema paths.
//
//-----------------------------------------------------------------------------
HRESULT
CForestVersion::Init(PCWSTR pwzDC)
{
   TRACE(CForestVersion,Init);
   HRESULT hr = S_OK;
   CWaitCursor Wait;

   _strDC = pwzDC;

   CDSBasePathsInfo BasePaths;

   hr = BasePaths.InitFromName(_strDC);

   CHECK_HRESULT(hr, return hr);
   CSmartPtr<WCHAR> spwzPartitionsPath, spwzSchemaPath, spwzConfigPath;

   if (!BasePaths.GetPartitionsPath(&spwzPartitionsPath))
   {
      CHECK_HRESULT(E_OUTOFMEMORY, return E_OUTOFMEMORY);
   }

   _strPartitionsPath = spwzPartitionsPath;

   if (!BasePaths.GetConfigPath(&spwzConfigPath))
   {
      CHECK_HRESULT(E_OUTOFMEMORY, return E_OUTOFMEMORY);
   }

   _strConfigPath = spwzConfigPath;

   if (!BasePaths.GetSchemaPath(&spwzSchemaPath))
   {
      CHECK_HRESULT(E_OUTOFMEMORY, return E_OUTOFMEMORY);
   }

   hr = FindSchemaMasterReadVersion(spwzSchemaPath);

   return hr;
}

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersion::Init
//
//  Synopsis:  Called from DSPROP_ForestVersionDlg.
//
//-----------------------------------------------------------------------------
HRESULT
CForestVersion::Init(PCWSTR pwzConfigPath, PCWSTR pwzPartitionsPath,
                     PCWSTR pwzSchemaPath)
{
   TRACE(CForestVersion,Init);
   CWaitCursor Wait;

   _strConfigPath = pwzConfigPath;
   _strPartitionsPath = pwzPartitionsPath;

   //
   // Save the local DC name in case the FSMO role holder is unavailable.
   //
   CPathCracker PathCrack;
   HRESULT hr = PathCrack.Set(_strConfigPath, ADS_SETTYPE_FULL);

   CHECK_HRESULT(hr, return hr);
   CComBSTR bstrDC;

   hr = PathCrack.Retrieve(ADS_FORMAT_SERVER, &bstrDC);

   CHECK_HRESULT(hr, return hr);

   _strDC = bstrDC;

   //
   // Locate the Schema FSMO role holder.
   //
   hr = FindSchemaMasterReadVersion(pwzSchemaPath);

   if (S_FALSE == hr)
   {
      return hr;
   }
   CHECK_HRESULT(hr, return hr);

   //
   // Check if the caller has write access.
   //

   CComPtr<IDirectoryObject> spPartitions;

   hr = DSAdminOpenObject(_strPartitionsPath, 
                          __uuidof(IDirectoryObject), 
                          (void **)&spPartitions);

   CHECK_HRESULT(hr, return hr);

   Smart_PADS_ATTR_INFO pAttrs;
   DWORD cAttrs = 0;
   PWSTR rgwzNames[] = {g_wzAllowed};

   hr = spPartitions->GetObjectAttributes(rgwzNames, ARRAYLENGTH(rgwzNames),
                                          &pAttrs, &cAttrs);

   if (!CHECK_ADS_HR_IGNORE_UNFOUND_ATTR(&hr, NULL))
   {
      return hr;
   }

   for (DWORD j = 0; cAttrs && j < pAttrs->dwNumValues; j++)
   {
      // NOTICE-2002/02/12-ericb - SecurityPush: after verifying the pointer is
      // non-null, compare it with a static string.
      if (pAttrs->pADsValues[j].CaseIgnoreString)
      {
         size_t cch = 0;
         hr = StringCchLength(pAttrs->pADsValues[j].CaseIgnoreString, wcslen(g_wzBehaviorVersion)+1, &cch);
         if (SUCCEEDED(hr) &&
             _wcsnicmp(pAttrs->pADsValues[j].CaseIgnoreString, g_wzBehaviorVersion, cch) == 0)
         {
            _fReadOnly = false;
            break;
         }
      }
   }

   return S_OK;
}

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersion::FindSchemaMasterReadVersion
//
//  Synopsis:  Discover which DC is the schema master by reading the
//             fSMORoleOwner attribute on the schema container. It lists the
//             nTDSDSA object for that DC. Strip off the nTDSDSA object CN,
//             bind to the Server object and read its dNSHostName value for
//             the name of the FSMO role holder DC.
//             Then using the DC bind to the partitions container to read the
//             forest version.
//
//-----------------------------------------------------------------------------
HRESULT
CForestVersion::FindSchemaMasterReadVersion(PCWSTR pwzSchemaPath)
{
   HRESULT hr = S_OK;
   CComPtr<IADs> spSchema;

   hr = DSAdminOpenObject(pwzSchemaPath,
                          __uuidof(IADs), 
                          (void **)&spSchema);

   CHECK_HRESULT(hr, return hr);
   CComVariant var;

   hr = spSchema->Get(CComBSTR(L"fSMORoleOwner"), &var);

   CHECK_HRESULT(hr, return hr);
   CComVariant varSrvDnsName;
   CComPtr<IADs> spServer;

   hr = ReadDnsSrvName(var.bstrVal, spServer, varSrvDnsName);

   if (E_ADS_PROPERTY_NOT_FOUND == hr)
   {
      ErrMsg(IDS_PROMOTION_STILL_REPLICATING, GetDlgHwnd());
      _fCanRaiseBehaviorVersion = false;
      return S_FALSE;
   }

   _strFsmoDC = varSrvDnsName.bstrVal;

   CHECK_HRESULT(hr, return hr);
   CPathCracker PathCrack;

   hr = PathCrack.Set(_strPartitionsPath, ADS_SETTYPE_FULL);

   CHECK_HRESULT(hr, return hr);

   hr = PathCrack.Set(varSrvDnsName.bstrVal, ADS_SETTYPE_SERVER);

   CHECK_HRESULT(hr, return hr);
   CComBSTR bstrPartitionsPath;

   hr = PathCrack.Retrieve(ADS_FORMAT_X500, &bstrPartitionsPath);

   CHECK_HRESULT(hr, return hr);

   CComPtr<IADs> spPartitions;

   hr = DSAdminOpenObject(bstrPartitionsPath, 
                          __uuidof(IADs), 
                          (void **)&spPartitions);
   if (FAILED(hr))
   {
      CHECK_HRESULT(hr, ;);
      //
      // Perhaps the FSMO holder is offline, try with the local DC.
      //
      hr = PathCrack.Set(_strDC, ADS_SETTYPE_SERVER);

      CHECK_HRESULT(hr, return hr);

      hr = PathCrack.Retrieve(ADS_FORMAT_X500, &bstrPartitionsPath);

      CHECK_HRESULT(hr, return hr);

      hr = DSAdminOpenObject(bstrPartitionsPath,
                             __uuidof(IADs), 
                             (void **)&spPartitions);

      CHECK_HRESULT(hr, return hr);

      _fFsmoDcFound = false;
   }
   else
   {
      // Save the FSMO DC name.
      //
      _strDC = _strFsmoDC;

      // Set the server name on the saved paths to the new DC.
      //
      hr = PathCrack.Set(_strConfigPath, ADS_SETTYPE_FULL);

      CHECK_HRESULT(hr, return hr);

      hr = PathCrack.Set(varSrvDnsName.bstrVal, ADS_SETTYPE_SERVER);

      CHECK_HRESULT(hr, return hr);
      CComBSTR bstrConfigPath;

      hr = PathCrack.Retrieve(ADS_FORMAT_X500, &bstrConfigPath);

      CHECK_HRESULT(hr, return hr);

      _strConfigPath = bstrConfigPath;
      _strPartitionsPath = bstrPartitionsPath;
   }

   CComVariant varVer;

   hr = spPartitions->Get(CComBSTR(g_wzBehaviorVersion), &varVer);

   if (E_ADS_PROPERTY_NOT_FOUND == hr)
   {
      _nCurVer = FOREST_VER_WIN2K;
      hr = S_OK;
   }
   else
   {
      CHECK_HRESULT(hr, return hr);

      _nCurVer = varVer.lVal;
   }

   _fInitialized = true;

   dspDebugOut((DEB_ITRACE, "Forest version on %ws is %u\n",
                _strPartitionsPath.GetBuffer(0), _nCurVer));
   return hr;
}

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersion::GetString
//
//  Synopsis:  Return the string representation of the behavior version.
//
//-----------------------------------------------------------------------------
bool
CForestVersion::GetString(UINT nVer, CStrW & strVer)
{
   UINT nID = 0;

   switch (nVer)
   {
   case FOREST_VER_WIN2K:
      nID = IDS_FOR_VER_W2K;
      break;

   case FOREST_VER_XP_BETA:
      nID = IDS_FOR_VER_XP_BETA;
      break;

   case FOREST_VER_XP:
      nID = IDS_FOR_VER_XP;
      break;

   default:
      dspAssert(FALSE);
      return false;
   }
   // NOTICE-2002/02/12-ericb - SecurityPush: see LoadString note above.
   strVer.LoadString(g_hInstance, nID);

   return true;
}

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersion::RaiseVersion
//
//  Synopsis:  Raise the forest behavior version.
//
//  Notes:     Currently only one choice for the new version which is why the
//             nVer parameter is not used.
//
//-----------------------------------------------------------------------------
HRESULT
CForestVersion::RaiseVersion(UINT nVer)
{
   TRACE(CForestVersion,RaiseVersion);
   UNREFERENCED_PARAMETER(nVer);
   if (!_fInitialized)
   {
      dspAssert(FALSE);
      return E_FAIL;
   }

   HRESULT hr = S_OK;
   CComPtr<IADs> spPartitions;

   hr = DSAdminOpenObject(_strPartitionsPath,
                          __uuidof(IADs), 
                          (void **)&spPartitions);

   CHECK_HRESULT(hr, return hr);
   CComVariant var;

   var.vt = VT_I4;
   var.lVal = FOREST_VER_XP;

   hr = spPartitions->Put(CComBSTR(g_wzBehaviorVersion), var);

   CHECK_HRESULT(hr, return hr);

   hr = spPartitions->SetInfo();

   CHECK_HRESULT(hr, return hr);

   return S_OK;
}

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersion::CheckHighestPossible
//
//  Synopsis:  Enumerate all of the DCs to find the lowest version which thus
//             imposes a constraint on the highest version to which the forest
//             can be raised.
//
//-----------------------------------------------------------------------------
HRESULT
CForestVersion::CheckHighestPossible(void)
{
   TRACE(CForestVersion,CheckHighestPossible);
   if (!_fInitialized)
   {
      dspAssert(FALSE);
      return E_FAIL;
   }

   if (IsHighest())
   {
      // We are already at the highest version, it can't be raised any further.
      //
      _fCanRaiseBehaviorVersion = false;
      return S_OK;
   }

   dspAssert(!_DcLogList.size());
   HRESULT hr = S_OK;
   CWaitCursor Wait;
   CPathCracker PathCrack;

   //
   // Search for the nTDSDSA objects. See if any DCs are running a downlevel
   // version of the OS.
   //
   hr = EnumDsaObjs(_strConfigPath, NULL, NULL, DOMAIN_VER_XP_NATIVE);

   CHECK_HRESULT(hr, return hr);

   // Don't check domain version unless all DCs are running .Net. This is
   // because the domain version check looks for the ntMixedDomain attribute
   // on the crossRef. There won't be one there until all forest DCs are
   // running .Net.
   //
   if (CanRaise())
   {
      //
      // Check the domain versions.
      //
      hr = CheckDomainVersions(_strPartitionsPath);

      CHECK_HRESULT(hr, return hr);
   }

   return S_OK;
}

WCHAR wzNcName[] = L"nCName";
WCHAR wzFlags[] = L"systemFlags";
WCHAR wzDnsRoot[] = L"dnsRoot";

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersion::CheckDomainVersions
//
//  Synopsis:  Enumerate the domains by enumerating the cross-refs and check
//             the version and mode on each. Build a list of mixed mode domains.
//
//-----------------------------------------------------------------------------
HRESULT
CForestVersion::CheckDomainVersions(PCWSTR pwzPartitionsPath)
{
   dspDebugOut((DEB_ITRACE, "Searching for crossRef objects under %ws\n",
                pwzPartitionsPath));
   CDSSearch Search;

   HRESULT hr = Search.Init(pwzPartitionsPath);

   CHECK_HRESULT(hr, return hr);
   PWSTR rgwzAttrs[] = {wzNcName, wzFlags, g_wzBehaviorVersion, g_wzDomainMode,
                        wzDnsRoot};

   Search.SetAttributeList(rgwzAttrs, ARRAYLENGTH(rgwzAttrs));

   hr = Search.SetSearchScope(ADS_SCOPE_SUBTREE);

   CHECK_HRESULT(hr, return hr);

   Search.SetFilterString(L"(objectCategory=crossRef)");

   hr = Search.DoQuery();

   CHECK_HRESULT(hr, return hr);
   CPathCracker PathCrack;

   while (SUCCEEDED(hr))
   {
      hr = Search.GetNextRow();

      if (hr == S_ADS_NOMORE_ROWS)
      {
         hr = S_OK;
         break;
      }

      CHECK_HRESULT(hr, return hr);
      ADS_SEARCH_COLUMN Column = {0};

      hr = Search.GetColumn(wzFlags, &Column);

      if (E_ADS_COLUMN_NOT_SET == hr)
      {
         continue;
      }
      CHECK_HRESULT(hr, return hr);

      DWORD dwFlags = Column.pADsValues->Integer;

      Search.FreeColumn(&Column);

      if (!(FLAG_CR_NTDS_DOMAIN & dwFlags))
      {
         // Only domain cross-refs have the FLAG_CR_NTDS_DOMAIN bit set.
         //
         continue;
      }

      CStrW strNcName;

      hr = Search.GetColumn(wzNcName, &Column);

      CHECK_HRESULT(hr, return hr);

      strNcName = Column.pADsValues->CaseIgnoreString;

      Search.FreeColumn(&Column);

      hr = PathCrack.Set(strNcName, ADS_SETTYPE_DN);

      CHECK_HRESULT(hr, return hr);

      hr = PathCrack.Set(_strDC, ADS_SETTYPE_SERVER);

      CHECK_HRESULT(hr, return hr);
      CComBSTR bstrDomain;

      hr = PathCrack.Retrieve(ADS_FORMAT_X500, &bstrDomain);

      CHECK_HRESULT(hr, return hr);

      bool fMixed = true;

      hr = Search.GetColumn(g_wzDomainMode, &Column);

      if (E_ADS_PROPERTY_NOT_FOUND == hr ||
          E_ADS_COLUMN_NOT_SET == hr)
      {
         // E_ADS_PROPERTY_NOT_FOUND: W2K forest won't have the mixed-mode
         // attribute on the cross-ref.
         // E_ADS_COLUMN_NOT_SET: Whistler domain still in mixed mode.
         // fMixed initialized to true above.
         //
         hr = S_OK;
      }
      else
      {
         CHECK_HRESULT(hr, return hr);

         fMixed = Column.pADsValues->Integer != 0;

         Search.FreeColumn(&Column);
      }

      DWORD dwVersion = 0;

      hr = Search.GetColumn(g_wzBehaviorVersion, &Column);

      if (E_ADS_PROPERTY_NOT_FOUND == hr ||
          E_ADS_COLUMN_NOT_SET == hr)
      {
         // E_ADS_PROPERTY_NOT_FOUND: W2K forest won't have the behavior version
         // attribute.
         // E_ADS_COLUMN_NOT_SET: Whistler domain still has BV = 0.
         // dwVersion initialized to zero above.
         //
         hr = S_OK;
      }
      else
      {
         CHECK_HRESULT(hr, return hr);

         dwVersion = Column.pADsValues->Integer;

         Search.FreeColumn(&Column);
      }

      if (dwVersion < DOMAIN_VER_XP_NATIVE && fMixed)
      {
         // The domain is still in mixed mode, can't do a forest behavior
         // version upgrade.
         //
         hr = Search.GetColumn(wzDnsRoot, &Column);

         CHECK_HRESULT(hr, return hr);
         CStrW strDnsDomainName;

         strDnsDomainName = Column.pADsValues->CaseIgnoreString;

         Search.FreeColumn(&Column);

         CDomainListItem * pDomainItem = new CDomainListItem(strDnsDomainName,
                                                             dwVersion,
                                                             true);
         CHECK_NULL(pDomainItem, return E_OUTOFMEMORY);

         _DomainLogList.push_back(pDomainItem);

         _fCanRaiseBehaviorVersion = false;
      }
   }

   return S_OK;
}

//+----------------------------------------------------------------------------
//
//  Method:    CForestVersion::BuildMixedModeList
//
//  Synopsis:  Build the display string for the log file.
//
//  Notes:     Using the domain object to get domain version strings. The
//             domain object does not need to be initialized in this case.
//
//-----------------------------------------------------------------------------
HRESULT
CForestVersion::BuildMixedModeList(CStrW & strList)
{
   CDomainVersion DomVer;

   for (DOMAIN_LIST::iterator i = _DomainLogList.begin();
        i != _DomainLogList.end();
        ++i)
   {
      strList += (*i)->GetDnsDomainName();

      strList += L"\t";

      CStrW strVer;

      DomVer.GetString((*i)->GetVer(), (*i)->GetMode(), strVer);

      strList += strVer;

      strList += g_wzCRLF;
   }
   return S_OK;
}

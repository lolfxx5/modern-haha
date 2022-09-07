/////////////////////////////////////////////////////////////////////////////////
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 2000-2001.
//
//  File:       TemplateV2RequestPropertyPage.h
//
//  Contents:   Definition of CTemplateV2RequestPropertyPage
//
//----------------------------------------------------------------------------
#if !defined(AFX_TEMPLATEV2REQUESTPROPERTYPAGE_H__A3E4D067_D3C3_4C85_A331_97D940A82063__INCLUDED_)
#define AFX_TEMPLATEV2REQUESTPROPERTYPAGE_H__A3E4D067_D3C3_4C85_A331_97D940A82063__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TemplateV2RequestPropertyPage.h : header file
//
#include "CertTemplate.h"
#include "TemplateV1RequestPropertyPage.h"
#include "SelectCSPDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CTemplateV2RequestPropertyPage dialog

class CTemplateV2RequestPropertyPage : public CHelpPropertyPage
{
// Construction
public:
    CTemplateV2RequestPropertyPage(CCertTemplate& rCertTemplate, bool& rbIsDirty);
    virtual ~CTemplateV2RequestPropertyPage();

// Dialog Data
    //{{AFX_DATA(CTemplateV2RequestPropertyPage)
    enum { IDD = IDD_TEMPLATE_V2_REQUEST };
    CComboBox   m_minKeySizeCombo;
    CComboBox   m_purposeCombo;
    //}}AFX_DATA


// Overrides
    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CTemplateV2RequestPropertyPage)
    public:
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    int GetSelectedCSPCount ();
    virtual void DoContextHelp (HWND hWndControl);
    void AddKeySizeToCombo (DWORD dwValue, PCWSTR strValue, DWORD dwSizeToSelect);
    HRESULT EnumerateCSPs(DWORD dwMinKeySize);
    // Generated message map functions
    //{{AFX_MSG(CTemplateV2RequestPropertyPage)
    afx_msg void OnSelchangePurposeCombo();
    afx_msg void OnExportPrivateKey();
    afx_msg void OnArchiveKeyCheck();
    afx_msg void OnIncludeSymmetricAlgorithmsCheck();
    afx_msg void OnSelchangeMinimumKeysizeValue();
    afx_msg void OnDeletePermanently();
    afx_msg void OnDestroy();
	afx_msg void OnCsps();
	afx_msg void OnEnrollWithoutInput();
	afx_msg void OnEnrollPromptUser();
	afx_msg void OnEnrollPromptUserRequireIfPrivateKey();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();
    virtual void EnableControls ();
    HRESULT CSPGetMaxKeySupported (
                PCWSTR pszProvider, 
                DWORD dwProvType, 
                DWORD& dwSigMaxKey, 
                DWORD& dwKeyExMaxKey);
    void NormalizeCSPList (DWORD dwMinKeySize);

private:
    bool&           m_rbIsDirty;
    CCertTemplate&  m_rCertTemplate;
    int             m_nProvDSSCnt;

    CSP_LIST        m_CSPList;
};




//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEMPLATEV2REQUESTPROPERTYPAGE_H__A3E4D067_D3C3_4C85_A331_97D940A82063__INCLUDED_)

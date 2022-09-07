//---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1999.
//
//  File:       helpmap.h
//
//  Contents:   Help Identifiers mapped with control IDs for CertTmpl.DLL
//
//----------------------------------------------------------------------------
#ifndef __CERTTMPL_HELPMAP_H
#define __CERTTMPL_HELPMAP_H
#include "resource.h"

const DWORD g_aHelpIDs_IDD_NEW_APPLICATION_OID[]=
{
	IDC_NEW_APPLICATION_OID_NAME,IDH_NEW_APPLICATION_OID_NAME,
	IDC_NEW_APPLICATION_OID_VALUE,IDH_NEW_APPLICATION_OID_VALUE,
	0,0
};


const DWORD g_aHelpIDs_IDD_NEW_ISSUANCE_OID[]=
{
	IDC_NEW_ISSUANCE_OID_NAME,IDH_NEW_ISSUANCE_OID_NAME,
	IDC_NEW_ISSUANCE_OID_VALUE,IDH_NEW_ISSUANCE_OID_VALUE,
    IDC_CPS_EDIT, IDH_CPS_EDIT, 
	0,0
};

const DWORD g_aHelpIDs_IDD_ADD_APPROVAL[]=
{
    IDC_APPROVAL_LIST, IDH_APPROVAL_LIST,
    0, 0
};

const DWORD g_aHelpIDs_IDD_BASIC_CONSTRAINTS[]=
{
    IDC_BASIC_CONSTRAINTS_CRITICAL, IDH_BASIC_CONSTRAINTS_CRITICAL,
    IDC_ONLY_ISSUE_END_ENTITIES, IDH_ONLY_ISSUE_END_ENTITIES,
    0, 0
};

const DWORD g_aHelpIDs_IDD_KEY_USAGE[]=
{
    IDC_KEY_USAGE_CRITICAL, IDH_KEY_USAGE_CRITICAL,
    IDC_CHECK_DIGITAL_SIGNATURE, IDH_CHECK_DIGITAL_SIGNATURE,
    IDC_CHECK_NON_REPUDIATION, IDH_CHECK_NON_REPUDIATION,
    IDC_CHECK_CERT_SIGNING, IDH_CHECK_CERT_SIGNING,
    IDC_CRL_SIGNING, IDH_CRL_SIGNING,
    IDC_CHECK_KEY_AGREEMENT, IDH_CHECK_KEY_AGREEMENT,
    IDC_CHECK_KEY_ENCIPHERMENT, IDH_CHECK_KEY_ENCIPHERMENT,
    IDC_CHECK_DATA_ENCIPHERMENT, IDH_CHECK_DATA_ENCIPHERMENT,
    0, 0
};


const DWORD g_aHelpIDs_IDD_POLICY[]=
{
    IDC_POLICY_CRITICAL, IDH_POLICY_CRITICAL,
    IDC_POLICIES_LIST, IDH_POLICIES_LIST,
    IDC_ADD_POLICY, IDH_ADD_POLICY,
    IDC_REMOVE_POLICY, IDH_REMOVE_POLICY,
    IDC_EDIT_POLICY, IDH_EDIT_POLICY,
    0, 0
};


const DWORD g_aHelpIDs_IDD_SELECT_OIDS[]=
{
    IDC_OID_LIST, IDH_OID_LIST,
    IDC_NEW_OID, IDH_NEW_OID,
    0, 0
};


const DWORD g_aHelpIDs_IDD_SELECT_TEMPLATE[]=
{
    IDC_TEMPLATE_LIST, IDH_TEMPLATE_LIST,
    IDC_TEMPLATE_PROPERTIES, IDH_TEMPLATE_PROPERTIES,
    0, 0
};


const DWORD g_aHelpIDs_IDD_TEMPLATE_EXTENSIONS[]=
{
    IDC_EXTENSION_LIST, IDH_EXTENSION_LIST,
    IDC_SHOW_DETAILS, IDH_SHOW_DETAILS,
    IDC_EXTENSION_DESCRIPTION, IDH_EXTENSION_DESCRIPTION,
    0, 0
};


const DWORD g_aHelpIDs_IDD_TEMPLATE_GENERAL[]=
{
    IDC_DISPLAY_NAME, IDH_DISPLAY_NAME,
    IDC_TEMPLATE_NAME, IDH_TEMPLATE_NAME,
    IDC_VALIDITY_EDIT, IDH_VALIDITY_EDIT,
    IDC_VALIDITY_UNITS, IDH_VALIDITY_UNITS, 
    IDC_RENEWAL_EDIT, IDH_RENEWAL_EDIT,
    IDC_RENEWAL_UNITS, IDH_RENEWAL_UNITS, 
    IDC_PUBLISH_TO_AD, IDH_PUBLISH_TO_AD,
    IDC_TEMPLATE_VERSION, IDH_TEMPLATE_VERSION,
    IDC_USE_AD_CERT_FOR_REENROLLMENT, IDH_USE_AD_CERT_FOR_REENROLLMENT,
    0, 0
};


const DWORD g_aHelpIDs_IDD_TEMPLATE_V1_REQUEST[]=
{
    IDC_PURPOSE_COMBO, IDH_V1_PURPOSE_COMBO,
    IDC_EXPORT_PRIVATE_KEY, IDH_V1_EXPORT_PRIVATE_KEY,
    IDC_CSP_LIST, IDH_V1_CSP_LIST, 
    0, 0
};

const DWORD g_aHelpIDs_IDD_TEMPLATE_V2_REQUEST[]=
{
    IDC_PURPOSE_COMBO, IDH_V2_PURPOSE_COMBO,
    IDC_ARCHIVE_KEY_CHECK, IDH_V2_ARCHIVE_KEY_CHECK,
    IDC_INCLUDE_SYMMETRIC_ALGORITHMS_CHECK, IDH_V2_INCLUDE_SYMMETRIC_ALGORITHMS_CHECK,
    IDC_MINIMUM_KEYSIZE_VALUE, IDH_V2_MINIMUM_KEYSIZE_VALUE, 
    IDC_EXPORT_PRIVATE_KEY, IDH_V2_EXPORT_PRIVATE_KEY,
	IDC_DELETE_PERMANENTLY, IDH_V2_DELETE_PERMANENTLY,
    IDC_ENROLL_WITHOUT_INPUT, IDH_V2_ENROLL_WITHOUT_INPUT,
    IDC_ENROLL_PROMPT_USER, IDH_V2_ENROLL_PROMPT_USER,
    IDC_ENROLL_PROMPT_USER_REQUIRE_IF_PRIVATE_KEY, IDH_V2_ENROLL_PROMPT_USER_REQUIRE_IF_PRIVATE_KEY ,
    IDC_CSPS, IDH_V2_CSPS,
    0, 0
};

const DWORD g_aHelpIDs_IDD_CSP_SELECTION[]=
{
    IDC_USE_ANY_CSP, IDH_USE_ANY_CSP,
    IDC_USE_SELECTED_CSPS, IDH_USE_SELECTED_CSPS,
    IDC_CSP_LIST, IDH_V2_CSP_LIST, 
    0, 0
};

const DWORD g_aHelpIDs_IDD_TEMPLATE_V1_SUBJECT_NAME[]=
{
    IDC_REQUIRE_SUBJECT, IDH_REQUIRE_SUBJECT,
    IDC_SUBJECT_AND_BUILD_SUBJECT_BY_CA, IDH_SUBJECT_AND_BUILD_SUBJECT_BY_CA,
    IDC_EMAIL_NAME, IDH_EMAIL_NAME, 
    IDC_SUBJECT_MUST_BE_MACHINE, IDH_SUBJECT_MUST_BE_MACHINE,
    IDC_SUBJECT_MUST_BE_USER, IDH_SUBJECT_MUST_BE_USER, 
    0, 0
};


const DWORD g_aHelpIDs_IDD_TEMPLATE_V2_AUTHENTICATION[]=
{
    IDC_ISSUANCE_POLICIES, IDH_ISSUANCE_POLICIES,
    IDC_ADD_APPROVAL, IDH_ADD_APPROVAL,
    IDC_REMOVE_APPROVAL, IDH_REMOVE_APPROVAL,
    IDC_NUM_SIG_REQUIRED_EDIT, IDH_NUM_SIG_REQUIRED_EDIT, 
    IDC_REENROLLMENT_REQUIRES_VALID_CERT, IDH_REENROLLMENT_REQUIRES_VALID_CERT,
    IDC_REENROLLMENT_SAME_AS_ENROLLMENT, IDH_REENROLLMENT_SAME_AS_ENROLLMENT,
    IDC_NUM_SIG_REQUIRED_CHECK, IDH_NUM_SIG_REQUIRED_CHECK,
    IDC_PEND_ALL_REQUESTS, IDH_PEND_ALL_REQUESTS,
    IDC_POLICY_TYPES, IDH_POLICY_TYPES,
    IDC_APPLICATION_POLICIES, IDH_APPLICATION_POLICIES,
    0, 0
};


const DWORD g_aHelpIDs_IDD_TEMPLATE_V2_SUBJECT_NAME[]=
{
    IDC_SUBJECT_NAME_SUPPLIED_IN_REQUEST, IDH_SUBJECT_NAME_SUPPLIED_IN_REQUEST,
    IDC_SUBJECT_NAME_BUILT_BY_CA, IDH_SUBJECT_NAME_BUILT_BY_CA,
    IDC_SUBJECT_NAME_NAME_LABEL, IDH_SUBJECT_NAME_NAME_LABEL,
    IDC_SUBJECT_NAME_NAME_COMBO, IDH_SUBJECT_NAME_NAME_COMBO, 
    IDC_EMAIL_IN_SUB, IDH_EMAIL_IN_SUB,
    IDC_EMAIL_IN_ALT, IDH_EMAIL_IN_ALT,
    IDC_DNS_NAME, IDH_DNS_NAME,
    IDC_UPN, IDH_UPN,
    IDC_SPN, IDH_SPN,
    0, 0
};


const DWORD g_aHelpIDs_IDD_TEMPLATE_V2_SUPERCEDES[]=
{
    IDC_SUPERCEDED_TEMPLATES_LIST, IDH_SUPERCEDED_TEMPLATES_LIST,
    IDC_ADD_SUPERCEDED_TEMPLATE, IDH_ADD_SUPERCEDED_TEMPLATE,
    IDC_REMOVE_SUPERCEDED_TEMPLATE, IDH_REMOVE_SUPERCEDED_TEMPLATE,
    0, 0
};

const DWORD g_aHelpIDs_IDD_VIEW_OIDS[]=
{
    IDC_OID_LIST, IDH_VIEW_OIDS_OID_LIST,
    IDC_COPY_OID, IDH_COPY_OID,
    0, 0
};

#endif // __CERTTMPL_HELPMAP_H
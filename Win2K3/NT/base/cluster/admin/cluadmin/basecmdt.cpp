/////////////////////////////////////////////////////////////////////////////
//
//	Copyright (c) 1996 Microsoft Corporation
//
//	Module Name:
//		BaseCmdT.cpp
//
//	Abstract:
//		Implementation of the CBaseCmdTarget class.
//
//	Author:
//		David Potter (davidp)	December 11, 1996
//
//	Revision History:
//
//	Notes:
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseCmdT.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CBaseCmdTarget
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CBaseCmdTarget, CCmdTarget)

/////////////////////////////////////////////////////////////////////////////
// Message Maps
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CBaseCmdTarget, CCmdTarget)
	//{{AFX_MSG_MAP(CBaseCmdTarget)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////

//

// displaycfg.cpp -- Implementation of MO Provider for Win32DisplayConfiguration

//

// Copyright (c) 1996-2001 Microsoft Corporation, All Rights Reserved 
//
//  10/05/96     jennymc     Initial Code
//  10/24/97     jennymc     Moved to new framework
//
///////////////////////////////////////////////////////////////////////
#include "precomp.h"
#include <cregcls.h>

#include <winuser.h>
#include "DisplayCfg.h"

// Property set declaration
//=========================

CWin32DisplayConfiguration MyCWin32DisplayConfigurationSet ( PROPSET_NAME_DISPLAY , IDS_CimWin32Namespace ) ;

/*****************************************************************************
 *
 *  FUNCTION    : CWin32DisplayConfiguration::CWin32DisplayConfiguration
 *
 *  DESCRIPTION : Constructor
 *
 *  INPUTS      : none
 *
 *  OUTPUTS     : none
 *
 *  RETURNS     : nothing
 *
 *  COMMENTS    : Registers property set with framework
 *
 *****************************************************************************/

CWin32DisplayConfiguration :: CWin32DisplayConfiguration (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
}

/*****************************************************************************
 *
 *  FUNCTION    : CWin32DisplayConfiguration::~CWin32DisplayConfiguration
 *
 *  DESCRIPTION : Destructor
 *
 *  INPUTS      : none
 *
 *  OUTPUTS     : none
 *
 *  RETURNS     : nothing
 *
 *  COMMENTS    : Deregisters property set from framework
 *
 *****************************************************************************/

CWin32DisplayConfiguration :: ~CWin32DisplayConfiguration ()
{
}

/*****************************************************************************
 *
 *  FUNCTION    : GetObject
 *
 *  DESCRIPTION : Assigns values to property set according to key value
 *                already set by framework
 *
 *  INPUTS      : none
 *
 *  OUTPUTS     : none
 *
 *  RETURNS     :
 *
 *  COMMENTS    :
 *
 *****************************************************************************/

HRESULT CWin32DisplayConfiguration :: GetObject (

	CInstance *pInstance,
	long lFlags /*= 0L*/
)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

	CHString littleOldKey ;
	if ( pInstance->GetCHString ( IDS_DeviceName, littleOldKey ) )
	{
		hr = GetDisplayInfo ( pInstance , TRUE ) ;
	}
	else
	{
		hr = WBEM_E_INVALID_PARAMETER ;
	}

	if ( SUCCEEDED ( hr ) )
	{
		CHString littleNewKey ;
		pInstance->GetCHString ( IDS_DeviceName, littleNewKey ) ;

		if ( littleNewKey.CompareNoCase ( littleOldKey ) != 0 )
		{
			hr = WBEM_E_NOT_FOUND ;
		}
	}

	return hr ;
}

/*****************************************************************************
 *
 *  FUNCTION    : EnumerateInstances
 *
 *  DESCRIPTION : Creates instance of property set for each
 *
 *  INPUTS      : none
 *
 *  OUTPUTS     : none
 *
 *  RETURNS     :
 *
 *  COMMENTS    :
 *
 *****************************************************************************/

HRESULT CWin32DisplayConfiguration :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags /*= 0L*/
)
{
	HRESULT hr = WBEM_E_FAILED;
    CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
    hr = GetDisplayInfo ( pInstance , TRUE ) ;
	if ( SUCCEEDED ( hr ) )
	{
		hr = pInstance->Commit ( ) ;
	}

    return hr;
}

/*****************************************************************************
 *
 *  FUNCTION    : EnumerateInstances
 *
 *  DESCRIPTION : Creates instance of property set for each
 *
 *  INPUTS      : none
 *
 *  OUTPUTS     : none
 *
 *  RETURNS     :
 *
 *  COMMENTS    :
 *
 *****************************************************************************/

HRESULT CWin32DisplayConfiguration :: GetDisplayInfo (

	CInstance *pInstance,
	BOOL fAssignKey
)
{
	CHString strName ;
	CHString strDesc ;

	CConfigManager      configMngr;
	CDeviceCollection   devCollection;

	if ( configMngr.GetDeviceListFilterByClass ( devCollection , L"Display" ) )
	{
		REFPTR_POSITION pos ;

		devCollection.BeginEnum ( pos ) ;

		if ( devCollection.GetSize () )
		{
			CConfigMgrDevicePtr pDevice(devCollection.GetNext ( pos ), false) ;
			if ( pDevice )
			{
				pDevice->GetDeviceDesc ( strDesc ) ;

				if ( strName.IsEmpty () )
				{
					strName = strDesc ;
				}

#ifdef NTONLY
				CHString strService ;
				CHString strFileName ;
				CHString strVersion ;

				// If under WinNT, get the version by getting the service
				// name and getting its version information.

				BOOL t_Status = pDevice->GetService ( strService ) &&
								GetServiceFileName ( strService , strFileName ) &&
								GetVersionFromFileName ( strFileName , strVersion ) ;

				if ( t_Status )
				{
					pInstance->SetCHString ( IDS_DriverVersion , strVersion ) ;
				}
#endif

			}
		}
	}

	if ( fAssignKey )
	{
		pInstance->SetCHString ( IDS_DeviceName , strName ) ;
	}

	pInstance->SetCHString ( IDS_Caption , strName ) ;

	pInstance->SetCHString ( L"SettingID" , strName ) ;

	pInstance->SetCHString ( IDS_Description , strDesc.IsEmpty () ? strName : strDesc ) ;

	//===============================================
	//  Get the info
	//===============================================

	DWORD dMode = ENUM_REGISTRY_SETTINGS ;

	HDC hdc = GetDC ( GetDesktopWindow () ) ;
	if (hdc)
	{
		try
		{

			DWORD dwTemp = (DWORD) GetDeviceCaps ( hdc , BITSPIXEL ) ;
			pInstance->SetDWORD ( IDS_BitsPerPel , dwTemp ) ;

			dwTemp = ( DWORD ) GetDeviceCaps ( hdc , HORZRES ) ;
			pInstance->SetDWORD( IDS_PelsWidth, dwTemp );

			dwTemp = ( DWORD ) GetDeviceCaps ( hdc , VERTRES ) ;
			pInstance->SetDWORD ( IDS_PelsHeight , dwTemp );

#ifdef NTONLY
			pInstance->SetDWORD ( IDS_DisplayFrequency, ( DWORD ) GetDeviceCaps ( hdc , VREFRESH ) ) ;
#endif
		}
		catch ( ... )
		{
			ReleaseDC ( GetDesktopWindow () , hdc ) ;
            throw;
		}

		ReleaseDC ( GetDesktopWindow () , hdc ) ;
	}

	DEVMODE DevMode ;

	memset ( & DevMode , 0 , sizeof ( DEVMODE ) ) ;

	DevMode.dmSize = sizeof ( DEVMODE ) ;
	DevMode.dmSpecVersion = DM_SPECVERSION ;

	BOOL t_Status = EnumDisplaySettings ( NULL, ENUM_CURRENT_SETTINGS , & DevMode ) ;
	if ( t_Status )
	{
		pInstance->SetDWORD ( IDS_SpecificationVersion, (DWORD) DevMode. dmSpecVersion ) ;

		if ( DevMode.dmFields & DM_LOGPIXELS )
		{
			pInstance->SetDWORD ( IDS_LogPixels , ( DWORD ) DevMode.dmLogPixels ) ;
		}

		if ( DevMode.dmFields & DM_DISPLAYFLAGS )
		{
			pInstance->SetDWORD ( IDS_DisplayFlags, DevMode.dmDisplayFlags ) ;
		}

	}


    return WBEM_S_NO_ERROR;
}



//***************************************************************************

//

// Copyright (c) 1997-2001 Microsoft Corporation, All Rights Reserved
//
//  1394.h
//
//  Purpose: 1394 Controller property set provider
//
//***************************************************************************

// Property set identification
//============================

#define	PROPSET_NAME_1394CONTROLLER	L"Win32_1394Controller"

#define SPECIAL_PROPS_ALL_REQUIRED          0xFFFFFFFF
#define SPECIAL_PROPS_NONE_REQUIRED         0x00000000
#define SPECIAL_PROPS_STATUS				0x00000004
#define SPECIAL_PROPS_DEVICEID				0x00000008
#define SPECIAL_PROPS_CREATIONNAME			0x00000010
#define SPECIAL_PROPS_SYSTEMNAME			0x00000020
#define SPECIAL_PROPS_DESCRIPTION			0x00000040
#define SPECIAL_PROPS_CAPTION				0x00000080
#define SPECIAL_PROPS_NAME					0x00000100
#define SPECIAL_PROPS_MANUFACTURER			0x00000200
#define SPECIAL_PROPS_PROTOCOLSSUPPORTED	0x00000400
#define SPECIAL_PROPS_PNPDEVICEID			0x00400000
#define SPECIAL_PROPS_CONFIGMERRORCODE		0x00800000
#define SPECIAL_PROPS_CONFIGMUSERCONFIG		0x01000000
#define SPECIAL_PROPS_CREATIONCLASSNAME		0x02000000


#define SPECIAL_ALL					( SPECIAL_CONFIGMANAGER )

#define SPECIAL_CONFIGMANAGER		( SPECIAL_PROPS_STATUS | \
									SPECIAL_PROPS_DEVICEID | \
									SPECIAL_PROPS_CREATIONNAME | \
									SPECIAL_PROPS_SYSTEMNAME | \
									SPECIAL_PROPS_DESCRIPTION | \
									SPECIAL_PROPS_CAPTION | \
									SPECIAL_PROPS_NAME | \
									SPECIAL_PROPS_MANUFACTURER | \
									SPECIAL_PROPS_PROTOCOLSSUPPORTED | \
									SPECIAL_PROPS_PNPDEVICEID | \
									SPECIAL_PROPS_CONFIGMERRORCODE | \
									SPECIAL_PROPS_CONFIGMUSERCONFIG | \
									SPECIAL_PROPS_CREATIONCLASSNAME )

#define SPECIAL_CONFIGPROPERTIES 	( SPECIAL_PROPS_PNPDEVICEID | \
									SPECIAL_PROPS_CONFIGMERRORCODE | \
									SPECIAL_PROPS_CONFIGMUSERCONFIG | \
									SPECIAL_PROPS_STATUS )

#define SPECIAL_DESC_CAP_NAME		( SPECIAL_PROPS_DESCRIPTION | \
									SPECIAL_PROPS_CAPTION | \
									SPECIAL_PROPS_NAME )

#define SPECIAL_DESC_CAP_NAME		( SPECIAL_PROPS_DESCRIPTION | \
									SPECIAL_PROPS_CAPTION | \
									SPECIAL_PROPS_NAME )

#define SPECIAL_CAP_NAME			( SPECIAL_PROPS_CAPTION | \
									SPECIAL_PROPS_NAME )

class CWin32_1394Controller : public Provider
{
    public:

        // Constructor/destructor
        //=======================

        CWin32_1394Controller ( const CHString &a_Name , LPCWSTR a_Namespace ) ;

       ~CWin32_1394Controller() ;

        // Functions provide properties with current values
        //=================================================

        virtual HRESULT GetObject ( CInstance *a_Instance , long lFlags , CFrameworkQuery &a_Query ) ;
		HRESULT ExecQuery ( MethodContext *a_MethodContext, CFrameworkQuery &a_Query, long a_Flags ) ;
        virtual HRESULT EnumerateInstances ( MethodContext *a_MethodContext , long a_Flags = 0L ) ;

    private:

        // Utility function(s)
        //====================

		HRESULT Enumerate ( 

			MethodContext *a_MethodContext , 
			long a_Flags , 
			DWORD a_SpecifiedPropertied = SPECIAL_PROPS_ALL_REQUIRED
		) ;

        HRESULT LoadPropertyValues ( 

			CInstance *a_Instance , 
			CConfigMgrDevice *a_Device , 
			const CHString &a_DeviceName , 
			DWORD a_SpecifiedPropertied = SPECIAL_PROPS_ALL_REQUIRED 
		) ;

        BOOL Is1394Controller ( CConfigMgrDevice *a_Device ) ;

        DWORD GetBitMask ( CFrameworkQuery &a_Query );
} ;

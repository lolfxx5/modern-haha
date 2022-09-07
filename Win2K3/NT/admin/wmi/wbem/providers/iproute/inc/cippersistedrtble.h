/******************************************************************

   CIPPersistedRTble.H -- WMI provider class definition

   Generated by Microsoft WBEM Code Generation Engine
 
   Description: 
   

*******************************************************************/

// Property set identification
//============================

#ifndef _CIPPersistedRTble_H_
#define _CIPPersistedRTble_H_

#define PROVIDER_NAME_CIPPERSISTEDROUTETABLE L"Win32_IP4PersistedRouteTable"

class CIPPersistedRouteTable : public Provider
{
private:

protected:

		HRESULT ExecMethod (

			const CInstance &Instance,
			const BSTR bstrMethodName,
			CInstance *pInParams,
			CInstance *pOutParams,
			long lFlags
		);

        // Writing Functions
        //============================

		HRESULT DeleteInstance (

			const CInstance &Instance,
			long lFlags
		);

		HRESULT CIPPersistedRouteTable :: PutInstance  (

			const CInstance &Instance,
			long lFlags
		);

        // Reading Functions
        //============================

        HRESULT EnumerateInstances ( 

			MethodContext *pMethodContext, 
			long lFlags = 0L
		) ;

        HRESULT GetObject (

			CInstance *pInstance, 
			long lFlags = 0L
		) ;


        // Declare any additional functions and accessor
        // functions for private data used by this class
        //===========================================================
		HRESULT CheckParameters (

			const CInstance &a_Instance ,
			CHString &a_ValueName
		);

		BOOL CIPPersistedRouteTable :: Parse (

			LPWSTR a_InStr ,
			CHString &a_Dest ,
			CHString &a_Mask ,
			CHString &a_NextHop ,
			long &a_Metric
		);

		void SetInheritedProperties (

			LPCWSTR a_dest ,
			LPCWSTR a_gateway ,
			LPCWSTR a_mask ,
			long a_metric ,
			CInstance &a_Instance
		) ;

public:

        // Constructor/destructor
        //=======================

        CIPPersistedRouteTable (

			LPCWSTR lpwszClassName, 
			LPCWSTR lpwszNameSpace
		) ;

        virtual ~CIPPersistedRouteTable () ;
} ;

#endif

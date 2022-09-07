// ServiceSpecificationService.cpp: implementation of the CServiceSpecificationService class.

//

// Copyright (c) 1997-2001 Microsoft Corporation, All Rights Reserved
//
//////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include "ServiceSpecificationService.h"

#include "ExtendString.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServiceSpecificationService::CServiceSpecificationService(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CServiceSpecificationService::~CServiceSpecificationService()
{

}

HRESULT CServiceSpecificationService::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcQuery[BUFF_SIZE];
    WCHAR wcProductCode[39];
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;

    //These will change from class to class
    bool bService, bCheck;
    IWbemClassObject *pObj = NULL;
    VARIANT v;

    VariantInit(&v);

	CStringExt wcKey;

	// safe operation
	// lenght is smaller than BUFF_SIZE ( 512 )
    wcscpy(wcQuery, L"select distinct `ServiceInstall`, `Component_`, `Name` from ServiceInstall");

	LPWSTR Buffer = NULL;
	LPWSTR dynBuffer = NULL;

	DWORD dwDynBuffer = 0L;

    while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
	{
		// safe operation:
		// Package ( i ) returns NULL ( tested above ) or valid WCHAR [39]

        wcscpy(wcProductCode, m_pRequest->Package(i));

		//Open our database
        try
		{
            if ( GetView ( &hView, wcProductCode, wcQuery, L"ServiceInstall", TRUE, FALSE ) )
			{
                uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                    CheckMSI(uiStatus);

                    if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                    //----------------------------------------------------

					// safe operation

                    wcKey.Copy ( L"Win32_ServiceSpecification.CheckID=\"" );

                    dwBufSize = BUFF_SIZE;
					GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

					wcKey.Append ( 3, Buffer, wcProductCode, L"\"" );
					PutKeyProperty(m_pObj, pCheck, wcKey, &bCheck, m_pRequest);

					if ( dynBuffer && dynBuffer [ 0 ] != 0 )
					{
						dynBuffer [ 0 ] = 0;
					}

                //====================================================

                    dwBufSize = BUFF_SIZE;
					GetBufferToPut ( hRecord, 2, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                    if ( ValidateComponentName ( msidata.GetDatabase (), wcProductCode, Buffer ) )
					{
						if ( dynBuffer && dynBuffer [ 0 ] != 0 )
						{
							dynBuffer [ 0 ] = 0;
						}

                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 3, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                        wcKey.Copy ( L"Win32_Service.Name=\"" );
                        wcKey.Append ( 2, Buffer, L"\"" );

						if ( dynBuffer && dynBuffer [ 0 ] != 0 )
						{
							dynBuffer [ 0 ] = 0;
						}

						BSTR bstrObj;
						if ( ( bstrObj = ::SysAllocString ( wcKey ) ) == NULL )
						{
							throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
						}

                        if SUCCEEDED ( hr = m_pNamespace->GetObject ( bstrObj, 0, m_pCtx, &pObj, NULL ) )
						{
                            PutKeyProperty ( m_pObj, pElement, wcKey, &bService, m_pRequest );
                            pObj->Release();
							pObj = NULL;

                        //----------------------------------------------------

                            if(bService && bCheck) bMatch = true;

                            if((atAction != ACTIONTYPE_GET)  || bMatch){

                                hr = pHandler->Indicate(1, &m_pObj);
                            }
                        }

						::SysFreeString ( bstrObj );
                    }
					else
					{
						if ( dynBuffer && dynBuffer [ 0 ] != 0 )
						{
							dynBuffer [ 0 ] = 0;
						}
					}
                    
                    m_pObj->Release();
                    m_pObj = NULL;

                    g_fpMsiCloseHandle(hRecord);

                    uiStatus = g_fpMsiViewFetch(hView, &hRecord);
                }
            }
        }
		catch(...)
		{
			if ( dynBuffer )
			{
				delete [] dynBuffer;
				dynBuffer = NULL;
			}

            g_fpMsiCloseHandle(hRecord);
            g_fpMsiViewClose(hView);
            g_fpMsiCloseHandle(hView);

			msidata.CloseDatabase ();

			if(m_pObj)
			{
				m_pObj->Release();
				m_pObj = NULL;
			}

            throw;
        }

        g_fpMsiCloseHandle(hRecord);
        g_fpMsiViewClose(hView);
        g_fpMsiCloseHandle(hView);

		msidata.CloseDatabase ();
    }

    if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}
	
	return hr;
}
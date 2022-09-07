// ServiceSpecification.cpp: implementation of the CServiceSpecification class.

//

// Copyright (c) 1997-2001 Microsoft Corporation, All Rights Reserved
//
//////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include "ServiceSpecification.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServiceSpecification::CServiceSpecification(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CServiceSpecification::~CServiceSpecification()
{

}

HRESULT CServiceSpecification::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcProductCode[39];
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;
    bool bGotID = false;
    WCHAR wcServiceInstall[BUFF_SIZE];
    WCHAR wcTestCode[39];

    //These will change from class to class
    bool bDiskID;
    
    SetSinglePropertyPath(L"CheckID");

    //improve getobject performance by optimizing the query
    if(atAction != ACTIONTYPE_ENUM)
	{
		// we are doing GetObject so we need to be reinitialized
		hr = WBEM_E_NOT_FOUND;

		BSTR bstrCompare;

        int iPos = -1;
        bstrCompare = SysAllocString ( L"CheckID" );

		if ( bstrCompare )
		{
			if(FindIn(m_pRequest->m_Property, bstrCompare, &iPos))
			{
				if ( ::SysStringLen ( m_pRequest->m_Value[iPos] ) < BUFF_SIZE )
				{
		            //Get the action we're looking for
					wcscpy(wcBuf, m_pRequest->m_Value[iPos]);

					// safe operation if wcslen ( wcBuf ) > 38
					if ( wcslen ( wcBuf ) > 38 )
					{
						wcscpy(wcTestCode, &(wcBuf[(wcslen(wcBuf) - 38)]));
					}
					else
					{
						// we are not good to go, they have sent us longer string
						SysFreeString ( bstrCompare );
						throw hr;
					}

					// safe because lenght has been tested already in condition
					RemoveFinalGUID(m_pRequest->m_Value[iPos], wcServiceInstall);

					bGotID = true;
				}
				else
				{
					// we are not good to go, they have sent us longer string
					SysFreeString ( bstrCompare );
					throw hr;
				}

			}

			SysFreeString ( bstrCompare );
		}
		else
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
    }

    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `ServiceInstall`, `Component_`, `Name`, `DisplayName`, `ServiceType`, `StartType`, `ErrorControl`, `LoadOrderGroup`, `Dependencies`, `StartName`, `Password` from ServiceInstall" );

    //optimize for GetObject
    if ( bGotID )
	{
		wcQuery.Append ( 3, L" where `ServiceInstall`=\'", wcServiceInstall, L"\'" );
	}

	LPWSTR Buffer = NULL;
	LPWSTR dynBuffer = NULL;

	DWORD dwDynBuffer = 0L;

    while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
	{
		// safe operation:
		// Package ( i ) returns NULL ( tested above ) or valid WCHAR [39]

        wcscpy(wcProductCode, m_pRequest->Package(i));

        if((atAction == ACTIONTYPE_ENUM) || (bGotID && (_wcsicmp(wcTestCode, wcProductCode) == 0))){

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
                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                        PutProperty(m_pObj, pID, Buffer);
						PutKeyProperty ( m_pObj, pCheckID, Buffer, &bDiskID, m_pRequest, 1, wcProductCode );

						if ( dynBuffer && dynBuffer [ 0 ] != 0 )
						{
							dynBuffer [ 0 ] = 0;
						}

                    //====================================================

                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 2, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                        if ( ValidateComponentName ( msidata.GetDatabase(), wcProductCode, Buffer ) )
						{
							if ( dynBuffer && dynBuffer [ 0 ] != 0 )
							{
								dynBuffer [ 0 ] = 0;
							}

                            dwBufSize = BUFF_SIZE;
							PutPropertySpecial ( hRecord, 3, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pName );

                            dwBufSize = BUFF_SIZE;
							PutPropertySpecial ( hRecord, 4, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, FALSE, 3, pDisplayName, pCaption, pDescription );

                            PutProperty(m_pObj, pServiceType, g_fpMsiRecordGetInteger(hRecord, 5));

                            PutProperty(m_pObj, pStartType, g_fpMsiRecordGetInteger(hRecord, 6));
                            
                            PutProperty(m_pObj, pErrorControl, g_fpMsiRecordGetInteger(hRecord, 7));
                            
                            dwBufSize = BUFF_SIZE;
							PutPropertySpecial ( hRecord, 8, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pLoadOrderGroup );

                            dwBufSize = BUFF_SIZE;
							PutPropertySpecial ( hRecord, 9, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pDependencies );

                            dwBufSize = BUFF_SIZE;
							PutPropertySpecial ( hRecord, 10, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pStartName );

                            dwBufSize = BUFF_SIZE;
							PutPropertySpecial ( hRecord, 11, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pPassword );

                        //----------------------------------------------------

                            if(bDiskID) bMatch = true;

                            if((atAction != ACTIONTYPE_GET)  || bMatch){

                                hr = pHandler->Indicate(1, &m_pObj);
                            }
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
    }

    if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}
	
	return hr;
}
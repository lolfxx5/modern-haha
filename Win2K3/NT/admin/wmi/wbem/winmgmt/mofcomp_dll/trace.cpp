/*++

Copyright (C) 1997-2001 Microsoft Corporation

Module Name:

    TRACE.CPP

Abstract:

    Support of trace output and internationalized strings.

History:

    a-davj  13-July-97   Created.

--*/

#include "precomp.h"
#include <wbemcli.h>
#include "trace.h"
#include <autoptr.h>

extern HINSTANCE ghModule; 


TCHAR JustInCase = 0;

BSTR GetFromStdErrorFacility(HRESULT hres)
{

	HRESULT hTemp = hres;
	
	// Certain strings are obtained from the standard facility rather
	// than the local string table.

	if(hres == WBEM_E_NOT_FOUND || hres == WBEM_E_TYPE_MISMATCH || hres == WBEM_E_OVERRIDE_NOT_ALLOWED ||
		hres == WBEM_E_PROPAGATED_QUALIFIER || hres == WBEM_E_VALUE_OUT_OF_RANGE)
		return NULL;

	// we are only interested in 0x8004xxxx values.

	hTemp &= 0xffff0000;
	if(hTemp != 0x80040000)
		return NULL;

	// attempt to read the string from the usual place
	IWbemStatusCodeText * pStatus = NULL;
	SCODE sc = CoCreateInstance(CLSID_WbemStatusCodeText, 0, CLSCTX_INPROC_SERVER,
										IID_IWbemStatusCodeText, (LPVOID *) &pStatus);

	if(FAILED(sc))
		return NULL;
	BSTR bstrError = 0;
	sc = pStatus->GetErrorCodeText(hres, 0, 0, &bstrError);
	pStatus->Release();
	if(sc == S_OK)
		return bstrError;
	else
		return NULL;
}

IntString::IntString(DWORD dwID)
{
    DWORD dwSize, dwRet, dwLen = 0;

    // see if the message can be obtained for the standard place
    
    BSTR bstrErrMsg = GetFromStdErrorFacility((HRESULT)dwID);
    if(bstrErrMsg)
    {
        dwLen = lstrlen(bstrErrMsg)+1;
        m_pString = new TCHAR[dwLen];
        if(m_pString == NULL)
        {
            SysFreeString(bstrErrMsg);
            m_pString = &JustInCase;     // should never happen!
            return; 
        }
        StringCchCopyW(m_pString, dwLen, bstrErrMsg);
        SysFreeString(bstrErrMsg);
        return; 
    }

	// Get the message from the string table.

    m_pString = &JustInCase;
    for(dwSize = 128; dwSize < 4096; dwSize *= 2)
    {

        wmilib::auto_buffer<TCHAR> p(new TCHAR[dwSize]);
        if(NULL == p.get())
        {
            return; 
        }
        dwRet = LoadString( ghModule, dwID, p.get(), dwSize);

        // Check for failure to load

        if(dwRet == 0)
        {
            return; 
        }
        // Check for the case where the buffer was too small

        if((dwRet + 1) >= dwSize)
        {
            //overrun, just loop
        }
        else
        {
            m_pString = p.release(); 
            return;             // all is well!
        }                   
    }
}

IntString::~IntString()
{
    if(m_pString != &JustInCase)
        delete(m_pString);
}
 
void CopyOrConvert(TCHAR * pTo, WCHAR * pFrom, int iLen)
{ 
#ifdef UNICODE
    wcsncpy(pTo, pFrom,iLen);
#else
    wcstombs(pTo, pFrom, iLen);
#endif
    pTo[iLen-1] = 0;
    return;
}

/*++

Copyright (C) 1996-2001 Microsoft Corporation

Module Name:

    PathParse.H

Abstract:

    Implements the default object path parser.

History:

    a-davj  5-feb-00       Created.

--*/

#ifndef _PATHPARSE_H_
#define _PATHPARSE_H_

#include "genlex.h"
#include <wmiutils.h>
#include <umi.h>
#include "wbemcli.h"
#include "flexarry.h"
typedef LPVOID * PPVOID;


class CSafeInCritSec : public  CInCritSec
{
public:
	CSafeInCritSec(CRITICAL_SECTION * pcs):CInCritSec(pcs){};
	~CSafeInCritSec(){};
    BOOL IsOK(){return true;};    
};


class CRefCntCS
{
private:
    long m_lRef;
    CRITICAL_SECTION m_cs;
    HRESULT m_Status;

public:
    CRefCntCS();
    ~CRefCntCS()
    {
        if(m_Status == S_OK)
            DeleteCriticalSection(&m_cs);
    }
    HRESULT GetStatus(){return m_Status;};
    long AddRef(void){long lRef = InterlockedIncrement(&m_lRef);return lRef;};
    long Release(void);
    CRITICAL_SECTION * GetCS(){return &m_cs;};
};


//***************************************************************************
//
//  STRUCT NAME:
//
//  CKeyRef
//
//  DESCRIPTION:
//
//  Holds information for a single key.  Includes name, data, and data type.
//
//***************************************************************************

struct CKeyRef
{
    LPWSTR  m_pName;
    DWORD m_dwType;
    DWORD m_dwSize;
    void * m_pData;

    CKeyRef();
    CKeyRef(LPCWSTR wszKeyName, DWORD dwType, DWORD dwSize, void * pData);
    HRESULT SetData(DWORD dwType, DWORD dwSize, void * pData);
   ~CKeyRef();

    // note that the caller is to free the returned string.
    LPWSTR GetValue(BOOL bQuotes=TRUE);
   
    DWORD GetValueSize();
    DWORD GetTotalSize();
};

//***************************************************************************
//
//  CLASS NAME:
//
//  CParsedComponent
//
//  DESCRIPTION:
//
//  Each namespace, scope and the class is represented by an instance of this.  It holds 
//  an array of CKeyRef objects and supports the IWbemPathKeyList interface.
//
//***************************************************************************

class  CParsedComponent : public IWbemPathKeyList
{
public:
    CParsedComponent(CRefCntCS *);
    ~CParsedComponent();
	friend class CDefPathParser;
	friend class CUmiPathParser;
	void ClearKeys ();
    HRESULT GetName(BSTR *pName);
    HRESULT Unparse(BSTR *pKey, bool bGetQuotes, bool bUseClassName);
    HRESULT GetComponentType(DWORD &dwType);
    BOOL AddKeyRef(CKeyRef* pAcquireRef);
	bool IsPossibleNamespace();
    bool IsInstance();
	HRESULT SetNS(LPCWSTR pName);

    //IUnknown members

    STDMETHODIMP         QueryInterface(REFIID riid, PPVOID ppv)
    {
        *ppv=NULL;

        if (IID_IUnknown==riid || IID_IWbemPathKeyList==riid)
            *ppv=this;
        else if (riid == IID_IMarshal && m_pFTM)
            return m_pFTM->QueryInterface(riid, ppv);
//postponed till Blackcomb        if (IID_IUmiURLKeyList==riid)
//postponed till Blackcomb			*ppv = &m_UmiWrapper;

        if (NULL!=*ppv)
        {
            AddRef();
            return NOERROR;
        }

        return E_NOINTERFACE;
    };

    STDMETHODIMP_(ULONG) AddRef(void)
    {    
        return InterlockedIncrement(&m_cRef);
    };
    STDMETHODIMP_(ULONG) Release(void)
    {
        long lRef = InterlockedDecrement(&m_cRef);
        if (0L == lRef)
            delete this;
        return lRef;
    };

    HRESULT STDMETHODCALLTYPE GetCount( 
            /* [out] */ ULONG __RPC_FAR *puKeyCount);
        
    HRESULT STDMETHODCALLTYPE SetKey( 
            /* [string][in] */ LPCWSTR wszName,
            /* [in] */ ULONG uFlags,
            /* [in] */ ULONG uCimType,
            /* [in] */ LPVOID pKeyVal);

    HRESULT STDMETHODCALLTYPE SetKey2( 
            /* [string][in] */ LPCWSTR wszName,
            /* [in] */ ULONG uFlags,
            /* [in] */ ULONG uCimType,
            /* [in] */ VARIANT __RPC_FAR *pKeyVal);

    HRESULT STDMETHODCALLTYPE GetKey( 
            /* [in] */ ULONG uKeyIx,
            /* [in] */ ULONG uFlags,
            /* [out][in] */ ULONG __RPC_FAR *puNameBufSize,
            /* [out][in] */ LPWSTR pszKeyName,
            /* [out][in] */ ULONG __RPC_FAR *puKeyValBufSize,
            /* [out][in] */ LPVOID pKeyVal,
            /* [out] */ ULONG __RPC_FAR *puApparentCimType);
        
    HRESULT STDMETHODCALLTYPE GetKey2( 
            /* [in] */ ULONG uKeyIx,
            /* [in] */ ULONG uFlags,
            /* [out][in] */ ULONG __RPC_FAR *puNameBufSize,
            /* [out][in] */ LPWSTR pszKeyName,
            /* [out][in] */ VARIANT __RPC_FAR *pKeyValue,
            /* [out] */ ULONG __RPC_FAR *puApparentCimType);

    HRESULT STDMETHODCALLTYPE RemoveKey( 
            /* [string][in] */ LPCWSTR wszName,
            /* [in] */ ULONG uFlags);

    HRESULT STDMETHODCALLTYPE RemoveAllKeys( 
            /* [in] */ ULONG uFlags);
        
    HRESULT STDMETHODCALLTYPE MakeSingleton( boolean bSet);
        
    HRESULT STDMETHODCALLTYPE GetInfo( 
            /* [in] */ ULONG uRequestedInfo,
            /* [out] */ ULONGLONG __RPC_FAR *puResponse);

	HRESULT STDMETHODCALLTYPE GetText( 
            /* [in] */ long lFlags,
            /* [out][in] */ ULONG __RPC_FAR *puBuffLength,
            /* [string][out] */ LPWSTR pszText);


private:

    BSTR        m_sClassName;
    CFlexArray  m_Keys;
    bool        m_bSingleton;
    long        m_cRef;
    CRefCntCS * m_pCS;
    IUnknown * m_pFTM;

};



//***************************************************************************
//
//  CLASS NAME:
//
//  CDefPathParser
//
//  DESCRIPTION:
//
//  Provides the default wmi path parser.
//
//***************************************************************************

class CDefPathParser : public IWbemPath
{
    public:
        CDefPathParser(void);
        ~CDefPathParser(void);
		DWORD GetNumComponents();
		bool IsEmpty(void);
		long GetNumNamespaces();
		void Empty(void);
        enum Status {UNINITIALIZED, BAD_STRING, EXECEPTION_THROWN, OK, FAILED_TO_INIT};
        BOOL ActualRelativeTest(LPWSTR wszMachine,
                               LPWSTR wszNamespace,
                               BOOL bChildrenOK);
        void InitEmpty(){};

        //IUnknown members
        STDMETHODIMP         QueryInterface(REFIID riid, PPVOID ppv)
        {
            *ppv=NULL;

            if(m_dwStatus == FAILED_TO_INIT)
                return WBEM_E_OUT_OF_MEMORY;
            if(m_pCS == NULL)
                return E_NOINTERFACE;
            if (IID_IUnknown==riid || IID_IWbemPath==riid)
                *ppv=(IWbemPath *)this;
            else if (riid == IID_IMarshal && m_pFTM)
                return m_pFTM->QueryInterface(riid, ppv);
            if (NULL!=*ppv)
            {
                AddRef();
                return NOERROR;
            }

            return E_NOINTERFACE;
        };

        STDMETHODIMP_(ULONG) AddRef(void)
        {    
            return InterlockedIncrement(&m_cRef);
        };
        STDMETHODIMP_(ULONG) Release(void)
        {
            long lRef = InterlockedDecrement(&m_cRef);
            if (0L == lRef)
                delete this;
            return lRef;
        };

        virtual HRESULT STDMETHODCALLTYPE SetText( 
            /* [in] */ ULONG uMode,
            /* [in] */ LPCWSTR pszPath);
        
        virtual HRESULT STDMETHODCALLTYPE GetText( 
            /* [in] */ long lFlags,
            /* [out][in] */ ULONG __RPC_FAR *puBuffLength,
            /* [string][out] */ LPWSTR pszText);

        virtual HRESULT STDMETHODCALLTYPE GetInfo( 
            /* [in] */ ULONG uRequestedInfo,
            /* [out] */ ULONGLONG __RPC_FAR *puResponse);
         
        virtual HRESULT STDMETHODCALLTYPE SetServer( 
            /* [string][in] */ LPCWSTR Name);
        
        virtual HRESULT STDMETHODCALLTYPE GetServer( 
            /* [out][in] */ ULONG __RPC_FAR *puNameBufLength,
            /* [string][out] */ LPWSTR pName);
        
        virtual HRESULT STDMETHODCALLTYPE GetNamespaceCount( 
            /* [out] */ ULONG __RPC_FAR *puCount);
        
        virtual HRESULT STDMETHODCALLTYPE SetNamespaceAt( 
            /* [in] */ ULONG uIndex,
            /* [string][in] */ LPCWSTR pszName);

        virtual HRESULT STDMETHODCALLTYPE GetNamespaceAt( 
            /* [in] */ ULONG uIndex,
            /* [out][in] */ ULONG __RPC_FAR *puNameBufLength,
            /* [string][out] */ LPWSTR pName);

        virtual HRESULT STDMETHODCALLTYPE RemoveNamespaceAt( 
            /* [in] */ ULONG uIndex);

		virtual HRESULT STDMETHODCALLTYPE RemoveAllNamespaces( void);
        
        virtual HRESULT STDMETHODCALLTYPE GetScopeCount( 
            /* [out] */ ULONG __RPC_FAR *puCount);
        
        virtual HRESULT STDMETHODCALLTYPE SetScope(
            unsigned long,unsigned short *);

        virtual HRESULT STDMETHODCALLTYPE SetScopeFromText( 
            /* [in] */ ULONG uIndex,
            /* [in] */ LPWSTR pszText);
        
        virtual HRESULT STDMETHODCALLTYPE GetScope( 
            /* [in] */ ULONG uIndex,
            /* [out][in] */ ULONG __RPC_FAR *puClassNameBufSize,
            /* [in] */ LPWSTR pszClass,
            /* [out] */ IWbemPathKeyList __RPC_FAR *__RPC_FAR *pKeyList);

        virtual HRESULT STDMETHODCALLTYPE GetScopeAsText( 
            /* [in] */ ULONG uIndex,
            /* [out][in] */ ULONG __RPC_FAR *puTextBufSize,
            /* [out][in] */ LPWSTR pszText);
        
        virtual HRESULT STDMETHODCALLTYPE RemoveScope( 
            /* [in] */ ULONG uIndex);

		virtual HRESULT STDMETHODCALLTYPE RemoveAllScopes( void);

        virtual HRESULT STDMETHODCALLTYPE SetClassName( 
            /* [string][in] */ LPCWSTR Name);
        
        virtual HRESULT STDMETHODCALLTYPE GetClassName( 
            /* [out][in] */ ULONG __RPC_FAR *puBuffLength,
            /* [string][out] */ LPWSTR pszName);
        
        virtual HRESULT STDMETHODCALLTYPE GetKeyList( 
            /* [out] */ IWbemPathKeyList __RPC_FAR *__RPC_FAR *pOut);

		virtual HRESULT STDMETHODCALLTYPE CreateClassPart( 
            /* [in] */ long lFlags,
            /* [string][in] */ LPCWSTR Name);
        
        virtual HRESULT STDMETHODCALLTYPE DeleteClassPart( 
            /* [in] */ long lFlags);

        virtual BOOL STDMETHODCALLTYPE IsRelative( 
            /* [string][in] */ LPWSTR wszMachine,
            /* [string][in] */ LPWSTR wszNamespace);
        
        virtual BOOL STDMETHODCALLTYPE IsRelativeOrChild( 
            /* [string][in] */ LPWSTR wszMachine,
            /* [string][in] */ LPWSTR wszNamespace,
            /* [in] */ long lFlags);

        virtual BOOL STDMETHODCALLTYPE IsLocal( 
            /* [string][in] */ LPCWSTR wszMachine);

        virtual BOOL STDMETHODCALLTYPE IsSameClassName( 
            /* [string][in] */ LPCWSTR wszClass);


		HRESULT SetServer(LPCWSTR Name, bool m_bServerNameSetByDefault, bool bAcquire);
        BOOL HasServer(){return m_pServer != NULL;};
        LPWSTR GetPath(DWORD nStartAt, DWORD nStopAt,bool bGetServer = false);
        BOOL AddNamespace(LPCWSTR wszNamespace);
        BOOL AddClass(LPCWSTR lpClassName);
        BOOL AddKeyRef(CKeyRef* pAcquireRef);
        BOOL SetSingletonObj();
        LPWSTR GetNamespacePart(); 
	    LPWSTR GetParentNamespacePart(); 
        BOOL SortKeys();
		CParsedComponent * GetLastComponent();
		HRESULT GetComponentString(ULONG Index, BSTR * pUnparsed, WCHAR & wDelim);
		HRESULT AddComponent(CParsedComponent * pComp);
		CParsedComponent * GetClass();
        CRefCntCS * GetRefCntCS(){return m_pCS;};
        void * m_pGenLex;               // for test purposes only

    protected:
		bool		m_bSetViaUMIPath;
        long        m_cRef;
        LPWSTR      m_pServer;           // NULL if no server
		CFlexArray  m_Components;        // list of namespaces and scopes
//		CParsedComponent * m_pClass;  // the class
        DWORD       m_dwStatus;
		bool		m_bParent;			 // true if text is ".."
		LPWSTR	m_pRawPath;				// temporary fix for Raja
        CRefCntCS * m_pCS;
        LPWSTR m_wszOriginalPath;
		bool   m_bServerNameSetByDefault;
        IUnknown * m_pFTM;
        DWORD m_dwException;

};


#endif

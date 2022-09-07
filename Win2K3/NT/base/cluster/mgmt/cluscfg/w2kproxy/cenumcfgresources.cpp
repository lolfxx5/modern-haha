//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2000-2001 Microsoft Corporation
//
//  Module Name:
//      CEnumCfgResource.cpp
//
//  Description:
//      CEnumCfgResource implementation.
//
//  Maintained By:
//      Galen Barbee (GalenB) 02-AUG-2000
//
//////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "CEnumCfgResources.h"
#include "CResourcePhysicalDisk.h"

DEFINE_THISCLASS("CEnumCfgResources")


//////////////////////////////////////////////////////////////////////////////
//
//  HRESULT
//  CEnumCfgResources::S_HrCreateInstance(
//      IUnknown ** ppunkOut
//      )
//
//////////////////////////////////////////////////////////////////////////////
HRESULT
CEnumCfgResources::S_HrCreateInstance(
    IUnknown ** ppunkOut,
    IUnknown * punkOuterIn,
    HCLUSTER * phClusterIn,
    CLSID * pclsidMajorIn
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CEnumCfgResources * pecr = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    } // if:

    pecr = new CEnumCfgResources;
    if ( pecr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    } // if:

    hr = THR( pecr->HrInit( punkOuterIn, phClusterIn, pclsidMajorIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pecr->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pecr != NULL )
    {
        pecr->Release();
    } // if:

    HRETURN( hr );

} //*** CEnumCfgResources::S_HrCreateInstance

//////////////////////////////////////////////////////////////////////////////
//
//  CEnumCfgResources::CEnumCfgResources
//
//////////////////////////////////////////////////////////////////////////////
CEnumCfgResources::CEnumCfgResources( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( m_punkOuter == NULL );
    Assert( m_phCluster == NULL );
    Assert( m_pclsidMajor == NULL );
    Assert( m_pcccb == NULL );
    Assert( m_hClusEnum == NULL );
    Assert( m_dwIndex == 0 );

    TraceFuncExit();

} //*** CEnumCfgResources::CEnumCfgResources

//////////////////////////////////////////////////////////////////////////////
//
//  CEnumCfgResources::~CEnumCfgResources
//
//////////////////////////////////////////////////////////////////////////////
CEnumCfgResources::~CEnumCfgResources( void )
{
    TraceFunc( "" );

    // m_cRef

    if ( m_punkOuter != NULL )
    {
        m_punkOuter->Release();
    }

    // m_phCluster - DO NOT CLOSE!

    // m_pclsidMajor

    if ( m_pcccb != NULL )
    {
        m_pcccb->Release();
    }

    if ( m_hClusEnum != NULL )
    {
        ClusterCloseEnum( m_hClusEnum );
    }

    // m_dwIndex

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

} //*** CEnumCfgResources::~CEnumCfgResources

//////////////////////////////////////////////////////////////////////////////
//
//  HRESULT
//  CEnumCfgResources::HrInit
//
//////////////////////////////////////////////////////////////////////////////
HRESULT
CEnumCfgResources::HrInit(
    IUnknown * punkOuterIn,
    HCLUSTER * phClusterIn,
    CLSID *    pclsidMajorIn
    )
{
    TraceFunc( "" );

    HRESULT hr;

    // IUnknown
    Assert( m_cRef == 1 );

    if ( punkOuterIn != NULL )
    {
        m_punkOuter = punkOuterIn;
        m_punkOuter->AddRef();
    }

    if ( phClusterIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    m_phCluster = phClusterIn;

    if ( pclsidMajorIn != NULL )
    {
        m_pclsidMajor = pclsidMajorIn;
    }
    else
    {
        m_pclsidMajor = (CLSID *) &TASKID_Major_Client_And_Server_Log;
    }

    if ( punkOuterIn != NULL )
    {
        hr = THR( punkOuterIn->TypeSafeQI( IClusCfgCallback, &m_pcccb ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

    //
    //  Open the enumer.
    //

    m_hClusEnum = ClusterOpenEnum( *m_phCluster, CLUSTER_ENUM_RESOURCE );
    if ( m_hClusEnum == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_Resources_HrInit_ClusterOpenEnum_Failed, hr );
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    HRETURN( hr );

} //*** CEnumCfgResources::HrInit


// ************************************************************************
//
// IUnknown
//
// ************************************************************************


//////////////////////////////////////////////////////////////////////////////
//++
//
//  CEnumCfgResources::QueryInterface
//
//  Description:
//      Query this object for the passed in interface.
//
//  Arguments:
//      riidIn
//          Id of interface requested.
//
//      ppvOut
//          Pointer to the requested interface.
//
//  Return Value:
//      S_OK
//          If the interface is available on this object.
//
//      E_NOINTERFACE
//          If the interface is not available.
//
//      E_POINTER
//          ppvOut was NULL.
//
//  Remarks:
//      None.
//
//--
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP
CEnumCfgResources::QueryInterface(
      REFIID    riidIn
    , void **   ppvOut
    )
{
    TraceQIFunc( riidIn, ppvOut );

    HRESULT hr = S_OK;

    //
    // Validate arguments.
    //

    Assert( ppvOut != NULL );
    if ( ppvOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    //
    // Handle known interfaces.
    //

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
        *ppvOut = static_cast< IEnumClusCfgManagedResources * >( this );
    } // if: IUnknown
    else if ( IsEqualIID( riidIn, IID_IEnumClusCfgManagedResources ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumClusCfgManagedResources, this, 0 );
    } // else if: IEnumClusCfgManagedResources
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    } // else

    //
    // Add a reference to the interface if successful.
    //

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown*) *ppvOut)->AddRef();
    } // if: success

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

} //*** CEnumCfgResources::QueryInterface

//////////////////////////////////////////////////////////////////////////////
//
//  STDMETHODIMP_(ULONG)
//  CEnumCfgResources::AddRef
//
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_( ULONG )
CEnumCfgResources::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

} //*** CEnumCfgResources::AddRef

//////////////////////////////////////////////////////////////////////////////
//
//  STDMETHODIMP_(ULONG)
//  CEnumCfgResources::Release
//
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_( ULONG )
CEnumCfgResources::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

} //*** CEnumCfgResources::Release


//****************************************************************************
//
//  IEnumClusCfgManagedResources
//
//****************************************************************************

//
//
//
STDMETHODIMP
CEnumCfgResources::Next(
    ULONG                           cNumberRequestedIn,
    IClusCfgManagedResourceInfo **  rgpManagedResourceInfoOut,
    ULONG *                         pcNumberFetchedOut
    )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr;
    ULONG   cFetched = 0;

    if ( rgpManagedResourceInfoOut == NULL )
    {
        hr= THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_Resources_Next_InvalidPointer, hr );
        goto Cleanup;
    }

    for( ; cFetched < cNumberRequestedIn; m_dwIndex++ )
    {
        hr = STHR( HrGetItem( &(rgpManagedResourceInfoOut[ cFetched ]) ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        if ( hr == S_FALSE )
        {
            continue; // resource was not type physical disk
        }

        if (  hr == MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_NO_MORE_ITEMS ) )
        {
            break;  // no more items
        }

        cFetched ++;

    } // for: cFetched

    if ( pcNumberFetchedOut != NULL )
    {
        *pcNumberFetchedOut = cFetched;
    } // if:

    if ( cFetched < cNumberRequestedIn )
    {
        hr = S_FALSE;
    } // if:
    else
    {
        hr = S_OK;
    }

Cleanup:
    if ( FAILED( hr ) )
    {
        ULONG idx;

        for ( idx = 0; idx < cFetched; idx++ )
        {
            (rgpManagedResourceInfoOut[ idx ])->Release();
        } // for:

        cFetched = 0;
    } // if:

    HRETURN( hr );

} //*** CEnumCfgResources::Next

//
//
//
STDMETHODIMP
CEnumCfgResources::Reset( void )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

    m_dwIndex = 0;

    HRETURN( hr );

} //*** CEnumCfgResources::Reset

//
//
//
STDMETHODIMP
CEnumCfgResources::Skip( ULONG cNumberToSkipIn )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

    //
    //  TODO:   GalenB  27 SEPT 2000
    //
    //  Need to ensure that we don't run off the end of the enumeration.
    //

    m_dwIndex += cNumberToSkipIn;

    HRETURN( hr );

} //*** CEnumCfgResources::Skip

//
//
//
STDMETHODIMP
CEnumCfgResources::Clone( IEnumClusCfgManagedResources ** ppEnumManagedResourcesOut )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

} //*** CEnumCfgResourcess::Clone

//
//
//
STDMETHODIMP
CEnumCfgResources::Count( DWORD * pnCountOut)
{

    TraceFunc( "[IEnumClusCfgManagedResources]" );

    Assert( m_hClusEnum != NULL );
    HRESULT hr = S_OK;

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pnCountOut = ClusterGetEnumCount(m_hClusEnum);

Cleanup:

    HRETURN( hr );

} //*** CEnumCfgResourcess::Count

//****************************************************************************
//
// IClusCfgCallback
//
//****************************************************************************


//////////////////////////////////////////////////////////////////////////////
//++
//
//  CEnumCfgResources::SendStatusReport
//
//  Description:
//
//  Arguments:
//
//  Return Value:
//
//  Remarks:
//      None.
//
//--
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP
CEnumCfgResources::SendStatusReport(
    BSTR        bstrNodeNameIn,
    CLSID       clsidTaskMajorIn,
    CLSID       clsidTaskMinorIn,
    ULONG       ulMinIn,
    ULONG       ulMaxIn,
    ULONG       ulCurrentIn,
    HRESULT     hrStatusIn,
    BSTR        bstrDescriptionIn,
    FILETIME *  pftTimeIn,
    BSTR        bstrReferenceIn
    )
{
    TraceFunc( "[IClusCfgCallback]" );

    HRESULT     hr = S_OK;

    if ( m_pcccb != NULL )
    {
        hr = THR( m_pcccb->SendStatusReport( bstrNodeNameIn,
                                             clsidTaskMajorIn,
                                             clsidTaskMinorIn,
                                             ulMinIn,
                                             ulMaxIn,
                                             ulCurrentIn,
                                             hrStatusIn,
                                             bstrDescriptionIn,
                                             pftTimeIn,
                                             bstrReferenceIn
                                             ) );
    } // if:

    HRETURN( hr );

} //*** CEnumCfgResources::SendStatusReport


//****************************************************************************
//
// Local methods.
//
//****************************************************************************


//
//
//
HRESULT
CEnumCfgResources::HrGetItem(
    IClusCfgManagedResourceInfo **  ppManagedResourceInfoOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    DWORD   sc;
    DWORD   dwTypeDummy;
    DWORD   cchName = 64;   // good starting value
    BSTR    bstrName = NULL;

    IUnknown * punk = NULL;

    Assert( ppManagedResourceInfoOut != NULL );
    Assert( m_hClusEnum != NULL );

    bstrName = TraceSysAllocStringLen( NULL, cchName );
    if ( bstrName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }
    cchName ++; // SysAllocStringLen allocates cchName + 1.

    // We are wrapping this a cchName should be significantly large enough to handle
    // most of our testing.
    sc = ClusterEnum( m_hClusEnum, m_dwIndex, &dwTypeDummy, bstrName, &cchName );
    if ( sc == ERROR_MORE_DATA )
    {
        //
        //  Our "typical" buffer is too small. Try make it to the size ClusterEnum
        //  returned.
        //

        TraceSysFreeString( bstrName );
        bstrName = NULL;

        bstrName = TraceSysAllocStringLen( NULL, cchName );
        if ( bstrName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
        cchName ++; // SysAllocStringLen allocates cchName + 1.

        sc = TW32( ClusterEnum( m_hClusEnum, m_dwIndex, &dwTypeDummy, bstrName, &cchName ) );
    }
    else if ( sc == ERROR_NO_MORE_ITEMS )
    {
        hr = MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_NO_MORE_ITEMS );
        goto Cleanup;
    }

    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( TW32( sc ) );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_Resources_HrGetItem_ClusterEnum_Failed, hr );
        goto Cleanup;
    }

    Assert( dwTypeDummy == CLUSTER_ENUM_RESOURCE );

    //
    // Create the requested object and store it.
    //

    hr = STHR( CResourcePhysicalDisk::S_HrCreateInstance( &punk, m_punkOuter, m_phCluster, m_pclsidMajor, bstrName ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrGetItem_Create_CResourcePhysicalDisk_Failed, hr );
        goto Cleanup;
    }

    if ( hr == S_FALSE )
    {
        goto Cleanup;   // This means that the object was not a physical disk resource.
    }

    //
    //  QI for the interface to return.
    //

    hr = THR( punk->TypeSafeQI( IClusCfgManagedResourceInfo, ppManagedResourceInfoOut ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_Resources_HrGetItem_QI_Failed, hr );
        goto Cleanup;
    }

Cleanup:

    TraceSysFreeString( bstrName );

    if ( punk != NULL )
    {
        punk->Release();
    }

    HRETURN( hr );

} //*** CEnumCfgResources::HrGetItem

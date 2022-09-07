//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 1999-2000 Microsoft Corporation
//
//  Module Name:
//      ClusterResTypeRes.cpp
//
//  Description:
//      Implementation of CClusterResTypeRes class 
//
//  Author:
//      Henry Wang (HenryWa) 24-AUG-1999
//
//////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "ClusterResTypeRes.h"

//****************************************************************************
//
//  CClusterResTypeRes
//
//****************************************************************************

//////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusterResTypeRes::CClusterResTypeRes(
//      LPCWSTR         pwszNameIn,
//      CWbemServices * pNamespaceIn,
//      DWORD           dwEnumTypeIn
//      )
//
//  Description:
//      Constructor.
//
//  Arguments:
//      pwszNameIn      -- Class name
//      pNamespaceIn    -- Namespace
//      dwEnumTypeIn    -- Type id
//
//  Return Values:
//      None.
//
//--
//////////////////////////////////////////////////////////////////////////////
CClusterResTypeRes::CClusterResTypeRes(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CClusterObjAssoc( pwszNameIn, pNamespaceIn, dwEnumTypeIn )
{

} //*** CClusterResTypeRes::CClusterResTypeRes()

//////////////////////////////////////////////////////////////////////////////
//++
//
//  static
//  CProvBase *
//  CClusterResTypeRes::S_CreateThis(
//      LPCWSTR         pwszNameIn,
//      CWbemServices * pNamespaceIn,
//      DWORD           dwEnumTypeIn
//      )
//
//  Description:
//      Create a cluster node object
//
//  Arguments:
//      pwszNameIn      -- Class name
//      pNamespaceIn    -- Namespace
//      dwEnumTypeIn    -- Type id
//
//  Return Values:
//      Pointer to the CProvBase
//
//--
//////////////////////////////////////////////////////////////////////////////
CProvBase *
CClusterResTypeRes::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
{
    return new CClusterResTypeRes(
                    pwszNameIn,
                    pNamespaceIn,
                    dwEnumTypeIn
                    );

} //*** CClusterResTypeRes::S_CreateThis()


//////////////////////////////////////////////////////////////////////////////
//++
//
//  SCODE
//  CClusterResTypeRes::EnumInstance(
//      long                 lFlagsIn,
//      IWbemContext *       pCtxIn,
//      IWbemObjectSink *    pHandlerIn
//      )
//
//  Description:
//      Enumerate resources of a particular type.
//
//  Arguments:
//      lFlagsIn    -- WMI flag
//      pCtxIn      -- WMI context
//      pHandlerIn  -- WMI sink pointer
//
//  Return Values:
//      WBEM_S_NO_ERROR
//
//--
//////////////////////////////////////////////////////////////////////////////
SCODE
CClusterResTypeRes::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFERESOURCE        shResource;
    LPCWSTR             pwszName = NULL;
    DWORD               cbTypeName = MAX_PATH;
    CWstrBuf            wsbTypeName;
    CError              er;
    DWORD               dwError;
    CWbemClassObject    wco;
    CWbemClassObject    wcoGroup;
    CWbemClassObject    wcoPart;
    _bstr_t             bstrGroup;
    _bstr_t             bstrPart;

    wsbTypeName.SetSize( cbTypeName ); 

    shCluster = OpenCluster( NULL );
    CClusterEnum clusEnum( shCluster, m_dwEnumType );

    m_wcoGroup.SpawnInstance( 0, & wcoGroup );
    m_wcoPart.SpawnInstance( 0, & wcoPart );

    while ( ( pwszName = clusEnum.GetNext() ) != NULL )
    {
        DWORD cbTypeNameReturned = 0;
        shResource = OpenClusterResource( shCluster, pwszName );
        //
        // get resource type name
        //
        dwError = ClusterResourceControl(
                        shResource,
                        NULL,
                        CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                        NULL,
                        0,
                        wsbTypeName,
                        cbTypeName,
                        & cbTypeNameReturned
                        );
        if ( dwError == ERROR_MORE_DATA )
        {
            cbTypeName = cbTypeNameReturned;
            wsbTypeName.SetSize( cbTypeNameReturned );
            er = ClusterResourceControl(
                        shResource,
                        NULL,
                        CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                        NULL,
                        0,
                        wsbTypeName,
                        cbTypeName,
                        & cbTypeNameReturned
                        );
        } // if: buffer was too small

        wcoPart.SetProperty( pwszName, CLUSREG_NAME_RES_NAME );
        wcoPart.GetProperty( bstrPart, PVD_WBEM_RELPATH );

        
        wcoGroup.SetProperty( wsbTypeName, PVD_PROP_RESTYPE_NAME );
        wcoGroup.GetProperty( bstrGroup, PVD_WBEM_RELPATH );

        m_pClass->SpawnInstance( 0, & wco );
        wco.SetProperty( (LPWSTR) bstrGroup, PVD_PROP_GROUPCOMPONENT );
        wco.SetProperty( (LPWSTR) bstrPart,  PVD_PROP_PARTCOMPONENT );
        pHandlerIn->Indicate( 1, & wco );
        
    } // while: more resource types (??

    return WBEM_S_NO_ERROR;

} //*** CClusterResTypeRes::EnumInstance()

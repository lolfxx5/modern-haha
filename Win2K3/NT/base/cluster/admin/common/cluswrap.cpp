/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 1997-2002 Microsoft Corporation
//
//  Module Name:
//      ClusWrap.cpp
//
//  Description:
//      Wrapper functions for Cluster APIs.
//
//  Author:
//      Galen Barbee    (GalenB)    15-Aug-1998
//
//  Notes:
//
/////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include <clusapi.h>
#include "cluswrap.h"
//#include "ClusWrap.tmh"

/////////////////////////////////////////////////////////////////////////////
// Type Definitions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//++
//
//  WrapGetClusterInformation
//
//  Description:
//      Wraps the GetClusterInformation function.
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI WrapGetClusterInformation(
    IN HCLUSTER                         hCluster,
    OUT LPWSTR *                        ppszClusterName,
    OUT OPTIONAL LPCLUSTERVERSIONINFO   pClusterInfo
    )
{
    DWORD   dwStatus;
    LPWSTR  pwszName = NULL;
    DWORD   cchName = 128;
    DWORD   cchTempName = cchName;

    // Zero the out parameter.
    if ( ppszClusterName != NULL )
    {
        *ppszClusterName = NULL;
    }

    pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
    if ( pwszName != NULL )
    {
        dwStatus = GetClusterInformation( hCluster, pwszName, &cchTempName, pClusterInfo );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszName );
            pwszName = NULL;

            cchName = ++cchTempName;
            pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
            if ( pwszName != NULL )
            {
                dwStatus = GetClusterInformation( hCluster, pwszName, &cchTempName, pClusterInfo );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppszClusterName != NULL ) )
    {
        *ppszClusterName = pwszName;
    }

    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppszClusterName == NULL ) )
    {
        LocalFree( pwszName );
    }

    return dwStatus;

} //*** WrapGetClusterInformation()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  WrapGetClusterQuorumResource
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI WrapGetClusterQuorumResource(
    IN  HCLUSTER    hCluster,
    OUT LPWSTR *    ppwszResourceName,
    OUT LPWSTR *    ppwszDeviceName,
    OUT LPDWORD     pdwMaxQuorumLogSize
    )
{
    DWORD   dwStatus;
    LPWSTR  pwszResourceName = NULL;
    DWORD   cchResourceName = 128;
    DWORD   cchTempResourceName = cchResourceName;
    LPWSTR  pwszDeviceName = NULL;
    DWORD   cchDeviceName = 128;
    DWORD   cchTempDeviceName = cchDeviceName;
    DWORD   dwMaxQuorumLogSize = 0;

    // Zero the out parameters
    if ( ppwszResourceName != NULL )
    {
        *ppwszResourceName = NULL;
    }

    if ( ppwszDeviceName != NULL )
    {
        *ppwszDeviceName = NULL;
    }

    if ( pdwMaxQuorumLogSize != NULL )
    {
        *pdwMaxQuorumLogSize = 0;
    }

    // Allocate the resource name buffer
    pwszResourceName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchResourceName * sizeof( *pwszResourceName ) );
    if ( pwszResourceName != NULL )
    {
        // Allocate the device name buffer
        pwszDeviceName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchDeviceName * sizeof( *pwszDeviceName ) );
        if ( pwszDeviceName != NULL )
        {
            dwStatus = GetClusterQuorumResource( hCluster,
                                                 pwszResourceName,
                                                 &cchTempResourceName,
                                                 pwszDeviceName,
                                                 &cchTempDeviceName,
                                                 &dwMaxQuorumLogSize );
            if ( dwStatus == ERROR_MORE_DATA )
            {
                LocalFree( pwszResourceName );
                pwszResourceName = NULL;

                cchResourceName = ++cchTempResourceName;
                // Allocate the resource name buffer
                pwszResourceName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchResourceName * sizeof( *pwszResourceName ) );
                if ( pwszResourceName != NULL )
                {
                    LocalFree( pwszDeviceName );
                    pwszDeviceName = NULL;

                    cchDeviceName = ++cchTempDeviceName;
                    // Allocate the device name buffer
                    pwszDeviceName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchDeviceName * sizeof( *pwszDeviceName ) );
                    if ( pwszDeviceName != NULL )
                    {
                        dwStatus = GetClusterQuorumResource( hCluster,
                                                             pwszResourceName,
                                                             &cchTempResourceName,
                                                             pwszDeviceName,
                                                             &cchTempDeviceName,
                                                             &dwMaxQuorumLogSize );
                    }
                    else
                    {
                        dwStatus = GetLastError();
                    }
                }
                else
                {
                    dwStatus = GetLastError();
                }
            }
        }
        else
        {
            dwStatus = GetLastError();
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

    //
    // if we succeeded and if the argument is not NULL then return it.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszResourceName != NULL ) )
    {
        *ppwszResourceName = pwszResourceName;
    }

    //
    // if we succeeded and if the argument is not NULL then return it.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszDeviceName != NULL ) )
    {
        *ppwszDeviceName = pwszDeviceName;
    }

    //
    // if we succeeded and if the argument is not NULL then return it.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( pdwMaxQuorumLogSize != NULL ) )
    {
        *pdwMaxQuorumLogSize = dwMaxQuorumLogSize;
    }

    //
    // if we didn't succeeded or if the string argument is NULL then free the string.
    //
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszResourceName == NULL ) )
    {
        LocalFree( pwszResourceName );
    }

    //
    // if we didn't succeeded or if the string argument is NULL then free the string.
    //
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszDeviceName == NULL ) )
    {
        LocalFree( pwszDeviceName );
    }

    return dwStatus;

} //*** WrapGetClusterQuorumResource()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  Function:   WrapClusterEnum
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI WrapClusterEnum(
    IN HCLUSENUM    hEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD     pdwType,
    OUT LPWSTR *    ppwszName
    )
{
    DWORD   dwStatus;
    DWORD   dwType = 0;
    LPWSTR  pwszName = NULL;
    DWORD   cchName = 128;
    DWORD   cchTempName = cchName;

    // Zero the out parameters
    if ( pdwType != NULL )
    {
        *pdwType = 0;
    }

    if ( ppwszName != NULL )
    {
        *ppwszName = NULL;
    }

    pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
    if ( pwszName != NULL )
    {
        dwStatus = ClusterEnum( hEnum, dwIndex, &dwType, pwszName, &cchTempName );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszName );
            pwszName = NULL;
            cchName = ++cchTempName;

            pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
            if ( pwszName != NULL )
            {
                dwStatus = ClusterEnum( hEnum, dwIndex, &dwType, pwszName, &cchTempName );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

    //
    // if we succeeded and if the argument is not NULL then return it.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( pdwType != NULL ) )
    {
        *pdwType = dwType;
    }

    //
    // if we succeeded and if the string argument is not NULL then return the string.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszName != NULL ) )
    {
        *ppwszName = pwszName;
    }

    //
    // if we didn't succeeded or if the string argument is NULL then free the string.
    //
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszName == NULL ) )
    {
        LocalFree( pwszName );
    }

    return dwStatus;

} //*** WrapClusterEnum()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  WrapGetClusterNodeId
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI WrapGetClusterNodeId(
    IN HNODE        hNode,
    OUT LPWSTR *    ppwszNodeId
    )
{
    DWORD   dwStatus;
    LPWSTR  pwszNodeId = NULL;
    DWORD   cchNodeId = 128;
    DWORD   cchTempNodeId = cchNodeId;

    // Zero the out parameters
    if ( ppwszNodeId != NULL )
    {
        *ppwszNodeId = NULL;
    }

    pwszNodeId = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchNodeId * sizeof( *pwszNodeId ) );
    if ( pwszNodeId != NULL)
    {
        dwStatus = GetClusterNodeId( hNode, pwszNodeId, &cchTempNodeId );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszNodeId );
            pwszNodeId = NULL;

            cchNodeId = ++cchTempNodeId;
            pwszNodeId = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchNodeId * sizeof( *pwszNodeId ) );
            if ( pwszNodeId != NULL)
            {
                dwStatus = GetClusterNodeId( hNode, pwszNodeId, &cchTempNodeId );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

    //
    // if we succeeded and if the argument is not NULL then return it.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszNodeId != NULL ) )
    {
        *ppwszNodeId = pwszNodeId;
    }

    //
    // if we didn't succeeded or if the string argument is NULL then free the string.
    //
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszNodeId == NULL ) )
    {
        LocalFree( pwszNodeId );
    }

    return dwStatus;

} //*** WrapGetClusterNodeId()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  WrapGetClusterGroupState
//
//  Description:
//      Wrapper function for GetClusterGroupState.
//
//  Arguments:
//      hGroup          [IN]    - The group handle.
//      ppwszNodeName   [OUT]   - Catches the name of the node that the group
//                              is online, if not NULL.
//
//  Return Value:
//      A cluster group state enum.
//
//--
/////////////////////////////////////////////////////////////////////////////
CLUSTER_GROUP_STATE WINAPI WrapGetClusterGroupState(
    IN  HGROUP              hGroup,
    OUT OPTIONAL LPWSTR *   ppwszNodeName   // = NULL
    )
{
    CLUSTER_GROUP_STATE cState = ClusterGroupStateUnknown;

    if ( ppwszNodeName == NULL )
    {
        // The caller is not interested in the node name.
        // So, just call the actual function.
        cState = GetClusterGroupState( hGroup, NULL, 0 );
    } // if: the pointer to the node name pointer is not provided.
    else
    {
        LPWSTR              pwszNodeName = NULL;
        DWORD               cchNodeName = 128;
        DWORD               cchTempNodeName = cchNodeName;

        // Zero the out parameters
        *ppwszNodeName = NULL;

        pwszNodeName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchNodeName * sizeof( *pwszNodeName ) );
        if ( pwszNodeName != NULL )
        {
            cState = GetClusterGroupState( hGroup, pwszNodeName, &cchTempNodeName );
            if ( GetLastError() == ERROR_MORE_DATA )
            {
                cState = ClusterGroupStateUnknown;      // reset to error condition

                LocalFree( pwszNodeName );

                cchNodeName = ++cchTempNodeName;
                pwszNodeName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchNodeName * sizeof( *pwszNodeName ) );
                if ( pwszNodeName != NULL )
                {
                    cState = GetClusterGroupState( hGroup, pwszNodeName, &cchTempNodeName );
                }
                else
                {
                    SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                } // else:
            }
        }
        else
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        } // else:

        //
        // if there was not an error, then return the string.
        //
        if ( cState != ClusterGroupStateUnknown )
        {
            *ppwszNodeName = pwszNodeName;
        }
        else
        {
            LocalFree( pwszNodeName );
        }
    } // else: the pointer to the node name pointer is not NULL.

    return cState;

} //*** WrapGetClusterGroupState()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  WrapClusterGroupEnum
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI WrapClusterGroupEnum(
    IN HGROUPENUM   hGroupEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD  pdwType,
    OUT LPWSTR *    ppwszName
    )
{
    DWORD   dwStatus;
    DWORD   dwType = 0;
    LPWSTR  pwszName = NULL;
    DWORD   cchName = 128;
    DWORD   cchTempName = cchName;

    // Zero the out parameters
    if ( pdwType != NULL )
    {
        *pdwType = NULL;
    }

    if ( ppwszName != NULL )
    {
        *ppwszName = NULL;
    }

    pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
    if ( pwszName != NULL )
    {
        dwStatus = ClusterGroupEnum( hGroupEnum, dwIndex, &dwType, pwszName, &cchTempName );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszName );
            pwszName = NULL;

            cchName = ++cchTempName;
            pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
            if ( pwszName != NULL )
            {
                dwStatus = ClusterGroupEnum( hGroupEnum, dwIndex, &dwType, pwszName, &cchTempName );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

    //
    // if there was not an error and the argument was not NULL, then return the value.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( pdwType != NULL ) )
    {
        *pdwType = dwType;
    }

    //
    // if there was not an error and the argument was not NULL, then return the string.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszName != NULL ) )
    {
        *ppwszName = pwszName;
    }

    //
    // if there was an error and the argument was NULL, then free the string.
    //
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszName == NULL ) )
    {
        LocalFree( pwszName );
    }

    return dwStatus;

} //*** WrapClusterGroupEnum()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  WrapClusterNetworkEnum
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI WrapClusterNetworkEnum(
    IN HNETWORKENUM hEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD  pdwType,
    OUT LPWSTR *    ppwszName
    )
{
    DWORD   dwStatus;
    DWORD   dwType = 0;
    LPWSTR  pwszName = NULL;
    DWORD   cchName = 128;
    DWORD   cchTempName = cchName;

    // Zero the out parameters
    if ( pdwType != NULL )
    {
        *pdwType = 0;
    }

    if ( ppwszName != NULL )
    {
        *ppwszName = NULL;
    }

    pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
    if ( pwszName != NULL )
    {
        dwStatus = ClusterNetworkEnum( hEnum, dwIndex, &dwType, pwszName, &cchTempName );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszName );
            pwszName = NULL;

            cchName = ++cchTempName;
            pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
            if ( pwszName != NULL )
            {
                dwStatus = ClusterNetworkEnum( hEnum, dwIndex, &dwType, pwszName, &cchTempName );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

    //
    // if there was not an error and the argument was not NULL, then return the value.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( pdwType != NULL ) )
    {
        *pdwType = dwType;
    }

    //
    // if there was not an error and the argument was not NULL, then return the string.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszName != NULL ) )
    {
        *ppwszName = pwszName;
    }

    //
    // if there was an error and the argument was NULL, then free the string.
    //
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszName == NULL ) )
    {
        LocalFree( pwszName );
    }

    return dwStatus;

} //*** WrapClusterNetworkEnum()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  WrapClusterNodeEnum
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI WrapClusterNodeEnum(
    IN HNODEENUM    hEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD  pdwType,
    OUT LPWSTR *    ppwszName
    )
{
    DWORD   dwStatus;
    DWORD   dwType = 0;
    LPWSTR  pwszName = NULL;
    DWORD   cchName = 128;
    DWORD   cchTempName = cchName;

    // Zero the out parameters
    if ( pdwType != NULL )
    {
        *pdwType = 0;
    }

    if ( ppwszName != NULL )
    {
        *ppwszName = NULL;
    }

    pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
    if ( pwszName != NULL )
    {
        dwStatus = ClusterNodeEnum( hEnum, dwIndex, &dwType, pwszName, &cchTempName );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszName );
            pwszName = NULL;

            cchName = ++cchTempName;
            pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
            if ( pwszName != NULL )
            {
                dwStatus = ClusterNodeEnum( hEnum, dwIndex, &dwType, pwszName, &cchTempName );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

    //
    // if there was not an error and the argument was not NULL, then return the value.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( pdwType != NULL ) )
    {
        *pdwType = dwType;
    }

    //
    // if there was not an error and the argument was not NULL, then return the string.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszName != NULL ) )
    {
        *ppwszName = pwszName;
    }

    //
    // if there was an error and the argument was NULL, then free the string.
    //
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszName == NULL ) )
    {
        LocalFree( pwszName );
    }

    return dwStatus;

} //*** WrapClusterNodeEnum()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  WrapGetClusterResourceState
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
CLUSTER_RESOURCE_STATE WINAPI WrapGetClusterResourceState(
    IN HRESOURCE hResource,
    OUT OPTIONAL LPWSTR * ppwszNodeName,
    OUT OPTIONAL LPWSTR * ppwszGroupName
    )
{
    CLUSTER_RESOURCE_STATE  cState = ClusterResourceStateUnknown;
    LPWSTR                  pwszNodeName = NULL;
    DWORD                   cchNodeName = 128;
    LPWSTR                  pwszGroupName = NULL;
    DWORD                   cchGroupName = 128;
    DWORD                   cchTempNodeName = cchNodeName;
    DWORD                   cchTempGroupName = cchGroupName;

    //printf( "Entering... " );

    // Zero the out parameters
    if ( ppwszNodeName != NULL )
    {
        *ppwszNodeName = NULL;
    }

    if ( ppwszGroupName != NULL )
    {
        *ppwszGroupName = NULL;
    }

    pwszNodeName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchNodeName * sizeof( *pwszNodeName ) );
    if ( pwszNodeName != NULL )
    {
        pwszGroupName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchGroupName * sizeof( *pwszGroupName ) );
        if ( pwszGroupName != NULL )
        {
            cState = GetClusterResourceState( hResource, pwszNodeName, &cchTempNodeName, pwszGroupName, &cchTempGroupName );
            if ( GetLastError() == ERROR_MORE_DATA )
            {
                cState = ClusterResourceStateUnknown;   // reset to error condition

                LocalFree( pwszNodeName );
                pwszNodeName = NULL;
                cchNodeName = ++cchTempNodeName;

                LocalFree( pwszGroupName );
                pwszGroupName = NULL;
                cchGroupName = ++cchTempGroupName;

                pwszNodeName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchNodeName * sizeof( *pwszNodeName ) );
                if ( pwszNodeName != NULL )
                {
                    pwszGroupName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchGroupName * sizeof( *pwszGroupName ) );
                    if ( pwszGroupName != NULL )
                    {
                        cState = GetClusterResourceState( hResource, pwszNodeName, &cchNodeName, pwszGroupName, &cchGroupName );
                    } // if: LocalAlloc succeeded
                    else
                    {
                        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                    } // else: LocalAlloc failed
                } // if: LocalAlloc succeeded
                else
                {
                    SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                } // else: LocalAlloc failed
            } // if: ERROR_MORE_DATA
        } // if: LocalAlloc succeeded
        else
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        } // else: LocalAlloc failed
    } // if: LocalAlloc succeeded
    else
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
    } // else: LocalAlloc failed

    //
    // if there was not an error and the argument was not NULL, then return the string.
    //
    if ( ( cState != ClusterResourceStateUnknown ) && ( ppwszNodeName != NULL ) )
    {
        *ppwszNodeName = pwszNodeName;
    }

    //
    // if there was not an error and the argument was not NULL, then return the string.
    //
    if ( ( cState != ClusterResourceStateUnknown ) && ( ppwszGroupName != NULL ) )
    {
        *ppwszGroupName = pwszGroupName;
    }

    //
    // if there was an error or the argument was NULL, then free the string.
    //
    if ( ( cState == ClusterResourceStateUnknown ) || ( ppwszNodeName == NULL ) )
    {
        LocalFree( pwszNodeName );
    }

    //
    // if there was an error or the argument was NULL, then free the string.
    //
    if ( ( cState == ClusterResourceStateUnknown ) || ( ppwszGroupName == NULL ) )
    {
        LocalFree( pwszGroupName );
    }

    //printf( "Exiting.  Resource state is %d.", cState );

    return cState;

} //*** WrapGetClusterResourceState()
/*
/////////////////////////////////////////////////////////////////////////////
//++
//
//  WrapGetClusterNetInterfaceState
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
CLUSTER_NETINTERFACE_STATE WINAPI WrapGetClusterNetInterfaceState(
    IN HNETINTERFACE hNetInterface
    )
{

    return GetClusterNetInterfaceState( hNetInterface );

} //*** WrapGetClusterNetInterfaceState()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  WrapGetClusterNetworkState
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
CLUSTER_NETWORK_STATE WINAPI WrapGetClusterNetworkState(
    IN HNETWORK hNetwork
    )
{

    return GetClusterNetworkState( hNetwork );

} //*** WrapGetClusterNetworkState()
*/
/////////////////////////////////////////////////////////////////////////////
//++
//
//  WrapClusterResourceEnum
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI WrapClusterResourceEnum(
    IN HRESENUM  hResEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD  pdwType,
    OUT LPWSTR *    ppwszName
    )
{
    DWORD   dwStatus;
    DWORD   dwType = 0;
    LPWSTR  pwszName = NULL;
    DWORD   cchName = 128;
    DWORD   cchTempName = cchName;

    // Zero the out parameters
    if ( pdwType != NULL )
    {
        *pdwType = 0;
    }

    if ( ppwszName != NULL )
    {
        *ppwszName = NULL;
    }

    pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
    if ( pwszName != NULL )
    {
        dwStatus = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pwszName, &cchTempName );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszName );
            pwszName = NULL;

            cchName = ++cchTempName;
            pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
            if ( pwszName != NULL )
            {
                dwStatus = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pwszName, &cchTempName );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

    //
    // if there was not an error and the argument was not NULL, then return the value.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( pdwType != NULL ) )
    {
        *pdwType = dwType;
    }

    //
    // if there was not an error and the argument was not NULL, then return the string.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszName != NULL ) )
    {
        *ppwszName = pwszName;
    }

    //
    // if there was an error and the argument was NULL, then free the string.
    //
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszName == NULL ) )
    {
        LocalFree( pwszName );
    }

    return dwStatus;

} //*** WrapClusterResourceEnum()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  WrapClusterResourceTypeEnum
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI WrapClusterResourceTypeEnum(
    IN HRESTYPEENUM hResEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD  pdwType,
    OUT LPWSTR *    ppwszName
    )
{
    DWORD   dwStatus;
    DWORD   dwType = 0;
    LPWSTR  pwszName = NULL;
    DWORD   cchName = 128;
    DWORD   cchTempName = cchName;

    // Zero the out parameters
    if ( pdwType != NULL )
    {
        *pdwType = 0;
    }

    if ( ppwszName != NULL )
    {
        *ppwszName = NULL;
    }

    pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
    if ( pwszName != NULL )
    {
        dwStatus = ClusterResourceTypeEnum( hResEnum, dwIndex, &dwType, pwszName, &cchTempName );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszName );
            pwszName = NULL;

            cchName = ++cchTempName;
            pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
            if ( pwszName != NULL )
            {
                dwStatus = ClusterResourceTypeEnum( hResEnum, dwIndex, &dwType, pwszName, &cchTempName );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

    //
    // if there was not an error and the argument was not NULL, then return the value.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( pdwType != NULL ) )
    {
        *pdwType = dwType;
    }

    //
    // if there was not an error and the argument was not NULL, then return the string.
    //
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszName != NULL ) )
    {
        *ppwszName = pwszName;
    }

    //
    // if there was an error and the argument was NULL, then free the string.
    //
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszName == NULL ) )
    {
        LocalFree( pwszName );
    }

    return dwStatus;

} //*** WrapClusterResourceTypeEnum()


//*************************************************************************//


/////////////////////////////////////////////////////////////////////////////
// Misc helper functions, etc.
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CClusterNotifyPort
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusterNotifyPort::CClusterNotifyPort
//
//  Description:    This class is a wrapper for the cluster notify port
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
CClusterNotifyPort::CClusterNotifyPort( void )
{
    m_dwNotifyKey = 0;
    m_dwFilterType = 0;
    m_szName = NULL;
    m_cchName = 0;
    m_hChange = NULL;

} //*** CClusterNotifyPort::CClusterNotifyPort()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusterNotifyPort::~CClusterNotifyPort
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
CClusterNotifyPort::~CClusterNotifyPort( void )
{
    if( NULL != m_szName )
    {
        delete [] m_szName;
    }

    Close();

} //*** CClusterNotifyPort::~CClusterNotifyPort()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusterNotifyPort::Create
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD CClusterNotifyPort::Create(
    HCHANGE     hChange,
    HCLUSTER    hCluster,
    DWORD       dwFilter,
    DWORD_PTR   dwNotifyKey
    )
{
    DWORD sc = ERROR_SUCCESS;

    //printf( "Entering..." );

    m_hChange = CreateClusterNotifyPort( hChange, hCluster, dwFilter, dwNotifyKey );
    if ( m_hChange == NULL )
    {
        sc = GetLastError();
    }

    //printf( "Exiting. sc = %#08x", sc );

    return sc;

} //*** CClusterNotifyPort::Create()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusterNotifyPort::Close
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD CClusterNotifyPort::Close( void )
{
    DWORD sc = ERROR_SUCCESS;

    //printf( "Entering..." );

    if ( m_hChange != NULL )
    {
        sc = CloseClusterNotifyPort( m_hChange );
    }

    //printf( "Exiting. sc = %#08x", sc );

    return sc;

} //*** CClusterNotifyPort::Close()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusterNotifyPort::Register
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD CClusterNotifyPort::Register(
    DWORD       dwFilterType,
    HANDLE      hObject,
    DWORD_PTR   dwNotifyKey
    )
{
    DWORD   _sc = ERROR_SUCCESS;

    //printf( "Entering..." );

    _sc = RegisterClusterNotify( m_hChange, dwFilterType, hObject, dwNotifyKey );

    //printf( "Exiting. sc = %#08x", _sc );

    return _sc;

} //*** CClusterNotifyPort::Register()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusterNotifyPort::GetNotify
//
//  Description:
//
//  Arguments:
//
//
//  Return Value:
//
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD CClusterNotifyPort::GetNotify( void )
{
    DWORD sc = ERROR_SUCCESS;
    DWORD cchName;

    //printf( "Entering..." );

    cchName = m_cchName;

    //
    // Wait until state changes or 1 second elapses
    //
    sc = GetClusterNotify( m_hChange, &m_dwNotifyKey, &m_dwFilterType, m_szName, &cchName, 1000 );
    //printf( "GetClusterNotify() returned. sc = %#08x", sc );

    //
    // If we got an error_more_data or we passed in a NULL buffer pointer and got error_success
    // then we have to resize our buffer.  Member m_szName is initialized to NULL.
    //
    if ( sc == ERROR_MORE_DATA ||
       ( m_szName == NULL && sc == ERROR_SUCCESS )  )
    {
        //
        // resize the buffer
        //
        delete [] m_szName;

        cchName++;          // add one for NULL

        m_cchName = cchName;
        m_szName = new WCHAR[ m_cchName ];
        if ( m_szName == NULL )
        {
            sc = ERROR_NOT_ENOUGH_MEMORY;
        } // if:
        else
        {
            cchName = m_cchName;
            sc = GetClusterNotify( m_hChange, &m_dwNotifyKey, &m_dwFilterType, m_szName, &cchName, 0 );
        } // else:
    } // if:

    //printf( "Exiting. sc = %#08x", sc );

    return sc;

} //*** CClusterNotifyPort::GetNotify()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  WaitForResourceStateChange
//
//  Description:
//      Wait for the resource state to change to a non pending state.
//
//  Arguments:
//      hCluster        [IN]        - handle to the cluster
//      pwszName        [IN]        - name of the resource to wait on
//      pPort           [IN]        - notification port to use
//      pnWait          [IN OUT]    - ~ number of seconds to wait
//
//  Return Value:
//      ERROR_SUCCESS       The resource is not in a pending state or pnWait is NULL.
//      ERROR_IO_PENDING    The resource is in a pending state (wait expired).
//      Win32 error code
//
//--
/////////////////////////////////////////////////////////////////////////////
static DWORD WaitForResourceStateChange(
    IN      HCLUSTER                hCluster,
    IN      LPWSTR                  pwszName,
    IN      CClusterNotifyPort *    pPort,
    IN OUT  DWORD *                 pnWait
    )
{
    CLUSTER_RESOURCE_STATE  crs = ClusterResourceStateUnknown;
    HRESOURCE               hResource = NULL;
    DWORD                   _sc = ERROR_SUCCESS;

    if ( pnWait != NULL )
    {
        hResource = OpenClusterResource( hCluster, pwszName );
        if ( hResource != NULL )
        {
            while ( *pnWait > 0 )
            {
                crs = WrapGetClusterResourceState( hResource, NULL, NULL );
                if ( crs != ClusterResourceStateUnknown )
                {
                    //
                    // if the state is greater than ClusterResourcePending then it's
                    // in a pending state and we want to wait for the next notification.
                    //
                    if ( crs > ClusterResourcePending )
                    {
                        pPort->GetNotify();  // this will only wait for up to 1 second.
                        --(*pnWait);
                    } // if: resource is in pending state
                    else
                    {
                        break;
                    } // else if: resource is no longer in a pending state
                } // if: WrapClusterResourceState
                else
                {
                    _sc = GetLastError();
                    break;
                } // else: WrapClusterResourceState failed
            } // while: *pnWait > 0

            //
            //  Either it's transitioned to a non-pending state, the wait
            //  has expired, or the wait was zero to begin with.
            //
            if ( _sc == ERROR_SUCCESS )
            {
                crs = WrapGetClusterResourceState( hResource, NULL, NULL );
                if ( crs > ClusterResourcePending )
                {
                    _sc = ERROR_IO_PENDING;
                }
            } // if: we didn't get an error above

            CloseClusterResource( hResource );
        } // if: OpenClusterResource ok
        else
        {
            _sc = GetLastError();
        } // else: OpenClusterResource failed
    } // if: pnWait not NULL, this is for safety only

    return _sc;

} //*** WaitForResourceStateChange()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  WaitForResourceGroupStateChange
//
//  Description:
//      Wait for the resource group state to change to a non pending state.
//
//  Arguments:
//      hCluster        [IN]        - handle to the cluster
//      hGroup          [IN]        - handle to the group to wait on
//      pnWait          [IN OUT]    - ~ number of seconds to wait
//
//  Return Value:
//      ERROR_SUCCESS or other Win32 error
//
//--
/////////////////////////////////////////////////////////////////////////////
static DWORD WaitForResourceGroupStateChange(
    IN      HCLUSTER    hCluster,
    IN      HGROUP      hGroup,
    IN OUT  DWORD *     pnWait
    )
{
    CLUSTER_GROUP_STATE _cgs = ClusterGroupStateUnknown;
    DWORD               _sc = ERROR_SUCCESS;

    if ( pnWait != NULL )
    {
        CClusterNotifyPort _port;       // Wait for a group state change event

        _sc = _port.Create( (HCHANGE) INVALID_HANDLE_VALUE, hCluster );
        if ( _sc == ERROR_SUCCESS )
        {
            _sc = _port.Register( CLUSTER_CHANGE_GROUP_STATE, hGroup );
            if ( _sc == ERROR_SUCCESS )
            {
                while ( *pnWait > 0 )
                {
                    _cgs = WrapGetClusterGroupState( hGroup, NULL );
                    if ( _cgs != ClusterGroupStateUnknown )
                    {
                        //
                        // if the state is ClusterGroupPending then it's
                        // in a pending state and we want to wait for the next notification.
                        //
                        if ( _cgs == ClusterGroupPending )
                        {
                            _port.GetNotify();   // this will only wait for up to 1 second.
                            --(*pnWait);
                        } // if: resource is in pending state
                        else
                        {
                            break;
                        } // else if: resource is no longer in a pending state
                    } // if: WrapClusterResourceState
                    else
                    {
                        _sc = GetLastError();
                        break;
                    } // else: WrapClusterResourceState failed
                } // while: *pnWait > 0
            } // if: port created
            else
            {
                _sc = GetLastError();
            } // else: port registration failed
        } // if: create notification port
    } // if: pnWait not NULL, this is for safety only

    return _sc;

} //*** WaitForResourceGroupStateChange()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  WaitForGroupToQuiesce
//
//  Description:
//      Wait for each of the resources in the group to leave a pending state.
//
//  Arguments:
//      hCluster        [IN]        - handle to the cluster
//      hGroup          [IN]        - handle to the group
//      pnWait          [IN OUT]    - ~ seconds to wait
//
//  Return Value:
//      ERROR_SUCCESS       No resources are pending or pnWait is NULL.
//      ERROR_IO_PENDING    At least one resource is still pending (timeout expired).
//      Win32 error code
//
//--
/////////////////////////////////////////////////////////////////////////////
static DWORD WaitForGroupToQuiesce(
    IN      HCLUSTER    hCluster,
    IN      HGROUP      hGroup,
    IN OUT  DWORD *     pnWait
    )
{
    HGROUPENUM  hEnum = NULL;
    DWORD       _sc = ERROR_SUCCESS;

    if ( pnWait != NULL )
    {
        hEnum = ClusterGroupOpenEnum( hGroup, CLUSTER_GROUP_ENUM_CONTAINS );
        if ( hEnum != NULL)
        {
            CClusterNotifyPort port;        // Wait for a group state change event

            _sc = port.Create( (HCHANGE) INVALID_HANDLE_VALUE, hCluster );
            if ( _sc == ERROR_SUCCESS )
            {
                LPWSTR  pwszName = NULL;
                DWORD   dwIndex = 0;
                DWORD   dwType = 0;

                _sc = port.Register( CLUSTER_CHANGE_GROUP_STATE, hGroup );
                if ( _sc == ERROR_SUCCESS )
                {
                    //
                    //  Enumerate each of the resources in the group.  If WaitForResourceStateChange
                    //  returns ERROR_IO_PENDING then we know that our wait expired and there's
                    //  still a pending resource, so we break.
                    //
                    for ( dwIndex = 0; _sc == ERROR_SUCCESS; dwIndex++ )
                    {
                        _sc = WrapClusterGroupEnum( hEnum, dwIndex, &dwType, &pwszName );
                        if ( _sc == ERROR_NO_MORE_ITEMS )
                        {
                            _sc = ERROR_SUCCESS;
                            break;
                        } // if: WrapClusterGroupEnum out of items -- leave!    we are done...
                        else if ( _sc == ERROR_SUCCESS )
                        {
                            _sc = WaitForResourceStateChange( hCluster, pwszName, &port, pnWait );
                            ::LocalFree( pwszName );
                            pwszName = NULL;
                        } // if: WrapClusterGroupEnum succeeded
                        else
                        {
                            _sc = GetLastError();
                        } // else: WrapClusterGroupEnum failed!
                    } // for: enum the resources in the group
                } // if: notification port registered
                else
                {
                    _sc = GetLastError();
                } // else: port registration failed
            } // if: create notification port

            ClusterGroupCloseEnum( hEnum );
        } // if: ClusterGroupOpenEnum succeeds
        else
        {
            _sc = GetLastError();
        } // else: ClusterGroupOpenEnum failed
    } // if: no wait time....

    return _sc;

} //*** WaitForGroupToQuiesce()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  WaitForResourceToQuiesce
//
//  Description:
//      Wrapper function that is called after OnlineClusterResouce  and
//      OfflineClusterResource that waits for the resource to finish its
//      state change.  Returns the pending state of the resource after the
//      wait period has expired and the state has not changed.
//
//  Arguments:
//      hCluster        [IN]    -   the cluster handle
//      hResource       [IN]    -   the resource handle to take on or offline
//      crsDesiredState [IN]    -   the desired end state of the resource
//      nWait           [IN]    -   ~ how many seconds to wait
//      pbPending       [OUT]   -   true if the resource is in a pending state
//
//  Return Value:
//      ERROR_SUCCESS or Win32 error code
//
//--
/////////////////////////////////////////////////////////////////////////////
static DWORD WaitForResourceToQuiesce(
    IN      HCLUSTER                hCluster,
    IN      HRESOURCE               hResource,
    IN      CLUSTER_RESOURCE_STATE  crsDesiredState,
    IN      DWORD                   nWait,
    OUT     long    *               pbPending
    )
{
    CLUSTER_RESOURCE_STATE  crs = ClusterResourceStateUnknown;
    DWORD                   _sc = ERROR_SUCCESS;
    CClusterNotifyPort      port;        // if wait is specified open a notify port.

    //printf( "Entering..." );

    if ( crsDesiredState >= ClusterResourcePending )
    {
        _sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    } // if:

    //
    //  nWait should never be zero, but leaving this test in is a good idea
    //  since someone may call it with zero someday...
    //

    if ( nWait == 0 )
    {
        //printf( "A wait time was not specified. Returning ClusterResourcePending" );
        crs = ClusterResourcePending;
        goto Cleanup;
    } // if: no wait time was specified...

    _sc = port.Create( (HCHANGE) INVALID_HANDLE_VALUE, hCluster );
    if ( _sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    } // if: port was not created ok

    _sc = port.Register( CLUSTER_CHANGE_RESOURCE_STATE, hResource );
    if ( _sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    } // if: port was not registered ok

    do
    {
        crs = WrapGetClusterResourceState( hResource, NULL, NULL );
        if ( crs == ClusterResourceFailed )
        {
            _sc = ERROR_INVALID_STATE;
            break;
        } // if:
        else if ( crs == ClusterResourceStateUnknown )
        {
            _sc = GetLastError();
            break;
        } // else if:
        else if ( crs == crsDesiredState )
        {
            break;
        } // else if:

        port.GetNotify();       // waits for ~ 1 second

        nWait--;
    } while ( nWait > 0 );

    //printf( "Exiting wait loop. Time remaining is %d seconds and resource state is %d", nWait, crs );

Cleanup:

    //
    // return the pending state if the caller has asked for it
    //

    if ( pbPending != NULL )
    {
        if ( crs >= ClusterResourcePending )
        {
            *pbPending = TRUE;
        } // if: is the resource still in a pending state
    } // if: does the argument exist?

    //printf( "Exiting. sc = %#08x. Pending = %d", _sc, *pbPending );

    return _sc;

} //*** WaitForResourceToQuiesce()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  ScWrapOnlineClusterResource
//
//  Description:
//      Wrapper function for OnlineClusterResouce that returns the pending
//      state of the resource after the wait period has expired.
//
//  Arguments:
//      hCluster    [IN]    - the cluster handle
//      hResource   [IN]    - the resource handle to take on or offline
//      nWait       [IN]    - ~ how many seconds to wait
//      pbPending   [OUT]   - true if the resource is in a pending state
//
//  Return Value:
//      ERROR_SUCCESS or Win32 error code
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD ScWrapOnlineClusterResource(
    IN  HCLUSTER    hCluster,
    IN  HRESOURCE   hResource,
    IN  DWORD       nWait,          //=0
    OUT long *      pbPending       //=NULL
    )
{
    DWORD   _sc = ERROR_SUCCESS;

    //printf( "Entering..." );

    _sc = OnlineClusterResource( hResource );
    if ( _sc == ERROR_IO_PENDING )
    {
        //printf( "OnlineClusterResource() returned ERROR_IO_PENDING." );
        if ( nWait > 0 )
        {
            _sc = WaitForResourceToQuiesce( hCluster, hResource, ClusterResourceOnline, nWait, pbPending );
        } // if:
    } // if: ERROR_IO_PENDING
    else if ( _sc == ERROR_SUCCESS )
    {
        //printf( "OnlineClusterResource() returned ERROR_SUCCESS." );
        if ( pbPending != NULL )
        {
            *pbPending = FALSE;
        }
    } // else if: ERROR_SUCCESS, resource must  be online!

    //printf( "Exiting. sc = %#08x.", _sc );

    return _sc;

} //*** ScWrapOnlineClusterResource()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  HrWrapOnlineClusterResource
//
//  Description:
//      Wrapper function for WrapOnlineClusterResouce
//
//  Arguments:
//      hCluster    [IN]    - the cluster handle
//      hResource   [IN]    - the resource handle to take on or offline
//      nWait       [IN]    - ~ how many seconds to wait
//      pbPending   [OUT]   - true if the resource is in a pending state
//
//  Return Value:
//      S_OK or Win32 error code
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT HrWrapOnlineClusterResource(
    IN  HCLUSTER    hCluster,
    IN  HRESOURCE   hResource,
    IN  DWORD       nWait,          //=0
    OUT long *      pbPending       //=NULL
    )
{
    //printf( "Entering..." );

    DWORD   _sc = ScWrapOnlineClusterResource( hCluster, hResource, nWait, pbPending );

    //printf( "Exiting. sc = %#08x.", _sc );

    return HRESULT_FROM_WIN32( _sc );

} //*** HrWrapOnlineClusterResource()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  ScWrapOfflineClusterResource
//
//  Description:
//      Wrapper function for OfflineClusterResouce that returns the pending
//      state of the resource after the wait period has expired.
//
//  Arguments:
//      hCluster    [IN]    - the cluster handle
//      hResource   [IN]    - the resource handle to take on or offline
//      pnWait      [IN]    - ~ how many seconds to wait
//      pbPending   [OUT]   - true if the resource is in a pending state
//
//  Return Value:
//      ERROR_SUCCESS or Win32 error code
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD ScWrapOfflineClusterResource(
    IN  HCLUSTER    hCluster,
    IN  HRESOURCE   hResource,
    IN  DWORD       nWait,          //=0
    OUT long *      pbPending       //=NULL
    )
{
    DWORD   _sc = ERROR_SUCCESS;

    //printf( "Entering..." );

    _sc = OfflineClusterResource( hResource );
    if ( _sc == ERROR_IO_PENDING )
    {
        //printf( "OfflineClusterResource() returned ERROR_IO_PENDING." );
        if ( nWait > 0 )
        {
            _sc = WaitForResourceToQuiesce( hCluster, hResource, ClusterResourceOffline, nWait, pbPending );
        } // if:
    } // if: ERROR_IO_PENDING
    else if ( _sc == ERROR_SUCCESS )
    {
        //printf( "OfflineClusterResource() returned ERROR_SUCCESS." );
        if ( pbPending != NULL )
        {
            *pbPending = FALSE;
        }
    } // else if: ERROR_SUCCESS, resource must  be online!

    //printf( "Exiting. sc = %#08x.", _sc );

    return _sc;

} //*** ScWrapOfflineClusterResource()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  HrWrapOfflineClusterResource
//
//  Description:
//      Wrapper function for ScWrapOfflineClusterResource
//
//  Arguments:
//      hCluster    [IN]    - the cluster handle
//      hResource   [IN]    - the resource handle to take on or offline
//      pnWait      [IN]    - ~ how many seconds to wait
//      pbPending   [OUT]   - true if the resource is in a pending state
//
//  Return Value:
//      S_OK or Win32 error code
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT HrWrapOfflineClusterResource(
    IN  HCLUSTER    hCluster,
    IN  HRESOURCE   hResource,
    IN  DWORD       nWait,          //=0
    OUT long *      pbPending       //=NULL
    )
{
    //printf( "Entering..." );

    DWORD   _sc = ScWrapOfflineClusterResource( hCluster, hResource, nWait, pbPending );

    //printf( "Exiting. sc = %#08x.", _sc );

    return HRESULT_FROM_WIN32( _sc );

} //*** HrWrapOfflineClusterResource()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  ScWrapOnlineClusterGroup
//
//  Description:
//      Wrapper function for OnlineClusterGroup that returns the pending state
//      of the group after the wait period has expired or the group quiesces.
//
//  Arguments:
//      hCluster    [IN]    - the cluster handle
//      hGroup      [IN]    - the group handle to online
//      hNode       [IN]    - the node the group should be brought online
//      pnWait      [IN]    - ~ how many seconds to wait
//      pbPending   [OUT] - true if the resource group is in a pending state
//
//  Return Value:
//      ERROR_SUCCESS or Win32 error code
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD ScWrapOnlineClusterGroup(
    IN  HCLUSTER    hCluster,
    IN  HGROUP      hGroup,
    IN  HNODE       hNode,          //=NULL
    IN  DWORD       nWait,          //=0
    OUT long *      pbPending       //=NULL
    )
{
    CLUSTER_GROUP_STATE cgs = ClusterGroupStateUnknown;
    DWORD               _sc = ERROR_SUCCESS;
    BOOL                bPending = FALSE;

    _sc = OnlineClusterGroup( hGroup, hNode );
    if ( _sc == ERROR_IO_PENDING )
    {
        //
        // Check the group state before we check the state of the resources. When reporting the
        // group state the cluster API pulls the resource states online and offline pending up
        // to online or offline respectivly.    It also pulls the failed state up to offline.   This
        // means that a group state of online or offline is misleading because one or more
        // resources could be in a pending state.   The only absolute state is PartialOnline, at
        // least one resource is offline (or failed).
        //
        cgs = WrapGetClusterGroupState( hGroup, NULL );
        if ( cgs == ClusterGroupPending )
        {
            _sc = WaitForResourceGroupStateChange( hCluster, hGroup, &nWait );

            cgs = WrapGetClusterGroupState( hGroup, NULL );
            if ( cgs == ClusterGroupPending )
            {
                bPending = TRUE;
            }
        } // if: group state is pending
        else if ( ( cgs == ClusterGroupOnline ) || ( cgs == ClusterGroupPartialOnline ) )
        {
            _sc = WaitForGroupToQuiesce( hCluster, hGroup, &nWait );
            if ( _sc == ERROR_IO_PENDING )
            {
                bPending = TRUE;
                _sc = ERROR_SUCCESS;
            } // if: HrWaitForGroupToQuiesce returned pending
        } // else if: group is online -- we have to check all of the resources, on downlevel clusters...
        else if ( cgs == ClusterGroupStateUnknown )
        {
            _sc = GetLastError();
        } // else if: get group state failed
    } // if: OnlineClusterGroup returned ERROR_IO_PENDING

    //
    // return the pending state if the caller has asked for it
    //
    if ( pbPending != NULL )
    {
        *pbPending = bPending;
    } // if: does the argument exist?

    return _sc;

} //*** ScWrapOnlineClusterGroup()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  HrWrapOnlineClusterGroup
//
//  Description:
//      Wrapper function for ScWrapOnlineClusterGroup
//
//  Arguments:
//      hCluster    [IN]    - the cluster handle
//      hGroup      [IN]    - the group handle to online
//      hNode       [IN]    - the node the group should be brought online
//      pnWait      [IN]    - ~ how many seconds to wait
//      pbPending   [OUT] - true if the resource is in a pending state
//
//  Return Value:
//      S_OK or Win32 error code
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT HrWrapOnlineClusterGroup(
    IN  HCLUSTER    hCluster,
    IN  HGROUP      hGroup,
    IN  HNODE       hNode,          //=NULL
    IN  DWORD       nWait,          //=0
    OUT long *      pbPending       //=NULL
    )
{
    DWORD   _sc = ScWrapOnlineClusterGroup( hCluster, hGroup, hNode, nWait, pbPending );

    return HRESULT_FROM_WIN32( _sc );

} //*** HrWrapOnlineClusterGroup()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  ScWrapOfflineClusterGroup
//
//  Description:
//      Wrapper function for OfflineClusterGroup that returns the pending
//      state of the group after the wait period has expired.
//
//  Arguments:
//      hCluster    [IN]    - the cluster handle
//      hGroup      [IN]    - the group handle to online
//      pnWait      [IN]    - ~ how many seconds to wait
//      pbPending   [OUT]   - true if the resource is in a pending state
//
//  Return Value:
//      ERROR_SUCCESS or Win32 error code
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD ScWrapOfflineClusterGroup(
    IN  HCLUSTER    hCluster,
    IN  HGROUP      hGroup,
    IN  DWORD       nWait,          //=0
    OUT long *      pbPending       //=NULL
    )
{
    CLUSTER_GROUP_STATE cgs = ClusterGroupStateUnknown;
    DWORD               _sc = ERROR_SUCCESS;
    BOOL                bPending = FALSE;

    _sc = OfflineClusterGroup( hGroup );
    if ( _sc == ERROR_IO_PENDING )
    {
        //
        // Check the group state before we check the state of the resources. When reporting the
        // group state the cluster API pulls the resource states online and offline pending up
        // to online or offline respectivly.    It also pulls the failed state up to offline.   This
        // means that a group state of online or offline is misleading because one or more
        // resources could be in a pending state.
        //
        cgs = WrapGetClusterGroupState( hGroup, NULL );
        if ( cgs == ClusterGroupPending )
        {
            _sc = WaitForResourceGroupStateChange( hCluster, hGroup, &nWait );

            cgs = WrapGetClusterGroupState( hGroup, NULL );
            if ( cgs == ClusterGroupPending )
            {
                bPending = TRUE;
            }
        } // if: group state is pending
        else if ( ( cgs == ClusterGroupOffline ) || ( cgs == ClusterGroupPartialOnline ) )
        {
            _sc = WaitForGroupToQuiesce( hCluster, hGroup, &nWait );
            if ( _sc == ERROR_IO_PENDING )
            {
                bPending = TRUE;
                _sc = ERROR_SUCCESS;
            } // if: HrWaitForGroupToQuiesce returned pending
        } // else if: group is offline -- we have to check all of the resources...
        else if ( cgs == ClusterGroupStateUnknown )
        {
            _sc = GetLastError();
        } // else if: get group state failed
    } // if: OfflineClusterGroup returned ERROR_IO_PENDING

    //
    // return the pending state if the caller has asked for it
    //
    if ( pbPending != NULL )
    {
        *pbPending = bPending;
    } // if: does the argument exist?

    return _sc;

} //*** ScWrapOfflineClusterGroup()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  HrWrapOfflineClusterGroup
//
//  Description:
//      Wrapper function for OfflineClusterGroup that returns the pending
//      state of the group after the wait period has expired.
//
//  Arguments:
//      hCluster    [IN]    - the cluster handle
//      hGroup      [IN]    - the group handle to online
//      pnWait      [IN]    - ~ how many seconds to wait
//      pbPending   [OUT]   - true if the resource is in a pending state
//
//  Return Value:
//      S_OK or Win32 error code
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT HrWrapOfflineClusterGroup(
    IN  HCLUSTER    hCluster,
    IN  HGROUP      hGroup,
    IN  DWORD       nWait,          //=0
    OUT long *      pbPending       //=NULL
    )
{
    DWORD   _sc = ScWrapOfflineClusterGroup( hCluster, hGroup, nWait, pbPending );

    return HRESULT_FROM_WIN32( _sc );

} //*** HrWrapOfflineClusterGroup()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  ScWrapMoveClusterGroup
//
//  Description:
//      Wrapper function for MoveClusterGroup that returns the pending state
//      of the group after the wait period has expired.
//
//  Arguments:
//      hCluster        [IN]    - the cluster handle
//      hGroup          [IN]    - the group handle to online
//      hNode           [IN]    - the node the group should be brought online
//      pnWait          [IN]    - ~ how many seconds to wait
//      pbPending       [OUT]   - true if the resource is in a pending state
//
//  Return Value:
//      ERROR_SUCCESS or Win32 error code
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD ScWrapMoveClusterGroup(
    IN  HCLUSTER                hCluster,
    IN  HGROUP                  hGroup,
    IN  HNODE                   hNode,              //=NULL
    IN  DWORD                   nWait,              //=0
    OUT long *                  pbPending           //=NULL
    )
{
    LPWSTR              pszOriginNodeName           = NULL;
    BOOL                bPending                    = FALSE;
    DWORD               _sc;

    CLUSTER_GROUP_STATE cgsInitialState             = ClusterGroupStateUnknown;
    CLUSTER_GROUP_STATE cgsCurrentState             = ClusterGroupStateUnknown;
    LPWSTR              pszCurrentNodeName          = NULL;

    // Get the initial group state.
    cgsInitialState = WrapGetClusterGroupState( hGroup, &pszOriginNodeName );
    if ( cgsInitialState == ClusterGroupStateUnknown )
    {
        // Error getting the group state
        _sc = GetLastError();
        goto Cleanup;
    }

    // Move the cluster group.
    _sc = MoveClusterGroup( hGroup, hNode );

    //
    // When MoveClusterGroup returns ERROR_SUCCESS, it just means that the group
    // has changed ownership successfully, but it does not mean that the group is
    // back to the state it was in before the move. Therefore, we still need to
    // wait, if a wait time is provided. If not, we are done.
    //
    if ( nWait <= 0 )
    {
        goto Cleanup;
    }

    //
    // MoveClusterGroup is not done yet
    //
    if ( _sc == ERROR_IO_PENDING )
    {
        _sc = ERROR_SUCCESS;

        do  // while (nWait > 0)
        {
            //
            // Get the name of the node which currently owns this group.
            //
            cgsCurrentState = WrapGetClusterGroupState( hGroup, &pszCurrentNodeName );
            if ( cgsCurrentState == ClusterGroupStateUnknown )
            {
                // Error getting the group state
                _sc = GetLastError();
                break;
            }

            if ( ClRtlStrICmp( pszOriginNodeName, pszCurrentNodeName ) != 0 )
            {
                //
                // If the current owner node is not the original owner, then the call to
                // move group has succeeded. So quit this loop (we still have to see
                // if the group is stable though)
                //
                break;
            } // if: current owner node is not the same as the original owner node
            else
            {
                //
                // Current owner is the same as the original owner.
                // Wait for one second and check again.
                //
                LocalFree( pszCurrentNodeName );
                pszCurrentNodeName = NULL;      // Required to prevent freeing memory twice
                --nWait;
                Sleep( 1000 );
            } // if: current owner node is the same as the original owner node
        }
        while ( nWait > 0 );

        LocalFree( pszCurrentNodeName );

        //
        // If we ran out of time waiting for MoveClusterGroup to complete, then
        // set the pending flag and quit.
        //
        if ( nWait <= 0 )
        {
            bPending = TRUE;
            goto Cleanup;
        }
    } // if: MoveClusterGroup returned ERROR_IO_PENDING
    else
    {
        cgsCurrentState = WrapGetClusterGroupState( hGroup, NULL );
        if ( cgsCurrentState == ClusterGroupStateUnknown )
        {
            // Error getting the group state
            _sc = GetLastError();
        }
    } // else: MoveClusterGroup returned ERROR_SUCCESS

    //
    // if something went wrong with MoveClusterGroup, while waiting
    // for it to comeplete or with WrapGetClusterGroupState, then quit.
    //
    if ( _sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    //
    // If the state of the group on the destination node is ClusterGroupFailed
    // then there is nothing much we can do.
    //
    if ( cgsCurrentState == ClusterGroupFailed )
    {
        goto Cleanup;
    }

    //
    // Check the group state before we check the state of the resources. When reporting the
    // group state the cluster API of a NT4 node pulls the resource states online and offline
    // pending up to online or offline respectivly. It also pulls the failed state up to offline.
    // This means that a group state of online or offline is misleading because one or more
    // resources could be in a pending state. The only absolute state is PartialOnline, at
    // least one resource is offline (or failed).
    //

    if ( cgsCurrentState == ClusterGroupPending )
    {
        // The current state is pending. So wait for a state change.
        _sc = WaitForResourceGroupStateChange( hCluster, hGroup, &nWait );
    } // if: the group state is pending.
    else
    {
        _sc = WaitForGroupToQuiesce( hCluster, hGroup, &nWait );
    } // else: group state is online, offline or partial online

    if ( _sc == ERROR_SUCCESS )
    {
        bPending = ( nWait == 0 );
    } // if: everything ok so far

 Cleanup:

    LocalFree( pszOriginNodeName );

    //
    // return the pending state if the caller has asked for it
    //
    if ( pbPending != NULL )
    {
        *pbPending = bPending;
    } // if: does the argument exist?

    return _sc;

} //*** ScWrapMoveClusterGroup()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  HrWrapMoveClusterGroup
//
//  Description:
//      Wrapper function for ScWrapMoveClusterGroup that returns the pending state
//      of the group after the wait period has expired.
//
//  Arguments:
//      hCluster        [IN]    - the cluster handle
//      hGroup          [IN]    - the group handle to online
//      hNode           [IN]    - the node the group should be brought online
//      pnWait          [IN]    - ~ how many seconds to wait
//      pbPending       [OUT]   - true if the resource is in a pending state
//
//  Return Value:
//      S_OK or Win32 error code
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT HrWrapMoveClusterGroup(
    IN  HCLUSTER                hCluster,
    IN  HGROUP                  hGroup,
    IN  HNODE                   hNode,              //=NULL
    IN  DWORD                   nWait,              //=0
    OUT long *                  pbPending           //=NULL
    )
{
    DWORD   _sc = ScWrapMoveClusterGroup ( hCluster, hGroup, hNode, nWait, pbPending );

    return HRESULT_FROM_WIN32( _sc );

} //*** HrWrapMoveClusterGroup()


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ScWrapClusterResourceControlGet
//
//  Routine Description:
//      Wrap a call to ClusterResourceControl with a "GET" control code.
//      If lppOutBufferOut is not NULL this will allocate the buffer if
//      needed, Call ClusterResourceControl, and return the out buffer
//      data through the BufferOut parameter.
//
//  Arguments:
//      hResourceIn         Passed to ClusterResourceControl.
//      hHostNodeIn         Optional handle to a node.
//      dwControlCodeIn     Passed to ClusterResourceControl.
//      lpInBufferIn        Passed to ClusterResourceControl.
//      cbInBufferSizeIn    Passed to ClusterResourceControl.
//      lppOutBufferOut     Optional (may be NULL).  On input pointer to a
//                          pointer to receive the buffer.  On output set
//                          to the buffer (if one was allocated).  Use
//                           LocalFree to deallocate this buffer.
//      lpcbBytesReturnedOut Pointer to receive the count of bytes
//                          allocated for the lppOutBufferOut buffer.
//
//
//  Return Value:
//      ERROR_SUCCESS on success.
//          The number of characters written (including NULL) is returned
//          via lpcbBytesReturnedOut.  A pointer to the allocated buffer is
//          returned via lppOutBufferOut.
//
//      Win32 Error code on failure.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
ScWrapClusterResourceControlGet(
      HRESOURCE hResourceIn
    , HNODE     hHostNodeIn
    , DWORD     dwControlCodeIn
    , LPVOID    lpInBufferIn
    , DWORD     cbInBufferSizeIn
    , LPVOID *  lppOutBufferOut
    , LPDWORD   lpcbBytesReturnedOut
      )
{
    DWORD       sc = ERROR_SUCCESS;
    LPVOID      lpOutBuffer = NULL;
    DWORD       cbBytesReturned = 0;

    //
    //  Call it once to get the needed buffer sizes.
    //
    sc = ClusterResourceControl(
              hResourceIn
            , hHostNodeIn
            , dwControlCodeIn
            , lpInBufferIn
            , cbInBufferSizeIn
            , NULL
            , 0
            , &cbBytesReturned
            );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    //
    //  If for some reason we don't need to allocate a buffer then skip that step.
    //
    if ( cbBytesReturned != 0 )
    {
        //
        //  Some codes return the length of a needed string (in bytes).  I think some
        //  don't account for the NULL, but that may be other API's.
        //
        cbBytesReturned += ( 1 * sizeof( WCHAR ) );

        lpOutBuffer = (PVOID) LocalAlloc( LMEM_ZEROINIT, cbBytesReturned );
        if ( lpOutBuffer == NULL )
        {
            sc = ERROR_OUTOFMEMORY;
            goto Cleanup;
        }

        //
        //  Do the real call.
        //
        sc = ClusterResourceControl(
                  hResourceIn
                , hHostNodeIn
                , dwControlCodeIn
                , lpInBufferIn
                , cbInBufferSizeIn
                , lpOutBuffer
                , cbBytesReturned
                , &cbBytesReturned
                );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }
    }

    //
    //  If the caller wanted the number of bytes returned.
    //
    if ( lpcbBytesReturnedOut != NULL )
    {
        *lpcbBytesReturnedOut = cbBytesReturned;
    }

    //
    //  If the caller actually wanted the out buffer contents.
    //
    if ( lppOutBufferOut != NULL )
    {
        *lppOutBufferOut = lpOutBuffer;
    }
    else
    {
        // Avoid a memory leak.
        LocalFree( lpOutBuffer );
    }

Cleanup:

    return sc;

} //*** ScWrapClusterResourceControlGet

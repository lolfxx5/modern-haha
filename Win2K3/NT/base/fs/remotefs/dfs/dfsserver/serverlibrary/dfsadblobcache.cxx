//
//  Copyright (C) 2000, Microsoft Corporation
//
//  File:       DfsADBlobCache.cxx
//
//  Contents:   the ADBlob DFS Store class, this contains the 
//              old AD blob store specific functionality. 
//
//  Classes:    DfsADBlobCache.
//
//  History:    Dec. 8 2000,   Author: udayh
//              April 9 2001  Rohanp - Added  AD specific code.
//
//-----------------------------------------------------------------------------
  
#include <DfsAdBlobCache.hxx>
#include <dfsrootfolder.hxx>
#include <dfserror.hxx>
#include "dfsadsiapi.hxx"
#include <sddl.h>
#include <dfssecurity.h>
#include <ntdsapi.h>
#include "dfsAdBlobCache.tmh"


DfsADBlobCache::DfsADBlobCache(
    DFSSTATUS *pStatus, 
    PUNICODE_STRING pShareName,
    DfsRootFolder * pRootFolder) :  DfsGeneric(DFS_OBJECT_TYPE_ADBLOB_CACHE)
{
    SHASH_FUNCTABLE FunctionTable;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DFSSTATUS Status = ERROR_SUCCESS;
    
    *pStatus = ERROR_SUCCESS;

    do {
        m_pBlob = NULL;
        m_pRootBlob = NULL;
        m_pTable = NULL;

        m_pAdHandle = NULL;
        m_AdReferenceCount = 0;
        m_RefreshCount = 0;
        m_ErrorOccured = DFS_DS_NOERROR;

        m_pRootFolder = pRootFolder;

        RtlInitUnicodeString(&m_LogicalShare, NULL);
        RtlInitUnicodeString(&m_ObjectDN, NULL);

        ZeroMemory(&FunctionTable, sizeof(FunctionTable));
        
        FunctionTable.AllocFunc = AllocateShashData;
        FunctionTable.FreeFunc = DeallocateShashData;

        ZeroMemory(&m_BlobAttributePktGuid, sizeof(GUID));
        
        m_fCritInit = InitializeCriticalSectionAndSpinCount( &m_Lock, DFS_CRIT_SPIN_COUNT);
        if(!m_fCritInit)
        {
            Status = GetLastError();
            break;
        }
        
        Status = DfsCreateUnicodeString(&m_LogicalShare, pShareName);

        if (Status != ERROR_SUCCESS)
        {
            break;
        }   

        Status = SetupObjectDN();

        if(Status != ERROR_SUCCESS)
        {
            if(DfsServerGlobalData.bDfsAdAlive)
            {
                DfsServerGlobalData.bDfsAdAlive = FALSE;
                DfsLogDfsEvent(DFS_ERROR_ACTIVEDIRECTORY_OFFLINE, 0, NULL, 0); 
            }
            break;
        }
        
        
        if(DfsServerGlobalData.bDfsAdAlive == FALSE)
        {
            DfsServerGlobalData.bDfsAdAlive = TRUE;
            DfsLogDfsEvent(DFS_INFO_ACTIVEDIRECTORY_ONLINE, 0, NULL, 0); 
        }

        NtStatus = ShashInitHashTable(&m_pTable, &FunctionTable);
        if (NtStatus != STATUS_SUCCESS)
        {
            Status = RtlNtStatusToDosError(NtStatus);
            break;
        }
        
    } while (FALSE);

    *pStatus = Status;
    
    return;
}


DfsADBlobCache::~DfsADBlobCache() 
{

    if(m_pBlob)
    {
        DeallocateShashData(m_pBlob);
        m_pBlob = NULL;
    }
    if(m_pRootBlob)
    {
        DeallocateShashData(m_pRootBlob);
        m_pRootBlob = NULL;
    }

    if (m_pTable != NULL)
    {
        InvalidateCache();
        ShashTerminateHashTable(m_pTable);
        m_pTable = NULL;
    }

    if (m_LogicalShare.Buffer != NULL)
    {
        delete [] m_LogicalShare.Buffer;
        m_LogicalShare.Buffer = NULL;
    }
    if (m_ObjectDN.Buffer != NULL)
    {
        delete [] m_ObjectDN.Buffer;
        m_ObjectDN.Buffer = NULL;
    }

    if(m_fCritInit)
    {
        DeleteCriticalSection(&m_Lock);
        m_fCritInit = FALSE;
    }

}    





#ifndef DFS_USE_LDAP


DFSSTATUS
DfsADBlobCache::UpdateCacheWithDSBlob(
    PVOID pHandle )
{
    HRESULT hr = S_OK;
    DFSSTATUS Status = ERROR_SUCCESS;
    BYTE *pBuffer = NULL;
    ULONG Length = 0;
    VARIANT BinaryBlob;

    IADs *pADs  = (IADs *)pHandle;

    DFS_TRACE_LOW( ADBLOB, "Cache %p: updating cache with blob \n", this);
    VariantInit(&BinaryBlob);

    hr = pADs->Get(ADBlobAttribute, &BinaryBlob);
    if ( SUCCEEDED(hr) )
    {
        Status = GetBinaryFromVariant( &BinaryBlob, &pBuffer, &Length );
        if (Status == ERROR_SUCCESS)
        {
            Status = UnpackBlob( pBuffer, &Length, NULL );

            DFS_TRACE_ERROR_LOW( Status, REFERRAL_SERVER, "Unpack blob done with status %x\n", Status);
            
            delete [] pBuffer;
        }
    }
    else
    {
        Status = DfsGetErrorFromHr(hr);
    }
    VariantClear(&BinaryBlob);

    DFS_TRACE_ERROR_LOW( Status, ADBLOB, "cache %p: Updated cache (length %x) status %x\n", 
                         this, Length, Status);

    return Status;
}

DFSSTATUS
DfsADBlobCache::GetObjectPktGuid( 
    PVOID pHandle,
    GUID *pGuid )
{
    HRESULT hr = S_OK;
    DFSSTATUS Status = ERROR_SUCCESS;
    BYTE *pBuffer = NULL;
    ULONG Length = 0;
    VARIANT BinaryBlob;

    IADs *pADs = (IADs *)pHandle;

    DFS_TRACE_LOW( ADBLOB, "Cache %p: getting pkt guid\n", this);

    VariantInit(&BinaryBlob);

    hr = pADs->Get(ADBlobPktGuidAttribute, &BinaryBlob);
    if ( SUCCEEDED(hr) )
    {
        Status = GetBinaryFromVariant( &BinaryBlob, &pBuffer, &Length );
        if (Status == ERROR_SUCCESS)
        {
            if (Length > sizeof(GUID)) Length = sizeof(GUID);

            RtlCopyMemory( pGuid, pBuffer, Length);
            
            delete [] pBuffer;
        }
    }
    VariantClear(&BinaryBlob);

    Status = DfsGetErrorFromHr(hr);

    DFS_TRACE_ERROR_LOW( Status, ADBLOB, "cache %p: got pkt guid, Status %x\n", 
                         this, Status );


    return Status;
}


DFSSTATUS 
DfsADBlobCache::UpdateDSBlobFromCache(
    PVOID pHandle,
    GUID *pGuid )
{
    HRESULT HResult = S_OK;
    DFSSTATUS Status = ERROR_SUCCESS;
    BYTE *pBuffer = NULL;
    ULONG Length;
    ULONG UseLength;
    ULONG TotalBlobBytes = 0;
    VARIANT BinaryBlob;
    IADs *pObject = (IADs *)pHandle;

    VariantInit(&BinaryBlob);

    DFS_TRACE_LOW( ADBLOB, "Cache %p: updating ds with cache \n", this);

    UseLength = ADBlobDefaultBlobPackSize;
retry:
    Length = UseLength;
    pBuffer =  (BYTE *) HeapAlloc(GetProcessHeap(), 0, 
                                  Length );
    if(pBuffer != NULL)
    {
        Status = PackBlob(pBuffer, &Length, &TotalBlobBytes); 
        if(Status == STATUS_SUCCESS)
        {
            Status = PutBinaryIntoVariant(&BinaryBlob, pBuffer, TotalBlobBytes);
            if(Status == STATUS_SUCCESS)
            {
                HResult = pObject->Put(ADBlobAttribute, BinaryBlob);
                if (SUCCEEDED(HResult) )
                {
                    HResult = pObject->SetInfo();
                }

                Status = DfsGetErrorFromHr(HResult);

                if (Status == ERROR_SUCCESS)
                {
                    Status = SetObjectPktGuid( pObject, pGuid );
                }
            }
        }
        DFS_TRACE_ERROR_LOW(Status, ADBLOB, "Cache %p: update ds (Buffer Len %x, Length %x) Status %x\n",
                            this, UseLength, Length, Status);

        HeapFree(GetProcessHeap(), 0, pBuffer);

        if (Status == ERROR_BUFFER_OVERFLOW)
        {
            if (UseLength < ADBlobMaximumBlobPackSize)
            {
                UseLength *= 2;
            }
            //
            // If we are still within the maximum bounds, retry.
            //
            if (UseLength <= ADBlobMaximumBlobPackSize)
            {
                goto retry;
            }
        }
    }
    else
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    VariantClear(&BinaryBlob);
    DFS_TRACE_ERROR_LOW(Status, ADBLOB, "Cache %p: update ds done (Buffer Length %x, Length %x) Status %x\n",
                        this, UseLength, Length, Status);

    return Status;
}


DFSSTATUS
DfsADBlobCache::SetObjectPktGuid( 
    IADs *pObject,
    GUID *pGuid )
{
    HRESULT HResult = S_OK;
    DFSSTATUS Status = ERROR_SUCCESS;
    VARIANT BinaryBlob;

    DFS_TRACE_LOW( ADBLOB, "Cache %p: setting pkt guid\n", this);
    VariantInit(&BinaryBlob);

    Status = PutBinaryIntoVariant( &BinaryBlob, (PBYTE)pGuid, sizeof(GUID));
    if (Status == ERROR_SUCCESS)
    {
        HResult = pObject->Put(ADBlobPktGuidAttribute, BinaryBlob);
        if (SUCCEEDED(HResult) )
        {
            HResult = pObject->SetInfo();
        }

        Status = DfsGetErrorFromHr(HResult);
    }

    VariantClear(&BinaryBlob);

    DFS_TRACE_ERROR_LOW( Status, ADBLOB, "cache %p: set pkt guid, Status %x\n", 
                         this, Status );


    return Status;
}

#else


DFSSTATUS
DfsADBlobCache::DfsLdapConnect(
    LPWSTR DCName,
    LDAP **ppLdap )
{
    LDAP *pLdap = NULL;
    LPWSTR ActualDC = NULL;
    DFSSTATUS Status = ERROR_SUCCESS;
    DFSSTATUS LocalStatus = ERROR_SUCCESS;
    LONG PreviousState = DFS_DS_ACTIVE;
    const TCHAR * apszSubStrings[4];

    apszSubStrings[0] = DCName;
    ActualDC = DCName;

    pLdap = ldap_initW(DCName, LDAP_PORT);
    if (pLdap != NULL)
    {
        Status = ldap_set_option(pLdap, LDAP_OPT_AREC_EXCLUSIVE, LDAP_OPT_ON);

        if (Status == LDAP_SUCCESS)
        {
            UNICODE_STRING DnsDomain;
            if (DfsGetDnsDomainName( &DnsDomain) == ERROR_SUCCESS)
            {
                Status = ldap_set_option(pLdap, LDAP_OPT_DNSDOMAIN_NAME, &DnsDomain.Buffer);

                DfsReleaseDomainName(&DnsDomain);
            }
        }
        if (Status == LDAP_SUCCESS)
        {

            DFS_TRACE_LOW(ADBLOB, 
                          "Ldap Connect, calling LDAP Bind DC: %ws, init ldap status %x\n",
                           DCName, Status);

            Status = ldap_bind_s(pLdap, NULL, NULL, LDAP_AUTH_NEGOTIATE);


            DFS_TRACE_LOW(ADBLOB, 
                          "Ldap Connect, returning from LDAP Bind DC: %ws, init ldap status %x\n",
                          DCName, Status);
        }
    }
    else
    {
        Status = LdapGetLastError();
        DFS_TRACE_ERROR_HIGH(Status, ADBLOB, 
                             "Ldap Connect, DC: %ws, init ldap status %x\n",
                             DCName, Status);
    }

    if (Status == LDAP_SUCCESS)
    {
        *ppLdap = pLdap;

        if(DCName == NULL)
        {

            //Get the actual host name that we connected to. We don't have to free
            //this name since it will get freed when ldap_unbind is called.
            LocalStatus = ldap_get_optionW(pLdap, LDAP_OPT_HOST_NAME , (void *) &ActualDC);
            apszSubStrings[0] = ActualDC;
        }

        DFS_TRACE_HIGH(ADBLOB, "ADBLOBConnected to actual DC: %ws, ldap status %x\n",
                       ActualDC, Status);

        PreviousState = InterlockedCompareExchange(&DfsServerGlobalData.FirstContact, DFS_DS_ACTIVE, DFS_DS_NOTACTIVE); 
        if(PreviousState == DFS_DS_NOTACTIVE)
        {
            DfsLogDfsEvent(DFS_INFO_DS_RECONNECTED, 1, apszSubStrings, Status);
        }
    }
    else
    {
        PreviousState = InterlockedCompareExchange(&DfsServerGlobalData.FirstContact, DFS_DS_NOTACTIVE, DFS_DS_ACTIVE); 
        if(PreviousState == DFS_DS_ACTIVE)
        {
            DfsLogDfsEvent(DFS_ERROR_DSCONNECT_FAILED, 1, apszSubStrings, Status); 
        }

        DFS_TRACE_ERROR_HIGH(Status, ADBLOB, 
                             "Ldap Connect, DC: %ws, ldap status %x\n",
                             DCName, Status);

        Status = LdapMapErrorToWin32(Status);
    }


    if (Status != ERROR_SUCCESS) 
    {
        if (pLdap != NULL)
        {
            //
            // 565302
            // we need to call ldap_unbind if we have a valid connection!
            // Note that this is true even if ldap_bind failed or we never
            // even called ldap_bind.
            //
            ldap_unbind(pLdap);
        }
    }

    return Status;

}



VOID
DfsADBlobCache::DfsLdapDisconnect(
    LDAP *pLdap )
{
    ldap_unbind(pLdap);
}


DFSSTATUS
DfsADBlobCache::DfsGetPktBlob(
    LDAP *pLdap,
    LPWSTR ObjectDN,
    PVOID *ppBlob,
    PULONG pBlobSize,
    PVOID *ppHandle,
    PVOID *ppHandle1 )
{

    DFSSTATUS Status = ERROR_SUCCESS;
    DFSSTATUS LdapStatus = ERROR_SUCCESS;
    PLDAPMessage pLdapSearchedObject = NULL;
    PLDAPMessage pLdapObject = NULL;
    PLDAP_BERVAL *pLdapPktAttr = NULL;
    LPWSTR Attributes[2];
    struct l_timeval Timeout;



    Status = ERROR_ACCESS_DENIED;     // fix this after we understand
                                      // ldap error correctly. When
                                      // ldap_get_values_len returns NULL
                                      // the old code return no more mem.

    Attributes[0] = ADBlobAttribute;
    Attributes[1] = NULL;

    Timeout.tv_sec = DfsServerGlobalData.LdapTimeOut;


    DFS_TRACE_LOW(ADBLOB, 
                 "DfsGetPktBlob, calling LDAP Search ObjectDN: %ws, init ldap status %x\n",
                  ObjectDN, Status);
    LdapStatus = ldap_search_ext_sW( pLdap,
                                     ObjectDN,
                                     LDAP_SCOPE_BASE,
                                     L"(objectClass=*)",
                                     Attributes,
                                     0,            // attributes only
                                     NULL,         // server controls
                                     NULL,         // client controls
                                     &Timeout,
                                     0,            // size limit
                                     &pLdapSearchedObject);

    DFS_TRACE_LOW(ADBLOB, 
                 "DfsGetPktBlob, returning from LDAP Search ObjectDN: %ws, init ldap status %x\n",
                  ObjectDN, Status);
    if (LdapStatus == LDAP_SUCCESS)
    {
        pLdapObject = ldap_first_entry( pLdap,
                                        pLdapSearchedObject );

        if (pLdapObject != NULL)
        {
            pLdapPktAttr = ldap_get_values_len( pLdap,
                                                pLdapObject,
                                                Attributes[0] );
            if (pLdapPktAttr != NULL)
            {
                *ppBlob = pLdapPktAttr[0]->bv_val;
                *pBlobSize = pLdapPktAttr[0]->bv_len;

                *ppHandle = (PVOID)pLdapPktAttr;
                *ppHandle1 = (PVOID)pLdapSearchedObject;
                pLdapSearchedObject = NULL;

                Status = ERROR_SUCCESS;
            }
            else
            {
                if(pLdap->ld_errno == LDAP_NO_SUCH_ATTRIBUTE)
                {
                    Status = ERROR_ACCESS_DENIED;
                }
                else
                {
                    Status = LdapMapErrorToWin32( pLdap->ld_errno );
                }


                DFS_TRACE_ERROR_HIGH( Status, ADBLOB, "DfsADBlobCache::DfsGetPktBlob1 (ldap error %x) status %x\n", 
                                      pLdap->ld_errno, Status);
            }
        }
    }
    else
    {
        Status = LdapMapErrorToWin32(LdapStatus);

        DFS_TRACE_ERROR_HIGH( Status, ADBLOB, "DfsADBlobCache::DfsGetPktBlob2 (ldap error %x) status %x\n", 
                              LdapStatus, Status);
    }

    if (pLdapSearchedObject != NULL)
    {
        ldap_msgfree( pLdapSearchedObject );
    }

    return Status;
}

VOID
DfsADBlobCache::DfsReleasePktBlob(
    PVOID pHandle,
    PVOID pHandle1 )
{
    PLDAP_BERVAL *pLdapPktAttr = (PLDAP_BERVAL *)pHandle;
    PLDAPMessage pLdapSearchedObject = (PLDAPMessage)pHandle1;

    ldap_value_free_len( pLdapPktAttr );
    ldap_msgfree( pLdapSearchedObject );
}


DFSSTATUS
DfsADBlobCache::UpdateCacheWithDSBlob(
    PVOID pHandle )

{
    DFSSTATUS Status = ERROR_SUCCESS;
    BYTE *pBuffer = NULL;
    ULONG Length = 0;
    PVOID pHandle1, pHandle2;

    LDAP *pLdap = (LDAP *)pHandle;
    UNICODE_STRING ObjectDN;

    
    DFS_TRACE_LOW( ADBLOB, "Cache %p: updating cache with blob \n", this);

    Status = GetObjectDN(&ObjectDN);

    if (Status == ERROR_SUCCESS)
    {
        Status = DfsGetPktBlob( pLdap,
                                ObjectDN.Buffer,
                                (PVOID *)&pBuffer,
                                &Length,
                                &pHandle1,
                                &pHandle2);
    }

    if ( Status == ERROR_SUCCESS)
    {
        Status = UnpackBlob( pBuffer, &Length, NULL );

        DFS_TRACE_ERROR_LOW( Status, REFERRAL_SERVER, "Unpack blob done with status %x\n", Status);

        DfsReleasePktBlob( pHandle1, pHandle2);
    }

    DFS_TRACE_ERROR_LOW( Status, ADBLOB, "cache %p: Updated cache (length %x) status %x\n", 
                         this, Length, Status);

    return Status;
}




DFSSTATUS
DfsADBlobCache::GetObjectPktGuid(
    PVOID pHandle,
    GUID *pGuid )
{

    DFSSTATUS Status = ERROR_SUCCESS;
    DFSSTATUS LdapStatus = ERROR_SUCCESS;
    PLDAPMessage pLdapSearchedObject = NULL;
    PLDAPMessage pLdapObject = NULL;
    PLDAP_BERVAL *pLdapGuidAttr = NULL;
    ULONG CopySize = 0;    
    LDAP *pLdap = (LDAP *)pHandle;
    UNICODE_STRING ObjectDN;
    struct l_timeval Timeout;
    LPWSTR Attributes[2];

    Status = GetObjectDN(&ObjectDN);
    if (Status != ERROR_SUCCESS)
    {
        return Status;
    }

    Status = ERROR_ACCESS_DENIED;     // fix this after we understand
                                      // ldap error correctly. When
                                      // ldap_get_values_len returns NULL
                                      // the old code return no more mem.


    Attributes[0] = ADBlobPktGuidAttribute;
    Attributes[1] = NULL;

    Timeout.tv_sec = DfsServerGlobalData.LdapTimeOut;



    DFS_TRACE_LOW(ADBLOB, 
                 "DfsGetPktGuid, calling LDAP Search - ldap status %x\n",
                  Status);
    LdapStatus = ldap_search_ext_sW( pLdap,
                                     ObjectDN.Buffer,
                                     LDAP_SCOPE_BASE,
                                     L"(objectClass=*)",
                                     Attributes,
                                     0,            // attributes only
                                     NULL,         // server controls
                                     NULL,         // client controls
                                     &Timeout,
                                     0,            // size limit
                                     &pLdapSearchedObject);

    DFS_TRACE_LOW(ADBLOB, 
                 "DfsGetPktGuid, returning from LDAP Search - ldap status %x\n",
                  Status);

    if (LdapStatus == LDAP_SUCCESS)
    {
        pLdapObject = ldap_first_entry( pLdap,
                                        pLdapSearchedObject );

        if (pLdapObject != NULL)
        {
            pLdapGuidAttr = ldap_get_values_len( pLdap,
                                                 pLdapObject,
                                                 Attributes[0] );
            if (pLdapGuidAttr != NULL)
            {
                CopySize = min( pLdapGuidAttr[0]->bv_len, sizeof(GUID));
                RtlCopyMemory( pGuid, pLdapGuidAttr[0]->bv_val, CopySize );

                ldap_value_free_len( pLdapGuidAttr );
                Status = ERROR_SUCCESS;
            }
            else
            {
                if(pLdap->ld_errno == LDAP_NO_SUCH_ATTRIBUTE)
                {
                    Status = ERROR_ACCESS_DENIED;
                }
                else
                {
                    Status = LdapMapErrorToWin32( pLdap->ld_errno );
                }


                DFS_TRACE_ERROR_HIGH( Status, ADBLOB, "DfsADBlobCache::GetObjectPktGuid1 (ldap error %x) status %x\n", 
                                      LdapStatus, Status);
            }
        }
    }
    else
    {
        Status = LdapMapErrorToWin32(LdapStatus);

        DFS_TRACE_ERROR_HIGH( Status, ADBLOB, "DfsADBlobCache::GetObjectPktGuid2 (ldap error %x) status %x\n", 
                              pLdap->ld_errno, Status);
    }

    if (pLdapSearchedObject != NULL)
    {
        ldap_msgfree( pLdapSearchedObject );
    }

    return Status;
}

DFSSTATUS
DfsADBlobCache::DfsSetPktBlobAndPktGuid ( 
    LDAP *pLdap,
    LPWSTR ObjectDN,
    PVOID pBlob,
    ULONG BlobSize,
    GUID *pGuid )
{
    LDAP_BERVAL  LdapPkt, LdapPktGuid;
    PLDAP_BERVAL  pLdapPktValues[2], pLdapPktGuidValues[2];
    LDAPModW LdapPktMod, LdapPktGuidMod;
    PLDAPModW pLdapDfsMod[3];
    DFSSTATUS Status = ERROR_SUCCESS;
    DFSSTATUS LdapStatus;

    LDAPControlW    LazyCommitControl =
                    {
                        LDAP_SERVER_LAZY_COMMIT_OID_W,  // the control
                        { 0, NULL},                     // no associated data
                        FALSE                           // control isn't mandatory
                    };

    PLDAPControlW   ServerControls[2] =
                    {
                        &LazyCommitControl,
                        NULL
                    };


    LdapPkt.bv_len = BlobSize;
    LdapPkt.bv_val = (PCHAR)pBlob;
    pLdapPktValues[0] = &LdapPkt;
    pLdapPktValues[1] = NULL;
    LdapPktMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    LdapPktMod.mod_type = ADBlobAttribute;
    LdapPktMod.mod_vals.modv_bvals = pLdapPktValues;

    LdapPktGuid.bv_len = sizeof(GUID);
    LdapPktGuid.bv_val = (PCHAR)pGuid;
    pLdapPktGuidValues[0] = &LdapPktGuid;
    pLdapPktGuidValues[1] = NULL;
    LdapPktGuidMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    LdapPktGuidMod.mod_type = ADBlobPktGuidAttribute;
    LdapPktGuidMod.mod_vals.modv_bvals = pLdapPktGuidValues;

    pLdapDfsMod[0] = &LdapPktMod;
    pLdapDfsMod[1] = &LdapPktGuidMod;
    pLdapDfsMod[2] = NULL;


    DFS_TRACE_LOW(ADBLOB, 
                 "DfsSetPktBlobAndPktGuid, calling LDAP ldap_modify_ext_sW ObjectDN: %ws, init ldap status %x\n",
                  ObjectDN, Status);
    LdapStatus = ldap_modify_ext_sW( pLdap,
                                     ObjectDN,
                                     pLdapDfsMod,
                                     (PLDAPControlW *)ServerControls,
                                     NULL );

    DFS_TRACE_LOW(ADBLOB, 
                 "DfsSetPktBlobAndPktGuid, returning from LDAP ldap_modify_ext_sW ObjectDN: %ws, init ldap status %x\n",
                  ObjectDN, Status);

    if (LdapStatus != LDAP_SUCCESS)
    {
        Status = LdapMapErrorToWin32(LdapStatus);
    }



    DFS_TRACE_ERROR_LOW( Status, ADBLOB, "DfsADBlobCache::DfsSetPktBlobAndPktGuid cache %p: LDAP Status %x, Win32 status %x\n", 
                         this, LdapStatus, Status);
    return Status;

}

DFSSTATUS 
DfsADBlobCache::UpdateDSBlobFromCache(
    PVOID pHandle,
    GUID *pGuid )
{
    LDAP *pLdap = (LDAP *)pHandle;
    UNICODE_STRING ObjectDN;
    BYTE *pBuffer = NULL;
    ULONG Length = 0;
    ULONG UseLength = 0;
    ULONG TotalBlobBytes = 0;
    DFSSTATUS Status = ERROR_SUCCESS;

    Status = GetObjectDN(&ObjectDN);
    if (Status != ERROR_SUCCESS)
    {
        DFS_TRACE_ERROR_LOW( Status, ADBLOB, "DfsADBlobCache::UpdateDSBlobFromCache cache %p: GetObjectDN failed status %x\n", 
                             this, Status);
        return Status;
    }

    UseLength = ADBlobDefaultBlobPackSize;
retry:
    Length = UseLength;
    pBuffer =  (BYTE *) HeapAlloc(GetProcessHeap(), 0, Length );
    if(pBuffer != NULL)
    {
        Status = PackBlob(pBuffer, &Length, &TotalBlobBytes); 
        if (Status != ERROR_SUCCESS)
        {
            HeapFree(GetProcessHeap(), 0, pBuffer);

            if (Status == ERROR_BUFFER_OVERFLOW)
            {
                if (UseLength < ADBlobMaximumBlobPackSize)
                {
                    UseLength *= 2;
                }
                goto retry;
            }
        }

        if(Status == STATUS_SUCCESS)
        {

            Status = DfsSetPktBlobAndPktGuid( pLdap,
                                              ObjectDN.Buffer,
                                              pBuffer,
                                              TotalBlobBytes,
                                              pGuid );
            DFS_TRACE_ERROR_LOW(Status, ADBLOB, "Cache %p: DfsADBlobCache::UpdateDSBlobFromCache (Buffer Len %x, Length %x) Status %x\n",
                                this, UseLength, Length, Status);


            HeapFree(GetProcessHeap(), 0, pBuffer);
        }
    }
    else
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    return Status;
}
#endif

DFSSTATUS
DfsADBlobCache::CacheRefresh(BOOLEAN fForceSync,
                             BOOLEAN fFromPDC)

{
    PVOID           pHandle = NULL;
    DFSSTATUS       Status = ERROR_SUCCESS;
    LONG            PreviousState = DFS_DS_NOERROR;
    LONG            RefreshCount = 0;
    PUNICODE_STRING pUseShare = NULL;
    GUID            CurrentGuid;
    const TCHAR * apszSubStrings[4];

    LPWSTR UseDC = NULL;
    DfsString *pPDC = NULL;
    DFSSTATUS PDCStatus = ERROR_SUCCESS;



    DFS_TRACE_LOW( ADBLOB, "Cache %p: from pdc? %d cache refresh\n", this, fFromPDC);

    if (fFromPDC == TRUE) 
    {
        PDCStatus = DfsGetBlobPDCName( &pPDC, 0 );
        //
        // Ignore the status here. If this call fails, we dont care
        // since we will just use a non-pdc dc. All callers who 
        // *really* care have already got the object with the right dc.
        // see apiprologue for more details.
        //
        if (PDCStatus == ERROR_SUCCESS) 
        {
            UseDC = pPDC->GetString();
        }
    }

    DFS_TRACE_LOW(ADBLOB, "Cache refresh using dc %ws\n", UseDC);

    Status = GetADObject( &pHandle, UseDC);
    
    if ((Status != ERROR_SUCCESS) &&
        (UseDC != NULL))
    {
        //
        // hmmm. failed here going to pdc. Just get the object without bothering
        // about the PDC.
        //
        Status = GetADObject(&pHandle, NULL);

    }

    if (Status == ERROR_SUCCESS)
    {
        // fFromPDC == FALSE means root scalability mode..every so often, get a new
        // Blob and guid
        if(fFromPDC == FALSE)
        {
            RefreshCount = InterlockedIncrement(&m_RefreshCount);
            if(RefreshCount >= DFS_ROOTSCALABILTY_FORCED_REFRESH_INTERVAL)
            {
                fForceSync = TRUE;
                InterlockedExchange((LPLONG volatile ) &m_RefreshCount, 0);
            }
        }

        Status = GetObjectPktGuid( pHandle, &CurrentGuid );

        if(Status == ERROR_SUCCESS)
        {
            if(!fForceSync)
            {
                //
                // here we pass the 2 guids by reference...
                //
                if (IsEqualGUID( CurrentGuid, m_BlobAttributePktGuid) == FALSE)
                {
                    Status = UpdateCacheWithDSBlob( pHandle );
                    if (Status == ERROR_SUCCESS)
                    {
                        m_BlobAttributePktGuid = CurrentGuid;
                    }
                }
            }
            else
            {
                Status = UpdateCacheWithDSBlob( pHandle );
                if (Status == ERROR_SUCCESS)
                {
                    m_BlobAttributePktGuid = CurrentGuid;
                }
            }
        }

        ReleaseADObject( pHandle );

    }

    DfsReleaseBlobPDCName( pPDC );

    if(Status != ERROR_SUCCESS)
    {
        PreviousState = InterlockedCompareExchange(&m_ErrorOccured, DFS_DS_ERROR, DFS_DS_NOERROR); 
        if(PreviousState == DFS_DS_NOERROR)
        {
            pUseShare = m_pRootFolder->GetRootPhysicalShareName();
            apszSubStrings[0] = pUseShare->Buffer;
            DfsLogDfsEvent(DFS_ERROR_NO_DFS_DATA, 1, apszSubStrings, Status);
        }
    }
    else
    {
        PreviousState = InterlockedCompareExchange(&m_ErrorOccured, DFS_DS_NOERROR, DFS_DS_ERROR); 
        if(PreviousState == DFS_DS_ERROR)
        {
            pUseShare  = m_pRootFolder->GetRootPhysicalShareName();
            apszSubStrings[0] = pUseShare->Buffer;
            DfsLogDfsEvent(DFS_INFO_RECONNECT_DATA, 1, apszSubStrings, Status);
        }

    }
    
    DFS_TRACE_ERROR_LOW( Status, ADBLOB, "Cache %p: CacheRefresh status 0x%x\n", this, Status);
    return Status;
}



void 
DfsADBlobCache::InvalidateCache()
{
    PDFSBLOB_DATA pBlobData;
    DFSBOB_ITER Iter;
    DFS_TRACE_LOW( ADBLOB, "Cache %p: invalidate cache\n", this);
    pBlobData = FindFirstBlob(&Iter);
    while (pBlobData != NULL)
    {
        DFSSTATUS RemoveStatus;

        RemoveStatus = RemoveNamedBlob(&pBlobData->BlobName);
        DFS_TRACE_ERROR_LOW( RemoveStatus, REFERRAL_SERVER, "BlobCache %p, invalidate cache, remove blob status %x\n",
                             this, RemoveStatus);
        pBlobData = FindNextBlob(&Iter);
    }

    FindCloseBlob(&Iter);
    DFS_TRACE_LOW( ADBLOB, "Cache %p: invalidate cache done\n", this);
}

DFSSTATUS
DfsADBlobCache::PutBinaryIntoVariant(VARIANT * ovData, BYTE * pBuf,
                                     unsigned long cBufLen)
{
     DFSSTATUS Status = ERROR_INVALID_PARAMETER;
     void * pArrayData = NULL;
     VARIANT var;
     SAFEARRAYBOUND  rgsabound[1];

     VariantInit(&var);  //Initialize our variant

     var.vt = VT_ARRAY | VT_UI1;

     rgsabound[0].cElements = cBufLen;
     rgsabound[0].lLbound = 0;

     var.parray = SafeArrayCreate(VT_UI1,1,rgsabound);

     if(var.parray != NULL)
     {
        //Get a safe pointer to the array
        SafeArrayAccessData(var.parray,&pArrayData);

        //Copy bitmap to it
        memcpy(pArrayData, pBuf, cBufLen);

        //Unlock the variant data
        SafeArrayUnaccessData(var.parray);

        *ovData = var;  

        Status = STATUS_SUCCESS;
     }
     else
     {
         Status = ERROR_NOT_ENOUGH_MEMORY;
        DFS_TRACE_HIGH( REFERRAL_SERVER, "PutBinaryIntoVariant failed error %d\n", Status);
     }


     return Status;
}



DFSSTATUS
DfsADBlobCache::GetBinaryFromVariant(VARIANT *ovData, BYTE ** ppBuf,
                                     unsigned long * pcBufLen)
{
     DFSSTATUS Status = ERROR_INVALID_PARAMETER;
     void * pArrayData = NULL;

     //Binary data is stored in the variant as an array of unsigned char
     if(ovData->vt == (VT_ARRAY|VT_UI1))  
     {
        //Retrieve size of array
        *pcBufLen = ovData->parray->rgsabound[0].cElements;

        *ppBuf = new BYTE[*pcBufLen]; //Allocate a buffer to store the data
        if(*ppBuf != NULL)
        {
            //Obtain safe pointer to the array
            SafeArrayAccessData(ovData->parray,&pArrayData);

            //Copy the bitmap into our buffer
            memcpy(*ppBuf, pArrayData, *pcBufLen);

            //Unlock the variant data
            SafeArrayUnaccessData(ovData->parray);

            Status = ERROR_SUCCESS;
        }
        else
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
        }
     }

     return Status;
}


DFSSTATUS 
DfsADBlobCache::CacheFlush(
    PVOID pHandle )
{
    DFSSTATUS Status = ERROR_SUCCESS;
    GUID NewGuid;

    IADs *pObject  = (IADs *)pHandle;

    DFS_TRACE_LOW( ADBLOB, "Cache %p: cache flush\n", this);
    Status = UuidCreate(&NewGuid);
    if (Status == ERROR_SUCCESS)
    {
        m_BlobAttributePktGuid = NewGuid;

        Status = UpdateDSBlobFromCache( pObject,
                                        &NewGuid );

    }
    DFS_TRACE_ERROR_LOW( Status, ADBLOB, "Cache %p: cache flush, Status %x\n", this, Status);
    return Status;
}


DFSSTATUS
DfsADBlobCache::UnpackBlob(
    BYTE *pBuffer,
    PULONG pLength,
    PDFSBLOB_DATA * pRetBlob )
{
    ULONG Discard = 0;
    DFSSTATUS Status = ERROR_SUCCESS;
    BYTE *pUseBuffer = NULL;
    ULONG BufferSize = 0;
    ULONG ObjNdx = 0;
    ULONG TotalObjects = 0;
    ULONG  BlobSize = 0;
    BYTE *BlobBuffer = NULL;
    PDFSBLOB_DATA pLocalBlob = NULL;
    UNICODE_STRING BlobName;
    UNICODE_STRING SiteRoot;
    UNICODE_STRING BlobRoot;
     
    pUseBuffer = pBuffer;
    BufferSize = *pLength;

    DFS_TRACE_LOW( ADBLOB, "BlobCache %p, UnPackBlob \n", this);
    UNREFERENCED_PARAMETER(pRetBlob);

    Status = DfsRtlInitUnicodeStringEx( &SiteRoot, ADBlobSiteRoot );
    if (Status != ERROR_SUCCESS)
    {
        goto done;
    }

    Status = DfsRtlInitUnicodeStringEx( &BlobRoot, ADBlobMetaDataNamePrefix);
    if (Status != ERROR_SUCCESS)
    {
        goto done;
    }

    //
    // Note the size of the whole blob.
    //
    m_BlobSize = BufferSize;
    
    //
    // dfsdev: we should not need an interlocked here: this code
    // is already mutex'd by the caller.
    //
    InterlockedIncrement( &m_CurrentSequenceNumber );
    
    //
    // dfsdev: investigate what the first ulong is and add comment
    // here as to why we are discarding it.
    //
    Status = PackGetULong( &Discard, (PVOID *) &pUseBuffer, &BufferSize ); 
    if (Status != ERROR_SUCCESS)
    {
        goto done;
    }

    if(BufferSize == 0)
    {
        goto done;
    }

    Status = PackGetULong(&TotalObjects, (PVOID *) &pUseBuffer, &BufferSize);
    if (Status != ERROR_SUCCESS) 
    {
        goto done;
    }


    for (ObjNdx = 0; ObjNdx < TotalObjects; ObjNdx++)
    {
        BOOLEAN FoundSite = FALSE;
        BOOLEAN FoundRoot = FALSE;

        Status = GetSubBlob( &BlobName,
                             &BlobBuffer,
                             &BlobSize,
                             &pUseBuffer,
                             &BufferSize );

        if (Status == ERROR_SUCCESS)
        {
            if (!FoundSite &&
               (RtlCompareUnicodeString( &BlobName, &SiteRoot, TRUE ) == 0))
            {
               FoundSite = TRUE;
               Status = CreateBlob(&BlobName, 
                                   BlobBuffer, 
                                   BlobSize,
                                   &pLocalBlob
                                   );
               if(Status == STATUS_SUCCESS)
               {
                   DeallocateShashData(m_pBlob);
                   m_pBlob = pLocalBlob;
               }

                continue;
            }
            if (!FoundRoot &&
               (RtlCompareUnicodeString( &BlobName, &BlobRoot, TRUE ) == 0))
            {
               FoundRoot = TRUE;
               UNICODE_STRING RootName;
               RtlInitUnicodeString(&RootName, NULL);
               Status = CreateBlob(&RootName,
                                   BlobBuffer, 
                                   BlobSize,
                                   &pLocalBlob
                                   );
               if(Status == STATUS_SUCCESS)
               {
                   DeallocateShashData(m_pRootBlob);
                   m_pRootBlob = pLocalBlob;
               }

                continue;
            }

            Status = StoreBlobInCache( &BlobName, BlobBuffer, BlobSize);
            if (Status != ERROR_SUCCESS)
            {
                break;
            }
        }
    }


done:

    DFS_TRACE_ERROR_LOW( Status, ADBLOB, "BlobCache %p: UnPackBlob status %x\n", this, Status);
    return Status;
}


DFSSTATUS
DfsADBlobCache::CreateBlob(PUNICODE_STRING BlobName, 
                           PBYTE pBlobBuffer, 
                           ULONG BlobSize,
                           PDFSBLOB_DATA *pNewBlob )
{
    PDFSBLOB_DATA BlobStructure = NULL;
    DFSSTATUS Status = ERROR_SUCCESS;
    PBYTE NextBufOffset;

    ULONG TotalSize = sizeof(DFSBLOB_DATA) + 
                      BlobName->Length + sizeof(WCHAR) +
                      BlobSize;

    BlobStructure = (PDFSBLOB_DATA) AllocateShashData( TotalSize );

    if (BlobStructure != NULL)
    {
        RtlZeroMemory(BlobStructure, sizeof(DFSBLOB_DATA));
        NextBufOffset = (PBYTE)(BlobStructure + 1);

        BlobStructure->Header.RefCount = 1;
        BlobStructure->Header.pvKey = &BlobStructure->BlobName;
        BlobStructure->Header.pData = (PVOID)BlobStructure;

        BlobStructure->SequenceNumber = m_CurrentSequenceNumber;        
        BlobStructure->BlobName.Length = BlobName->Length;
        BlobStructure->BlobName.MaximumLength = BlobName->Length + sizeof(WCHAR);
        BlobStructure->BlobName.Buffer = (WCHAR *) (NextBufOffset);

        NextBufOffset = (PBYTE)((ULONG_PTR)(NextBufOffset) + 
                                BlobName->Length + 
                                sizeof(WCHAR));

        if (BlobName->Length != 0)
        {
            RtlCopyMemory(BlobStructure->BlobName.Buffer, 
                          BlobName->Buffer, 
                          BlobName->Length);
        }
        BlobStructure->BlobName.Buffer[BlobName->Length/sizeof(WCHAR)] = UNICODE_NULL;


        BlobStructure->Size = BlobSize;    
        BlobStructure->pBlob = (PBYTE)(NextBufOffset);

        if(pBlobBuffer && BlobSize)
        {
            RtlCopyMemory(BlobStructure->pBlob, pBlobBuffer, BlobSize);
        }
    }
    else
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    *pNewBlob = BlobStructure;

    return Status;
}

DFSSTATUS
DfsADBlobCache::CreateSiteBlobIfNecessary(void)
{
    DFSSTATUS Status = ERROR_SUCCESS;
    PDFSBLOB_DATA pSiteBlob = NULL;
    PBYTE pBuffer = NULL;
    PVOID pUseBuffer = NULL;
    ULONG SiteBlobSize = 0;
    ULONG SizeRemaining = 0;
    GUID NewGuid;
    UNICODE_STRING SiteMetadataName;

    if(m_pBlob == NULL)
    {
        do
        {
            Status = UuidCreate(&NewGuid);
            if (Status != ERROR_SUCCESS)
            {
              break;
            }

            Status = DfsRtlInitUnicodeStringEx(&SiteMetadataName, ADBlobSiteRoot);
            if(Status != ERROR_SUCCESS)
            {
                break;
            }

            SiteBlobSize = sizeof(ULONG) + sizeof(GUID);

            pBuffer = (PBYTE) AllocateShashData (SiteBlobSize);
            if (pBuffer == NULL)
            {
                Status = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            pUseBuffer = pBuffer;
            SizeRemaining = SiteBlobSize;

            Status = PackSetGuid( &NewGuid, &pUseBuffer, &SizeRemaining);
            if (Status != ERROR_SUCCESS)
            {
                break;
            }

            Status = PackSetULong( 0, &pUseBuffer, &SizeRemaining );
            if (Status != ERROR_SUCCESS)
            {
                break;
            }

            Status = CreateBlob(&SiteMetadataName, 
                                pBuffer, 
                                SiteBlobSize,
                                &pSiteBlob);
            if(Status != ERROR_SUCCESS)
             {
                break;
             }

            m_pBlob = pSiteBlob;

        }while(0);
    }

    if (pBuffer) 
    {
       DeallocateShashData(pBuffer);
    }

    return Status;

}

//
// UpdateSiteBlob: This routine takes a binary stream and stuffs that
// as the site information. It gets rid of the old site information.
// No update is done here, the caller has to call us back to say
// that data needs to be written out.
//
//
DFSSTATUS
DfsADBlobCache::UpdateSiteBlob(PVOID pBuffer, ULONG Size)
{
    DFSSTATUS Status = ERROR_SUCCESS;
    PDFSBLOB_DATA pSiteBlob = NULL;
    UNICODE_STRING SiteMetadataName;

    Status = DfsRtlInitUnicodeStringEx(&SiteMetadataName, ADBlobSiteRoot);
    if(Status == ERROR_SUCCESS)
    {
        Status = CreateBlob(&SiteMetadataName, 
                            (PBYTE)pBuffer,
                            Size,
                            &pSiteBlob);
        if(Status == ERROR_SUCCESS)
        {
            if (m_pBlob != NULL)
            {
                DeallocateShashData(m_pBlob);
            }
            m_pBlob = pSiteBlob;
        }
    }

    return Status;

}


DFSSTATUS
DfsADBlobCache::StoreBlobInCache(PUNICODE_STRING BlobName, 
                                 PBYTE pBlobBuffer, 
                                 ULONG BlobSize)
{
    PDFSBLOB_DATA BlobStructure = NULL;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DFSSTATUS Status = ERROR_SUCCESS;

    DFS_TRACE_LOW( ADBLOB, "cache %p: Storing Blob %wZ in cache, size %x\n",
                   this, BlobName, BlobSize );

    Status = CreateBlob( BlobName,
                         pBlobBuffer,
                         BlobSize,
                         &BlobStructure );

    if (Status == ERROR_SUCCESS)
    {
        if (IsEmptyString(BlobStructure->BlobName.Buffer))
        {
            ReleaseRootBlob();
            m_pRootBlob = BlobStructure;
        }
        else
        {
            NtStatus = SHashInsertKey(m_pTable, 
                                      BlobStructure, 
                                      &BlobStructure->BlobName, 
                                      SHASH_REPLACE_IFFOUND);
            if(NtStatus == STATUS_SUCCESS)
            {
                InterlockedDecrement(&BlobStructure->Header.RefCount);
            }
            else
            {
                DeallocateShashData( BlobStructure );
                Status = RtlNtStatusToDosError(NtStatus);
            }
        }
    }
    DFS_TRACE_LOW( ADBLOB, "cache %p: storing Blob %wZ done, status %x\n",
                   this, BlobName, Status);

    return Status;
}




DFSSTATUS
DfsADBlobCache::WriteBlobToAd(
    BOOLEAN ForceFlush)
{
    DFSSTATUS Status = STATUS_SUCCESS;
    PVOID pHandle = NULL;

    DFS_TRACE_LOW(ADBLOB, "cache %p: writing blob to ad\n", this);

    //
    //  ForceFlush flag is currently implemented as a DirectMode
    //  specific functionality. Essentially all normal flushes that happen
    //  during direct-mode operations are no-ops unless the ForceFlush
    //  flag is specified. All regular mode operations are left unaffected.
    //
    if (DfsCheckDirectMode() && !ForceFlush) 
    {
        DFS_TRACE_LOW(ADBLOB, "cache %p: WriteBlobToAd is a NOOP\n", this);
        return Status;
    }

    Status = GetCachedADObject ( &pHandle );

    if(Status == ERROR_SUCCESS)
    {
        Status = CacheFlush(pHandle);

        ReleaseADObject( pHandle );
    }
    DFS_TRACE_ERROR_LOW(Status, ADBLOB, "cache %p: writing blob to ad, status %x\n",
                        this, Status);

    return Status;

}



//+-------------------------------------------------------------------------
//
//  Function:   GetSubBlob
//
//  Arguments:    
//
//   PUNICODE_STRING pBlobName (name of the sub blob)
//   BYTE **ppBlobBuffer - holds pointer to sub blob buffer
//   PULONG  pBlobSize -  holds the blob size
//   BYTE **ppBuffer -  holds the pointer to the main blob buffer 
//   PULONG pSize  - holds size of the main blob stream.
//
//  Returns:    Status: Success or Error status code
//
//  Description: This routine reads the next stream in the main blob, and
//               returns all the information necessary to unravel the
//               sub blob held within the main blob,
//               It adjusts the main blob buffer and size appropriately
//               to point to the next stream or sub-blob within the main
//               blob.
//
//--------------------------------------------------------------------------

DFSSTATUS
DfsADBlobCache::GetSubBlob(
    PUNICODE_STRING pName,
    BYTE **ppBlobBuffer,
    PULONG  pBlobSize,
    BYTE **ppBuffer,
    PULONG pSize )
{
    DFSSTATUS Status = ERROR_SUCCESS;

    //
    // ppbuffer is the main blob, and it is point to a stream at this
    // point
    // the first part is the name of the sub-blob.
    //
    Status = PackGetString( pName, (PVOID *) ppBuffer, pSize );
    if (Status == ERROR_SUCCESS)
    {
        //
        // now get the size of the sub blob.
        //
        Status = PackGetULong( pBlobSize, (PVOID *) ppBuffer, pSize );
        if (Status == ERROR_SUCCESS)
        {
            if(*pBlobSize > *pSize)
            {
                Status = ERROR_INVALID_DATA ;
            }
            else
            {

                //
                // At this point the main blob is point to the sub-blob itself.
                // So copy the pointer of the main blob so we can return it
                // as the sub blob.
                //
                *ppBlobBuffer = *ppBuffer;

                //
                // update the main blob pointer to point to the next stream
                // in the blob.

                *ppBuffer = (BYTE *)*ppBuffer + *pBlobSize;
                *pSize -= *pBlobSize;
            }
        }
    }

    return Status;
}


DFSSTATUS
DfsADBlobCache::GetNamedBlob(PUNICODE_STRING pBlobName,
                             PDFSBLOB_DATA *pBlobStructure)
{
    DFSSTATUS Status = ERROR_SUCCESS;
    NTSTATUS NtStatus = STATUS_SUCCESS;

    if (IsEmptyString(pBlobName->Buffer))
    {
        *pBlobStructure = AcquireRootBlob();
        if (*pBlobStructure == NULL)
        {
            Status = ERROR_NOT_FOUND;
        }
    }
    else
    {
        NtStatus = SHashGetDataFromTable(m_pTable, 
                                         (void *)pBlobName,
                                         (void **) pBlobStructure);
        Status = RtlNtStatusToDosError(NtStatus);
    }

    return Status;
}


DFSSTATUS
DfsADBlobCache::SetNamedBlob(PDFSBLOB_DATA pBlobStructure)
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DFSSTATUS Status = ERROR_SUCCESS;

    if (IsEmptyString(pBlobStructure->BlobName.Buffer))
    {
        ReleaseRootBlob();
        m_pRootBlob = pBlobStructure;
        AcquireRootBlob();
    }
    else
    {
        NtStatus = SHashInsertKey(m_pTable, 
                                  pBlobStructure, 
                                  &pBlobStructure->BlobName, 
                                  SHASH_REPLACE_IFFOUND);

        Status = RtlNtStatusToDosError(NtStatus);
    }
    
    return Status;
}

DFSSTATUS
DfsADBlobCache::RemoveNamedBlob(PUNICODE_STRING pBlobName )
{
    NTSTATUS NtStatus;
    DFSSTATUS Status = ERROR_SUCCESS;

    if (IsEmptyString(pBlobName->Buffer))
    {
        if (m_pRootBlob == NULL)
        {
            Status = ERROR_NOT_FOUND;
        }
        ReleaseRootBlob();
    }
    else
    {
        NtStatus = SHashRemoveKey(m_pTable, 
                                  pBlobName,
                                  NULL );

        Status = RtlNtStatusToDosError(NtStatus);
    }
    
    return Status;

}

DWORD 
PackBlobEnumerator( PSHASH_HEADER   pEntry,
                    void*  pContext ) 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUNICODE_STRING pBlobName = (PUNICODE_STRING) pEntry->pvKey;
    PDFSBLOB_DATA pBlobStructure = (PDFSBLOB_DATA) pEntry;
    PPACKBLOB_ENUMCTX pEnumCtx = (PPACKBLOB_ENUMCTX) pContext;

    if (pBlobStructure->SequenceNumber == pEnumCtx->SequenceNumber)
    {
        Status = AddStreamToBlob( pBlobName,
                                  pBlobStructure->pBlob,
                                  pBlobStructure->Size,
                                  &pEnumCtx->pBuffer,
                                  &pEnumCtx->Size );

    
        pEnumCtx->NumItems++;
        pEnumCtx->CurrentSize += (pBlobName->Length + 
                                  sizeof(USHORT) +
                                  sizeof(ULONG) +
                                  pBlobStructure->Size);
    }
    return Status;
}

//
// skeleton of pack blob. 
// The buffer is passed in. The length is passed in.
// If the information does not fit the buffer, return required length.
// Required for API implementation.
//
//
DFSSTATUS
DfsADBlobCache::PackBlob(
    BYTE *pBuffer,
    PULONG pLength,
    PULONG TotalBlobBytes )

{
    ULONG Discard = 0;
    DFSSTATUS Status = ERROR_SUCCESS;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BYTE *pUseBuffer = NULL;
    BYTE *pSavedBuffer = NULL;
    ULONG SavedBufferSize = 0;
    ULONG BufferSize = 0;
    PACKBLOB_ENUMCTX EnumCtx;
     
    pUseBuffer = pBuffer;
    BufferSize = *pLength;

    DFS_TRACE_LOW(ADBLOB, "BlobCache %p: packing blob\n", this);
    //
    // dfsdev: investigate what the first ulong is and add comment
    // here as to why we are setting it to 0.
    //
    Status = PackSetULong( 0, (PVOID *) &pUseBuffer, &BufferSize ); 
    if (Status == ERROR_SUCCESS)
    {

        //save the place where we should write back the number of objects
        pSavedBuffer = pUseBuffer;
        SavedBufferSize = BufferSize;
        //
        // the next argument is the number of objects in the blob.
        // set 0 until we find how many blobs there are
        //
        Status = PackSetULong(0, (PVOID *) &pUseBuffer, &BufferSize);
    }

    if (Status == ERROR_SUCCESS) 
    {

        EnumCtx.pBuffer = pUseBuffer;
        EnumCtx.Size = BufferSize;
        EnumCtx.NumItems = 0;
        EnumCtx.SequenceNumber = m_CurrentSequenceNumber;
        EnumCtx.CurrentSize = sizeof(ULONG);
    }

    if (Status == ERROR_SUCCESS)
    {
        if (m_pRootBlob != NULL)
        {
            PDFSBLOB_DATA pRootBlob;
            UNICODE_STRING RootMetadataName;

            Status = DfsRtlInitUnicodeStringEx(&RootMetadataName, ADBlobMetaDataNamePrefix);
            if(Status == ERROR_SUCCESS)
            {
                Status = CreateBlob( &RootMetadataName,
                                     m_pRootBlob->pBlob,
                                     m_pRootBlob->Size,
                                     &pRootBlob);

                if (Status == ERROR_SUCCESS)
                {
                    Status = PackBlobEnumerator( (SHASH_HEADER *)pRootBlob, (PVOID) &EnumCtx);    

                    DeallocateShashData(pRootBlob);
                }
            }

        }
    }


    
    if (Status == ERROR_SUCCESS)
    {
        NtStatus = ShashEnumerateItems(m_pTable, 
                                       PackBlobEnumerator, 
                                       &EnumCtx);

        //dfsdev: make sure that shash enumerate retuns NTSTATUS.
        // it does not appear to do so... I think it is the packblobenumerator
        // that is returning a non-ntstatus. Till we fix it dont convert err

        //    Status = RtlNtStatusToDosError(NtStatus);
        Status = NtStatus;
    }

    //
    // Now add the site blob as the LAST blob. It appears that old
    // downlevel dfs may rely on this.
    //
    if (Status == ERROR_SUCCESS) 
    {
        CreateSiteBlobIfNecessary();

        if (m_pBlob != NULL)
        {
            Status = PackBlobEnumerator( (SHASH_HEADER *) m_pBlob, (PVOID) &EnumCtx);    
        }
    }

    if (Status == ERROR_SUCCESS)
    {

        if (EnumCtx.NumItems > 0)
        {
            Status = PackSetULong(EnumCtx.NumItems, 
                                  (PVOID *) &pSavedBuffer, 
                                  &SavedBufferSize);

            EnumCtx.CurrentSize += sizeof(ULONG);
        }

        *TotalBlobBytes = EnumCtx.CurrentSize;
    }

    m_BlobSize = *TotalBlobBytes;
    //*TotalBlobBytes =  (ULONG) (EnumCtx.pBuffer -  pBuffer);

    DFS_TRACE_ERROR_LOW( Status, ADBLOB, "BlobCache %p, PackBlob status %x\n",
                         this, Status);
    return Status;
}

ULONG
DfsADBlobCache::GetBlobSize()
{
    return m_BlobSize;
}



PDFSBLOB_DATA
DfsADBlobCache::FindFirstBlob(PDFSBLOB_ITER pIter)
{
    PDFSBLOB_DATA pBlob = NULL;

    pIter->RootReferenced = AcquireRootBlob();
    if (pIter->RootReferenced != NULL) 
    {
        pIter->Started = FALSE;
        pBlob = pIter->RootReferenced;
    }
    else
    {
        pIter->Started = TRUE;
        pBlob = (PDFSBLOB_DATA) SHashStartEnumerate(&pIter->Iter, m_pTable);
    }

    return pBlob;
}


PDFSBLOB_DATA
DfsADBlobCache::FindNextBlob(PDFSBLOB_ITER pIter)
{
    PDFSBLOB_DATA pBlob = NULL;

    if (pIter->Started == FALSE)
    {
        pIter->Started = TRUE;
        pBlob = (PDFSBLOB_DATA) SHashStartEnumerate(&pIter->Iter, m_pTable);
    }
    else
    {
        pBlob = (PDFSBLOB_DATA) SHashNextEnumerate(&pIter->Iter, m_pTable);
    }

    return pBlob;
}



void
DfsADBlobCache::FindCloseBlob(PDFSBLOB_ITER pIter)
{

    if (pIter->RootReferenced)
    {
        ReleaseBlobReference(pIter->RootReferenced);
        pIter->RootReferenced = NULL;
    }
    if (pIter->Started)
    {
        SHashFinishEnumerate(&pIter->Iter, m_pTable);    
        pIter->Started = FALSE;
    }
}


DFSSTATUS
AddStreamToBlob(PUNICODE_STRING BlobName,
                BYTE *pBlobBuffer,
                ULONG  BlobSize,
                BYTE ** pUseBuffer,
                ULONG *BufferSize )
{
    DFSSTATUS Status = ERROR_SUCCESS;

    Status = PackSetString(BlobName, (PVOID *) pUseBuffer, BufferSize);
    if(Status == ERROR_SUCCESS)
    {
         Status = PackSetULong(BlobSize, (PVOID *) pUseBuffer, BufferSize);
         if(Status == ERROR_SUCCESS)
         {
             if ( *BufferSize >= BlobSize )
             {
                 RtlCopyMemory((*pUseBuffer), pBlobBuffer, BlobSize);

                 *pUseBuffer = (BYTE *)((ULONG_PTR)*pUseBuffer + BlobSize);
                 *BufferSize -= BlobSize;
             }
             else
             {
                 Status = ERROR_BUFFER_OVERFLOW;
             }
         }

    }
    if (Status == ERROR_INVALID_DATA)
    {
        Status = ERROR_BUFFER_OVERFLOW;
    }

    return Status;
}

PVOID
AllocateShashData(ULONG Size )
{
    PVOID RetValue = NULL;

    if (Size)
    {
        RetValue = (PVOID) new BYTE[Size];
    }
    return RetValue;
}

VOID
DeallocateShashData(PVOID pPointer )
{
    if(pPointer)
    {
        delete [] (PBYTE)pPointer;
    }
}

DFSSTATUS 
DfsADBlobCache::DfsDoesUserHaveAccess(DWORD DesiredAccess)
{
    DFSSTATUS Status = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR pSD = NULL;
    PVOID pHandle = NULL;
    UNICODE_STRING ObjectDN;
    
    DFS_TRACE_LOW(ADBLOB, "BlobCache %p: DfsADBlobCache::DfsDoesUserHaveAccess\n", this);

    do
    {

        Status = GetCachedADObject ( &pHandle );
        if(Status != ERROR_SUCCESS)
        {
            DFS_TRACE_ERROR_HIGH( Status, ADBLOB, "DfsADBlobCache::DfsDoesUserHaveAccess: GetCachedADObject failed status %x\n", 
                                 Status);
            break;
        }

        Status = GetObjectDN(&ObjectDN);
        if(Status != ERROR_SUCCESS)
        {
            DFS_TRACE_ERROR_HIGH( Status, ADBLOB, "DfsADBlobCache::DfsDoesUserHaveAccess: GetObjectDN failed status %x\n", 
                                 Status);
            break;
        }


        Status = DfsGetObjSecurity((LDAP *) pHandle,
                                   ObjectDN.Buffer,
                                   &pSD);

        if(Status != ERROR_SUCCESS)
        {
            DFS_TRACE_ERROR_HIGH( Status, ADBLOB, "DfsADBlobCache::DfsDoesUserHaveAccess: DfsGetObjectSecurity failed status %x\n", 
                                 Status);
            break;
        }

        Status = DfsDoesUserHaveDesiredAccessToAd(DesiredAccess, 
                                                  pSD);
        if(Status != ERROR_SUCCESS)
        {
            DFS_TRACE_ERROR_HIGH( Status, ADBLOB, "DfsADBlobCache::DfsDoesUserHaveAccess: DfsDoesUserHaveDesiredAccessToAd failed status %x\n", 
                                 Status);
            break;
        }

    }while (0);

    if(pHandle != NULL)
    {
        ReleaseADObject( pHandle );
    }

    if(pSD)
    {
        DfsDeallocateSecurityData (pSD);
    }

    return Status;
}


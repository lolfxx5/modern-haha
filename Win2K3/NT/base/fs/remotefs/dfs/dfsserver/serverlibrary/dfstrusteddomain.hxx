
//+----------------------------------------------------------------------------
//
//  Copyright (C) 2001, Microsoft Corporation
//
//  File:       DfsTrustedDomain.hxx
//
//  Contents:   the Dfs trusted domain info class
//
//  Classes:    Dfstrusteddomain
//
//  History:    apr. 8 2001,   Author: udayh
//
//-----------------------------------------------------------------------------


#ifndef __DFS_TRUSTED_DOMAIN__
#define __DFS_TRUSTED_DOMAIN__

#include "DfsGeneric.hxx"
#include "dsgetdc.h"
#include "lm.h"
#include "DfsReferralData.hxx"

enum DfsTrustedDomainDcLoadState
{
    DfsTrustedDomainDcLoadStateUnknown = 0,
    DfsTrustedDomainDcLoaded,
    DfsTrustedDomainDcNotLoaded,
    DfsTrustedDomainDcLoadInProgress,
    DfsTrustedDomainDcLoadFailed,
};    

class DfsTrustedDomain
{
private:
    UNICODE_STRING _DomainName;
    UNICODE_STRING _BindDomainName;
    BOOLEAN _Netbios;
    BOOLEAN _UseBindDomain;
    DfsReferralData *_pDcReferralData; // The loaded referral data
    DfsTrustedDomainDcLoadState _LoadState;
    CRITICAL_SECTION *_pLock;
    DFSSTATUS _LoadStatus;
    ULONG _RetryFailedLoadTimeout;
    
private:
    DFSSTATUS
    LoadDcReferralData(
        IN DfsReferralData *pReferralData );

public:


    DfsTrustedDomain()
    {
        RtlInitUnicodeString( &_DomainName, NULL );
        RtlInitUnicodeString( &_BindDomainName, NULL );

        _pDcReferralData = NULL;
        _pLock = NULL;
        _Netbios = FALSE;
        _UseBindDomain = FALSE;

        _LoadState = DfsTrustedDomainDcNotLoaded;
        _LoadStatus = 0;
        _RetryFailedLoadTimeout = 0;
    }


    ~DfsTrustedDomain()
    {

        DfsFreeUnicodeString( &_DomainName );

        if (_pDcReferralData != NULL)
        {
            _pDcReferralData->ReleaseReference();
            _pDcReferralData = NULL;
        }

    }

    VOID
    Initialize( CRITICAL_SECTION *pLock)
    {
        _pLock = pLock;

        return NOTHING;
    }

    DFSSTATUS
    AcquireLock()
    {
        return DfsAcquireLock( _pLock );
    }

    VOID
    ReleaseLock()
    {
        return DfsReleaseLock( _pLock );
    }


    DFSSTATUS
    SetDomainName( 
        LPWSTR Name,
        BOOLEAN Netbios )
    {
        DFSSTATUS Status;

        Status = DfsCreateUnicodeStringFromString( &_DomainName,
                                                   Name );
        _Netbios = Netbios;
        return Status;
    }
    
    DFSSTATUS
    SetDomainName( 
        PUNICODE_STRING Name,
        BOOLEAN Netbios )
    {
        DFSSTATUS Status;

        Status = DfsCreateUnicodeString( &_DomainName,
                                         Name );
        _Netbios = Netbios;
        return Status;
    }

    DFSSTATUS
    SetBindDomainName( PUNICODE_STRING pName )
    {
        DFSSTATUS Status;

        Status = DfsCreateUnicodeString( &_BindDomainName,
                                         pName );
        if (Status == ERROR_SUCCESS)
        {
            _UseBindDomain = TRUE;
        }
        return Status;
    }

    
    PUNICODE_STRING
    GetDomainName()
    {
        return &_DomainName;
    }




    //
    // Function GetReferralData: Returns the referral data for this
    // c;ass, and indicates if the referral data was cached or needed
    // to be loaded.
    //
    DFSSTATUS
    GetDcReferralData( OUT DfsReferralData **ppReferralData,
                       OUT BOOLEAN *pCacheHit );


    BOOLEAN
    IsMatchingDomainName(
        PUNICODE_STRING pName )
    {
        BOOLEAN ReturnValue = FALSE;

        if (_DomainName.Length == pName->Length)
        {
            if (_wcsnicmp( pName->Buffer, 
                           _DomainName.Buffer, 
                           (pName->Length/sizeof(WCHAR)) ) == 0)
            {
                ReturnValue = TRUE;
            }
        }
        return ReturnValue;
    }
    
    BOOLEAN
    IsTimeToRetry(VOID)
    {
        ULONG CurrentTime = GetTickCount();

        if ((CurrentTime > _RetryFailedLoadTimeout) &&
            ((CurrentTime - _RetryFailedLoadTimeout) > DfsServerGlobalData.RetryFailedReferralLoadInterval))
        {
            return TRUE;
        }
        
        // overflow
        if ((CurrentTime < _RetryFailedLoadTimeout) &&
            ((CurrentTime - 0) + (0xFFFFFFFF - _RetryFailedLoadTimeout) > DfsServerGlobalData.RetryFailedReferralLoadInterval))
        {
            return TRUE;
        }

        return FALSE;
    }
    
    DFSSTATUS
    RemoveDcReferralData(
        DfsReferralData *pRemoveReferralData,
        PBOOLEAN pRemoved );

};

#endif  __DFS_TRUSTED_DOMAIN__

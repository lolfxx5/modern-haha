////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2000, Microsoft Corporation.
//
//  All rights reserved.
//
//	Module Name:
//
//					wmiadapter_stuff.h
//
//	Abstract:
//
//					declaration of stuff for performance refresh
//
//	History:
//
//					initial		a-marius
//
////////////////////////////////////////////////////////////////////////////////////

#ifndef	__ADAPTER_STUFF_H__
#define	__ADAPTER_STUFF_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

// include
#include "wmi_reverse_memory_ext.h"
#include "wmi_perf_data.h"

#include "RefresherStuff.h"
#include "WMIAdapter_Stuff_Refresh.h"

//enum
#include <refreshergenerate.h>

///////////////////////////////////////////////////////////////////////////////
//  storing handles of counters of object
///////////////////////////////////////////////////////////////////////////////
class WmiRefreshObject
{
	DECLARE_NO_COPY ( WmiRefreshObject );

	public:

	LONG*	m_pHandles;

	WmiRefreshObject():
		m_pHandles ( NULL )
	{
	};

	~WmiRefreshObject()
	{
		delete [] m_pHandles;
		m_pHandles = NULL;
	};
};

///////////////////////////////////////////////////////////////////////////////
// performance refreshing
///////////////////////////////////////////////////////////////////////////////
template < class WmiRefreshParent >
class WmiRefresh;

///////////////////////////////////////////////////////////////////////////////
// adapter stuff
///////////////////////////////////////////////////////////////////////////////
class WmiAdapterStuff
{
	DECLARE_NO_COPY ( WmiAdapterStuff );

	// wmi :))
	WmiPerformanceData										m_data;		// data helper
	WmiMemoryExt < WmiReverseMemoryExt<WmiReverseGuard> >	m_pMem;		// shared memory

	WmiRefresh < WmiAdapterStuff > * m_pWMIRefresh;

	public:

	WmiRefresherStuff	m_Stuff;

	// construction & destruction
	WmiAdapterStuff();
	~WmiAdapterStuff();

	///////////////////////////////////////////////////////
	// construction & destruction helpers
	///////////////////////////////////////////////////////
	public:

	BOOL	IsValidBasePerfRegistry	( void );
	BOOL	IsValidInternalRegistry	( void );

	HRESULT Init ( void );
	HRESULT	Uninit ( void );

	///////////////////////////////////////////////////////
	// generate files & registry
	///////////////////////////////////////////////////////
	HRESULT Generate ( BOOL bInitialize = TRUE, GenerateEnum type = Normal );

	/////////////////////////////////////////////////////////////////////////
	// check usage of shared memory ( protect against perfmon has killed )
	/////////////////////////////////////////////////////////////////////////
	void CheckUsage (void );

	///////////////////////////////////////////////////////
	// performance refreshing
	///////////////////////////////////////////////////////
	HRESULT	Initialize		( void );
	void	Uninitialize	( void );

	HRESULT	InitializePerformance	( void );
	HRESULT	UninitializePerformance	( void );

	HRESULT Refresh ( void );

	///////////////////////////////////////////////////////////////////////////
	// registry refresh
	///////////////////////////////////////////////////////////////////////////
	BOOL	RequestGet();
	BOOL	RequestSet();
};

#endif	__ADAPTER_STUFF_H__
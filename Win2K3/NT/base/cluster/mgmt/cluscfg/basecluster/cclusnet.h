//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2000 Microsoft Corporation
//
//  Module Name:
//      CClusNet.h
//
//  Description:
//      Header file for CClusNet class.
//      The CClusNet class performs operations that are common to many
//      configuration tasks of the ClusSvc service.
//
//  Implementation Files:
//      CClusNet.cpp
//
//  Maintained By:
//      Vij Vasu (Vvasu) 03-MAR-2000
//
//////////////////////////////////////////////////////////////////////////////


// Make sure that this file is included only once per compile path.
#pragma once


//////////////////////////////////////////////////////////////////////////
// Include Files
//////////////////////////////////////////////////////////////////////////

// For the CAction base class
#include "CAction.h"

// For the CService class
#include "CService.h"


//////////////////////////////////////////////////////////////////////////
// Forward declaration
//////////////////////////////////////////////////////////////////////////

class CBaseClusterAction;


//////////////////////////////////////////////////////////////////////////////
//++
//
//  class CClusNet
//
//  Description:
//      The CClusNet class performs operations that are common to many
//      configuration tasks of the ClusSvc service.
//
//      This class is intended to be used as the base class for other ClusNet
//      related action classes.
//
//--
//////////////////////////////////////////////////////////////////////////////
class CClusNet : public CAction
{
protected:
    //////////////////////////////////////////////////////////////////////////
    // Protected constructors and destructors
    //////////////////////////////////////////////////////////////////////////

    // Constructor.
    CClusNet(
          CBaseClusterAction * pbcaParentActionIn
        );

    // Default destructor.
    ~CClusNet();


    //////////////////////////////////////////////////////////////////////////
    // Protected methods
    //////////////////////////////////////////////////////////////////////////

    // Create and configure the service.
    void
        ConfigureService();

    // Cleanup and remove the service.
    void
        CleanupService();

    
    //////////////////////////////////////////////////////////////////////////
    // Protected accessors
    //////////////////////////////////////////////////////////////////////////

    // Get the ClusNet service object.
    CService &
        RcsGetService() throw()
    {
        return m_cservClusNet;
    }

    // Get the parent action
    CBaseClusterAction *
        PbcaGetParent() throw()
    {
        return m_pbcaParentAction;
    }


private:
    //////////////////////////////////////////////////////////////////////////
    // Private member functions
    //////////////////////////////////////////////////////////////////////////

    // Copy constructor
    CClusNet( const CClusNet & );

    // Assignment operator
    const CClusNet & operator =( const CClusNet & );


    //////////////////////////////////////////////////////////////////////////
    // Private data
    //////////////////////////////////////////////////////////////////////////

    // The CService object representing the ClusNet service.
    CService                m_cservClusNet;

    // Pointer to the base cluster action of which this action is a part.
    CBaseClusterAction *    m_pbcaParentAction;

}; //*** class CClusNet
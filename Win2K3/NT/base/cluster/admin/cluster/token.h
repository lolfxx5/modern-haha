/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 1996-2002 Microsoft Corporation
//
//  Module Name:
//      token.h
//
//  Abstract:
//      This file contains the declaration of the tokens that are valid on the
//      command line of cluster.exe
//
//  Implementation File:
//      token.cpp
//
//  Author:
//      Vijayendra Vasu (vvasu)               28-Oct-1998
//
//  Maintained By:
//      George Potts (GPotts)                 11-Apr-2002
//
//  Revision History:
//      001. This  has been drastically changed from the previous version.
//           The tokens have now been categorized into three types: objects,
//           options and parameters (enumerated in the file cmdline.h). This
//           functions in this file help categorize tokens into these 
//           categories.
//      April 10, 2002      Updated for the security push.
//
//  Notes:
//
/////////////////////////////////////////////////////////////////////////////
#pragma once


/////////////////////////////////////////////////////////////////////////////
//  Include files
/////////////////////////////////////////////////////////////////////////////
#include "cmdline.h"


/////////////////////////////////////////////////////////////////////////////
//  Type definitions
/////////////////////////////////////////////////////////////////////////////
template <class EnumType> struct LookupStruct
{
    LPCWSTR pszName;
    EnumType type;
};


/////////////////////////////////////////////////////////////////////////////
//  External variable declarations
/////////////////////////////////////////////////////////////////////////////

extern const LookupStruct<ObjectType> objectLookupTable[];
extern const LookupStruct<OptionType> optionLookupTable[];
extern const LookupStruct<ParameterType> paramLookupTable[];
extern const LookupStruct<ValueFormat> formatLookupTable[];

extern const size_t objectLookupTableSize;
extern const size_t optionLookupTableSize;
extern const size_t paramLookupTableSize;
extern const size_t formatLookupTableSize;


// Separator character constants.
extern const CString OPTION_SEPARATOR;
extern const CString OPTION_VALUE_SEPARATOR;
extern const CString PARAM_VALUE_SEPARATOR;
extern const CString VALUE_SEPARATOR;

extern const CString SEPERATORS;
extern const CString DELIMITERS;


/////////////////////////////////////////////////////////////////////////////
//  Template function definitions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//++
//
//  LookupType
//
//  Routine Description:
//      This template function looks up a particular token in a lookup table
//      and returns the type of the token if found.
//
//  Arguments:
//      IN  const CString & strToken
//          The token to be looked up
//
//      IN  struct LookupStruct<EnumType> lookupTable[]
//          The lookup table. This table must have at least one entry and the 
//          first entry must be the default type (the type to be returned if
//          the given token is not found.
//
//      IN  const int nTableSize
//          Size of the lookup table.
//
//  Return Value:
//      The type of the token if it is found or the type specified in the first
//      entry of the lookup table if it is not.
//
//--
/////////////////////////////////////////////////////////////////////////////
template <class EnumType>
EnumType LookupType( const CString & strToken, 
                     const LookupStruct<EnumType> lookupTable[],
                     const size_t nTableSize )
{
    for ( size_t idx = 1; idx < nTableSize; ++idx )
    {
        if ( strToken.CompareNoCase( lookupTable[idx].pszName ) == 0 )
            return lookupTable[idx].type;
    }
    
    // The given token is not found in the lookup table
    // lookupTable[0].type contains the default return value.
    return lookupTable[0].type;
}


/////////////////////////////////////////////////////////////////////////////
//++
//
//  LookupName
//
//  Routine Description:
//      This template function looks up a particular type in a lookup table
//      and returns the name of the type if found.
//
//  Arguments:
//      IN  EnumType type
//          The type whose name is to be looked up
//
//      IN  struct LookupStruct<EnumType> lookupTable[]
//          The lookup table. This table must have at least one entry and the 
//          first entry must be the default type (the type to be returned if
//          the given token is not found.
//
//      IN  const int nTableSize
//          Size of the lookup table.
//
//  Return Value:
//      The name of the token if it is found or the name specified in the first
//      entry of the lookup table if it is not.
//
//--
/////////////////////////////////////////////////////////////////////////////
template <class EnumType>
LPCWSTR LookupName( EnumType type, 
                    const LookupStruct<EnumType> lookupTable[],
                    const int nTableSize )
{
    for ( int i = 1; i < nTableSize; ++i )
    {
        if ( type == lookupTable[i].type )
            return lookupTable[i].pszName;
    }
    
    // The given type is not found in the lookup table
    // lookupTable[0].pszName contains the default return value.
    return lookupTable[0].pszName;
}

#ifndef _ALGORITHMS_H
#define _ALGORITHMS_H

#include "Allocator.h"
#include "Stack.h"

template <class WmiElement>
LONG CompareElement ( const WmiElement &a_Arg1 , const WmiElement &a_Arg2 ) ;

template <class WmiElement>
WmiStatusCode QuickSort ( WmiElement *a_Array , ULONG a_Size ) ;

#include <Algorithms.cpp>

#endif _ALGORITHMS_H

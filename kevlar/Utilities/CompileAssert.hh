#ifndef Compile_Assert_hh_
#define Compile_Assert_hh_

#include <stdlib.h>
#include <iostream>

/** \file CompileAssert.hh
  \author Kevin Wierman
  \date April 12th, 2014
  \brief Defines static and runtime assertions.
  These are to be used as follows:
  ~~~~~~~~
        Compile_RUNTIME_ASSERT(true, ThisIsATrueStatement);
        Compile_RUNTIME_ASSERT(false, ThisIsAFalseStatement);
  ~~~~~~~~
  \note The second argument must be a single word, or else it will be interpreted as several arguments.
**/


//! Body-less template
template<int> 
struct CompileTimeError;

//! Fully-defined template
template<> 
struct CompileTimeError<true> {};

//! Use for performing static assertion
#define Compile_STATIC_ASSERT(expr, msg) { CompileTimeError<((expr) != 0)> ERROR_##msg; (void)ERROR_##msg; } 
    
//! Use for runtime assertions
#define Compile_RUNTIME_ASSERT(expr, msg) { if(!expr) std::cerr<<#msg<<std::endl; abort(); }

#endif //Compile_Assert_h_
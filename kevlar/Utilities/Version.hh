#ifndef kevlar_Version_hh_
#define kevlar_Version_hh_ 1

//#include "kevlar/Utilities/CompileAssert.hh"
#include "H5pubconf.h"

#define KEVLAR_VERSION "v04_01_00"
  
//Compile_RUNTIME_ASSERT(H5_HAVE_FILTER_DEFLATE, PleaseCompileHDF5WithZLibSupport);
static_assert(H5_HAVE_FILTER_DEFLATE, "Please Compile HDF5 With ZLib Support");

#endif
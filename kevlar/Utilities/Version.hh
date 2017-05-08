#ifndef kevlar_Version_hh_
#define kevlar_Version_hh_ 1

#include "kevlar/Utilities/CompileAssert.h"
#include "hdf5/H5pubconf.h"

#define KEVLAR_VERSION "v04_01_00"
  
Compile_RUNTIME_ASSERT(H5_HAVE_FILTER_DEFLATE, PleaseCompileHDF5WithZLibSupport);

#endif
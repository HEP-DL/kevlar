#ifndef kevlar_Version_hh_
#define kevlar_Version_hh_ 1

#include "H5pubconf.h"

#define KEVLAR_VERSION "v04_01_01"
  
static_assert(H5_HAVE_FILTER_DEFLATE, "Please Compile HDF5 With ZLib Support");

#endif
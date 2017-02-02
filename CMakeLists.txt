# ======================================================================
#  kevlar main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/kevlar/ups/setup_for_development <-d|-p>
#  cmake [-DCMAKE_INSTALL_PREFIX=/install/path]
#        -DCMAKE_BUILD_TYPE=$CETPKG_TYPE
#        $CETPKG_SOURCE
#  make
#  make test
#  make install
#  make package (builds distribution tarfile)
# ======================================================================


# use cmake 2.8 or later
cmake_minimum_required (VERSION 2.8)

project(kevlar)

# cetbuildtools contains our cmake modules
SET ( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
IF (NOT CETBUILDTOOLS_VERSION)
    MESSAGE (FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules")
ENDIF()

set(CMAKE_MODULE_PATH $ENV{ART_DIR}/Modules
		      $ENV{CETBUILDTOOLS_DIR}/Modules
		      ${CMAKE_MODULE_PATH})

include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS CAUTIOUS
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic -Wno-unused-local-typedefs
)

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_product( larsim v1_00_00 )
find_ups_product( art v1_08_00 )
find_ups_product( cetbuildtools v4_14_01 )
find_ups_product( hdf5 v01_00_00 )
find_ups_product( boost v1_00_00 )
#do the extra hdf5 step
cet_find_library( hdf5 NAMES hdf5_debug PATHS ${HDF5_SEARCH_PATH}/lib NO_DEFAULT_PATH  )
cet_find_library( hdf5_cpp NAMES hdf5_cpp_debug PATHS ${HDF5_SEARCH_PATH}/lib NO_DEFAULT_PATH  )
cet_find_library( hdf5_hl NAMES hdf5_hl_debug PATHS ${HDF5_SEARCH_PATH}/lib NO_DEFAULT_PATH  )
cet_find_library( hdf5_tools NAMES hdf5_tools_debug PATHS ${HDF5_SEARCH_PATH}/lib NO_DEFAULT_PATH  )
set(HDF5_LIBRARIES ${hdf5} ${hdf5_cpp} ${hdf5_hl} ${hdf5_tools} )

# macros for dictionary and simple_plugin
include(ArtDictionary)
include(ArtMake)
include(BuildPlugins)

# ADD SOURCE CODE SUBDIRECTORIES HERE
add_subdirectory(kevlar)

# tests
add_subdirectory(test)

# ups - table and config files
add_subdirectory(ups)

# packaging utility
include(UseCPack)

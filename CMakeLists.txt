cmake_minimum_required (VERSION 2.8)

project(kevlar)

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

find_ups_product( larcoreobj v1_00_00 )
find_ups_product( lardataobj v1_00_00 )
find_ups_product( larsim v1_00_00 )
find_ups_product( art v1_08_00 )
find_ups_product( cetbuildtools v4_14_01 )
find_ups_product( hdf5 v01_00_00 )
find_ups_product( boost v1_00_00 )

cet_find_library( hdf5 NAMES hdf5_debug hdf5-shared hdf5_opt PATHS ${HDF5_SEARCH_PATH}/lib ${hdf5_DIR}/../../ NO_DEFAULT_PATH  )
cet_find_library( hdf5_cpp NAMES hdf5_cpp_debug hdf5_cpp-shared hdf5_cpp_opt PATHS ${HDF5_SEARCH_PATH}/lib ${hdf5_DIR}/../../ NO_DEFAULT_PATH  )
cet_find_library( hdf5_hl NAMES hdf5_hl_debug hdf5_hl-shared hdf5_hl_opt PATHS ${HDF5_SEARCH_PATH}/lib ${hdf5_DIR}/../../ NO_DEFAULT_PATH  )
cet_find_library( hdf5_tools NAMES hdf5_tools_debug hdf5_tools-shared hdf5_tools_opt PATHS ${HDF5_SEARCH_PATH}/lib ${hdf5_DIR}/../../ NO_DEFAULT_PATH  )
set(HDF5_LIBRARIES ${hdf5} ${hdf5_cpp} ${hdf5_hl} ${hdf5_tools} )

include(ArtDictionary)
include(ArtMake)
include(BuildPlugins)

add_subdirectory(kevlar)
add_subdirectory(test)
add_subdirectory(ups)
include(UseCPack)

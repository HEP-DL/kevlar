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
  EXTRA_FLAGS -pedantic -Wno-unused-local-typedefs -Wno-attributes
)

cet_report_compiler_flags()

find_ups_product( lardataobj v1_00_00 )
find_ups_product( larsim v1_00_00 )
find_ups_product( art v1_08_00 )
find_ups_product( cetbuildtools v4_14_01 )
find_ups_product( hdf5 v1_9_236 )
find_ups_product( boost v1_00_00 )

include(ArtDictionary)
include(ArtMake)
include(BuildPlugins)

add_subdirectory(kevlar)
add_subdirectory(test)
add_subdirectory(ups)
include(UseCPack)

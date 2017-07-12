include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories(
  $ENV{HDF5_DIR}/hdf5/src
  $ENV{HDF5_DIR}/hdf5/c++/src
  $ENV{HDF5_DIR}/hdf5/hl/src
  $ENV{HDF5_DIR}/hdf5/hl/c++/src
)


simple_plugin( HDF5File service
  ${ART_FRAMEWORK_PRINCIPAL}
  ${ART_FRAMEWORK_SERVICES_BASIC}
  ${ART_FRAMEWORK_SERVICES_REGISTRY}
  ${ART_FRAMEWORK_SERVICES_OPTIONAL}
  ${HDF5_LIBRARIES_TO_EXPORT}
  BASENAME_ONLY
)

install_headers()
install_source()
install_fhicl()
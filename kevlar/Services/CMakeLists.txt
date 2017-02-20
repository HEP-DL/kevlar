include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

simple_plugin( HDF5File service
  ${ART_FRAMEWORK_PRINCIPAL}
  ${ART_FRAMEWORK_SERVICES_BASIC}
  ${ART_FRAMEWORK_SERVICES_REGISTRY}
  ${ART_FRAMEWORK_SERVICES_OPTIONAL}
  ${HDF5_LIBRARIES}
  BASENAME_ONLY
)

install_headers()
install_source()
install_fhicl()
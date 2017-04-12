include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( $ENV{NUTOOLS_INC} )


#HDF5 Image Converter
simple_plugin( Coldata module
    ${ART_FRAMEWORK_PRINCIPAL}
    ${ART_FRAMEWORK_SERVICES_REGISTRY}
    larcore_Geometry
    ${FrameGen_LIBRARIES}
  BASENAME_ONLY
)

install_headers()
install_source()
install_fhicl()

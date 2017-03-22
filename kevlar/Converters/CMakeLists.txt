include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( $ENV{NUTOOLS_INC} )
include_directories( ${HDF5_INC} )

#HDF5 Image Converter
simple_plugin( HDF5RawDigits module
    HDF5File_service
    ${ART_FRAMEWORK_PRINCIPAL}
    ${ART_FRAMEWORK_SERVICES_REGISTRY}
    larcore_Geometry
    nusimdata_SimulationBase
    ${HDF5_LIBRARIES}
  BASENAME_ONLY
)

#HDF5 Label Converter
simple_plugin( HDF5ParticleLabelVector module
    HDF5File_service
    ${ART_FRAMEWORK_PRINCIPAL}
    ${ART_FRAMEWORK_SERVICES_REGISTRY}
    larcore_Geometry
    nusimdata_SimulationBase
    ${ROOT_BASIC_LIB_LIST}
    ${ROOT_EG}
    ${HDF5_LIBRARIES}
  BASENAME_ONLY
)

install_headers()
install_source()
install_fhicl()

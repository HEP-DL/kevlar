include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( $ENV{NUTOOLS_INC} )

include_directories(
  $ENV{HDF5_DIR}/hdf5/src
  $ENV{HDF5_DIR}/hdf5/c++/src
  $ENV{HDF5_DIR}/hdf5/hl/src
  $ENV{HDF5_DIR}/hdf5/hl/c++/src
)

#HDF5 Image Converter
simple_plugin( HDF5RawDigits module
    HDF5File_service
    ${ART_FRAMEWORK_PRINCIPAL}
    ${ART_FRAMEWORK_SERVICES_REGISTRY}
    larcore_Geometry
    nusimdata_SimulationBase
    ${HDF5_CPP} ${HDF5_HL} ${HDF5_HL_CPP} ${HDF5} ${HDF5_LIBRARIES_TO_EXPORT} 
    BASENAME_ONLY
)

#HDF5 Image Converter
simple_plugin( HDF5Wires module
    HDF5File_service
    ${ART_FRAMEWORK_PRINCIPAL}
    ${ART_FRAMEWORK_SERVICES_REGISTRY}
    larcore_Geometry
    lardata_RecoObjects
    lardataobj_RecoBase
    nusimdata_SimulationBase
    ${HDF5_CPP} ${HDF5_HL} ${HDF5_HL_CPP} ${HDF5} ${HDF5_LIBRARIES_TO_EXPORT} 
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
    ${HDF5_CPP} ${HDF5_HL} ${HDF5_HL_CPP} ${HDF5} ${HDF5_LIBRARIES_TO_EXPORT} 
    ${MF_MESSAGELOGGER}
  BASENAME_ONLY
)

#HDF5 U,V,W,t0 of the primary vertices
simple_plugin( HDF5VertexPlaneProjection module
    HDF5File_service
    ${ART_FRAMEWORK_PRINCIPAL}
    ${ART_FRAMEWORK_SERVICES_REGISTRY}
    larcore_Geometry
    nusimdata_SimulationBase
    ${ROOT_BASIC_LIB_LIST}
    ${ROOT_EG}
    ${HDF5_CPP} ${HDF5_HL} ${HDF5_HL_CPP} ${HDF5} ${HDF5_LIBRARIES_TO_EXPORT} 
    BASENAME_ONLY
)

#Semantic Segmentation by particle label
simple_plugin( HDF5ParticleLabelSS module
    HDF5File_service
    ${ART_FRAMEWORK_PRINCIPAL}
    ${ART_FRAMEWORK_SERVICES_REGISTRY}
    larcore_Geometry
    nusimdata_SimulationBase
    ${ROOT_BASIC_LIB_LIST}
    ${ROOT_EG}
    ${HDF5_CPP} ${HDF5_HL} ${HDF5_HL_CPP} ${HDF5} ${HDF5_LIBRARIES_TO_EXPORT} 
    BASENAME_ONLY
)

install_headers()
install_source()
install_fhicl()

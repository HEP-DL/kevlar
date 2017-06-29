include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( $ENV{NUTOOLS_INC} )
include_directories( ${FRAMEGEN_INC} )

# WIB frame creator
simple_plugin( Coldata module
    ${ART_FRAMEWORK_PRINCIPAL}
    ${ART_FRAMEWORK_SERVICES_REGISTRY}
    ${FRAMEGEN_LIB}
    larcore_Geometry
    framegen
  BASENAME_ONLY
)

# WIB file-reader daq::rawdigit creator
simple_plugin( ColdataValidation module
    ${ART_FRAMEWORK_PRINCIPAL}
    ${ART_FRAMEWORK_SERVICES_REGISTRY}
    ${FRAMEGEN_LIB}
    larcore_Geometry
    lardata_RawData_utils
    lardataobj_RawData
    framegen
  BASENAME_ONLY
)

install_headers()
install_source()
install_fhicl()

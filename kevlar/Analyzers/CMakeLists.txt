include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( $ENV{NUTOOLS_INC} )

#Neutron Scanner
simple_plugin( NeutronScanner module
    ${ART_FRAMEWORK_PRINCIPAL}
  BASENAME_ONLY
)

install_headers()
install_source()
install_fhicl()

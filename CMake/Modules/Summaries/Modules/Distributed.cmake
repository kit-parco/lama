set ( REQUIRED_FOUND FALSE )
if    ( MPI_ENABLED OR GPI_ENABLED )
  set ( REQUIRED_FOUND TRUE )
endif ( MPI_ENABLED OR GPI_ENABLED )

heading3 ( "Distributed" "REQUIRED_FOUND" )
    found_message ( "MPI" "MPI_FOUND" "OPTIONAL" "Version ${MPI_VERSION} at ${SCAI_MPI_INCLUDE_DIR}" )
    found_message ( "GPI" "GPI_FOUND" "OPTIONAL" "with:" )
    message ( STATUS "                                 GPI2 Version ${GPI2_VERSION} at ${GPI2_INCLUDE_DIR}" )
    message ( STATUS "                                 IBVERBS at ${IBVERBS_INCLUDE_DIR}" )
    # no IBVERBS_VERSION
    #foreach    ( _LIB GPI2 IBVERBS )
    #    message ( STATUS "                                 ${_LIB} Version ${${_LIB}_VERSION} at ${${_LIB}_INCLUDE_DIR}" )
    #endforeach ( _LIB GPI2 IBVERBS )
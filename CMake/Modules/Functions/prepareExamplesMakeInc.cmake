macro    ( prepareExamplesMakeInc )

	## set SCAI_EXAMPLE_LINK_LIBRARIES with own project and all dependent libraries
	set ( SCAI_EXAMPLE_LINK_LIBRARIES "-l${PROJECT_NAME}" )
	set ( REVERT_LIST ${${UPPER_PROJECT_NAME}_INTERNAL_DEPS} ) # because list does not accept variable recursion

	if ( REVERT_LIST ) # is empty for common
		list ( REVERSE REVERT_LIST )
		foreach    ( module ${REVERT_LIST} )
			set ( SCAI_EXAMPLE_LINK_LIBRARIES "${SCAI_EXAMPLE_LINK_LIBRARIES} -l${module}" )
		endforeach ( module ${REVERT_LIST} )
	endif ( REVERT_LIST )

	## set project specific SCAI_DEFINES
	if    ( SCAI_ASSERT_LEVEL )
		set ( SCAI_DEFINES "${SCAI_DEFINES} -DSCAI_ASSERT_LEVEL_${SCAI_ASSERT_LEVEL}" )
	endif ( SCAI_ASSERT_LEVEL )
	
	if    ( SCAI_LOGGING_LEVEL )
		set ( SCAI_DEFINES "${SCAI_DEFINES} -DSCAI_LOG_LEVEL_${SCAI_LOGGING_LEVEL}" )
	endif ( SCAI_LOGGING_LEVEL )

	if    ( SCAI_TRACING_FLAG )
		set ( SCAI_DEFINES "${SCAI_DEFINES} -D${SCAI_TRACING_FLAG}" )
	endif ( SCAI_TRACING_FLAG )

endmacro ( prepareExamplesMakeInc )
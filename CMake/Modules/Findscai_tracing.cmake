#
# Find the common includes and libraries
#
# SCAI_TRACING_FOUND       - Do not attempt to use if "no" or undefined
# SCAI_TRACING_INCLUDE_DIR - the common include dir
# SCAI_TRACING_LIBRARY     - libraries to link against

if    ( NOT DEFINED SCAI_TRACING_INCLUDE_DIR )
    find_path ( SCAI_TRACING_INCLUDE_DIR tracing.hpp
        /usr/local/include/scai
        /usr/include/scai
        ${CMAKE_INSTALL_PREFIX}/include/scai
        $ENV{SCAI_TRACING_INCLUDE_PATH}/scai
        ${SCAI_TRACING_ROOT}/include/scai
    )
endif ( NOT DEFINED SCAI_TRACING_INCLUDE_DIR )

find_library ( SCAI_TRACING_LIBRARY scai_tracing
    /usr/local/lib
    /usr/lib
    $ENV{SCAI_TRACING_LIBRARY_PATH}
    ${SCAI_TRACING_ROOT}/lib
)

if    ( SCAI_TRACING_INCLUDE_DIR )
    if    (SCAI_TRACING_LIBRARY)
        set ( SCAI_TRACING_FOUND TRUE )
    endif ( SCAI_TRACING_LIBRARY )
endif (SCAI_TRACING_INCLUDE_DIR)

mark_as_advanced ( SCAI_TRACING_FOUND SCAI_TRACING_INCLUDE_DIR SCAI_TRACING_LIBRARY )
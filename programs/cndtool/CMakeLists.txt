project(
    ${PM_CNDTOOL}
    LANGUAGES CXX
    VERSION 0.7.1
    HOMEPAGE_URL ${CMAKE_PROJECT_HOMEPAGE_URL}
)

configure_file(config.h.in config.h)

set(CNDTOOL_SRC_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    "cndtoolargs.h"
    "main.cpp"
    "cnd.h"
    "ndy.h"
    "obj.h"
    "resource.h"
)

add_executable( ${PROJECT_NAME}
    ${CNDTOOL_SRC_FILES}
    $<TARGET_OBJECTS:${PM_LIBCMD}>
)

target_include_directories( ${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PM_PROGRAM_DIR}
)

target_link_libraries( ${PROJECT_NAME} ${PM_LIBIM} )

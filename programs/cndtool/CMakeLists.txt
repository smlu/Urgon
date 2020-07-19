project(
    ${PM_CNDTOOL}
    LANGUAGES CXX
    VERSION 0.4.2
)

configure_file(config.h.in config.h)

set(CNDTOOL_SRC_FILES
    "config.h"
    "cndtoolargs.h"
    "main.cpp"
    "patch.h"
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

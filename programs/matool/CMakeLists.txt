project(
    ${PM_MATOOL}
    LANGUAGES CXX
    VERSION 1.0.0
)

configure_file(config.h.in config.h)

set(MATOOL_SRC_FILES
    "config.h"
    "main.cpp"
)

add_executable( ${PROJECT_NAME}
    ${MATOOL_SRC_FILES}
    $<TARGET_OBJECTS:${PM_LIBCMD}>
)

target_include_directories( ${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries( ${PROJECT_NAME} ${PM_LIBIM} )

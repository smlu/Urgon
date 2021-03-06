project(
    ${PM_GOBEXT}
    LANGUAGES CXX
    VERSION 1.1.0
    HOMEPAGE_URL ${CMAKE_PROJECT_HOMEPAGE_URL}
)

configure_file(config.h.in config.h)

set(GOBEXT_SRC_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    "main.cpp"
)

add_executable( ${PROJECT_NAME}
    ${GOBEXT_SRC_FILES}
    $<TARGET_OBJECTS:${PM_LIBCMD}>
)

target_include_directories( ${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries( ${PROJECT_NAME} ${PM_LIBIM} )

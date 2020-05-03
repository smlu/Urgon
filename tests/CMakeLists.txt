project(
    ${PM_TEST}
    LANGUAGES CXX
    VERSION 0.1.0
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test)

set(TEST_SRC_FILES "test.cpp")
add_executable (${PROJECT_NAME}
    ${TEST_SRC_FILES}
    $<TARGET_OBJECTS:${PM_LIBCMD}>
    $<TARGET_OBJECTS:${PM_IMFIXES}>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSRC_ROOT=\"${CMAKE_SOURCE_DIR}\"")
target_link_libraries(${PROJECT_NAME} ${PM_LIBIM})

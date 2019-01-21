cmake_minimum_required (VERSION 3.6 FATAL_ERROR)
project(
    ProjectMarduk
    LANGUAGES CXX
    VERSION 0.2.2
)

# Global constants
set(PM_LIBIM   "libim")
set(PM_LIBCND  "libcnd")
set(PM_GOBEXT  "gobext")
set(PM_CNDEXT  "cndext")
set(PM_CNDTOOL "cndtool")
set(PM_TEST    "test")

# Compiler flags
set(CMAKE_CXX_STANDARD 17) # c++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CFlags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /sdl")
    add_definitions("-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING=1")
    if(${MSVC_VERSION} GREATER_EQUAL 1914)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /diagnostics:caret")
    endif()
elseif(MINGW)
  add_definitions("-mno-ms-bitfields") # TODO: this is bad
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fPIC -pipe -static")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fPIC -pipe -static")
elseif(NOT WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fPIC -pipe -static")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fPIC -pipe -static")
endif()

add_definitions("-D__STDC_WANT_LIB_EXT1__=1")



set(SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
include_directories ("${SOURCE_DIR}")

# LibIM source

file(GLOB_RECURSE
  LIBIM_HEADER_FILES
  "${SOURCE_DIR}/libim/*.h"
  [FOLLOW_SYMLINKS]
)

file(GLOB_RECURSE
  LIBIM_SRC_FILES
  "${SOURCE_DIR}/libim/*.cpp"
  [FOLLOW_SYMLINKS]
)

# Cmd utils
file(GLOB_RECURSE
  CMDUTILS_HEADER_FILES
  "${SOURCE_DIR}/cmdutils/*.h"
  [FOLLOW_SYMLINKS]
)

file(GLOB_RECURSE
  CMDUTILS_SRC_FILES
  "${SOURCE_DIR}/cmdutils/*.cpp"
  [FOLLOW_SYMLINKS]
)

# LibIM
add_library(${PM_LIBIM} STATIC
    ${LIBIM_HEADER_FILES}
    ${LIBIM_SRC_FILES}
)
set_target_properties(${PM_LIBIM}  PROPERTIES PREFIX  "")

# GCC < 9
target_link_libraries(${PM_LIBIM} PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)

# CND utils 
add_library(${PM_LIBCND} OBJECT
    ${CMDUTILS_HEADER_FILES}
    ${CMDUTILS_SRC_FILES}
)

# CND Extractor
set(CNDEXT_SRC_FILES "${SOURCE_DIR}/cndext/main.cpp")
add_executable (${PM_CNDEXT}
    ${CNDEXT_SRC_FILES}
    $<TARGET_OBJECTS:${PM_LIBCND}>
)
target_link_libraries(${PM_CNDEXT} ${PM_LIBIM})

# CND tool
set(CNDTOOL_SRC_FILES
    "${SOURCE_DIR}/cndtool/main.cpp"
    "${SOURCE_DIR}/cndtool/cndtoolargs.h"
    "${SOURCE_DIR}/cndtool/patch.h"
)
add_executable (${PM_CNDTOOL}
    ${CNDTOOL_SRC_FILES}
    $<TARGET_OBJECTS:${PM_LIBCND}>
)
target_link_libraries(${PM_CNDTOOL} ${PM_LIBIM})

# GOB Extractor
set(GOBEXT_SRC_FILES "${SOURCE_DIR}/gobext/main.cpp")
add_executable (${PM_GOBEXT}
    ${GOBEXT_SRC_FILES}
    $<TARGET_OBJECTS:${PM_LIBCND}>
)
target_link_libraries(${PM_GOBEXT} ${PM_LIBIM})

# Test exe
set(TEST_SRC_FILES "${SOURCE_DIR}/test/test.cpp")
add_executable (${PM_TEST}
    ${TEST_SRC_FILES}
    $<TARGET_OBJECTS:${PM_LIBCND}>
)
target_link_libraries(${PM_TEST} ${PM_LIBIM})


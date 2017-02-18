cmake_minimum_required (VERSION 3.6 FATAL_ERROR)
project (ProjectMarduk CXX)

# Compiler flags
set(CMAKE_CXX_STANDARD 14) # c++14
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MINGW)
  add_definitions("-mno-ms-bitfields")
endif()

set(SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
include_directories ("${SOURCE_DIR}")

# LibIm source
include_directories("${SOURCE_DIR}/libim/")
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

# CND Extractor
set(CNDEXT_SRC_FILES "${SOURCE_DIR}/cndext/main.cpp")
add_executable ("cndext"
    ${CNDEXT_SRC_FILES}
    ${LIBIM_HEADER_FILES}
    ${LIBIM_SRC_FILES}
    ${CMDUTILS_HEADER_FILES}
    ${CMDUTILS_SRC_FILES}
)

# GOB Extractor
set(GOBEXT_SRC_FILES "${SOURCE_DIR}/gobext/main.cpp")
add_executable ("gobext"
    ${GOBEXT_SRC_FILES}
    ${LIBIM_HEADER_FILES}
    ${LIBIM_SRC_FILES}
    ${CMDUTILS_HEADER_FILES}
    ${CMDUTILS_SRC_FILES}
)

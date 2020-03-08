cmake_minimum_required (VERSION 3.6 FATAL_ERROR)
project(
    ProjectMarduk
    LANGUAGES CXX
    VERSION 0.6.6
)

# Global constants
set(PM_CNDEXT  "cndext")
set(PM_CNDTOOL "cndtool")
set(PM_GOBEXT  "gobext")
set(PM_IMFIXES "imfixes")
set(PM_LIBCMD  "libcmd")
set(PM_LIBIM   "libim")
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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fPIC -pipe -static")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fPIC -pipe -static")
elseif(NOT WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fPIC -pipe -static")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fPIC -pipe -static")
endif()

add_definitions("-D__STDC_WANT_LIB_EXT1__=1")

add_subdirectory( libraries )
add_subdirectory( programs )
add_subdirectory( tests )

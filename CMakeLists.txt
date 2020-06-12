cmake_minimum_required (VERSION 3.15 FATAL_ERROR)
project(
    ProjectMarduk
    LANGUAGES CXX
    VERSION 0.8.3
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

# CFlags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /sdl")
    add_definitions("-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING=1")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>$<$<CONFIG:Release>:>")
    if(${MSVC_VERSION} GREATER_EQUAL 1914)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /diagnostics:caret")
    endif()
elseif(MINGW)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fPIC -pipe -static")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wdouble-promotion -fvisibility=hidden -fPIC -pipe -static")
elseif(NOT WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fPIC -pipe")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -static")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -static")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wdouble-promotion -Wpedantic -fvisibility=hidden -fPIC -pipe")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -static")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -glldb")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wdocumentation -glldb")
    if(NOT APPLE)
        option(CLANG_LINK_LIBCPP "Link libc++" $<$<CONFIG:Debug>:OFF>$<$<CONFIG:Release>:ON>)
        if(CLANG_LINK_LIBCPP)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -lc++abi -pthread")
        endif()
    endif()
endif()

add_definitions("-D__STDC_WANT_LIB_EXT1__=1")

add_subdirectory( libraries )
add_subdirectory( programs )
add_subdirectory( tests )

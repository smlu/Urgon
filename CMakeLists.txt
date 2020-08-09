cmake_minimum_required (VERSION 3.15 FATAL_ERROR)
project(
    ProjectMarduk
    LANGUAGES CXX
    VERSION 0.9
)

# Global constants
set(PM_CNDEXT  "cndext" )
set(PM_CNDTOOL "cndtool")
set(PM_GOBEXT  "gobext" )
set(PM_IMFIXES "imfixes")
set(PM_MATOOL  "matool" )
set(PM_LIBCMD  "libcmd" )
set(PM_LIBIM   "libim"  )
set(PM_TEST    "test"   )

set(PM_LIB_DIR     "${CMAKE_SOURCE_DIR}/libraries")
set(PM_PROGRAM_DIR "${CMAKE_SOURCE_DIR}/programs" )
set(PM_TEST_DIR    "${CMAKE_SOURCE_DIR}/tests"    )

# Compiler flags
set(CMAKE_CXX_STANDARD 17) # c++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CFlags
if(MSVC AND NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /sdl")
    add_definitions("-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING=1")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>$<$<CONFIG:Release>:>")
    if(${MSVC_VERSION} GREATER_EQUAL 1914)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /diagnostics:caret")
    endif()
elseif(MINGW)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fPIC -pipe -static")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic -pedantic-errors -Werror=return-type -Wall -Wextra -Wdouble-promotion -fvisibility=hidden -fPIC -pipe -static")
elseif(NOT WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fPIC -pipe")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -static")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -static")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -pedantic-errors -Wthread-safety -Werror=return-type -Waddress-of-temporary -Wdouble-promotion -Wpedantic -fvisibility=hidden -fPIC -pipe")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wconversion -Wsign-conversion -static")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Wconversion -Wsign-conversion -static")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wdocumentation")
    if(NOT MSVC)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -glldb")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -glldb")
    endif()
    if(NOT (APPLE OR MSVC))
        option(CLANG_LINK_LIBCPP "Link libc++" $<$<CONFIG:Debug>:OFF>$<$<CONFIG:Release>:ON>)
        if(CLANG_LINK_LIBCPP)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -lc++abi -pthread")
        endif()
    endif()
endif()

add_definitions("-D__STDC_WANT_LIB_EXT1__=1")

add_subdirectory( ${PM_LIB_DIR}     )
add_subdirectory( ${PM_PROGRAM_DIR} )
add_subdirectory( ${PM_TEST_DIR}    )
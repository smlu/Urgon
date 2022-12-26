cmake_minimum_required (VERSION 3.15 FATAL_ERROR)
project(
    Urgon
    LANGUAGES CXX
    VERSION 0.10.0

    HOMEPAGE_URL  "https://github.com/smlu/Urgon"
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

set(CMAKE_CXX_STANDARD 20) # C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(WIN32)
    if(MINGW)
        add_compile_options(
            -Wpedantic
            -pedantic-errors
            -Werror=return-type
            -Wall -Wextra
            -Wdouble-promotion
            -fvisibility=hidden
            -fPIC
            -pipe
            "$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-Wconversion;-Wsign-conversion;-Wno-unused-function>"
        )
        add_link_options("$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-Wl,--no-insert-timestamp;-static;-s>")
    else(MSVC)
        if(MSVC_VERSION LESS 1914 AND NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
            message(FATAL_ERROR "Only Visual Studio 2017 version 15.7 or newer is supported!")
        endif()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>$<$<CONFIG:Release>:>")
        add_definitions("-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING=1")
        add_compile_options(
            /utf-8
            /Zc:__cplusplus
            /diagnostics:caret
            /sdl
            "$<$<NOT:$<CXX_COMPILER_ID:Clang>>:/MP>"
        )
        add_link_options("/Brepro")
    endif()
elseif(NOT WIN32)
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -pedantic-errors
        -Werror=return-type
        -Wdouble-promotion
        -Wpedantic
        -fvisibility=hidden
        -fPIC
        -pipe
        "$<$<CXX_COMPILER_ID:Clang>:-Waddress-of-temporary;-Wthread-safety>"
        "$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-Wconversion;-Wsign-conversion;-Wno-unused-function>"
    )
    add_link_options("$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-static;-s>")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options("$<$<CONFIG:Debug>:-Wdocumentation>")
    if(NOT MSVC)
        add_compile_options("$<$<CONFIG:Debug>:-glldb>")
    endif()
    if(NOT (APPLE OR MSVC))
        option(CLANG_LINK_LIBCPP "Link libc++" "$<$<CONFIG:Debug>:OFF>$<$<CONFIG:Release>:ON>")
        if(CLANG_LINK_LIBCPP)
            add_compile_options(-stdlib=libc++)
            add_link_options(-fuse-ld=lld -lc++abi -pthread)
        endif()
    endif()
endif()

add_compile_options(
    "-D__STDC_WANT_LIB_EXT1__=1"
    "$<$<CONFIG:DEBUG>:-DDEBUG>"
)

add_subdirectory( ${PM_LIB_DIR}     )
add_subdirectory( ${PM_PROGRAM_DIR} )
add_subdirectory( ${PM_TEST_DIR}    )

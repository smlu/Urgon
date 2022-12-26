project(
    ${PM_LIBIM}
    LANGUAGES CXX
    VERSION 0.9.0
)

# LibIM source
file(GLOB_RECURSE
  LIBIM_HEADER_FILES
  "*.h"
  [FOLLOW_SYMLINKS]
)

file(GLOB_RECURSE
  LIBIM_SRC_FILES
  "*.cpp"
  [FOLLOW_SYMLINKS]
)

# LibIM
add_library(${PROJECT_NAME} STATIC
  ${LIBIM_HEADER_FILES}
  ${LIBIM_SRC_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC "../")
target_include_directories(${PROJECT_NAME} PUBLIC ${ZLIB_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${PNG_INCLUDE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX  "")
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES STATIC_LIBRARY_OPTIONS
  $<$<CXX_COMPILER_ID:MSVC>:/Brepro> # /Brepro for MSVC
)

target_link_libraries(${PROJECT_NAME} PUBLIC
  $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs> # std::filesystem GCC < 9
  $<$<AND:$<CXX_COMPILER_ID:Clang>,$<NOT:$<PLATFORM_ID:Windows>>>:stdc++fs> # std::filesystem clang
  png_static
  zlibstatic
)

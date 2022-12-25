set(PNG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libpng/" "${CMAKE_CURRENT_BINARY_DIR}/libpng/"  CACHE PATH "Path to png.h folder")
set(ZLIB_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/zlib/zlib" CACHE PATH "Path to zlib.h folder")

#libpng
set(PNG_SHARED OFF CACHE BOOL "Build shared lib")
set(PNG_SHARED ON CACHE BOOL "Build static lib")
set(PNG_TESTS OFF CACHE BOOL "Build libpng tests")
set(PNG_BUILD_ZLIB ON CACHE BOOL "Custom zlib Location, else find_package is used")
add_subdirectory( libpng )

#zlib
add_subdirectory( zlib )
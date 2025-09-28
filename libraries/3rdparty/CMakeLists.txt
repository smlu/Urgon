#zlib
add_subdirectory( zlib )

#libpng
set(PNG_SHARED OFF CACHE BOOL "Build shared lib")
set(PNG_STATIC ON CACHE BOOL "Build static lib")
set(PNG_TESTS OFF CACHE BOOL "Build libpng tests")
add_subdirectory( libpng )
add_library(PNG::PNG ALIAS png_static) # Create alias for libpng static library

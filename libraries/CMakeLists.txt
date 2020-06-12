set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory( 3rdparty ) # 3rd party libraries
add_subdirectory( cmdutils )
add_subdirectory( imfixes )
add_subdirectory( libim )
#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "GLEW::glew" for configuration "Debug"
set_property(TARGET GLEW::glew APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(GLEW::glew PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/debug/lib/glew32d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opengl32;glu32"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/bin/glew32d.dll"
  )

list(APPEND _cmake_import_check_targets GLEW::glew )
list(APPEND _cmake_import_check_files_for_GLEW::glew "${_IMPORT_PREFIX}/debug/lib/glew32d.lib" "${_IMPORT_PREFIX}/debug/bin/glew32d.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)

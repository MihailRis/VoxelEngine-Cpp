#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "OpenAL::OpenAL" for configuration "Debug"
set_property(TARGET OpenAL::OpenAL APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(OpenAL::OpenAL PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/debug/lib/OpenAL32.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/bin/OpenAL32.dll"
  )

list(APPEND _cmake_import_check_targets OpenAL::OpenAL )
list(APPEND _cmake_import_check_files_for_OpenAL::OpenAL "${_IMPORT_PREFIX}/debug/lib/OpenAL32.lib" "${_IMPORT_PREFIX}/debug/bin/OpenAL32.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)

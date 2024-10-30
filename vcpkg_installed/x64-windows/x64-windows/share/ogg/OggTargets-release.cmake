#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Ogg::ogg" for configuration "Release"
set_property(TARGET Ogg::ogg APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Ogg::ogg PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/ogg.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/ogg.dll"
  )

list(APPEND _cmake_import_check_targets Ogg::ogg )
list(APPEND _cmake_import_check_files_for_Ogg::ogg "${_IMPORT_PREFIX}/lib/ogg.lib" "${_IMPORT_PREFIX}/bin/ogg.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)

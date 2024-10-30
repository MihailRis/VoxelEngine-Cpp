#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "glm::glm" for configuration "Release"
set_property(TARGET glm::glm APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(glm::glm PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/glm.lib"
  )

list(APPEND _cmake_import_check_targets glm::glm )
list(APPEND _cmake_import_check_files_for_glm::glm "${_IMPORT_PREFIX}/lib/glm.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)

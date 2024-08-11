option(VOXELENGINE_BUILD_WINDOWS_VCPKG ON)
if(VOXELENGINE_BUILD_WINDOWS_VCPKG AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

cmake_minimum_required(VERSION 3.15)
project(VoxelEngine)

option(VOXELENGINE_BUILD_APPDIR OFF)
option(VOXELENGINE_BUILD_TESTS OFF)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
add_executable(${PROJECT_NAME} src/voxel_engine.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(VOXELENGINE_BUILD_APPDIR)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/AppDir/usr/bin)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/AppDir/usr/lib)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/AppDir/usr/share/icons/hicolor/256x256)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/AppDir/usr/share/applications)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/AppDir/usr/share/VoxelEngine)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/AppDir/usr/share/VoxelEngine)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dev/VoxelEngine.png DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/AppDir/usr/share/icons/hicolor/256x256)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dev/VoxelEngine.png DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/AppDir)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dev/VoxelEngine.png DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/AppDir/.dirIcon)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dev/VoxelEngine.desktop DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/AppDir/usr/share/applications)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dev/VoxelEngine.desktop DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/AppDir/)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/AppDir/usr/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/AppDir/usr/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/AppDir/usr/bin)
  install(TARGETS VoxelEngine DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/AppDir/usr/bin)
endif()

if(MSVC)
    if(NOT CMAKE_BUILD_TYPE)
      set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    endif()
    if((CMAKE_BUILD_TYPE EQUAL "Release") OR (CMAKE_BUILD_TYPE EQUAL "RelWithDebInfo"))
      set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
      target_compile_options(${PROJECT_NAME} PRIVATE /W4 /MT /O2)
    else()
      target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:UTF-8")
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra
    # additional warnings
    -Wformat-nonliteral -Wcast-align
    -Wpointer-arith -Wundef
    -Wwrite-strings -Wno-unused-parameter)
  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -Og)
  endif()
endif()

if(VOXELENGINE_BUILD_WINDOWS_VCPKG AND WIN32)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/.git")
    find_package(Git QUIET)
    if(GIT_FOUND)
      message(STATUS "Adding vcpkg as a git submodule...")
      execute_process(COMMAND ${GIT_EXECUTABLE} submodule add https://github.com/microsoft/vcpkg.git vcpkg WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    else()
      message(FATAL_ERROR "Git not found, cannot add vcpkg submodule.")
    endif()
  endif()

  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/.git")
    message(STATUS "Initializing and updating vcpkg submodule...")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} -E chdir vcpkg ./bootstrap-vcpkg.bat WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/${CONFIG_TYPE_UPPER}/res)
  endforeach()
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie -lstdc++fs")
endif()

target_link_libraries(${PROJECT_NAME} VoxelEngineSrc ${CMAKE_DL_LIBS})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if (VOXELENGINE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

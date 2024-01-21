option(VOXELENGINE_BUILD_WINDOWS_VCPKG OFF)
if(VOXELENGINE_BUILD_WINDOWS_VCPKG AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

cmake_minimum_required(VERSION 3.15)
project(VoxelEngine)

option(VOXELENGINE_BUILD_APPDIR OFF)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

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
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -lstdc++fs
    # additional warnings
    -Wformat-nonliteral -Wcast-align
    -Wpointer-arith -Wundef
    -Wwrite-strings -Wno-unused-parameter)
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

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenAL REQUIRED)
find_package(ZLIB REQUIRED)

if (WIN32)
    if(VOXELENGINE_BUILD_WINDOWS_VCPKG)
      set(LUA_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/packages/luajit_x64-windows/lib/lua51.lib")
      set(LUA_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/packages/luajit_x64-windows/include/luajit")
      find_package(glfw3 REQUIRED)
      find_package(spng REQUIRED)
      find_package(glm REQUIRED)
      set(PNGLIB spng::spng)
    else()
      find_package(Lua REQUIRED)
      set(PNGLIB spng)
      add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw)
    endif()
else()
    # luajit has no CMakeLists.txt to use it as subdirectory, so install it manually
    find_package(Lua REQUIRED)
    find_package(PNG REQUIRED)
    set(PNGLIB PNG::PNG)
endif()

if (APPLE)
    find_package(glfw3 3.3 REQUIRED)
endif ()

set(LIBS "")

if(UNIX)
    find_package(Threads REQUIRED)
    set(LIBS ${LIBS} Threads::Threads)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie")
endif()

include_directories(${LUA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${LIBS} glfw OpenGL::GL ${OPENAL_LIBRARY} GLEW::GLEW ZLIB::ZLIB ${PNGLIB} ${LUA_LIBRARIES} ${CMAKE_DL_LIBS})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


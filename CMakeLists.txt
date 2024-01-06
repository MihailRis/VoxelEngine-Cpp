cmake_minimum_required(VERSION 3.15)
project(VoxelEngine)

option(VOXELENGINE_BUILD_APPDIR OFF)

option(VOXEL_ENGINE_USE_VULKAN "Use vulkan as backend for render" ON)
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

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenAL REQUIRED)
# luajit has no CMakeLists.txt to use it as subdirectory, so install it
find_package(Lua REQUIRED)

set(LIBS "")
if (VOXEL_ENGINE_USE_VULKAN)
    add_compile_definitions(USE_VULKAN)
    find_package(Vulkan REQUIRED)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/vma)

    set(LIBS ${LIBS} Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator)
endif ()

if (WIN32)
    set(PNGLIB spng)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw)
else()
    find_package(PNG REQUIRED)
    set(PNGLIB PNG::PNG)
endif()

if (APPLE)
    find_package(glfw3 3.3 REQUIRED)
endif ()


if(UNIX)
    find_package(Threads REQUIRED)
    set(LIBS ${LIBS} Threads::Threads)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie")
endif()

include_directories(${LUA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${LIBS} glfw OpenGL::GL ${OPENAL_LIBRARY} GLEW::GLEW ${PNGLIB} ${LUA_LIBRARIES} ${CMAKE_DL_LIBS})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


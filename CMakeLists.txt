cmake_minimum_required(VERSION 3.15)
project(VoxelEngine)

option(VOXELENGINE_BUILD_APPDIR "" OFF)
option(VOXELENGINE_BUILD_TESTS "" OFF)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(VOXELENGINE_BUILD_APPDIR)
    include(${CMAKE_CURRENT_SOURCE_DIR}/dev/cmake/BuildAppdir.cmake)
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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:UTF-8 /D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR")
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra
    # additional warnings
    -Wformat-nonliteral -Wcast-align
    -Wpointer-arith -Wundef
    -Wwrite-strings -Wno-unused-parameter)
  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -Og)
  endif()
  if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
  endif()
endif()

# if(VOXELENGINE_BUILD_WINDOWS_VCPKG AND WIN32)
#   foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
#     string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
#     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#                        COMMAND ${CMAKE_COMMAND} -E copy_directory
#                        ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/${CONFIG_TYPE_UPPER}/res)
#   endforeach()
# endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie -lstdc++fs")
endif()

if (WIN32)
    target_link_libraries(${PROJECT_NAME} VoxelEngineSrc winmm)
endif()

target_link_libraries(${PROJECT_NAME} VoxelEngineSrc ${CMAKE_DL_LIBS})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if (VOXELENGINE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

add_subdirectory(vctest)

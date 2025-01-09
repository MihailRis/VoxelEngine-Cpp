cmake_minimum_required(VERSION 3.26)
project(VoxelEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(VOXELENGINE_BUILD_APPDIR "" OFF)
option(VOXELENGINE_BUILD_TESTS "" OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # We use two types linking: for clang build is static (vcpkg triplet
    # x64-windows-static) and for msvc build is dynamic linking (vcpkg triplet
    # x64-windows) By default CMAKE_MSVC_RUNTIME_LIBRARY set by
    # MultiThreaded$<$<CONFIG:Debug>:Debug>DLL
    if(VCPKG_TARGET_TRIPLET MATCHES "static")
        # Need for MSVC clang
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        # Need for any clang
        target_link_options(VoxelEngine PRIVATE $<$<COMPILER_ID:Clang>:-static>)
    endif()
endif()

add_subdirectory(src)
add_executable(VoxelEngine src/main.cpp)

if(VOXELENGINE_BUILD_APPDIR)
    include(${CMAKE_CURRENT_SOURCE_DIR}/dev/cmake/BuildAppdir.cmake)
endif()

if(MSVC)
    target_compile_options(
        VoxelEngine
        PRIVATE /source-charset:UTF-8 /D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR
                /W4 $<$<CONFIG:Release,RelWithDebInfo>:/MT /O2>)
else()
    target_compile_options(
        VoxelEngine
        PRIVATE -Wall
                -Wextra
                # additional warnings
                -Wformat-nonliteral
                -Wcast-align
                -Wpointer-arith
                -Wundef
                -Wwrite-strings
                -Wno-unused-parameter
                $<$<CONFIG:Debug>:-Og>)
endif()

target_link_options(VoxelEngine PRIVATE $<$<CXX_COMPILER_ID:GNU>:-no-pie
                    -lstdc++fs>)

target_link_libraries(VoxelEngine PRIVATE VoxelEngineSrc
                                          $<$<PLATFORM_ID:Windows>:winmm>)

# Deploy res to build dir
add_custom_command(
    TARGET VoxelEngine
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/res $<TARGET_FILE_DIR:VoxelEngine>/res)

if(VOXELENGINE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

add_subdirectory(vctest)

project(VoxelEngineSrc)

file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_library(VoxelEngineSrc STATIC ${sources} ${headers})

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # specific for vcpkg
    find_package(OpenAL CONFIG REQUIRED)
else()
    find_package(OpenAL REQUIRED)
endif()
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(CURL REQUIRED)
find_package(glfw3 REQUIRED)
if(NOT APPLE)
    find_package(EnTT REQUIRED)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_package(vorbis REQUIRED)
    if(MSVC)
        add_library(luajit SHARED IMPORTED)
        set_target_properties(
            luajit
            PROPERTIES
                IMPORTED_LOCATION
                "$ENV{VCPKG_ROOT}/packages/luajit_${VCPKG_TARGET_TRIPLET}/bin/lua51.dll"
                IMPORTED_IMPLIB
                "$ENV{VCPKG_ROOT}/packages/luajit_${VCPKG_TARGET_TRIPLET}/lib/lua51.lib"
                INTERFACE_INCLUDE_DIRECTORIES
                "$ENV{VCPKG_ROOT}/packages/luajit_${VCPKG_TARGET_TRIPLET}/include/luajit"
        )
    else()
        add_library(luajit STATIC IMPORTED)
        set_target_properties(
            luajit
            PROPERTIES
                IMPORTED_LOCATION
                "$ENV{VCPKG_ROOT}/packages/luajit_${VCPKG_TARGET_TRIPLET}/lib/lua51.lib"
                INTERFACE_INCLUDE_DIRECTORIES
                "$ENV{VCPKG_ROOT}/packages/luajit_${VCPKG_TARGET_TRIPLET}/include/luajit"
        )
    endif()

    add_library(luajit::luajit ALIAS luajit)
else()
    find_package(PkgConfig)

    pkg_check_modules(luajit REQUIRED IMPORTED_TARGET luajit)
    pkg_check_modules(vorbis REQUIRED IMPORTED_TARGET vorbis)
    pkg_check_modules(vorbisfile REQUIRED IMPORTED_TARGET vorbisfile)
    add_library(Vorbis::vorbis ALIAS PkgConfig::vorbis)
    add_library(Vorbis::vorbisfile ALIAS PkgConfig::vorbisfile)
    add_library(luajit::luajit ALIAS PkgConfig::luajit)
endif()

target_include_directories(
    VoxelEngineSrc
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${LUAJIT_INCLUDE_DIR})

target_link_libraries(
    VoxelEngineSrc
    PRIVATE glfw
            OpenGL::GL
            GLEW::GLEW
            ZLIB::ZLIB
            PNG::PNG
            CURL::libcurl
            OpenAL::OpenAL
            Vorbis::vorbis
            Vorbis::vorbisfile
            luajit::luajit
    PUBLIC glm::glm # Need public for src/delegates.hpp, which including to
                    # main.cpp
)

target_compile_options(
    VoxelEngineSrc
    PUBLIC $<$<CXX_COMPILER_ID:MSVC>:
           /source-charset:UTF-8
           /D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR
           /W4
           >
           $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
           -Wall
           -Wextra
           # additional warnings
           -Wformat-nonliteral
           -Wcast-align
           -Wpointer-arith
           -Wundef
           -Wwrite-strings
           -Wno-unused-parameter
           $<$<CONFIG:Debug>:-Og>
           >)

target_link_options(
    VoxelEngineSrc
    PUBLIC
    $<$<CXX_COMPILER_ID:GNU>:
    -no-pie
    -lstdc++fs
    >
    # Need for static compilation on Windows with clang TODO: Make single build
    # on Windows to avoid dependence on combinations of platforms and compilers
    # and make it independent
    $<$<PLATFORM_ID:Windows>:$<$<CXX_COMPILER_ID:Clang>:-static>>)

project(VoxelEngineSrc)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_library(VoxelEngineSrc STATIC ${SOURCES} ${HEADERS})

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
if(NOT APPLE)
    find_package(EnTT REQUIRED)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Use directly linking to lib instead PkgConfig (because pkg-config dont
    # install on windows as default) TODO: Do it with findLua.
    if(MSVC)
        set(LUA_INCLUDE_DIR
            "$ENV{VCPKG_ROOT}/packages/luajit_${VCPKG_TARGET_TRIPLET}/include/luajit"
        )
        find_package(Lua REQUIRED)
    else()
        # Used for mingw-clang cross compiling from msys2
        target_link_libraries(VoxelEngineSrc PRIVATE luajit-5.1)
    endif()
    find_package(glfw3 REQUIRED)
    find_package(vorbis REQUIRED)
    set(VORBIS_LIBRARIES Vorbis::vorbis Vorbis::vorbisfile)

elseif(APPLE)
    find_package(PkgConfig)

    pkg_check_modules(LUAJIT REQUIRED luajit)
    set(LUA_INCLUDE_DIR "/opt/homebrew/include/luajit-2.1")
    set(LUA_LIBRARIES "/opt/homebrew/lib/libluajit-5.1.a")

    pkg_check_modules(VORBIS REQUIRED vorbis vorbisfile)
    set(VORBIS_LIBRARIES ${VORBIS_LDFLAGS})
else()
    find_package(PkgConfig)

    pkg_check_modules(LUAJIT REQUIRED luajit)
    set(LUA_LIBRARIES ${LUAJIT_LIBRARIES})
    set(LUA_INCLUDE_DIR ${LUAJIT_INCLUDE_DIRS})

    pkg_check_modules(VORBIS REQUIRED vorbis vorbisfile)
    set(VORBIS_LIBRARIES ${VORBIS_LDFLAGS})
endif()

if(UNIX)
    find_package(glfw3 3.3 REQUIRED)
endif()

target_include_directories(
    VoxelEngineSrc
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(
    VoxelEngineSrc
    PRIVATE glfw
            OpenGL::GL
            GLEW::GLEW
            ZLIB::ZLIB
            PNG::PNG
            CURL::libcurl
            OpenAL::OpenAL
            ${VORBIS_LIBRARIES}
            ${LUA_LIBRARIES}
    PUBLIC glm::glm # Need public for src/delegates.hpp, which including to
                    # main.cpp
)

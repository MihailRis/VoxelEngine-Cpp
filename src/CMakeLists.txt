project(VoxelEngineSrc)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenAL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(CURL REQUIRED)
if (NOT APPLE)
    find_package(EnTT REQUIRED)
endif()

set(LIBS "")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Use directly linking to lib instead PkgConfig (why? idk) TODO: Do it with find_package/PkgConfig
    set(LUA_LIBRARIES "$ENV{VCPKG_ROOT}/packages/luajit_${VCPKG_TARGET_TRIPLET}/lib/lua51.lib")
    set(LUA_INCLUDE_DIR "$ENV{VCPKG_ROOT}/packages/luajit_${VCPKG_TARGET_TRIPLET}/include/luajit")
   
    find_package(glfw3 REQUIRED)
    find_package(glm REQUIRED)
    find_package(vorbis REQUIRED)
    set(VORBISLIB Vorbis::vorbis Vorbis::vorbisfile)

elseif(APPLE)
    find_package(PkgConfig)
    pkg_check_modules(LUAJIT REQUIRED luajit)
    pkg_check_modules(VORBIS REQUIRED vorbis vorbisfile)
    set(LUA_INCLUDE_DIR "/opt/homebrew/include/luajit-2.1")
    set(LUA_LIBRARIES "/opt/homebrew/lib/libluajit-5.1.a")
    message(STATUS "LUA Libraries: ${LUA_LIBRARIES}")
    message(STATUS "LUA Include Dir: ${LUA_INCLUDE_DIR}")
    
    set(VORBISLIB ${VORBIS_LDFLAGS})
    message(STATUS "Vorbis Lib: ${VORBIS_LDFLAGS}")
else()
    find_package(PkgConfig)
    pkg_check_modules(LUAJIT REQUIRED luajit)
    pkg_check_modules(VORBIS REQUIRED vorbis vorbisfile)
    set(LUA_LIBRARIES ${LUAJIT_LIBRARIES})
    set(LUA_INCLUDE_DIR ${LUAJIT_INCLUDE_DIRS})
    set(VORBISLIB ${VORBIS_LDFLAGS})
endif()

if(UNIX)
    find_package(glfw3 3.3 REQUIRED)
    find_package(Threads REQUIRED)
    set(LIBS ${LIBS} Threads::Threads)
endif()

include_directories(${LUA_INCLUDE_DIR})
include_directories(${CURL_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} ${LIBS} glfw OpenGL::GL ${OPENAL_LIBRARY} GLEW::GLEW ZLIB::ZLIB PNG::PNG CURL::libcurl ${VORBISLIB} ${LUA_LIBRARIES} ${CMAKE_DL_LIBS})

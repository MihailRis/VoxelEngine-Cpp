cmake_minimum_required(VERSION 3.15)
project(VoxelEngine)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
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

set(LIBS "")

if(UNIX)
    find_package(Threads REQUIRED)
    set(LIBS ${LIBS} Threads::Threads)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie")
endif()


target_link_libraries(${PROJECT_NAME} ${LIBS} glfw OpenGL::GL ${OPENAL_LIBRARY} GLEW::GLEW ${PNGLIB})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

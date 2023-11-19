cmake_minimum_required(VERSION 3.15)
project(VoxelEngine)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:UTF-8")
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -lstdc++fs
    # additional warnings
    -Wformat-nonliteral -Wcast-align 
    -Wpointer-arith -Wundef
    -Wswitch-enum
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
endif(WIN32)

set(LIBS "")

if(UNIX)
    find_package(Threads REQUIRED)
    set(LIBS ${LIBS} Threads::Threads)
endif(UNIX)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie")
endif()


target_link_libraries(${PROJECT_NAME} ${LIBS} glfw OpenGL::GL ${OPENAL_LIBRARY} GLEW::GLEW ${PNGLIB})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

cmake_minimum_required(VERSION 3.15)
project(VoxelEngine)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

option(VE_USE_SYSTEM_LIBS "Use system installed libraries" ON)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenAL REQUIRED)
find_package(PNG REQUIRED)

if(NOT VE_USE_SYSTEM_LIBS)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/glm)
else()
    find_package(glfw3 REQUIRED)
    find_package(glm REQUIRED)
endif(NOT VE_USE_SYSTEM_LIBS)

target_link_libraries(${PROJECT_NAME} glfw glm OpenGL::GL ${OPENAL_LIBRARY} GLEW::GLEW PNG::PNG)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/build)

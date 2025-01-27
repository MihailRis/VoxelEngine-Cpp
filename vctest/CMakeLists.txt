project(vctest)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

if(MSVC)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE
            Release
            CACHE STRING "Build type" FORCE)
    endif()
    if((CMAKE_BUILD_TYPE EQUAL "Release") OR (CMAKE_BUILD_TYPE EQUAL
                                              "RelWithDebInfo"))
        set(CMAKE_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Release>:Release>")
        target_compile_options(${PROJECT_NAME} PRIVATE /W4 /MT /O2)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE -Wall
                -Wextra
                -Wformat-nonliteral
                -Wcast-align
                -Wpointer-arith
                -Wundef
                -Wwrite-strings
                -Wno-unused-parameter)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie")
endif()

target_include_directories(
    ${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_DL_LIBS})

project(vctest)

add_executable(vctest ${CMAKE_CURRENT_LIST_DIR}/main.cpp)

# Needed for header-only source util/ArgsReader.hpp
target_include_directories(vctest PRIVATE ${CMAKE_SOURCE_DIR}/src)

if(MSVC)
    target_compile_options(vctest PRIVATE /W4)
else()
    target_compile_options(
        vctest
        PRIVATE -Wall
                -Wextra
                -Wformat-nonliteral
                -Wcast-align
                -Wpointer-arith
                -Wundef
                -Wwrite-strings
                -Wno-unused-parameter)
endif()

target_link_options(vctest PRIVATE $<$<CXX_COMPILER_ID:GNU>:-no-pie -lstdc++fs>)

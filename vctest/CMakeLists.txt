project(vctest)

add_executable(vctest ${CMAKE_CURRENT_LIST_DIR}/main.cpp)

# Needed for header-only source util/ArgsReader.hpp
target_include_directories(vctest PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_compile_options(
    vctest
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
            -Wall
            -Wextra
            -Wformat-nonliteral
            -Wcast-align
            -Wpointer-arith
            -Wundef
            -Wwrite-strings
            -Wno-unused-parameter
            >)

target_link_options(vctest PRIVATE $<$<CXX_COMPILER_ID:GNU>:-no-pie>)

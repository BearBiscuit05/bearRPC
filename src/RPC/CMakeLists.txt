cmake_minimum_required(VERSION 2.6)
project(bearRPC)


set(CXX_FLAGS
        -fno-omit-frame-pointer # linux perf
        -Wall
        -Wextra
        -Werror
        -Wconversion
        -Wno-unused-parameter
        -Wold-style-cast
        -Woverloaded-virtual
        -Wpointer-arith
        -Wshadow
        -Wwrite-strings
        -std=c++17
        -march=native
        -rdynamic)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

include_directories(
        3rd/tinyev
        3rd/jackson
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_BINARY_DIR})
        

add_subdirectory(3rd/tinyev)
add_subdirectory(3rd/jackson)

add_executable(parse11 parse.cc)
target_link_libraries(parse11 jackson)
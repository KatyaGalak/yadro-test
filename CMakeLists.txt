cmake_minimum_required(VERSION 3.12)

project(
    yadro-test
    VERSION 1.0
    DESCRIPTION "C++ YADRO test task"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib)
add_subdirectory(bin)

enable_testing()
add_subdirectory(tests)
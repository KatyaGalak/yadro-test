include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    tape_tests 
    TapeTests.cpp
)

target_link_libraries(
    tape_tests 
    TapeLib   
    GTest::gtest_main
)

target_include_directories(tape_tests PUBLIC ${PROJECT_SOURCE_DIR}/lib)

include(GoogleTest)
gtest_discover_tests(tape_tests)
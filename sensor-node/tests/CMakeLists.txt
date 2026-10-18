include(FetchContent)
FetchContent_Declare(googletest
	URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz)
FetchContent_MakeAvailable(googletest)

add_executable(smoke test_smoke.cpp)
target_link_libraries(smoke PRIVATE GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(smoke)
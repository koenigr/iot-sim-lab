add_library(stats INTERFACE)
target_include_directories(stats INTERFACE include)
target_compile_features(stats INTERFACE cxx_std_17)

include(FetchContent)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(stats_tests tests/test_ring_buffer.cpp)
target_link_libraries(stats_tests PRIVATE stats GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(stats_tests)
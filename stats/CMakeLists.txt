add_library(stats INTERFACE)
target_include_directories(stats INTERFACE include)
target_compile_features(stats INTERFACE cxx_std_20)

include(FetchContent)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)
set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES CXX_CLANG_TIDY "")

add_executable(stats_tests
	tests/test_ring_buffer.cpp
	tests/test_moving_average.cpp
	tests/test_ewma.cpp
	tests/test_ewma_fixed.cpp
	tests/test_median_filter.cpp
	tests/test_welford.cpp
	tests/test_zscore_detector.cpp
	tests/test_ground_truth.cpp
)

target_link_libraries(stats_tests
	PRIVATE
		stats 
		gtest_main
		generator
)

include(GoogleTest)
gtest_discover_tests(stats_tests)
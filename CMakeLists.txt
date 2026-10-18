cmake_minimum_required(VERSION 3.20)
project(iot_sim_lab VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -Wpedantic)

enable_testing()
add_subdirectory(sensor-node)
add_subdirectory(collector)

option(ENABLE_CLANG_TIDY "Run clang-tidy during build" OFF)
if(ENABLE_CLANG_TIDY)
	find_program(CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
	set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE})
endif()
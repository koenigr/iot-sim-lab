add_library(generator src/generator.cpp)

target_include_directories(generator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(sensor-node src/main.cpp)
target_link_libraries(sensor-node PRIVATE generator)

add_subdirectory(tests)
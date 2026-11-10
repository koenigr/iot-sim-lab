add_library(generator src/generator.cpp)
target_include_directories(generator PUBLIC src)

add_executable(sensor-node src/main.cpp)
target_link_libraries(sensor-node PRIVATE generator)

add_subdirectory(tests)
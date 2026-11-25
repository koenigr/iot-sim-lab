add_library(generator src/generator.cpp)

target_include_directories(generator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

find_package(PahoMqttCpp REQUIRED)

add_executable(sensor-node
	src/main.cpp
	src/publisher.cpp
)
target_link_libraries(sensor-node PRIVATE generator PahoMqttCpp::paho-mqttpp3)

add_subdirectory(tests)

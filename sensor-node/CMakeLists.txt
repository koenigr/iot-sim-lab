add_library(generator src/generator.cpp)
target_include_directories(generator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(payload src/payload.cpp)
target_include_directories(payload PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(payload PUBLIC generator)

find_package(PahoMqttCpp REQUIRED)

add_executable(sensor-node
	src/main.cpp
	src/publisher.cpp
)
target_link_libraries(sensor-node PRIVATE payload PahoMqttCpp::paho-mqttpp3)

add_subdirectory(tests)

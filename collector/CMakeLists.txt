find_package(PahoMqttCpp REQUIRED)

add_library(topic stc/topic.cpp)
target_include_directories(topic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(collector
	src/main.cpp
	src/subscriber.cpp
)
target_link_libraries(collector PRIVATE topic PahoMqttCpp::paho-mqttpp3)

add_subdirectory(tests)
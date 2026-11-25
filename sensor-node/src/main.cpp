#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "generator.hpp"
#include "publisher.hpp"

int main() {
	PublisherConfig cfg;
	if (const char* uri = std::getenv("MQTT_URI")) {
		cfg.broker_uri = uri;
	}

	Generator gen;
	try {
		Publisher pub{cfg};
		for (int i = 0; i < 20; ++i) {
			pub.publish(gen.next());
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	} catch (const mqtt::exception& e) {
		std::cerr << "mqtt error: " << e.what() << '\n';
		return 1;
	}
}

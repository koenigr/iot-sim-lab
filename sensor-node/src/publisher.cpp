#include "publisher.hpp"
#include "payload.hpp"

#include <chrono>
#include <iostream>

Publisher::Publisher(PublisherConfig cfg)
    : cfg_(std::move(cfg)), client_(cfg_.broker_uri, cfg_.node) {
	auto opts =
	    mqtt::connect_options_builder()
	        .clean_session()
	        .keep_alive_interval(std::chrono::seconds(10))
	        .automatic_reconnect(true)
	        .will(mqtt::message(status_topic(), "offline", cfg_.qos, true))
	        .finalize();

	client_.connect(opts);
	client_.publish(
	    mqtt::make_message(status_topic(), "online", cfg_.qos, true));
}

Publisher::~Publisher() {
	// LWT only fires on unclean disconnect, so say goodbye explicitly
	try {
		client_.publish(
		    mqtt::make_message(status_topic(), "offline", cfg_.qos, true));
		client_.disconnect();
	} catch (const mqtt::exception& e) {
		std::cerr << "mqtt shutdown error: " << e.what() << '\n';
	}
}

std::string Publisher::status_topic() const {
	return cfg_.site + '/' + cfg_.area + '/' + cfg_.node + "/status";
}

std::string Publisher::data_topic() const {
	return cfg_.site + '/' + cfg_.area + '/' + cfg_.node + '/' + cfg_.sensor +
	       '/' + cfg_.metric;
}

void Publisher::publish(const Sample& s) {
	client_.publish(mqtt::make_message(
	    data_topic(), make_payload_v1(s, seq_++, cfg_.unit), cfg_.qos, false));
}

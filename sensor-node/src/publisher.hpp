#pragma once
#include <mqtt/client.h>

#include <cstdint>
#include <string>

#include "generator.hpp"

struct PublisherConfig {
	std::string broker_uri = "tcp://localhost:1883";
	std::string site = "lab";
	std::string area = "room1";
	std::string node = "node01";
	std::string sensor = "temp01";
	std::string metric = "temperature";
	std::string unit = "C";
	int qos = 1;
};

class Publisher {
public:
	explicit Publisher(PublisherConfig cfg);
	~Publisher();
	Publisher(const Publisher&) = delete;
	Publisher& operator=(const Publisher&) = delete;
	Publisher(Publisher&&) = delete;
	Publisher& operator=(Publisher&&) = delete;

	void publish(const Sample& s);

private:
	std::string status_topic() const;
	std::string data_topic() const;

	PublisherConfig cfg_;
	mqtt::client client_;
	std::uint64_t seq_ = 0;
};

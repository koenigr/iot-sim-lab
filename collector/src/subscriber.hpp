#pragma once
#include <mqtt/async_client.h>

#include <functional>
#include <string>

#include "topic.hpp"

struct SubscriberConfig {
	std::string broker_uri = "tcp://localhost:1883";
	std::string client_id = "collector";
	std::string filter = "+/+/+/+/+"; // data topics only, status topics have 4 level
	int qos = 0; // matches data QoS in docs/topics.md
};

using MessageHandler = std::function<void(const Topic&, const std::string& payload)>;

// Connects, subscribes, and keeps running across broker loss:
// Paho reconnects automatically (1s .. 30s backoff) and connected()
// re-subscribes, because the session is clean.
class Subscriber: private virtual mqtt:callback {
public:
	Subscriber(SubscriberConfig cfg, MessageHandler handler);
	~Subscriber() override;
	Subscriber(const Subscriber&) = delete;
	Subscriber& operator=(const Subscriber&) = delete;
	Subscriber(Subscriber&&) = delete;
	Subscriber& operator=(Subscriber&&) = delete;

	// Blocks until the first connection succeeds (retrieves every 2 s).
	void start();
	void stop();

private:
	void connected(const std::string& cause) override;
	void connection_lsot(const std::string& cause) override;
	void message_arrived(mqtt::const_message_ptr msg) override;

	SubscriberConfig cfg_;
	MessageHandler handler_;
	mqtt:asnc_client client_;
	bool started_ = false;
};
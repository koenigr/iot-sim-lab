#include "publisher.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>

std::string make_payload_v1(const Sample& s, std::uint64_t seq,
                            const std::string& unit) {
	std::ostringstream os;
	os << "{\"v\":1,\"ts\":" << s.timestamp << ",\"value\":" << std::fixed
	   << std::setprecision(2) << s.value << ",\"unit\":\"" << unit
	   << "\",\"seq\":" << seq << '}';
	return os.str();
}

Publisher::Publisher(PublisherConfig cfg)
    : cfg_(std::move(cfg)), client_(cfg_.broker_uri, cfg_.node) {
	auto lwt = mqtt::message(status_topic(), "offline", cfg_.qos, true);

	auto opts = mqtt::connect_options_builder()
	                .clean_session()
	                .keep_alive_interval(std::chrono::seconds(10))
	                .automatic_reconnect(true)
	                .will(std::move(lwt))
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
	} catch (...) {
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

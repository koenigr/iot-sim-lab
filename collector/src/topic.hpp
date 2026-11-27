#pragma once
#include <optional>
#include <string>
#include <string_view>

// Data topic: <site>/<area>/<node>/<sensor>/<metric> (see docs/topics.md)
struct Topic {
	std::string site;
	std::string area;
	std::string node;
	std::string sensor;
	std::string metric;

	bool operator==(const Topic&) const = default;
};

// Returhs std::nullopt unless the topic has exactly 5 non-empty levels
// consisting only of [a-z0-9_-]. Status topics (4 levels) are rejeced.
std::optional<Topic> parse_topic(std::string_view topic);
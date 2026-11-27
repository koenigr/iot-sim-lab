#include <gtest/gtest.h>

#include "payload.hpp"

TEST(Payload, V1Format) {
	const Sample s{
	    .timestamp = 1000, .value = 21.456, .truth = 21.0, .is_anomaly = false};
	EXPECT_EQ(make_payload_v1(s, 7, "C"),
	          R"({"v":1,"ts":1000,"value":21.46,"unit":"C","seq":7})");
}

TEST(Payload, DoesNotLeakGroundTruth) {
	const Sample s{
	    .timestamp = 1, .value = 1.0, .truth = 1.0, .is_anomaly = true};
	const auto p = make_payload_v1(s, 0, "C");
	EXPECT_EQ(p.find("truth"), std::string::npos);
	EXPECT_EQ(p.find("is_anomaly"), std::string::npos);
}

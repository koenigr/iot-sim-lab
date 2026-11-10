#include <gtest/gtest.h>

#include <cmath>

#include "stats/zscore_detector.hpp"

using stats::ZScoreDetector;

namespace {

// Alternating 19/21 -> mean 20, population stddev 1.
ZScoreDetector warmed_up(ZScoreDetector::Config cfg = {3.0, 10, false}) {
	ZScoreDetector d(cfg);
	for (int i = 0; i < 100; ++i) d.update(i % 2 == 0 ? 19.0 : 21.0);
	return d;
}

}  // namespace

TEST(ZScoreDetector, IssueExample) {
	auto d = warmed_up();
	ASSERT_NEAR(d.mean(), 20.0, 1e-9);
	ASSERT_NEAR(d.stddev(), 1.0, 1e-9);

	const auto r = d.update(26.0);
	EXPECT_NEAR(r.z, 6.0, 1e-9);
	EXPECT_TRUE(r.is_anomaly);
}

TEST(ZScoreDetector, NormalSampleIsNotAnomaly) {
	auto d = warmed_up();
	const auto r = d.update(21.0);
	EXPECT_NEAR(r.z, 1.0, 1e-9);
	EXPECT_FALSE(r.is_anomaly);
}

TEST(ZScoreDetector, NegativeDeviationIsAnomaly) {
	auto d = warmed_up();
	const auto r = d.update(14.0);
	EXPECT_NEAR(r.z, -6.0, 1e-9);
	EXPECT_TRUE(r.is_anomaly);
}

TEST(ZScoreDetector, ThresholdIsConfigurable) {
	auto d = warmed_up({10.0, 10, false});
	EXPECT_FALSE(d.update(26.0).is_anomaly);  // z = 6 < 10
}

TEST(ZScoreDetector, AnomalyIsNotFedByDefault) {
	auto d = warmed_up();
	const auto n = d.count();
	d.update(26.0);
	EXPECT_EQ(d.count(), n);
	EXPECT_NEAR(d.mean(), 20.0, 1e-9);
	EXPECT_NEAR(d.stddev(), 1.0, 1e-9);
}

TEST(ZScoreDetector, AnomalyIsFedWhenConfigured) {
	auto d = warmed_up({3.0, 10, true});
	const auto n = d.count();
	d.update(26.0);
	EXPECT_EQ(d.count(), n + 1);
	EXPECT_GT(d.mean(), 20.0);
}

TEST(ZScoreDetector, WarmUpNeverFlagsAndAlwaysFeeds) {
	ZScoreDetector d({3.0, 10, false});
	for (int i = 0; i < 9; ++i) {
		EXPECT_FALSE(d.update(i * 100.0).is_anomaly);
	}
	EXPECT_EQ(d.count(), 9u);
}

TEST(ZScoreDetector, ZeroStddev) {
	ZScoreDetector d({3.0, 2, false});
	d.update(5.0);
	d.update(5.0);
	EXPECT_DOUBLE_EQ(d.update(5.0).z, 0.0);
	const auto r = d.update(6.0);
	EXPECT_TRUE(std::isinf(r.z));
	EXPECT_TRUE(r.is_anomaly);
}

TEST(ZScoreDetector, ResetClearsState) {
	auto d = warmed_up();
	d.reset();
	EXPECT_EQ(d.count(), 0u);
	EXPECT_DOUBLE_EQ(d.mean(), 0.0);
}
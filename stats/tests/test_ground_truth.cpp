// stats/tests/test_ground_truth.cpp
#include <gtest/gtest.h>
#include <cmath>
#include <vector>

#include "stats/moving_average.hpp"
#include "stats/ewma.hpp"
#include "stats/zscore_detector.hpp"
#include "generator.hpp"

namespace {

constexpr unsigned kSeed = 42;
constexpr std::size_t kN = 2000;

struct Confusion { int tp = 0, fp = 0, fn = 0, tn = 0; };

double precision(const Confusion& c) { return c.tp + c.fp ? double(c.tp) / (c.tp + c.fp) : 0.0; }
double recall(const Confusion& c)    { return c.tp + c.fn ? double(c.tp) / (c.tp + c.fn) : 0.0; }

double rmse(const std::vector<double>& est, const std::vector<double>& truth) {
	double s = 0;
	for (std::size_t i = 0; i < est.size(); ++i) { double d = est[i] - truth[i]; s += d * d; }
	return std::sqrt(s / est.size());
}

}  // namespace

TEST(GroundTruth, FiltersReduceErrorVsRaw) {
	auto samples = generate(kSeed, kN);              // fixed seed
	stats::MovingAverage<16> ma;
	stats::Ewma ewma{0.2};
	std::vector<double> raw, ma_out, ewma_out, truth;
	for (const auto& s : samples) {
		raw.push_back(s.value);
		ma_out.push_back(ma.update(s.value));
		ewma_out.push_back(ewma.update(s.value));
		truth.push_back(s.truth);
	}
	const double e_raw = rmse(raw, truth);
	EXPECT_LT(rmse(ma_out, truth),   e_raw);
	EXPECT_LT(rmse(ewma_out, truth), e_raw);
}

TEST(GroundTruth, DetectorPrecisionRecall) {
	auto samples = generate(kSeed, kN);
	stats::ZScoreDetector det{/*threshold=*/3.0};
	Confusion c;
	for (const auto& s : samples) {
		const bool pred = det.update(s.value).is_anomaly;
		if (pred && s.is_anomaly) ++c.tp;
		else if (pred)            ++c.fp;
		else if (s.is_anomaly)    ++c.fn;
		else                      ++c.tn;
	}
	EXPECT_GE(precision(c), 0.8);
	EXPECT_GE(recall(c),    0.8);
}
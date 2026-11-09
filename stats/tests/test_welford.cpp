#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "stats/welford.hpp"

using stats::Welford;

namespace {

// Naive sample variance via E[x^2] - E[x]^2, for comparison only.
double naive_variance(const std::vector<double>& xs) {
	double s = 0.0;
	double s2 = 0.0;
	for (double x : xs) {
		s += x;
		s2 += x * x;
	}
	const double n = static_cast<double>(xs.size());
	const double mean = s / n;
	return (s2 / n - mean * mean) * n / (n - 1.0);
}

}  // namespace

TEST(Welford, EmptyHasZeroCountAndNaNVariance) {
	Welford w;
	EXPECT_EQ(w.count(), 0U);
	EXPECT_TRUE(std::isnan(w.variance()));
}

TEST(Welford, SingleSampleHasNaNVariance) {
	Welford w;
	w.add(1.0);
	EXPECT_DOUBLE_EQ(w.mean(), 1.0);
	EXPECT_TRUE(std::isnan(w.variance()));
}

TEST(Welford, MatchesKnownMeanAndVariance) {
	Welford w;
	for (double x : {2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0}) {
		w.add(x);
	}
	EXPECT_DOUBLE_EQ(w.mean(), 5.0);
	EXPECT_DOUBLE_EQ(w.variance_pop(), 4.0);
}

TEST(Welford, StableWithLargeOffset) {
	const std::vector<double> xs{1e9 + 4, 1e9 + 7, 1e9 + 13, 1e9 + 16};
	Welford w;
	for (double x : xs) {
		w.add(x);
	}
	EXPECT_DOUBLE_EQ(w.mean(), 1e9 + 10.0);
	EXPECT_NEAR(w.variance(), 30.0, 1e-6);
}

TEST(Welford, NaiveFormulaFailsWithLargeOffset) {
	const std::vector<double> xs{1e9 + 4, 1e9 + 7, 1e9 + 13, 1e9 + 16};
	// Cancellation of ~1e18 terms: naive result is far from the true 30.
	EXPECT_GT(std::abs(naive_variance(xs) - 30.0), 1.0);
}
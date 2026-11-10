// stats/tests/ewma_fixed_test.cpp
#include <gtest/gtest.h>
#include <stats/ewma_fixed.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>

using stats::EwmaFixed;

TEST(EwmaFixed, FirstSampleInitializes) {
	EwmaFixed f{16384};
	EXPECT_EQ(f.update(EwmaFixed::to_fixed(3.5)), EwmaFixed::to_fixed(3.5));
}

TEST(EwmaFixed, MatchesSnippetForAlphaQuarter) {
	EwmaFixed f{16384};
	f.update(0);
	EXPECT_EQ(f.update(EwmaFixed::to_fixed(4.0)), EwmaFixed::to_fixed(1.0));
}

TEST(EwmaFixed, AlphaZeroHoldsAlphaOneFollows) {
	EwmaFixed hold{0}, follow{EwmaFixed::kOne};
	hold.update(100); follow.update(100);
	EXPECT_EQ(hold.update(5000), 100);
	EXPECT_EQ(follow.update(5000), 5000);
}

TEST(EwmaFixed, ExtremeJumpDoesNotOverflow) {
	constexpr int32_t lo = std::numeric_limits<int32_t>::min();
	constexpr int32_t hi = std::numeric_limits<int32_t>::max();
	for (int32_t a : {1, 16384, 65535, EwmaFixed::kOne}) {
		EwmaFixed f{a};
		f.update(lo);
		const int32_t y = f.update(hi);
		EXPECT_GE(y, lo);
		EXPECT_LE(y, hi);
		const int32_t z = f.update(lo);
		EXPECT_GE(z, lo);
		EXPECT_LE(z, hi);
	}
}

TEST(EwmaFixed, ConvergesToConstantInput) {
	EwmaFixed f{16384};
	f.update(0);
	const int32_t target = EwmaFixed::to_fixed(-12.25);
	for (int i = 0; i < 200; ++i) f.update(target);
	EXPECT_LE(std::abs(f.value() - target), 2);  // rounding dead-band ~ 0.5/alpha LSB
}

TEST(EwmaFixed, ErrorVsDoubleIsBounded) {
	for (int32_t alpha_q : {EwmaFixed::to_fixed(0.05), 16384, 32768, 52429}) {
		const double alpha = EwmaFixed::to_double(alpha_q);
		EwmaFixed fixed{alpha_q};
		double ref = 0.0;
		bool first = true;
		double max_err = 0.0;

		std::mt19937 rng{42};
		std::uniform_real_distribution<double> dist(-1000.0, 1000.0);
		for (int i = 0; i < 100000; ++i) {
			const int32_t xq = EwmaFixed::to_fixed(dist(rng));
			const double  x  = EwmaFixed::to_double(xq);  // same quantized input
			ref = first ? x : ref + alpha * (x - ref);
			first = false;
			const double y = EwmaFixed::to_double(fixed.update(xq));
			max_err = std::max(max_err, std::abs(y - ref));
		}
		// Per-step rounding error <= 0.5 LSB; the filter contracts it,
		// so the steady-state bound is 0.5/alpha LSB (+1 LSB slack).
		const double lsb   = 1.0 / EwmaFixed::kOne;
		const double bound = (0.5 / alpha + 1.0) * lsb;
		EXPECT_LE(max_err, bound) << "alpha=" << alpha << " max_err=" << max_err;
		RecordProperty("max_abs_err_alpha_" + std::to_string(alpha_q), std::to_string(max_err));
	}
}
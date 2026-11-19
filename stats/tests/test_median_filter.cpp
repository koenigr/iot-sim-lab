#include <gtest/gtest.h>

#include <stats/median_filter.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

using stats::MedianFilter;

TEST(MedianFilter, NoOutputUntilWindowIsFull) {
	MedianFilter<float, 3> m;
	EXPECT_FALSE(m.push(10.F).has_value());
	EXPECT_FALSE(m.full());
	EXPECT_FALSE(m.push(11.F).has_value());
	EXPECT_EQ(m.size(), 2u);
	EXPECT_TRUE(m.push(90.F).has_value());
	EXPECT_TRUE(m.full());
}

TEST(MedianFilter, RemovesSingleSampleSpike) {
	MedianFilter<float, 3> m;
	m.push(10.F);
	m.push(11.F);
	EXPECT_FLOAT_EQ(m.push(90.F).value(), 11.F);
	EXPECT_FLOAT_EQ(m.push(12.F).value(), 12.F);
}

TEST(MedianFilter, WindowOfOneIsPassThrough) {
	MedianFilter<int, 1> m;
	EXPECT_EQ(m.push(5).value(), 5);
	EXPECT_EQ(m.push(-7).value(), -7);
}

TEST(MedianFilter, LargerWindowRemovesTwoSpikes) {
	MedianFilter<int, 5> m;
	for (int v : {1, 2, 3, 4}) {
		EXPECT_FALSE(m.push(v).has_value());
	}
	EXPECT_EQ(m.push(100).value(), 3); // {1,2,3,4,100}
	EXPECT_EQ(m.push(100).value(), 4); // {2,3,4,100,100}
}

TEST(MedianFilter, ConstantInputStaysConstant) {
	MedianFilter<double, 5> m;
	std::optional<double> out;
	for (int i = 0; i < 20; ++i) {
		out = m.push(3.5);
	}
	EXPECT_DOUBLE_EQ(out.value(), 3.5);
}

TEST(MedianFilter, HandlesNegativeAndMonotonicInput) {
	MedianFilter<int, 3> m;
	m.push(-1);
	m.push(-2);
	EXPECT_EQ(m.push(-3).value(), -2);
	EXPECT_EQ(m.push(-4).value(), -3);
}

TEST(MedianFilter, ResetClearsWindow) {
	MedianFilter<int, 3> m;
	m.push(1);
	m.push(2);
	m.push(3);
	m.reset();
	EXPECT_EQ(m.size(), 0u);
	EXPECT_FALSE(m.push(100).has_value());
	EXPECT_FALSE(m.push(100).has_value());
	const auto out = m.push(5);
	ASSERT_TRUE(out.has_value());
	EXPECT_EQ(out.value(), 100); // old samples must not leak through
}

TEST(MedianFilter, MatchesBruteForceAcrossWraparound) {
	constexpr std::size_t W = 5;
	MedianFilter<int, W> m;
	std::vector<int> in;
	unsigned x = 12345;
	for (int i = 0; i < 200; ++i) {
		x = (x * 1103515245U) + 12345U; // deterministic pseudo-random
		in.push_back(static_cast<int>((x >> 16) % 100));
	}
	for (std::size_t i = 0; i < in.size(); ++i) {
		auto out = m.push(in[i]);
		if (i + 1 < W) {
			EXPECT_FALSE(out.has_value());
			continue;
		}
		std::array<int, W> win{};
		const auto lo = static_cast<std::ptrdiff_t>(i + 1 - W);
		const auto hi = static_cast<std::ptrdiff_t>(i + 1);
		std::copy(in.begin() + lo, in.begin() + hi, win.begin());
		std::sort(win.begin(), win.end());
		ASSERT_TRUE(out.has_value());
		EXPECT_EQ(out.value(), win[W / 2]) << "at i=" << i;
	}
}

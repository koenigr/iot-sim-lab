#include <gtest/gtest.h>

#include <stats/median_filter.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

using stats::MedianFilter;

TEST(MedianFilter, NoOutputUntilWindowIsFull) {
    MedianFilter<float, 3> m;
    EXPECT_FALSE(m.push(10.f).has_value());
    EXPECT_FALSE(m.full());
    EXPECT_FALSE(m.push(11.f).has_value());
    EXPECT_EQ(m.size(), 2u);
    EXPECT_TRUE(m.push(90.f).has_value());
    EXPECT_TRUE(m.full());
}

TEST(MedianFilter, RemovesSingleSampleSpike) {
    MedianFilter<float, 3> m;
    m.push(10.f);
    m.push(11.f);
    EXPECT_FLOAT_EQ(*m.push(90.f), 11.f);
    EXPECT_FLOAT_EQ(*m.push(12.f), 12.f);
}

TEST(MedianFilter, WindowOfOneIsPassThrough) {
    MedianFilter<int, 1> m;
    EXPECT_EQ(*m.push(5), 5);
    EXPECT_EQ(*m.push(-7), -7);
}

TEST(MedianFilter, LargerWindowRemovesTwoSpikes) {
    MedianFilter<int, 5> m;
    for (int v : {1, 2, 3, 4}) {
        EXPECT_FALSE(m.push(v).has_value());
    }
    EXPECT_EQ(*m.push(100), 4);  // {1,2,3,4,100}
    EXPECT_EQ(*m.push(100), 4);  // {2,3,4,100,100}
}

TEST(MedianFilter, ConstantInputStaysConstant) {
    MedianFilter<double, 5> m;
    std::optional<double> out;
    for (int i = 0; i < 20; ++i) {
        out = m.push(3.5);
    }
    EXPECT_DOUBLE_EQ(*out, 3.5);
}

TEST(MedianFilter, HandlesNegativeAndMonotonicInput) {
    MedianFilter<int, 3> m;
    m.push(-1);
    m.push(-2);
    EXPECT_EQ(*m.push(-3), -2);
    EXPECT_EQ(*m.push(-4), -3);
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
    EXPECT_EQ(*m.push(5), 100);  // old samples must not leak through
}

TEST(MedianFilter, MatchesBruteForceAcrossWraparound) {
    constexpr std::size_t W = 5;
    MedianFilter<int, W> m;
    std::vector<int> in;
    unsigned x = 12345;
    for (int i = 0; i < 200; ++i) {
        x = x * 1103515245u + 12345u;  // deterministic pseudo-random
        in.push_back(static_cast<int>((x >> 16) % 100));
    }
    for (std::size_t i = 0; i < in.size(); ++i) {
        auto out = m.push(in[i]);
        if (i + 1 < W) {
            EXPECT_FALSE(out.has_value());
            continue;
        }
        std::array<int, W> win{};
        std::copy(in.begin() + (i + 1 - W), in.begin() + (i + 1), win.begin());
        std::sort(win.begin(), win.end());
        ASSERT_TRUE(out.has_value());
        EXPECT_EQ(*out, win[W / 2]) << "at i=" << i;
    }
}
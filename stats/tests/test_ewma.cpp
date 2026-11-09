#include <gtest/gtest.h>
#include <stdexcept>

#include "stats/ewma.hpp"

using stats::Ewma;

TEST(EwmaTest, FirstSampleInitializes) {
	Ewma<float> f(0.5f);
	EXPECT_FALSE(f.initialized());
	EXPECT_FLOAT_EQ(f.update(10.0f), 10.0f);
	EXPECT_TRUE(f.initialized());
}

TEST(EwmaTest, IssueExampleSequence) {
    Ewma<float> f(0.5f);
    EXPECT_FLOAT_EQ(f.update(10.0f), 10.0f);
    EXPECT_FLOAT_EQ(f.update(20.0f), 15.0f);
    EXPECT_FLOAT_EQ(f.update(20.0f), 17.5f);
}

TEST(EwmaTest, AlphaOneReturnsInput) {
    Ewma<float> f(1.0f);
    EXPECT_FLOAT_EQ(f.update(3.0f), 3.0f);
    EXPECT_FLOAT_EQ(f.update(-7.0f), -7.0f);
    EXPECT_FLOAT_EQ(f.update(42.0f), 42.0f);
}

TEST(EwmaTest, ConstantInputStaysConstant) {
    Ewma<double> f(0.3);
    for (int i = 0; i < 100; ++i) {
        EXPECT_DOUBLE_EQ(f.update(5.0), 5.0);
    }
}

TEST(EwmaTest, ConvergesTowardStep) {
    Ewma<double> f(0.2);
    f.update(0.0);
    double y = 0.0;
    for (int i = 0; i < 100; ++i) y = f.update(10.0);
    EXPECT_NEAR(y, 10.0, 1e-6);
}

TEST(EwmaTest, SmallAlphaSmoothsMoreThanLargeAlpha) {
    Ewma<double> slow(0.1);
    Ewma<double> fast(0.9);
    slow.update(0.0);
    fast.update(0.0);
    EXPECT_LT(slow.update(10.0), fast.update(10.0));
}

TEST(EwmaTest, ValueReturnsLastOutput) {
    Ewma<float> f(0.5f);
    f.update(10.0f);
    f.update(20.0f);
    EXPECT_FLOAT_EQ(f.value(), 15.0f);
}

TEST(EwmaTest, ResetReinitializesOnNextSample) {
    Ewma<float> f(0.5f);
    f.update(10.0f);
    f.update(20.0f);
    f.reset();
    EXPECT_FALSE(f.initialized());
    EXPECT_FLOAT_EQ(f.update(100.0f), 100.0f);
}

TEST(EwmaTest, InvalidAlphaThrows) {
    EXPECT_THROW(Ewma<float>(0.0f), std::invalid_argument);
    EXPECT_THROW(Ewma<float>(-0.1f), std::invalid_argument);
    EXPECT_THROW(Ewma<float>(1.1f), std::invalid_argument);
}

TEST(EwmaTest, ValidAlphaBoundsDoNotThrow) {
    EXPECT_NO_THROW(Ewma<float>(1.0f));
    EXPECT_NO_THROW(Ewma<float>(0.001f));
}
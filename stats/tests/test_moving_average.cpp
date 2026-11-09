#include <gtest/gtest.h>

#include "stats/moving_average.hpp"

using stats::MovingAverage;

TEST(MovingAverage, PartialWindowAveragesSamplesSoFar) {
	MovingAverage<float, 3> ma;
	EXPECT_FLOAT_EQ(ma.update(3), 3.0F);
	EXPECT_FLOAT_EQ(ma.update(6), 4.5F);
	EXPECT_FLOAT_EQ(ma.update(9), 6.0F);
}

TEST(MovingAverage, SlidesAfterWindowIsFull) {
	MovingAverage<float, 3> ma;
	ma.update(3);
	ma.update(6);
	ma.update(9);
	EXPECT_FLOAT_EQ(ma.update(12), 9.0F);   // (6 + 9 + 12) / 3
	EXPECT_FLOAT_EQ(ma.update(15), 12.0F);  // (9 + 12 + 15) / 3
}

TEST(MovingAverage, EmptyReturnsZero) {
	MovingAverage<float, 3> ma;
	EXPECT_FLOAT_EQ(ma.value(), 0.0F);
}

TEST(MovingAverage, WindowOfOneReturnsLastSample) {
	MovingAverage<double, 1> ma;
	EXPECT_DOUBLE_EQ(ma.update(5.0), 5.0);
	EXPECT_DOUBLE_EQ(ma.update(7.0), 7.0);
}

TEST(MovingAverage, ResetClearsState) {
	MovingAverage<float, 3> ma;
	ma.update(10);
	ma.reset();
	EXPECT_EQ(ma.size(), 0U);
	EXPECT_FLOAT_EQ(ma.update(4), 4.0F);
}
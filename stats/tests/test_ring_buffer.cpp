#include <gtest/gtest.h>

#include <stats/ring_buffer.hpp>

using stats::RingBuffer;

TEST(RingBuffer, StartsEmpty) {
    RingBuffer<float, 3> rb;
    EXPECT_EQ(rb.size(), 0u);
    EXPECT_TRUE(rb.empty());
    EXPECT_FALSE(rb.full());
    EXPECT_EQ(rb.capacity(), 3u);
}

TEST(RingBuffer, PushBelowCapacity) {
    RingBuffer<float, 3> rb;
    rb.push(1);
    rb.push(2);
    EXPECT_EQ(rb.size(), 2u);
    EXPECT_FLOAT_EQ(rb[0], 1);
    EXPECT_FLOAT_EQ(rb[1], 2);
    EXPECT_FALSE(rb.full());
}

TEST(RingBuffer, OverwritesOldestWhenFull) {
    RingBuffer<float, 3> rb;
    rb.push(1);
    rb.push(2);
    rb.push(3);
    rb.push(4);
    EXPECT_EQ(rb.size(), 3u);
    EXPECT_TRUE(rb.full());
    EXPECT_FLOAT_EQ(rb[0], 2);
    EXPECT_FLOAT_EQ(rb[1], 3);
    EXPECT_FLOAT_EQ(rb[2], 4);
}

TEST(RingBuffer, WrapsMultipleTimes) {
    RingBuffer<int, 3> rb;
    for (int i = 1; i <= 7; ++i) rb.push(i);
    EXPECT_EQ(rb[0], 5);
    EXPECT_EQ(rb[1], 6);
    EXPECT_EQ(rb[2], 7);
}

TEST(RingBuffer, NonConstIndexAllowsWriting) {
    RingBuffer<int, 3> rb;
    rb.push(1);
    rb.push(2);
    rb[0] = 42;
    EXPECT_EQ(rb[0], 42);
}

TEST(RingBuffer, ConstAccess) {
    RingBuffer<int, 3> rb;
    rb.push(10);
    rb.push(20);
    const auto& cref = rb;
    EXPECT_EQ(cref[0], 10);
    EXPECT_EQ(cref[1], 20);
}

TEST(RingBuffer, IterationInInsertionOrder) {
    RingBuffer<int, 3> rb;
    for (int i = 1; i <= 5; ++i) rb.push(i);  // contents: 3, 4, 5
    int expected = 3;
    for (int v : rb) {
        EXPECT_EQ(v, expected++);
    }
    EXPECT_EQ(expected, 6);
}

TEST(RingBuffer, IterationOverEmptyDoesNothing) {
    RingBuffer<int, 3> rb;
    EXPECT_TRUE(rb.begin() == rb.end());
}
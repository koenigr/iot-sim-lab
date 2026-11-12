#pragma once

#include <cstddef>
#include <type_traits>

#include "stats/ring_buffer.hpp"

namespace stats {
/*
Moving average over the last N samples, using a running sum (o(1) per update).

While the window is not full yet, the average is taken over the samples
received so far (e.g. after 2 samples: (a + b) / 2), not over N.
*/
template <typename T, std::size_t N>
class MovingAverage {
    static_assert(std::is_floating_point_v<T>, "MovingAverage requires a floating-point type");
    static_assert(N > 0, "window size must be > 0");

public:
    // Adds a sample and returns the current average.
    T update(T sample) {
        if (buffer_.size() == N) {
            sum_ -= *buffer_.begin();  // oldest sample is about to be overwritten
        }
        buffer_.push(sample);
        sum_ += sample;
        return value();
    }

    // Current average; 0 if no sample has been received yet.
    T value() const { return buffer_.size() == 0 ? T{0} : sum_ / static_cast<T>(buffer_.size()); }

    std::size_t size() const { return buffer_.size(); }
    static constexpr std::size_t window() { return N; }

    void reset() {
        buffer_ = RingBuffer<T, N>{};
        sum_ = T{0};
    }

private:
    RingBuffer<T, N> buffer_{};
    T sum_{0};
};
}  // namespace stats

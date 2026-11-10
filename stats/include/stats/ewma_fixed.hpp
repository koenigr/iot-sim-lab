// stats/include/stats/ewma_fixed.hpp
#pragma once
#include <cstdint>

namespace stats {

// Integer-only EWMA, values and alpha in Q16.16 (range ±32768, LSB = 2^-16).
// y[n] = y[n-1] + alpha * (x[n] - y[n-1]),  alpha in [0, 1.0] (0 .. 65536)
class EwmaFixed {
public:
	static constexpr int     kFrac = 16;
	static constexpr int32_t kOne  = 1 << kFrac;

	static constexpr int32_t to_fixed(double v) {  // for tests / setup only
		return static_cast<int32_t>(v * kOne + (v >= 0 ? 0.5 : -0.5));
	}
	static constexpr double to_double(int32_t q) { return static_cast<double>(q) / kOne; }

	// Out-of-range alpha is clamped to [0, kOne].
	constexpr explicit EwmaFixed(int32_t alpha_q16 = kOne / 4) noexcept
		: alpha_{alpha_q16 < 0 ? 0 : (alpha_q16 > kOne ? kOne : alpha_q16)} {}

	// Returns the new filtered value (Q16.16).
	constexpr int32_t update(int32_t x) noexcept {
		if (!init_) {
			y_ = x;
			init_ = true;
			return y_;
		}
		// x - y can exceed int32 -> compute in int64.
		const int64_t diff  = static_cast<int64_t>(x) - y_;
		// |alpha*diff| <= 2^16 * 2^32 = 2^48, fits int64.
		// +0.5 LSB before the shift = round half up (also for negatives).
		const int64_t delta = (static_cast<int64_t>(alpha_) * diff + (kOne >> 1)) >> kFrac;
		// alpha in [0,1] => y + delta lies between y and x => no int32 overflow.
		y_ = static_cast<int32_t>(y_ + delta);
		return y_;
	}

	constexpr int32_t value() const noexcept { return y_; }
	constexpr bool    initialized() const noexcept { return init_; }
	constexpr void    reset() noexcept { y_ = 0; init_ = false; }

private:
	int32_t alpha_;
	int32_t y_{0};
	bool    init_{false};
};

}  // namespace stats
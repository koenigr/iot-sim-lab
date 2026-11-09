#pragma once
#include <stdexcept>

namespace stats {

template <typename T>
class Ewma {
public:
	explicit Ewma(T alpha) : alpha_(alpha) {
		if (!(alpha > T{0} && alpha <= T{1})) {
			throw std::invalid_argument("alpha must be in (0,1]");
		}
	}

	T update(T x) {
		y_ = initialized_ ? alpha_ * x + (T{1} - alpha_) * y_ : x;
		initialized_ = true;
		return y_;
	}

	T value() const { return y_; }
	bool initialized() const { return initialized_; }
	void reset() { initialized_ = false; y_ = T{}; }

private:
	T alpha_;
	T y_{};
	bool initialized_{false};
};

} // namespace stats
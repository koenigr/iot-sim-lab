#pragma once
#include <cstddef>
#include <limits>

namespace stats {

class Welford {
public:
	void add(double x) noexcept {
		++n_;
		const double delta = x - mean_;
		mean_ += delta / static_cast<double>(n_);
		m2_   += delta * (x - mean_);   // uses the *updated* mean
	}

	std::size_t count() const noexcept { return n_; }
	double mean() const noexcept { return mean_; }

	// sample variance (n-1)
	double variance() const noexcept {
		return n_ > 1 ? m2_ / static_cast<double>(n_ - 1)
					  : std::numeric_limits<double>::quiet_NaN();
	}

	// population variance (n)
	double variance_pop() const noexcept {
		return n_ > 0 ? m2_ / static_cast<double>(n_)
					  : std::numeric_limits<double>::quiet_NaN();
	}

private:
	std::size_t n_{0};
	double mean_{0.0};
	double m2_{0.0};
};

} // namespace stats
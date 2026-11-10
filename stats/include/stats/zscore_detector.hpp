#pragma once

#include <cmath>
#include <cstddef>
#include <limits>

#include "stats/welford.hpp"

namespace stats {

/// Online z-score anomaly detector on top of stats::Welford.
///
/// Uses the population stddev (sqrt(variance_pop)), so alternating 19/21
/// samples give mean = 20, stddev = 1.
///
/// Design decision: anomalous samples are NOT fed into the statistics.
///   - Rationale: an outlier would inflate stddev and shift the mean, making
///     the next outliers look normal (masking).
///   - Exception: during warm-up (fewer than `min_samples` accepted samples)
///     every sample is accepted, because there is no reliable baseline yet.
///   - Trade-off: after a permanent level shift the detector keeps flagging
///     every sample, because the baseline never adapts. Set
///     `feed_anomalies = true` or call reset() if that matters.
class ZScoreDetector {
public:
	struct Config {
		double threshold = 3.0;        ///< |z| > threshold  =>  anomaly
		std::size_t min_samples = 10;  ///< warm-up: no anomalies reported before this
		bool feed_anomalies = false;   ///< feed anomalous samples into the statistics?
	};

	struct Result {
		double z;
		bool is_anomaly;
	};

	ZScoreDetector() = default;
	explicit ZScoreDetector(Config cfg) noexcept : cfg_(cfg) {}

	/// Scores `x` against the current statistics, then (maybe) updates them.
	Result update(double x) noexcept {
		const bool warm = w_.count() >= cfg_.min_samples && w_.count() >= 2;
		const double z = zscore(x);
		const bool anomaly = warm && std::fabs(z) > cfg_.threshold;

		if (!anomaly || cfg_.feed_anomalies) {
			w_.add(x);
		}
		return {z, anomaly};
	}

	void reset() noexcept { w_ = Welford{}; }

	std::size_t count() const noexcept { return w_.count(); }
	double mean() const noexcept { return w_.mean(); }
	double stddev() const noexcept {
		return w_.count() > 1 ? std::sqrt(w_.variance_pop()) : 0.0;
	}
	const Config& config() const noexcept { return cfg_; }

private:
	double zscore(double x) const noexcept {
		const double sd = stddev();
		const double m = w_.mean();
		if (sd == 0.0) {
			// No spread: identical value is "normal", anything else is infinitely far away.
			return x == m ? 0.0
						  : (x > m ? 1.0 : -1.0) * std::numeric_limits<double>::infinity();
		}
		return (x - m) / sd;
	}

	Config cfg_{};
	Welford w_{};
};

}  // namespace stats
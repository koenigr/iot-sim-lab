// sensor-node/src/generator.cpp
#include "generator.hpp"

#include <cmath>
#include <iomanip>

Generator::Generator(GeneratorConfig cfg)
	: cfg_(cfg),
	  rng_(cfg.seed),
	  spike_mag_(cfg.spike_min, cfg.spike_max) {}

Sample Generator::next() {
	constexpr double kTwoPi = 6.283185307179586;
	const double t = static_cast<double>(step_);

	// noise-free reference
	double truth = cfg_.base + cfg_.drift * t;
	if (cfg_.amplitude != 0.0) {
		truth += cfg_.amplitude * std::sin(kTwoPi * t / cfg_.period);
	}

	double value = truth + cfg_.noise_sigma * noise_(rng_);

	bool anomaly = false;
	if (unit_(rng_) < cfg_.anomaly_rate) {
		const double sign = (unit_(rng_) < 0.5) ? -1.0 : 1.0;
		value += sign * spike_mag_(rng_);
		anomaly = true;
	}

	return Sample{cfg_.start_ts + step_++, value, truth, anomaly};
}

void write_csv_header(std::ostream& os) {
	os << "timestamp,value,truth,is_anomaly\n";
}

void write_csv_row(std::ostream& os, const Sample& s) {
	os << s.timestamp << ',' << std::fixed << std::setprecision(2) << s.value
	   << ',' << s.truth << ',' << (s.is_anomaly ? 1 : 0) << '\n';
}
// sensor-node/src/generator.hpp
#pragma once
#include <cstdint>
#include <ostream>
#include <random>

struct Sample {
	std::int64_t timestamp;
	double value;      // noisy, possibly anomalous
	double truth;      // noise-free signal
	bool is_anomaly;
};

struct GeneratorConfig {
	std::uint64_t seed = 42;
	std::int64_t start_ts = 1000;
	double base = 20.0;
	double drift = 0.1;         // per step
	double amplitude = 0.0;     // sine amplitude, 0 = off
	double period = 100.0;      // sine period in steps
	double noise_sigma = 0.5;
	double anomaly_rate = 0.01; // probability per sample
	double spike_min = 5.0;     // absolute spike size
	double spike_max = 10.0;
};

class Generator {
public:
	explicit Generator(GeneratorConfig cfg = {});
	Sample next();

private:
	GeneratorConfig cfg_;
	std::mt19937_64 rng_;
	std::normal_distribution<double> noise_{0.0, 1.0};
	std::uniform_real_distribution<double> unit_{0.0, 1.0};
	std::uniform_real_distribution<double> spike_mag_;
	std::int64_t step_ = 0;
};

void write_csv_header(std::ostream& os);
void write_csv_row(std::ostream& os, const Sample& s);
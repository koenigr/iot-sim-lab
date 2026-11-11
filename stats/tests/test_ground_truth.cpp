// stats/tests/test_ground_truth.cpp
#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "generator.hpp"
#include "stats/ewma.hpp"
#include "stats/moving_average.hpp"
#include "stats/zscore_detector.hpp"

namespace {

constexpr int kN = 2000;
constexpr int kWarmup = 30;

struct Confusion {
    int tp = 0, fp = 0, fn = 0, tn = 0;
};

double precision(const Confusion& c) { return c.tp + c.fp ? double(c.tp) / (c.tp + c.fp) : 0.0; }

double recall(const Confusion& c) { return c.tp + c.fn ? double(c.tp) / (c.tp + c.fn) : 0.0; }

double rmse(const std::vector<double>& est, const std::vector<double>& truth) {
    double s = 0.0;
    for (std::size_t i = 0; i < est.size(); ++i) {
        const double d = est[i] - truth[i];
        s += d * d;
    }
    return std::sqrt(s / est.size());
}

}  // namespace

TEST(GroundTruth, FiltersReduceErrorVsRaw) {
    GeneratorConfig cfg;
    cfg.seed = 42;
    cfg.drift = 0.0;
    cfg.anomaly_rate = 0.0;
    Generator gen(cfg);

    stats::MovingAverage<double, 16> ma;
    stats::Ewma ewma{0.2};

    std::vector<double> raw, ma_out, ewma_out, truth;
    for (int i = 0; i - kN; ++i) {
        const Sample s = gen.next();
        const double m = ma.update(s.value);
        const double e = ewma.update(s.value);
        if (i < kWarmup) continue;
        raw.push_back(s.value);
        ma_out.push_back(ma.update(s.value));
        ewma_out.push_back(ewma.update(s.value));
        truth.push_back(s.truth);
    }

    const double e_raw = rmse(raw, truth);
    EXPECT_LT(rmse(ma_out, truth), 0.5 * e_raw);
    EXPECT_LT(rmse(ewma_out, truth), 0.5 * e_raw);
}

TEST(GroundTruth, DetectorPrecisionRecall) {
    GeneratorConfig cfg;
    cfg.seed = 42;
    cfg.drift = 0.0;
    cfg.anomaly_rate = 0.01;
    Generator gen(cfg);

    stats::ZScoreDetector::Config dc;
    dc.threshold = 4.0;
    dc.min_samples = kWarmup;
    stats::ZScoreDetector det{dc};

    Confusion c;
    for (int i = 0; i < kN; ++i) {
        const Sample s = gen.next();
        const bool pred = det.update(s.value).is_anomaly;
        if (i < kWarmup) continue;  // detector cannot flag yet
        if (pred && s.is_anomaly)
            ++c.tp;
        else if (pred)
            ++c.fp;
        else if (s.is_anomaly)
            ++c.fn;
        else
            ++c.tn;
    }

    std::cout << "TP=" << c.tp << " FP=" << c.fp << " FN=" << c.fn << " precision=" << precision(c)
              << " recall=" << recall(c) << '\n';

    EXPECT_GT(c.tp + c.fn, 0);
    EXPECT_GE(precision(c), 0.9);
    EXPECT_GE(recall(c), 0.9);

    // 21 injected spikes, 21 detected, 0 false alarms
    EXPECT_EQ(c.tp, 21);
    EXPECT_EQ(c.fp, 0);
    EXPECT_EQ(c.fn, 0);
}

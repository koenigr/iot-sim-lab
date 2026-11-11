// sensor-node/tests/test_generator.cpp
#include <gtest/gtest.h>

#include <cmath>
#include <sstream>
#include <string>

#include "generator.hpp"

TEST(Generator, SameSeedGivesSameSequence) {
    Generator a{GeneratorConfig{}}, b{GeneratorConfig{}};
    for (int i = 0; i < 100; ++i) {
        const Sample x = a.next(), y = b.next();
        EXPECT_EQ(x.timestamp, y.timestamp);
        EXPECT_DOUBLE_EQ(x.value, y.value);
        EXPECT_DOUBLE_EQ(x.truth, y.truth);
        EXPECT_EQ(x.is_anomaly, y.is_anomaly);
    }
}

TEST(Generator, TimestampsIncreaseByOne) {
    GeneratorConfig cfg;
    cfg.start_ts = 1000;
    Generator gen{cfg};
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(gen.next().timestamp, 1000 + i);
    }
}

TEST(Generator, TruthIsNoiseFree) {
    GeneratorConfig cfg;
    cfg.base = 20.0;
    cfg.drift = 0.1;
    cfg.amplitude = 0.0;
    Generator gen{cfg};
    for (int i = 0; i < 50; ++i) {
        EXPECT_NEAR(gen.next().truth, 20.0 + 0.1 * i, 1e-9);
    }
}

TEST(Generator, AnomaliesDeviateAtLeastSpikeMin) {
    GeneratorConfig cfg;
    cfg.noise_sigma = 0.0;  // isolate the spike
    cfg.anomaly_rate = 0.2;
    cfg.spike_min = 5.0;
    cfg.spike_max = 10.0;
    Generator gen{cfg};
    int anomalies = 0;
    for (int i = 0; i < 1000; ++i) {
        const Sample s = gen.next();
        const double dev = std::abs(s.value - s.truth);
        if (s.is_anomaly) {
            ++anomalies;
            EXPECT_GE(dev, cfg.spike_min - 1e-9);
        } else {
            EXPECT_NEAR(dev, 0.0, 1e-9);
        }
    }
    EXPECT_GT(anomalies, 0);
}

TEST(Generator, ZeroAnomalyRateGivesNoAnomalies) {
    GeneratorConfig cfg;
    cfg.anomaly_rate = 0.0;
    Generator gen{cfg};
    for (int i = 0; i < 1000; ++i) EXPECT_FALSE(gen.next().is_anomaly);
}

TEST(GeneratorCsv, HeaderAndRowFormat) {
    std::ostringstream os;
    write_csv_header(os);
    write_csv_row(os, Sample{1000, 21.4, 20.0, false});
    write_csv_row(os, Sample{1001, 45.0, 20.1, true});
    EXPECT_EQ(os.str(),
              "timestamp,value,truth,is_anomaly\n"
              "1000,21.40,20.00,0\n"
              "1001,45.00,20.10,1\n");
}

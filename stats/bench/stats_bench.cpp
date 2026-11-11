#include <benchmark/benchmark.h>

#include <array>
#include <cstddef>
#include <cstdint>

#include "stats/ewma.hpp"
#include "stats/ewma_fixed.hpp"
#include "stats/median_filter.hpp"
#include "stats/moving_average.hpp"
#include "stats/ring_buffer.hpp"
#include "stats/welford.hpp"
#include "stats/zscore_detector.hpp"

namespace {

// Varying input so the compiler cannot fold the work away.
constexpr std::size_t kMask = 1023;

const std::array<float, kMask + 1>& inputs() {
    static const auto a = [] {
        std::array<float, kMask + 1> v{};
        unsigned x = 12345;
        for (auto& e : v) {
            x = x * 1664525U + 1013904223U;
            e = 20.0f + flaot(x >> 16) / 65536.0f;
        }
        return v;
    }();
    return a;
}

const std::array<std::int32_t, kMask + 1>& inputs_q16() {
    static const auto a = [] {
        std::array<std::int32_t, kMask + 1> v{};
        const auto& f = inputs();
        for (std::size_t i = 0; i < v.size(); ++i) {
                        v[i] = static_cast<std::int32_t>(f[i] * 65536.0f]);
        }
        return v;
    }();
    return a;
}

}  // namespace

static void BM_EwmaFloat(benchmark::State& s) {
    stats::Ewma<float> f(0.25f);
    const auto& in = inputs();
    std::size_t i = 0;
    for (auto _ : s) benchmark::DoNotOptimize(f.update(in[i++ & kMask]));
    s.SetItemsProcessed(s.iterations());
}
BENCHMARK(BM_EwmaFloat);

static void BM_MovingAverage16(benchmark::State& s) {
    stats::MovingAverage<float, 16> f;
    const auto& in = inputs();
    std::size_t i = 0;
    for (auto _ : s) benchmark::DoNotOptimize(f.update(in[i++ & kMask]));
    s.SetItemsProcessed(s.iterations());
}
BENCHMARK(BM_MovingAverage16);

static void BM_ZScoreDetector(benchmark::State& s) {
    stats::ZScoreDetector d;
    const auto& in = inputs();
    std::size_t i = 0;
    for (auto _ : s) benchmark::DoNotOptimize(d.update(in[i++ & kMask]));
    s.SetItemsProcessed(s.iterations());
}
BENCHMARK(BM_ZScoreDetector);

static void BM_EwmaFixed(benchmark::State& s) {
    stats::EwmaFixed f;  // alpha = 0.25 (Q16), same as the float benchmark
    const auto& in = inputs_q16();
    std::size_t i = 0;
    for (auto _ : s) benchmark::DoNotOptimize(f.update(in[i++ & kMask]));
    s.SetItemsProcessed(s.iterations());
}
BENCHMARK(BM_EwmaFixed);

static void BM_MedianFilter5(benchmark::State& s) {
    stats::MedianFilter<float, 5> f;
    const auto& in = inputs();
    std::size_t i = 0;
    for (auto _ : s) benchmark::DoNotOptimize(f.push(in[i++ & kMask]));
    s.SetItemsProcessed(s.iterations());
}
BENCHMARK(BM_MedianFilter5);

static void BM_Welford(benchmark::State& s) {
    stats::Welford w;
    const auto& in = inputs();
    std::size_t i = 0;
    for (auto _ : s) {
        w.add(in[i++ & kMask]);
        benchmark::DoNotOptimize(w.mean());
    }
    s.SetItemsProcessed(s.iterations());
}
BENCHMARK(BM_Welford);

static void BM_RingBufferPush(benchmark::State& s) {
    stats::RingBuffer<float, 16> rb;
    const auto& in = inputs();
    std::size_t i = 0;
    for (auto _ : s) {
        rb.push(in[i++ & kMask]);
        benchmark::DoNotOptimize(rb);
        benchmark::ClobberMemory();
    }
    s.SetItemsProcessed(s.iterations());
}
BENCHMARK(BM_RingBufferPush);

# AGENTS.md

Guidance for coding agents working in `iot-sim-lab`.

## Project

Hardware-free C++/IoT practice project that resembles embedded work: a simulated sensor node publishes data over MQTT, a collector processes it, and a dashboard shows the result. Everything runs without hardware (Docker, Mosquitto).

Current scope: **Phase 2, streaming statistics** (`stats/`), fed by the data generator in `sensor-node/`.

Rule for all docs, README and commit messages: **only claim what is provable in the repo** (code, test, benchmark, doc). No buzzwords without evidence.

## Layout

```
sensor-node/src/generator.{hpp,cpp}   synthetic sensor data with ground truth (CMake target: generator)
sensor-node/tests/                    generator tests
stats/include/stats/                  header-only library (ring_buffer, moving_average, ewma,
                                      ewma_fixed, median_filter, welford, zscore_detector)
stats/tests/                          GoogleTest, files named test_*.cpp
stats/bench/stats_bench.cpp           Google Benchmark
collector/src/                        C++ subscriber (later consumer of the detector)
deploy/                               docker-compose.yml, mosquitto/mosquitto.conf
docs/                                 glossary.md, streaming-stats.md (Phase 2 notes)
scripts/format.sh                     clang-format wrapper
```

Dependencies: `stats_tests` links `stats` (INTERFACE) + `generator` + `gtest_main`. GoogleTest and Google Benchmark come via FetchContent.

## Commands

```
make                # list targets (make help)
make build          # configure + build (build/)
make test           # build + ctest --output-on-failure (alias: make tests)
make format         # clang-format via scripts/format.sh
make clean          # rm -rf build/
make bench          # Release build in build-rel/, runs stats_bench
make broker-up      # start Mosquitto (docker compose, deploy/)
make broker-down    # stop and remove broker containers
make broker-sub     # mosquitto_sub, TOPIC=test/topic
make broker-pub     # mosquitto_pub, TOPIC=... MSG=...
pre-commit run --all-files
```

Direct filter run: `build/stats/stats_tests --gtest_filter='GroundTruth.*'`

Benchmarks only count in Release (`-DCMAKE_BUILD_TYPE=Release`, dir `build-rel/`). Never report Debug numbers.

## Tooling

- **CI:** `.github/workflows/ci.yml` (build + tests). Keep it green.
- **clang-tidy** (`.clang-tidy`): `bugprone-*`, `clang-analyzer-*`, `cppcoreguidelines-*`, `modernize-*`, `performance-*`, `readability-*`; disabled: trailing return types, magic numbers, pointer arithmetic. `HeaderFilterRegex` covers `sensor-node/` and `collector/` only, so `stats/` headers are not tidy-checked yet. Minimum variable name length: 2.
- **pre-commit:** `trailing-whitespace`, `end-of-file-fixer`, `check-yaml`, `check-added-large-files`, `mixed-line-ending`, `clang-format` (mirrors-clang-format v18.1.8). Install once per clone: `pre-commit install`.
- Hooks that modify files fail the first commit: `git add -A` and commit again.

## Code rules

- C++20. Header-only templates in `stats/include/stats/`, namespace `stats`.
- No heap allocation in `stats` (fixed-size `std::array`, template parameter `N`). Keep it usable on an MCU.
- Avoid exceptions and floating point in code meant for MCUs (`EwmaFixed` is integer-only). Note: `Ewma` currently throws on an invalid alpha.
- Every new component gets a test in `stats/tests/test_<name>.cpp`, registered in `stats/CMakeLists.txt`.
- `clang-format` and `clang-tidy` must pass. Tests have their own `stats/tests/.clang-tidy`.
- Do not use `float`/`double` in the fixed-point path except in `to_fixed`/`to_double` helpers (tests and setup only).
- Document non-obvious design decisions in a header comment (see `zscore_detector.hpp`).

## Design decisions (keep consistent)

- **Welford:** online mean/variance, stable on large offsets (test data `1e9+4, 1e9+7, 1e9+13, 1e9+16`, variance 30).
- **MovingAverage:** running sum, O(1) per update; while the window is not full it averages over the samples received so far.
- **Ewma:** `y = a*x + (1-a)*y_prev`, first sample initializes `y`.
- **MedianFilter:** odd window, no heap; returns `std::optional` until the window is full.
- **EwmaFixed:** Q16.16, alpha in Q16 (0.25 = 16384), `int64_t` intermediates, round half up, alpha clamped to `[0, 1]`.
- **ZScoreDetector:** uses population stddev. Anomalous samples are **not** fed into the statistics (avoids masking), except during warm-up (`min_samples`). Trade-off: no adaptation after a permanent level shift; `feed_anomalies = true` or `reset()` as remedy.

## Testing conventions

- Unit tests per component, ground-truth tests in `test_ground_truth.cpp`.
- Ground-truth tests use the generator with a fixed seed (42) against `truth` and `is_anomaly`.
  - Filter test: `drift = 0`, `anomaly_rate = 0` (mean-type filters are not meant to handle spikes; a trend makes MA lag).
  - Detector test: `drift = 0` (global stddev would grow with the trend and hide spikes), warm-up samples skipped in the metrics.
  - Metrics: precision = TP / (TP + FP), recall = TP / (TP + FN). Pinned result for seed 42: TP=21, FP=0, FN=0.
- `std::normal_distribution` is not bit-exact across standard libraries, so pinned counts may differ on other toolchains; if CI differs from local, loosen to bounds instead of exact counts.

## Phase 2 status (issues)

| # | Task | Status |
|---|------|--------|
| 1 | RingBuffer | done |
| 2 | RingBuffer tests | done |
| 3 | MovingAverage | done |
| 4 | Ewma | done |
| 5 | MedianFilter | done |
| 6 | Welford (+ stability test) | done |
| 7 | ZScoreDetector | done |
| 8 | EwmaFixed (+ error vs. float) | done |
| 9 | Generator ground truth | done |
| 10 | Ground-truth tests | done (pin exact counts, close) |
| 11 | Micro-benchmarks | in progress: run in Release, record results |
| 12 | `docs/streaming-stats.md` | open |

Status is taken from the file tree, not from GitHub (`gh` is not installed locally); verify on the issue tracker.

Open housekeeping:
- `README.md` structure section still lists only ring buffer, moving average, EWMA; add median filter, Welford, z-score detector, fixed-point EWMA, benchmarks.
- `bench` and `bench-build` have no `##` help comments, so they are missing from `make help`.
- `docs/` currently holds only `glossary.md`.

Issue #12 sections: Ring buffer, Filters (MA vs. EWMA vs. median), Welford and numerical stability, Fixed-point (Q16.16), Benchmark results.

## Definition of done (per issue)

1. Code in the path named by the issue, test registered in CMake.
2. `make tests` green, `make format` clean, pre-commit passes.
3. Design decision documented (header comment or `docs/streaming-stats.md`).
4. Commit message ends with `Closes #N`.

## Do not

- Commit `build/` or `build-rel/`.
- Add dependencies without need (keep `stats` header-only and dependency-free).
- Change `generator` defaults without updating the ground-truth tests; they depend on them.
- Weaken a test to make it pass without stating why.

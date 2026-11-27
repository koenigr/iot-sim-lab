# iot-sim-lab

Hardware-free C++/IoT practice project: simulated sensor nodes publish data via MQTT to a collector.

## Requirements

| Tool | Needed for | Install (Debian/Ubuntu) |
|------|------------|-------------------------|
| C++ compiler (C++17 or newer), `make` | build | `sudo apt install build-essential` |
| CMake | build, tests, benchmarks | `sudo apt install cmake` |
| Docker + Compose v2 | Mosquitto broker (`make broker-*`) | `sudo apt install docker.io docker-compose-v2` |
| `mosquitto-clients` | `broker-sub`, `broker-pub`, `make smoke` | `sudo apt install mosquitto-clients` |
| `clang-format` | `make format` | `sudo apt install clang-format` |
| `pre-commit` | commit hooks | `pip install pre-commit` |
| Eclipse Paho MQTT C++ | `sensor-node` (MQTT publisher) | `sudo apt install libpaho-mqttpp-dev libpaho-mqtt-dev` |

GoogleTest and Google Benchmark are used for tests and benchmarks (see `CMakeLists.txt` for how they are fetched).

Add your user to the `docker` group to run Docker without `sudo`:

    sudo usermod -aG docker $USER

## Structure

- `sensor-node/`: simulated sensor node and data generator (noisy value, noise-free `truth`, `is_anomaly` flag, fixed seed); publishes to MQTT (v1 payload with `seq`, Last Will status). See [sensor-node.md](docs/sensor-node.md).
- `collector/`: receives and processes the data
- `stats/`: header-only streaming statistics with tests and benchmarks
- `deploy/`: docker compose (Mosquitto broker)
- `docs/`: documentation, glossary
- `AGENTS.md`: conventions and commands for coding agents

## Streaming statistics (`stats/`)

No heap allocation, header-only, namespace `stats`.

| Component | Header | Notes |
|-----------|--------|-------|
| `RingBuffer<T, N>` | `ring_buffer.hpp` | fixed size, overwrites oldest |
| `MovingAverage<T, N>` | `moving_average.hpp` | running sum, O(1) per sample |
| `Ewma<T>` | `ewma.hpp` | exponentially weighted average |
| `EwmaFixed` | `ewma_fixed.hpp` | integer-only, Q16.16 |
| `MedianFilter<T, N>` | `median_filter.hpp` | removes single-sample spikes |
| `Welford` | `welford.hpp` | online mean/variance, numerically stable |
| `ZScoreDetector` | `zscore_detector.hpp` | z-score anomaly detection on Welford statistics |

The detector does not feed anomalous samples into its statistics (avoids masking); see the header comment for the trade-off.

See more details: [streaming-stats.md](docs/streaming-stats.md)

### Tests with known data

`stats/tests/test_ground_truth.cpp` runs filters and the detector on generator data (seed 42):

- filter error against `truth`
- detector precision/recall against `is_anomaly`

### Benchmarks

Google Benchmark, reported as samples/s, including float vs. fixed-point EWMA. Release build only:

    make bench

## Development

    make build     # configure and build
    make test      # build and run tests
    make format    # run clang-format
    make bench     # build (Release) and run micro-benchmarks
    make clean     # remove build directory

Run `make` to list all targets.

Pre-commit hooks (once per clone):

    pip install pre-commit
    pre-commit install

## Start the broker

    make broker-up

## Test the broker

    make broker-sub                  # subscribe to test/topic
    make broker-pub                  # publish "hello" to test/topic
    make broker-pub TOPIC=sensors/temp MSG=21.

## Run the sensor node

    make broker-up
    make sensor-node                 # publishes 20 samples, 1 per second
    make broker-sub               # subscribe to lab/# (override with TOPIC=...)

The broker URI can be overridden with `MQTT_URI` (default `tcp://localhost:1883`).

Topics and payload: [topics.md](docs/topics.md)

Check the Last Will: kill the node with `kill -9`. The retained status topic `lab/<area>/<node>/status` switches from `online` to `offline` after the keep-alive timeout.

## Close the broker

    make broker-down     # stop and remove containers
    make broker-stop     # stop, keep containers

## Without make

    cmake -B build
    cmake --build build
    ctest --test-dir build --output-on-failure
    ./scripts/format.sh
    cmake -S . -B build-rel -DCMAKE_BUILD_TYPE=Release
    cmake --build build-rel --target stats_bench
    build-rel/stats/stats_bench
    docker compose -f deploy/docker-compose.yml up -d
    docker compose -f deploy/docker-compose.yml down

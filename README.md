# iot-sim-lab

Hardware-free C++/IoT practice project: simulated sensor nodes publish data via MQTT to a collector.

## Structure

- `sensor-node/`: simulated sensor node
- `collector/`: receives and processes the data
- `stats/`: header-only statistics (ring buffer, moving average, EWMA) with tests
- `deploy/`: docker compose (Mosquitto broker)
- `docs/`: documentation, glossary

## Development

    make build     # configure and build
    make test      # build and run tests
    make format    # run clang-format
    make clean     # remove build directory

Run `make` to list all targets.

## Start the broker

    make broker-up

## Test the broker

    make broker-sub                  # subscribe to test/topic
    make broker-pub                  # publish "hello" to test/topic
    make broker-pub TOPIC=sensors/temp MSG=21.5

## Close the broker

    make broker-down     # stop and remove containers
    make broker-stop     # stop, keep containers

## Without make

    cmake -B build
    cmake --build build
    ctest --test-dir build --output-on-failure
    ./scripts/format.sh
    docker compose -f deploy/docker-compose.yml up -d
    docker compose -f deploy/docker-compose.yml down
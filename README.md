# iot-sim-lab

Hardware-free C++/IoT practice project: simulated sensor nodes publish data via MQTT to a collector.

## Structure

- `sensor-node/`: simulated sensor node
- `collector/`: receives and processes the data
- `deploy/`: docker compose (Mosquitto broker)
- `docs/`: documentation, glossary

## Development

    cmake -B build
    cmake --build build
    ctest --test-dir build --output-on-failure
    ./scripts/format.sh

## Start the broker

    docker compose -f deploy/docker-compose.yml up -d

## Test the broker

    mosquitto_sub -h localhost -t test/topic
    mosquitto_pub -h localhost -t test/topic -m "hello"

## Close the broker

    docker compose -f deploy/docker-compose.yml down
    
    (docker compose -f deploy/docker-compose.yml stop)
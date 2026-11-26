# sensor-node

Simulated sensor node: generates synthetic readings and publishes them over MQTT.

## Structure

| File | Target | Role |
|------|--------|------|
| `src/generator.{hpp,cpp}` | `generator` (library) | synthetic data with ground truth |
| `src/payload.{hpp,cpp}` | `payload` (library) | `make_payload_v1`: `Sample` to JSON string, no MQTT dependency |
| `src/publisher.{hpp,cpp}` | part of `sensor-node` | MQTT connection, topics, `seq`, Last Will (paho-mqtt-cpp) |
| `src/main.cpp` | `sensor-node` (executable) | wires `Generator` to `Publisher` |
| `tests/` | `smoke` (GoogleTest) | generator and payload tests |

Dependencies: `sensor-node` -> `payload` -> `generator`. Only `sensor-node` links Paho.

## Data flow

    Generator::next() -> Sample -> make_payload_v1(sample, seq, unit) -> Publisher::publish() -> broker

## Generator

`Generator` is configured by `GeneratorConfig` and is deterministic for a fixed `seed`. Each `Sample` contains:

| Field | Meaning |
|-------|---------|
| `timestamp` | `start_ts` + step |
| `value` | noisy reading (published) |
| `truth` | noise-free reference (tests only) |
| `is_anomaly` | spike was injected (tests only) |

Signal: `base` + `drift` * t + `amplitude` * sin(2 pi t / `period`), plus Gaussian noise (`noise_sigma`). With probability `anomaly_rate` a spike of random sign and magnitude in [`spike_min`, `spike_max`] is added.

The ground-truth tests in `stats/tests/` depend on the generator defaults. Do not change them without updating those tests.

`write_csv_header` / `write_csv_row` write samples as CSV. They are not used by `main`; only the CSV format test in `tests/test_generator.cpp` uses them. Kept as a helper to inspect generator output.

## Payload

`make_payload_v1` builds the v1 JSON by hand (five fields, no JSON library). It is a separate library so it is unit-tested without a broker. Format and field meaning: [topics.md](topics.md).

`truth` and `is_anomaly` are never part of the payload.

## Publisher

- Connects with client ID = node name, clean session, keep-alive 10 s, automatic reconnect.
- Registers the Last Will `offline` (retained) on the status topic, then publishes `online` (retained).
- `publish()` sends one reading on the data topic, with `seq` starting at 0 and rising by 1 per message.
- The destructor publishes `offline` itself and disconnects, because the broker only sends the Last Will after an unclean disconnect.
- `Publisher` is neither copyable nor movable (it owns the connection).

Topics, QoS and retained flags: [topics.md](topics.md).

## main

Creates a `Generator` and a `Publisher` with the defaults from `PublisherConfig`, publishes 20 samples at 1 per second, then exits. The broker address is read from `MQTT_URI` (default `tcp://localhost:1883`). Broker errors print `mqtt error: ...` and return exit code 1.

## Build and run

    make sensor-node                  # build and run against localhost:1883
    make broker-sub                   # second terminal, subscribes to lab/#

Needs `libpaho-mqttpp-dev libpaho-mqtt-dev`.

## Tests

| Test | What it checks |
|------|----------------|
| `test_generator.cpp` | generator behaviour |
| `test_payload.cpp` | exact v1 JSON string; `truth` and `is_anomaly` not leaked |
| `make smoke` | publisher round-trip: first message on a data topic has `"seq":0` |

Run only the payload tests: `build/sensor-node/tests/smoke --gtest_filter='Payload.*'`

Not covered by an automated test: the `offline` message after an unclean disconnect (manual check with `kill -9`).

## Known limitations

- Configuration (site, area, node, sensor, metric, unit, QoS) is fixed at the `PublisherConfig` defaults. Only the broker URI can be set via the environment.
- One sensor per node process.
- `online` is not republished after an automatic reconnect.

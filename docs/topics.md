<!--
Specify site/area/node/sensor/metric and the JSON payload (ts, value, unit, seq) in docs/topics.md. Include examples, naming rules, and the status topic (LWT, retained).
-->
# MQTT Topics & Payloads

## Topic structure

Data:   `<site>/<area>/<node>/<sensor>/<metric>`  
Status: `<site>/<area>/<node>/status`

| Level  | Meaning                 | Example   |
|--------|-------------------------|-----------|
| site   | location / installation | `school1` |
| area   | room or zone            | `lab2`    |
| node   | device                  | `node01`  |
| sensor | sensor on the node      | `bme280`  |
| metric | measured quantity       | `temp`    |

## Naming rules

- Lowercase only, characters `a-z 0-9 - _`
- No spaces, no leading or trailing `/`
- No `+`, `#` or `$` in names
- Fixed depth: exactly 5 levels for data, 4 for status
- Names describe what, not how (`temp`, not `temp_celsius`; the unit is in the payload)

## Data payload (JSON)

```json
{"ts": 1732450000, "value": 21.5, "unit": "C", "seq": 42}
```

| Field | Type   | Description                                        |
|-------|--------|----------------------------------------------------|
| ts    | int    | Unix time, seconds, UTC                            |
| value | number | Measured value                                     |
| unit  | string | Unit of `value` (`C`, `%`, `hPa`, ...)             |
| seq   | int    | Per-sensor counter, +1 per message, resets on boot |

QoS 0, not retained (only current subscribers get the message).

## Examples

| Topic                                  | Payload                                                  |
|----------------------------------------|----------------------------------------------------------|
| `school1/lab2/node01/bme280/temp`      | `{"ts":1732450000,"value":21.5,"unit":"C","seq":42}`     |
| `school1/lab2/node01/bme280/humidity`  | `{"ts":1732450000,"value":48.2,"unit":"%","seq":42}`     |
| `school1/lab2/node01/bme280/pressure`  | `{"ts":1732450000,"value":1013.2,"unit":"hPa","seq":42}` |

Subscriptions:
- All of one node: `school1/lab2/node01/#`
- All temperatures: `school1/+/+/+/temp`
- All node states: `+/+/+/status`

## Status topic (LWT)

Topic: `<site>/<area>/<node>/status`
Payload: plain string `online` or `offline`
QoS 1, **retained**

- On connect, the client registers an LWT: topic = status topic, payload = `offline`, retained, QoS 1.
- After a successful connect, the client publishes `online` (retained, QoS 1).
- On clean shutdown, the client publishes `offline` itself.
- If the connection drops unexpectedly, the broker publishes the LWT (`offline`).
- Because it is retained, new subscribers see the current state immediately.

## Mapping data sources to topic levels

The structure stays the same for every source. Only the names change.

### Simulated data

Names are invented but kept consistent. One node per simulated device.

| Level  | Value                          |
|--------|--------------------------------|
| site   | `sim`     |
| area   | `lab`             |
| node   | `sim01`, `sim02`, ...          |
| sensor | `fake-temp`   |
| metric | `temp`, `humidity`, `pressure` |

```
sim/lab/sim01/fake-temp/temp
sim/lab/sim01/status
```

- With multiple containers, set a unique node name via env var (`NODE_ID=sim01`, `sim02`) so topics and LWT stay unique.

### Open data sources (e.g. weather)

Model where the measurement physically comes from. The node is whatever produces the messages (fetcher script or station).

| Level  | Value                                                       |
|--------|-------------------------------------------------------------|
| site   | city: `berlin`                                              |
| area   | district or `outdoor` (`mitte`, `airport`)                  |
| node   | station ID or fetcher script (`station-10382`, `fetcher01`) |
| sensor | data source: `openmeteo`, `dwd`                             |
| metric | `temp`, `humidity`, `pressure`, `wind`                      |

```
berlin/outdoor/fetcher01/openmeteo/temp
berlin/outdoor/fetcher01/openmeteo/humidity
berlin/outdoor/fetcher01/status
```

- API with several stations: one node per station, so topics stay unique.
- Script fetches everything: the script is the node; its status topic (`online`/`offline`, LWT) describes the script.
- `ts` = observation time from the API, not the fetch time.
- Free APIs without key: Open-Meteo, Bright Sky (DWD data).

## Discovery (which topics exist?)

MQTT has no "list all topics" request. Options:

- **Subscribe to `#`**: shows every message from now on (plus retained ones). A topic that has not published yet stays invisible.
  ```bash
  mosquitto_sub -h localhost -t '#' -v
  ```
- **Node registry via status topics**: every node publishes a retained status, so subscribing to `+/+/+/status` lists all known nodes immediately, with online/offline state.
  ```bash
  mosquitto_sub -h localhost -t '+/+/+/status' -v
  ```
-

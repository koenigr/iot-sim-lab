# Roadmap: iot-sim-lab

Hardware-freies C++/IoT-Übungsprojekt: Sensor-Node-Simulator, Protokolle, Statistik, Dashboard, Docker und Ansible.

Faustregel für die Bewerbung: Nur Begriffe nennen, die du im Repo belegen kannst (Code, Test, README).

## Phase 0: Fundament (ca. 1 Woche)

- Repo, CMake, Ordnerstruktur (`sensor-node/`, `collector/`, `deploy/`, `docs/`)
- GoogleTest/Catch2, clang-format, clang-tidy, GitHub Actions (Build + Tests)
- `docker compose` mit Mosquitto-Broker

**Begriffe:** CMake, CI/CD, Unit Testing, Static Analysis, Containerisierung

## Phase 1: Sensor-Node in C++ (2-3 Wochen)

- Generator: Sinus + Rauschen, Drift, Ausreißer, Ausfälle
- State Machine: INIT → CONNECT → SAMPLE → PUBLISH → RECONNECT
- Feste Puffer, `std::array`, kein Heap in der Hauptschleife, keine Exceptions
- Konfiguration per Datei/Umgebungsvariablen
- MQTT-Publish (z. B. Paho oder libmosquitto), QoS 0/1, Retained, Last Will

**Begriffe:** Telemetrie, Publish/Subscribe, MQTT, QoS, Last Will and Testament, State Machine, Sampling Rate, Deterministisches Verhalten, RAII, `constexpr`

## Phase 2: Statistik / Signalverarbeitung (2-3 Wochen)

- Ringpuffer (templatisiert, ohne Heap)
- Gleitender Mittelwert, EWMA, Median-Filter
- Welford (Online-Varianz), z-Score-Anomalieerkennung
- Fixed-Point-Variante einer Funktion (Embedded-Bezug)
- Benchmarks und Tests mit bekannten Daten (Generator liefert Ground Truth)

**Begriffe:** Streaming-Statistik, Ringpuffer, Filterung, Anomalieerkennung, Fixed-Point-Arithmetik, Numerische Stabilität, Templates

## Phase 3: Collector, Speicherung, Dashboard (2 Wochen)

- C++-Subscriber: Validierung, Zeitstempel, Statistik, Schreiben in InfluxDB (oder Prometheus-Exporter)
- Grafana-Dashboard: Rohdaten, Filter, Anomalien, Node-Status
- Topic-Design: `site/area/node/sensor/metric`

**Begriffe:** Time-Series-Datenbank, Datenpipeline, Observability, Dashboard, Topic-Hierarchie, Payload-Format (JSON, optional CBOR/Protobuf)

## Phase 4: Zweites und drittes Protokoll (3 Wochen)

- Modbus TCP: Sensor-Node als Server (Holding/Input Registers), Collector als Client mit Polling
- CoAP: Observe, Confirmable/Non-confirmable Messages
- Gemeinsame Abstraktionsschicht (`ITransport`) im C++-Code, Protokolle austauschbar

**Begriffe:** Modbus TCP, Register Map, Polling vs. Event-driven, CoAP, REST für Constrained Devices, UDP vs. TCP, Hardware Abstraction Layer (HAL), Interface/Dependency Injection

## Phase 5: Robustheit und Fehlerinjektion (2 Wochen)

- `tc netem` (Latenz, Paketverlust, Jitter), Container-Kill, Broker-Ausfall
- Reconnect mit Exponential Backoff, Watchdog (Heartbeat-Topic), Offline-Pufferung
- Sanitizer (ASan/UBSan/TSan), Valgrind, Fuzzing eines Parsers (libFuzzer)
- Logging mit Levels

**Begriffe:** Fault Injection, Resilienz, Exponential Backoff, Watchdog, Heartbeat, Store-and-Forward, Fuzzing, Memory Safety, Thread Safety

## Phase 6: Sicherheit (2 Wochen)

- Mosquitto mit TLS (selbstsignierte CA), Client-Zertifikate (mTLS), ACLs pro Node
- Secrets nicht im Repo
- Optional: signierte Konfig/Payloads

**Begriffe:** TLS, mTLS, PKI, X.509, Zugriffskontrolle (ACL), Secure by Design, Threat Model (kurz in `docs/`)

## Phase 7: Deployment mit Ansible (1-2 Wochen)

- Rollen: Docker-Host, Broker, Collector, Grafana, Nodes
- Inventory, Variablen, Idempotenz, Vault für Secrets
- Test lokal mit Vagrant/zweitem Container oder VM

**Begriffe:** Infrastructure as Code, Ansible, Idempotenz, Provisionierung, Fleet-/Device-Management (nur als Konzept, ehrlich formulieren)

## Phase 8: Embedded-Brücke (optional, 2-3 Wochen)

- Node-Code so trennen, dass die Logik ohne OS-Abhängigkeit kompiliert (Core-Bibliothek + Plattform-Adapter)
- Cross-Compile-Build für ARM (Toolchain-File), Lauf unter QEMU
- Optional: Portierung der Statistik auf den STM32F407

**Begriffe:** Cross-Compilation, Toolchain File, QEMU, Portabilität, Plattformabstraktion, Ressourcenbeschränkte Systeme, Memory Footprint (mit `size`/`bloaty` gemessen)

## Phase 9: Doku und Bewerbung (laufend)

- README mit Architekturdiagramm, Quickstart (`make up`), Screenshots
- `docs/`: Designentscheidungen, Messergebnisse, Grenzen
- Im Lebenslauf: 1 Zeile + Link, Stichworte gruppiert

## Begriffsübersicht

| Bereich | Begriffe |
| --- | --- |
| Protokolle | MQTT (Broker, Topic, QoS 0/1/2, Retained, LWT, Clean Session), Modbus TCP (Register, Function Codes), CoAP (Observe, CON/NON), HTTP/REST, UDP/TCP, optional OPC UA |
| Architektur | Publish/Subscribe, Edge/Gateway/Cloud, Telemetrie, Datenpipeline, Polling vs. Event-driven, HAL, Digital Twin (nur der Sim-Node als Konzept) |
| Daten | Zeitreihen, Sampling Rate, Zeitstempel/Zeitsynchronisation (NTP), JSON, CBOR, Protobuf, Schema |
| Statistik/DSP | Gleitender Mittelwert, EWMA, Median-Filter, Varianz/Standardabweichung (Welford), z-Score, Anomalieerkennung, Ringpuffer, Fixed-Point |
| C++/Embedded-Stil | RAII, `constexpr`, Templates, `std::array`/`std::span`, keine dynamische Allokation, Exceptions-frei, Zustandsautomat, Watchdog, Memory Footprint, Cross-Compilation |
| Qualität | Unit Tests, CI, Static Analysis, Sanitizer, Fuzzing, Code Coverage |
| Robustheit | Fault Injection, Reconnect/Backoff, Store-and-Forward, Heartbeat, Latenz/Jitter/Paketverlust |
| Sicherheit | TLS, mTLS, PKI, ACL, Secret Management, Threat Model |
| DevOps | Docker, Compose, Ansible (Idempotenz, Vault), IaC, Observability, Grafana, InfluxDB/Prometheus |

## Formulierung im Lebenslauf (Entwurf)

Nur verwenden, was gebaut wurde.

> **iot-sim-lab**: C++17-Sensor-Node-Simulator (State Machine, statische Puffer) mit MQTT, Modbus TCP und CoAP; Streaming-Statistik (EWMA, Welford, z-Score), TLS/mTLS, Fehlerinjektion (`tc netem`), Docker/Ansible-Deployment, Grafana/InfluxDB, CI mit Sanitizern.

Gesamtdauer realistisch: ca. 3-4 Monate neben dem Alltag. Wenn die Zeit knapp wird: Phase 4 (CoAP) und Phase 8 streichen, der Rest trägt die Bewerbung.

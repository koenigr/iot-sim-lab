# iot-sim-lab
C++ sensor node simulator with MQTT/Modbus/CoAP, streaming statistics and Grafana dashboard in Docker

## Run-commands

    cmake -B build                 # konfigurieren (optional: -DENABLE_CLANG_TIDY=ON)
    cmake --build build            # bauen
    ctest --test-dir build --output-on-failure   # Tests
    find sensor-node collector -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i   # formatieren
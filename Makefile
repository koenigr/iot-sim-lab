BUILD_DIR ?= build
COMPOSE   := docker compose -f deploy/docker-compose.yml
SUB_TOPIC ?= lab/\#
TOPIC 	  ?= test/topic
MSG       ?= hello
BENCH_DIR := build-rel

FILES := $(shell find . -type f \( -name "*.cpp" -o -name "*.hpp" \) -not -path "./build*/*")

.DEFAULT_GOAL := help
.PHONY: help configure build sensor-node test tests format clean \
        broker-up broker-down broker-stop broker-sub broker-pub \
        bench-build bench tabs smoke tidy tidy-changed bench bench-build

help:  ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*?## ' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  %-14s %s\n", $$1, $$2}'

configure:  ## Configure CMake
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release

build: configure  ## Build everything
	cmake --build $(BUILD_DIR)

sensor-node: configure ## Build and run the MQTT publisher
	cmake --build $(BUILD_DIR) --target sensor-node
	MQTT_URI=tcp://localhost:1883 ./$(BUILD_DIR)/sensor-node/sensor-node


test: build  ## Build and run tests
	ctest --test-dir $(BUILD_DIR) --output-on-failure

tests: test  ## Alias for test

format:  tabs
	./scripts/format.sh

clean:  ## Remove build directory
	rm -rf $(BUILD_DIR)

broker-up:  ## Start Mosquitto broker
	$(COMPOSE) up -d

broker-down:  ## Stop and remove broker containers
	$(COMPOSE) down

broker-stop:  ## Stop broker, keep containers
	$(COMPOSE) stop

broker-sub:  ## Subscribe to TOPIC (default: test/topic)
	mosquitto_sub -h localhost -t $(SUB_TOPIC) -v

broker-pub:  ## Publish MSG to TOPIC
	mosquitto_pub -h localhost -t $(TOPIC) -m "$(MSG)"

bench-build:
	cmake -S . -B $(BENCH_DIR) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BENCH_DIR) --target stats_bench

bench: bench-build
	$(BENCH_DIR)/stats/stats_bench

tabs:
	@for f in $(FILES); do \
		unexpand --first-only -t 4 "$$f" > "$$f.tmp" && mv "$$f.tmp" "$$f"; \
	done

CLANG_TIDY_RUN ?= $(or $(shell command -v run-clang-tidy 2>/dev/null),$(shell command -v run-clang-tidy-18 2>/dev/null))

tidy:  ## Run clang-tidy on all sources
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	$(CLANG_TIDY_RUN) -p build -quiet '.*/iot-sim-lab/(sensor-node|stats|collector)/.*'

tidy-changed:  ## clang-tidy on files changed vs. main
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	git diff --name-only --diff-filter=d main -- '*.cpp' | xargs -r clang-tidy-18 -p build -quiet

smoke:  ## Smoke-test all important targets (build, test, tidy, bench, broker + publisher round-trip)
	@set -e; \
	trap '$(MAKE) --no-print-directory broker-down' EXIT; \
	echo "==> clean + build + test"; \
	$(MAKE) --no-print-directory clean; \
	$(MAKE) --no-print-directory test; \
	echo "==> tidy"; \
	$(MAKE) --no-print-directory tidy; \
	echo "==> format"; \
	$(MAKE) --no-print-directory format; \
	echo "==> bench"; \
	$(MAKE) --no-print-directory bench; \
	echo "==> broker round-trip"; \
	$(MAKE) --no-print-directory broker-up; \
	sleep 2; \
	out=$$(mktemp); \
	mosquitto_sub -h localhost -t '$(TOPIC)' -C 1 -W 5 > $$out & sub=$$!; \
	sleep 1; \
	$(MAKE) --no-print-directory broker-pub MSG=smoke-ok; \
	wait $$sub; \
	grep -q smoke-ok $$out; \
	rm -f $$out; \
	echo "==> publisher round-trip"; \
	out=$$(mktemp); \
	mosquitto_sub -h localhost -t 'lab/+/+/+/+' -C 1 -W 10 > $$out & sub=$$!; \
	sleep 1; \
	timeout 3 ./$(BUILD_DIR)/sensor-node/sensor-node || true; \
	wait $$sub; \
	grep -q '"seq":0' $$out; \
	rm -f $$out;
	$(MAKE) --no-print-directory broker-stop; \
	echo "==> smoke OK"

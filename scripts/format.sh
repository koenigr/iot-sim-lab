#!/bin/sh
find sensor-node collector -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i
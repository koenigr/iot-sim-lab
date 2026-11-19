#!/bin/sh
clang-format --version | grep -q 'version 18\.' || { echo "need clang-format 18.x"; exit 1; }
find sensor-node collector stats \( -name '*.cpp' -o -name '*.hpp' \) -print0 | xargs -0 clang-format -i

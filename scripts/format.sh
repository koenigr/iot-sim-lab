#!/bin/sh
want=18.1.8
clang-format --version | grep -q "$want" || { echo "need clang-format $want"; exit 1; }
find sensor-node collector stats \( -name '*.cpp' -o -name '*.hpp' \) -print0 | xargs -0 clang-format -i

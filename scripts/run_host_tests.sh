#!/bin/sh
set -eu
test_binary="${TMPDIR:-/tmp}/lake-sensor-common-tests"
g++ -std=c++17 -Wall -Wextra -Werror \
  -Icommon/include \
  common/src/lake_protocol.cpp common/src/measurement.cpp test/test_common.cpp \
  -o "$test_binary"
"$test_binary"


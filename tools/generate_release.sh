#!/bin/sh

# This should only be run on Mac!
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "This script should only be run on MacOS."
    exit 1
fi

# A PlatformIO installation is required for this script
if ! [ -x "$(command -v platformio)" ]; then
  echo 'Error: platformio is not installed.' >&2
  exit 1
fi

set -e
rm -rf release
mkdir -p release

# Create and copy MacOS binaries
platformio run -e fsw_native_leader
platformio run -e fsw_native_follower
platformio run -e fsw_teensy35_hitl_leader
platformio run -e fsw_teensy35_hitl_follower
platformio run -e fsw_teensy36_hitl_leader
platformio run -e fsw_teensy36_hitl_follower
platformio run -e fsw_preflight_leader
platformio run -e fsw_preflight_follower
platformio run -e fsw_flight_leader
platformio run -e fsw_flight_follower
platformio run -e gsw_downlink_parser
platformio run -e gsw_uplink_producer
platformio run -e gsw_telem_info_generator

cp .pio/build/fsw_native_leader/program               release/hootl_leader_macOS
cp .pio/build/fsw_native_follower/program             release/hootl_follower_macOS
cp .pio/build/fsw_teensy35_hitl_leader/firmware.hex     release/fsw_teensy35_hitl_leader.hex
cp .pio/build/fsw_teensy35_hitl_follower/firmware.hex   release/fsw_teensy35_hitl_follower.hex
cp .pio/build/fsw_teensy36_hitl_leader/firmware.hex     release/fsw_teensy36_hitl_leader.hex
cp .pio/build/fsw_teensy36_hitl_follower/firmware.hex   release/fsw_teensy36_hitl_follower.hex
cp .pio/build/fsw_preflight_leader/firmware.hex         release/fsw_preflight_leader.hex
cp .pio/build/fsw_preflight_follower/firmware.hex       release/fsw_preflight_follower.hex
cp .pio/build/fsw_flight_leader/firmware.hex            release/fsw_flight_leader.hex
cp .pio/build/fsw_flight_follower/firmware.hex          release/fsw_flight_follower.hex
cp .pio/build/gsw_downlink_parser/program      release/downlink_parser_macOS
cp .pio/build/gsw_uplink_producer/program      release/uplink_producer_macOS
cp .pio/build/gsw_telem_info_generator/program   release/telem_info_generator_macOS

# Create and copy Linux binaries
rm -rf .pio
docker build -t fsw .
docker run -v "$(pwd)"/release:/release fsw /bin/sh -c \
  "pio run -e fsw_native_leader; \
   pio run -e fsw_native_follower; \
   pio run -e gsw_downlink_parser; \
   pio run -e gsw_uplink_producer; \
   pio run -e gsw_telem_info_generator; \
   cp .pio/build/fsw_native_leader/program                 /release/hootl_leader_linux-x86_64; \
   cp .pio/build/fsw_native_follower/program               /release/hootl_follower_linux-x86_64; \
   cp .pio/build/gsw_downlink_parser/program      /release/downlink_parser_linux-x86_64; \
   cp .pio/build/gsw_uplink_producer/program      /release/uplink_producer_linux-x86_64; \
   cp .pio/build/gsw_telem_info_generator/program /release/telem_info_generator_linux-x86_64"

# Produce the telemetry report
cd release
chmod +x telem_info_generator_macOS
./telem_info_generator_macOS telemetry_report.json

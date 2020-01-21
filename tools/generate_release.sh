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
platformio run -e native
platformio run -e teensy35_hitl
platformio run -e teensy36_hitl
platformio run -e preflight
platformio run -e flight
platformio run -e downlink_parser
platformio run -e uplink_producer
platformio run -e telem_info_generator

cp .pio/build/native/program               release/hootl_macOS
cp .pio/build/teensy35_hitl/firmware.hex   release/teensy35_hitl.hex
cp .pio/build/teensy36_hitl/firmware.hex   release/teensy36_hitl.hex
cp .pio/build/preflight/firmware.hex       release/preflight.hex
cp .pio/build/flight/firmware.hex          release/flight.hex
cp .pio/build/downlink_parser/program      release/downlink_parser_macOS
cp .pio/build/uplink_producer/program      release/uplink_producer_macOS
cp .pio/build/telem_info_generator/program release/telem_info_generator_macOS

# Create and copy Linux binaries
rm -rf .pio
docker build -t fsw .
docker run -v "$(pwd)"/release:/release fsw /bin/sh -c \
  "pio run -e native; \
   pio run -e downlink_parser; \
   pio run -e uplink_producer; \
   pio run -e telem_info_generator; \
   cp .pio/build/native/program               /release/hootl_linux-x86_64; \
   cp .pio/build/downlink_parser/program      /release/downlink_parser_linux-x86_64; \
   cp .pio/build/uplink_producer/program      /release/uplink_producer_linux-x86_64; \
   cp .pio/build/telem_info_generator/program /release/telem_info_generator_linux-x86_64"

# Produce the telemetry report
cd release
chmod +x telem_info_generator_macOS
./telem_info_generator_macOS telemetry_report.json

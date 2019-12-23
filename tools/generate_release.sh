# This should only be run on Mac!
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "This script should only be run on MacOS."
    exit 1
fi

set -e
mkdir -p release

platformio run -e native
platformio run -e teensy35_hitl
platformio run -e teensy36_hitl
platformio run -e preflight
platformio run -e flight
platformio run -e downlink_parser
# platformio run -e telem_info_generator

cp .pio/build/native/program release/macOS_hootl
cp .pio/build/teensy35_hitl/firmware.hex release/teensy35_hitl.hex
cp .pio/build/teensy36_hitl/firmware.hex release/teensy36_hitl.hex
cp .pio/build/preflight/firmware.hex release/preflight.hex
cp .pio/build/flight/firmware.hex release/flight.hex
cp .pio/build/downlink_parser/program release/macOS_downlink_parser
# cp .pio/build/telem_info_generator/program release/macOS_telem_info_generator

docker build -t fswbase -f tools/Dockerfile.base .
docker build -t release -f tools/Dockerfile.release .
docker rm release
docker create --name release release
docker cp release:/FlightSoftware/.pio/build/native/program release/linux-x86_64_hootl
docker cp release:/FlightSoftware/.pio/build/downlink_parser/program release/linux-x86_64_downlink_parser
# docker cp release:/FlightSoftware/.pio/build/telem_info_generator/program release/linux-x86_64_telem_info_generator

# Produce the telemetry report
# cd release
# chmod +x macOS_telem_info_generator
# ./macOS_telem_info_generator telemetry_report.json
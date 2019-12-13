# This should only be run on Mac!

set -e
mkdir -p release

platformio run -e native
platformio run -e teensy35_hootl
platformio run -e teensy36_hootl
platformio run -e preflight
platformio run -e flight
platformio run -e downlink_parser
platformio run -e telem_info_generator

cp .pio/build/native/program release/macOS
cp .pio/build/teensy35_hootl/firmware.hex release/teensy35_hootl
cp .pio/build/teensy36_hootl/firmware.hex release/teensy36_hootl
cp .pio/build/preflight/firmware.hex release/preflight
cp .pio/build/flight/firmware.hex release/flight
cp .pio/build/downlink_parser/program release/downlink_parser
cp .pio/build/telem_info_generator/program release/telem_info_generator

cd release
chmod +x telem_info_generator
./telem_info_generator telemetry_report.json # Produce the telemetry report

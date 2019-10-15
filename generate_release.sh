# This should only be run on Mac!

set -e
mkdir -p release

platformio run -e native
platformio run -e teensy35_cli_hootl
platformio run -e teensy36_cli_hootl
platformio run -e teensy_cli_preflight
platformio run -e teensy_cli_flight

cp .pio/build/native/program release/macOS
cp .pio/build/teensy35_cli_hootl/firmware.hex release/teensy35_hootl
cp .pio/build/teensy36_cli_hootl/firmware.hex release/teensy36_hootl
cp .pio/build/teensy_cli_preflight/firmware.hex release/preflight
cp .pio/build/teensy_cli_flight/firmware.hex release/flight

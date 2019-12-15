# This should only be run on Mac!
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "This script should only be run on MacOS."
    exit 1
fi

set -e
mkdir -p release

platformio run -e native
platformio run -e teensy35_hootl
platformio run -e teensy36_hootl
platformio run -e preflight
platformio run -e flight

cp .pio/build/native/program release/macOS
cp .pio/build/teensy35_hootl/firmware.hex release/teensy35_hootl
cp .pio/build/teensy36_hootl/firmware.hex release/teensy36_hootl
cp .pio/build/preflight/firmware.hex release/preflight
cp .pio/build/flight/firmware.hex release/flight

docker build -t fswbase -f tools/Dockerfile.base .
docker build -t fswrelease -f tools/Dockerfile.release .
docker create --name fswrelease fswrelease
docker cp fswrelease:/FlightSoftware/.pio/build/native/program release/linux-x86_64

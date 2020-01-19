# Fail on any error
set -e

# Ensure that code compiles first.
pio run -e teensy

# Run tests on the PAN Teensy farm's Teensy 3.5.
# For now don't do this because the Teensy farm isn't working.

# platformio remote --agent PANTeensyFarm update
# platformio remote --agent PANTeensyFarm test --environment teensy --force-remote --test-port=/dev/ttyACM0 --upload-port=dev/ttyACM0 -v

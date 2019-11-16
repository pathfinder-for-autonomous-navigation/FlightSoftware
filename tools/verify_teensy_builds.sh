set -e # Exit on any error

# Run static analysis
cppcheck test src lib/utils lib/Drivers lib/Devices lib/rwmutex

# Compile HOOTL/Flight Teensy environments
platformio run -e teensy35_cli_hootl
platformio run -e teensy36_cli_hootl
platformio run -e teensy_cli_preflight

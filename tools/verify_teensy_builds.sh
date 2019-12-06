set -e # Exit on any error

# Run static analysis
cppcheck test src lib/utils lib/Drivers lib/Devices lib/rwmutex

# Compile HOOTL/Flight Teensy environments
platformio run -e teensy35_hootl
platformio run -e teensy36_hootl
platformio run -e preflight
platformio run -e flight

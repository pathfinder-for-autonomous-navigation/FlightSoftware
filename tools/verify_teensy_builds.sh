set -e # Exit on any error

# Run static analysis
cppcheck test src lib/fsw/utils lib/fsw/Drivers lib/fsw/Devices

# Compile HOOTL/HITL/Flight Teensy environments
platformio run -e fsw_teensy35_hitl
platformio run -e fsw_teensy36_hitl
platformio run -e fsw_preflight
platformio run -e fsw_flight

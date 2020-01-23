set -e # Exit on any error

# Run static analysis
cppcheck test src lib/fsw/utils lib/fsw/Drivers lib/fsw/Devices

# Compile HOOTL/HITL/Flight Teensy environments
platformio run -e fsw_teensy35_hitl_leader
platformio run -e fsw_teensy35_hitl_follower
platformio run -e fsw_teensy36_hitl_leader
platformio run -e fsw_teensy36_hitl_follower
platformio run -e fsw_preflight_leader
platformio run -e fsw_preflight_follower
platformio run -e fsw_flight_leader
platformio run -e fsw_flight_follower

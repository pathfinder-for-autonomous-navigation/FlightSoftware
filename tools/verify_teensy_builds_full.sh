set -e # Exit on any error

# Compile HOOTL/HITL/Flight Teensy environments
platformio run -e fsw_teensy36_hitl_leader
platformio run -e fsw_teensy36_hitl_follower
platformio run -e fsw_preflight_leader
platformio run -e fsw_preflight_follower
platformio run -e fsw_flight_leader
platformio run -e fsw_flight_follower

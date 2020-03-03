set -e # Exit on any error

# Compile ADCS environments
platformio run -e adcs_teensy35_flight_leader
platformio run -e adcs_teensy35_flight_follower
platformio run -e adcs_teensy35_flight_debug_leader
platformio run -e adcs_teensy35_flight_debug_follower
platformio run -e adcs_teensy35_script_smoke_test

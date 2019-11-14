set -e # Exit on any error

# Run static analysis
cppcheck test src lib/utils lib/Drivers lib/Devices lib/rwmutex

# Compile HOOTL/Flight Teensy environments
platformio run -e teensy35_cli_hootl
platformio run -e teensy36_cli_hootl
platformio run -e teensy_cli_preflight

# Compile hardware functional tests
platformio run -e teensy_35_test_quake
platformio run -e teensy_35_test_quake_no_network
platformio run -e teensy_35_test_quake_network
platformio run -e teensy_36_test_quake
platformio run -e teensy_36_test_quake_no_network
platformio run -e teensy_36_test_quake_network

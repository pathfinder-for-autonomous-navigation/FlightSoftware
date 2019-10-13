set -e # Exit on error

# Compile and run native environments
platformio run -e native
platformio test -e native
platformio run -e native_dummy
python -m unittest test_dummy

# Compile and run HOOTL/Flight Teensy environments
platformio run -e teensy35_cli_hootl
platformio run -e teensy36_cli_hootl
platformio run -e teensy_cli_preflight

# Compile and run hardware functional tests
platformio run -e teensy_35_test_quake
platformio run -e teensy_35_test_quake_no_network
platformio run -e teensy_35_test_quake_network
platformio run -e teensy_36_test_quake
platformio run -e teensy_36_test_quake_no_network
platformio run -e teensy_36_test_quake_network

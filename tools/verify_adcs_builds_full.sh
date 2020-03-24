set -e

platformio run -e adcs_teensy35_flight_leader
platformio run -e adcs_teensy35_flight_follower
platformio run -e adcs_teensy35_flight_debug_leader
platformio run -e adcs_teensy35_flight_debug_follower
platformio run -e adcs_teensy35_script_smoke_test
platformio run -e adcs_teensy35_script_imu_test
platformio run -e adcs_teensy35_script_mtr_test
platformio run -e adcs_teensy35_script_rwa_test
platformio run -e adcs_teensy35_script_ssa_test
platformio run -e adcs_teensy35_script_state_test
platformio run -e adcs_teensy32_script_state_test
platformio run -e adcs_teensy35_script_ADS1015_test
platformio run -e adcs_teensy35_script_LSM6DSM_test
platformio run -e adcs_teensy35_script_MMC34160PJ_t
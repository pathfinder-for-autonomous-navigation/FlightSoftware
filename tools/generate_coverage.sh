# Script to locally generate coverage reports.

lcov --directory . --capture --output-file coverage.info
lcov --remove coverage.info '*/test/*/*' '*/usr/*' '*/lib/Devices*' '*/lib/json*' '*/lib/Drivers*' '*/lib/libsbp*' '*/.pio/*' '*/.platformio/*' --output-file coverage.info
lcov --list coverage.info

# Generate HTML report for local developer review.
genhtml coverage.info --output-directory html_coverage/

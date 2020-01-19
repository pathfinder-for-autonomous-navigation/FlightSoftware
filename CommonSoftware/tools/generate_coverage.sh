lcov --directory . --capture --output-file coverage.info
lcov --remove coverage.info '*/test/*' '*/usr/*' '*/ArduinoJson/*' '*/.platformio/*' '*/include/*' --output-file coverage.info
lcov --list coverage.info

# Generate HTML report for local developer review.
genhtml coverage.info --output-directory html_coverage/

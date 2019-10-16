lcov --directory . --capture --output-file coverage.info
lcov --remove coverage.info '*/test/*' '*/usr/*' '*/lib/Devices*' '*/lib/Drivers*' '*/lib/libsbp*' '*/lib/rwmutex*' '*/ArduinoJson/*' '*/.pio/*' '*/.platformio/*' --output-file coverage.info
lcov --list coverage.info

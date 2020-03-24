# Fail on any error
set -e

# Run unit tests
platformio test -e native_common -v
platformio test -e native_common_ci -v

# Check for memory mismanagement
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose .pio/build/native_common_ci/program

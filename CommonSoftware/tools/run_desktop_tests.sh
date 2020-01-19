# Fail on any error
set -e

# Statically analyze program
cppcheck test src

# Run compilation and unit tests
platformio run -e native
platformio test -e native -v
platformio run -e native_ci
platformio test -e native_ci -v

# Check for memory mismanagement
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose .pio/build/native_ci/program

set -e # Exit on any error

# Compile and run functional test for non-optimized native environment
platformio run -e native_ci
python -m unittest -v test_dummy_fsw

# Verify compilation and unit tests for optimized native environment
platformio run -e native
platformio test -e native

# Run tests for non-optimized native environment. These are
# identical to the tests for the optimized environment,
# so they're lower-priority in this CI build step (they're less
# likely to fail the build sequence.)
#
# We also need this step to run last so that code coverage is available.
platformio test -e native_ci

# Check for memory mismanagement
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose .pio/build/native_ci/program

# Fail on any error
set -e

# Run unit tests
platformio test -e native_common -v
platformio test -e native_common_ci -v

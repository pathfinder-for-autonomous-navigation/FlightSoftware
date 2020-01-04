#### Repository Tools

Note: these tools must be run from the root directory of the repository, e.g.

    ./tools/generate_coverage.sh

## Summary of Tools
- `generate_coverage.sh`: after running desktop unit tests via `run_desktop_tests.sh`, this file can be used to generate a coverage report
- `generate_release.sh`: can be used to fetch release binaries from the `.pio` folder when desired.
- `reformat_code.sh`: runs Clang formatter on the entire repository.
- `run_desktop_tests.sh`: runs flight software unit tests on your desktop computer in optimized and non-optimized environments.
- `verify_teensy_builds.sh`: Ensures that all Teensy environments compile correctly.

Travis CI runs the files `verify_teensy_builds.sh` and `run_desktop_tests.sh` in parallel. After running `run_desktop_tests.sh` it also runs `generate_coverage.sh`.

Separately, Travis CI updates the coverage data to Coveralls (this is not part of the coverage generator script, so that only remote build data gets pushed to Coveralls.)

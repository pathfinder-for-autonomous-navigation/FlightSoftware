#### Repository Tools

Note: these tools must be run from the root directory of the repository, e.g.

    ./tools/generate_coverage.sh

## Summary of Tools
- `count_loc.sh`: Displays a line count using [cloc](https://github.com/AlDanial/cloc)
- `deploy_release.sh`: (Work in Progress) Deploys all compiled assets to satellites and mission control software
- `generate_coverage.sh`: after running desktop unit tests via `run_desktop_tests.sh`, this file can be used to generate a coverage report
- `generate_release.sh`: can be used to fetch release binaries from the `.pio` folder when desired.
- `reformat_code.sh`: runs Clang formatter on the entire repository.
- `run_common_software_tests.sh`: runs common software unit tests on your desktop computer in optimized and non-optimized environments.
- `run_desktop_flight_tests.sh`: runs flight software unit and functional tests on your desktop computer.
- `run_ground_tests.sh`: runs ground software unit and functional tests.
- `verify_teensy_builds.sh`: Ensures that all Teensy environments compile correctly.

Travis CI runs the files `verify_teensy_builds.sh`, `run_desktop_flight_tests.sh` and `run_common_software_tests.sh` in parallel. After running `run_desktop_flight_tests.sh` and `run_common_software_tests.sh` it also runs `generate_coverage.sh`.

Separately, Travis CI updates the coverage data to Coveralls (this is not part of the coverage generator script, so that only remote build data gets pushed to Coveralls.)

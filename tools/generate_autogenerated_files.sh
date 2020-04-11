# Generate `telemetry` file
pio run -e gsw_telem_info_generator
.pio/build/gsw_telem_info_generator/program telemetry

# Generate `constants` file
python tools/constant_reporter.py

# Generate flow_data.cpp
python src/flow_data_generator.py

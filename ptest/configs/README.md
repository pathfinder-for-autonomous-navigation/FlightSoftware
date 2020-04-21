# PTest Configuration

I recommend copy-pasting a template from this folder to somewhere else on your disk and updating it with real values. We don't want to upload actual configs to the repository since they contain sensitive data.

## Items
- `seed` : Randomization seed for simulation.
- `single_sat_sim`
  - If set to true, the MATLAB simulation expects there to be one device, `FlightController`, so that
    it can run a mission simulation on just one satellite.
  - If set to false, the MATLAB simulation expects there to be two devices, `FlightControllerLeader` and
    `FlightControllerFollower` to be defined, so that a full mission simulation may be carried out.
- `devices`: A list of device configuration objects, each with the following data:
  - `name` of device.
  - `run_mode`: Either set to `teensy` or `native`, depending on whether the device is connected via a Teensy or if the device should be emulated with a desktop binary.
  - `binary_filepath`: If the `run_mode` was specified as `native`, this filepath describes where to find the desktop binary. The filepath must be absolute.
- `radios`: A list of radio configuration objects, each with the following data:
  - `name` of device that the radio is connected to. It should correspond to one of the devices above.
  - `imei` IMEI # of the radio. This is used to tell Iridium which radio we're trying to collect/send data from/to.
  - `connect` If false, no attempt is made to establish the radio connection. This option exists so that the simulation can be run separately from radio testing.
- `tlm`: Information for connecting to the Iridium email account and the telemetry service.
  - `email_username`: Iridium email account.
  - `email_password`: Password for the Iridium email account.
  - `webservice` : Server and port information for connecting to the telemetry webservice.
    - `server`
    - `port`
  - `elasticsearch` : Server and port information for the Elasticsearch instance that stores telemetry.
    - `server`
    - `port`

All of the above fields are required, but the `devices` and `radios` list may potentially be empty.

#!/usr/local/bin/python3

from argparse import ArgumentParser
from cerberus import Validator
from .state_session import StateSession
from .radio_session import RadioSession
from .cmdprompt import StateCmdPrompt
from .simulation import Simulation, SingleSatSimulation
import json, sys, os, tempfile, time, threading

try:
    import pty, subprocess
except ImportError:
    # The current OS is Windows, and pty doesn't exist
    pass

class SimulationRun(object):
    def __init__(self, config_data, testcase_name, data_dir, radio_keys_config, flask_keys_config):
        self.testcase_name = testcase_name

        self.random_seed = config_data["seed"]
        self.single_sat_sim = config_data["single_sat_sim"]

        self.simulation_run_dir = os.path.join(data_dir, time.strftime("%Y%m%d-%H%M%S"))
        # Create directory for run data
        os.makedirs(self.simulation_run_dir, exist_ok=True)

        self.device_config = config_data["devices"]
        self.radios_config = config_data["radios"]
        self.radio_keys_config = radio_keys_config
        self.flask_keys_config = flask_keys_config

        self.devices = {}
        self.radios = {}
        self.binaries = []

    def start(self):
        """Starts a run of the simulation."""

        pan_logo_filepath = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'pan_logo.txt')
        with open(pan_logo_filepath, 'r') as pan_logo_file:
            print(pan_logo_file.read())

        self.is_running = True
        self.set_up_devices()
        self.set_up_radios()
        self.set_up_sim()
        self.set_up_cmd_prompt()

        if "CI" in os.environ:
            self.stop_all("Exiting in CI environment.", is_error=False)

    def set_up_devices(self):
        # Set up test table by connecting to each device specified in the config.
        for device in self.device_config:
            try:
                device_name = device["name"]
            except:
                self.stop_all("Invalid configuration file. A device's name was not specified.")

            # Check device configuration.
            if device.get('run_mode') not in ['teensy', 'native']:
                self.stop_all(f"Device configuration for {device_name} is invalid.")
            if device['run_mode'] == "teensy":
                if not device.get("baud_rate"):
                    self.stop_all(f"device configuration for {device_name} does not specify baud rate.")
            elif "binary_filepath" not in device.keys():
                self.stop_all(f"Binary firmware location not specified for {device_name}")

            # If we want to use the native desktop binary for a device, instead of
            # a connected Teensy, we can do that by wrapping a serial port around it.
            if device['run_mode'] == 'native':
                try:
                    master_fd, slave_fd = pty.openpty()
                    binary_filepath = device['binary_filepath']
                    if "CI" in os.environ:
                        cwd = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..")
                        binary_filepath = os.path.join(cwd, binary_filepath)
                    binary_process = subprocess.Popen(binary_filepath, stdout=master_fd, stderr=master_fd, stdin=master_fd)
                    self.binaries.append({
                        "device_name" : device["name"],
                        "subprocess": binary_process,
                        "pty_master_fd": master_fd,
                        "pty_slave_fd": slave_fd,
                    })
                    device['port'] = os.ttyname(slave_fd)
                    device['baud_rate'] = 9600
                except NameError:
                    # pty isn't defined because we're on Windows
                    self.stop_all(f"Cannot connect to a native binary for device {device_name}, since the current OS is Windows.")

            device_session = StateSession(device_name, self.simulation_run_dir)

            # Connect to device, failing gracefully if device connection fails
            if device_session.connect(device["port"], device["baud_rate"]):
                self.devices[device_name] = device_session
            else:
                self.stop_all("A required device is disconnected.")

        self.binary_monitor_thread = threading.Thread(
            name="Binary Monitor", target=self.binary_monitor)
        self.binary_monitor_thread.start()

    def binary_monitor(self):
        while self.is_running:
            for binary in self.binaries:
                status = binary['subprocess'].poll()
                if status is not None:
                    print(
                        f"Device {binary['device_name']} exited with status {status}."
                    )
            time.sleep(1.0)

    def set_up_radios(self):
        for radio in self.radios_config:
            radio_connected_device = radio["connected_device"]
            radio_name = radio["connected_device"] + "Radio"
            imei = radio["imei"]

            if radio['connect']:
                radio_data_name = radio_connected_device + "_radio"
                radio_session = RadioSession(radio_name, imei, self.simulation_run_dir, self.radio_keys_config, self.flask_keys_config)
                self.radios[radio_name] = radio_session

    def set_up_sim(self):
        _ = __import__("ptest.cases")
        testcases = getattr(_, "cases")
        try:
            testcase = getattr(testcases, self.testcase_name)
        except:
            self.stop_all(f"Nonexistent test case: {self.testcase_name}")
        print(f"Running mission testcase {self.testcase_name}.")

        if self.single_sat_sim:
            self.sim = SingleSatSimulation(self.devices, self.random_seed, testcase())
        else:
            self.sim = Simulation(self.devices, self.random_seed)
        self.sim.start()

    def set_up_cmd_prompt(self):
        # Set up user command prompt
        self.cmd_prompt = StateCmdPrompt(self.devices, self.radios, self.sim, self.stop_all)
        try:
            self.cmd_prompt.cmdloop()
        except (KeyboardInterrupt, SystemExit):
            # Gracefully exit session
            self.cmd_prompt.do_quit(None)
            self.stop_all("Exiting due to keyboard interrupt.", is_error=False)

    def stop_all(self, reason_for_stop, is_error = True):
        """Gracefully ends simulation run."""

        # Prevent multiple threads from trying to stop the simulation at the same time.
        if not self.is_running:
            return
        self.is_running = False

        stop_str = ("Error: " if is_error else "") + reason_for_stop
        print(stop_str)

        print("Stopping binary monitor thread...")
        time.sleep(1.0)
        self.binary_monitor_thread.join()

        print("Stopping simulation (please be patient)...")
        try:
            self.sim.stop(self.simulation_run_dir)
        except:
            # Simulation was never created
            pass

        num_radios = len(self.radios.values())
        print(f"Terminating {num_radios} radio connection(s)...")
        for radio in self.radios.values():
            radio.disconnect()

        num_devices = len(self.devices.values())
        print(f"Terminating {num_devices} device connection(s)...")
        for device in self.devices.values():
            device.disconnect()
        for binary in self.binaries:
            binary['subprocess'].terminate()
            os.close(binary['pty_master_fd'])
            os.close(binary['pty_slave_fd'])

        sys.exit(1 if is_error else 0)

def main(args):
    if sys.version_info[0] != 3 or sys.version_info[1] < 6:
        print("Running this script requires Python 3.6 or above.")
        sys.exit(1)

    parser = ArgumentParser(description='''
    Interactive console allows sending state commands to PAN Teensy devices, and parses console output 
    from Teensies into human-readable, storable logging information.''', prog="ptest runsim")

    parser.add_argument('-t', '--testcase', action='store', help='Name of mission testcase, specified in cases/.',
                        default = "EmptyCase")

    parser.add_argument('-c', '--conf', action='store', help='JSON file listing serial ports and Teensy computer names.',
                        default = "ptest/configs/ci.json")

    parser.add_argument('-rc', '--radio-conf', action='store', help='JSON file listing Iridium radio email username and password.',
                        default = "ptest/configs/radio_keys.json")

    parser.add_argument('-gc', '--ground-conf', action='store', help='JSON file listing ground software server and port.',
                        default = "ptest/configs/flask_keys.json")

    log_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'logs')
    parser.add_argument('-d', '--data-dir', action='store',
        help='''Directory for storing run data. Must be an absolute path. Default is logs/ relative to this script's location on disk.
                For the current run, a subdirectory of DATA_DIR is created in which the actual data is stored.''', default=log_dir)
    args = parser.parse_args(args)

    try:
        with open(args.conf, 'r') as config_file:
            config_data = json.load(config_file)

            config_schema = {
                "seed" : {"type" : "integer"},
                "single_sat_sim" : {"type": "boolean"},
                "devices" : {
                    "type" : "list",
                    "schema" : {
                        "type" : "dict",
                        "schema" : {
                            "name" : {"type" : "string"},
                            "run_mode" : {"type" : "string", "allowed" : ["native", "teensy"]},
                            "binary_filepath" : {
                                "type" : "string", 
                                "dependencies" : {"run_mode" : ["native"]}, 
                                "excludes" : ["port", "baud_rate"]
                            },
                            "port" : {
                                "type" : "string",
                                "dependencies" : {"run_mode" : ["teensy"]},
                                "excludes" : "binary_filepath"
                            },
                            "baud_rate" : {
                                "type" : "integer",
                                "dependencies" : {"run_mode" : ["teensy"]},
                                "excludes" : "binary_filepath"
                            },
                        }
                    }
                },
                "radios" : {
                    "type" : "list",
                    "schema" : {
                        "type" : "dict",
                        "schema" : {
                            "connected_device" : {"type" : "string"},
                            "imei" : {"type" : "string"},
                            "connect" : {"type" : "boolean"}
                        }
                    }
                }
            }

            v = Validator(config_schema)
            if not v.validate(config_data, config_schema):
                print("Malformed config file. The following errors were found. Exiting.")
                print(v.errors)
                sys.exit(1)

        if "CI" in os.environ:
            with open(args.radio_conf, "w") as radio_keys_config_file:
                json.dump({
                    "email_username" : os.environ.get("IRIDIUM_EMAIL_USERNAME"),
                    "email_password" : os.environ.get("IRIDIUM_EMAIL_PASSWORD"),
                }, radio_keys_config_file)

            with open(args.ground_conf, "w") as flask_keys_config_file:
                json.dump({
                    "server" : os.environ.get("GSW_SERVER"),
                    "port" : os.environ.get("GSW_PORT")
                }, flask_keys_config_file)

        with open(args.radio_conf) as radio_keys_config_file:
            radio_keys_config = json.load(radio_keys_config_file)

            radio_keys_schema = {
                "email_username" : {"type" : "string"},
                "email_password" : {"type" : "string"}
            }
            v = Validator(radio_keys_schema)
            if not v.validate(radio_keys_config, radio_keys_schema):
                print("Malformed radio keys file. The following errors were found. Exiting.")
                print(v.errors)
                sys.exit(1)

        with open(args.ground_conf) as flask_keys_config_file:
            flask_keys_config = json.load(flask_keys_config_file)

            flask_keys_schema = {
                "server": {
                    "type": "string"
                },
                "port": {
                    "type": "string"
                }
            }
            v = Validator(flask_keys_schema)
            if not v.validate(flask_keys_config, flask_keys_schema):
                print(
                    "Malformed flask keys file. The following errors were found. Exiting."
                )
                print(v.errors)
                sys.exit(1)

    except json.JSONDecodeError:
        print("Could not load config file. Exiting.")
        sys.exit(1)
    except KeyError:
        print("Malformed config file. Exiting.")
        sys.exit(1)

    simulation_run = SimulationRun(config_data, args.testcase, args.data_dir, radio_keys_config, flask_keys_config)
    simulation_run.start()

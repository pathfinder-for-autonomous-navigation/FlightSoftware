#!/usr/local/bin/python3

from argparse import ArgumentParser
from .cases.base.ptest_case import PTestCase
from .configs.schemas import *
from .usb_session import USBSession
from .radio_session import RadioSession
from .uplink_console import UplinkConsole
from .cmdprompt import StateCmdPrompt
from . import get_pio_asset
import json, sys, os, time, threading
import multiprocessing
multiprocessing.set_start_method('fork') 


try:
    import pty, subprocess
except ImportError:
    # The current OS is Windows, and pty doesn't exist
    pass

class PTest(object):
    def __init__(self, config_data, testcase_name, data_dir, is_interactive):
        self.testcase_name = testcase_name

        self.random_seed = config_data["seed"]

        self.simulation_run_dir = os.path.join(data_dir, testcase_name + "_" + time.strftime("%Y%m%d-%H%M%S"))
        # Create directory for run data
        os.makedirs(self.simulation_run_dir, exist_ok=True)

        self.device_config = config_data["devices"]
        self.radios_config = config_data["radios"]
        self.tlm_config = config_data["tlm"]

        self.is_interactive = is_interactive

        self.devices = {}
        self.radios = {}
        self.binaries = []

    def start(self):
        """Starts a run of the simulation."""

        pan_logo_filepath = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'pan_logo.txt')
        with open(pan_logo_filepath, 'r') as pan_logo_file:
            print(pan_logo_file.read())

        self.uplink_console = UplinkConsole(self.simulation_run_dir)

        self.is_running = True
        self.set_up_devices()
        self.set_up_radios()
        self.set_up_testcase()

        self.testcase.start()

        if self.is_interactive:
            self.set_up_cmd_prompt()
        else:
            self.stop_all("Exiting since user requested non-interactive execution.", is_error=False)

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
            elif "pio_target" not in device.keys():
                self.stop_all(f"PIO target not specified for {device_name}")

            # If we want to use the native desktop binary for a device, instead of
            # a connected Teensy, we can do that by wrapping a serial port around it.
            is_teensy = device['run_mode'] != 'native'
            if not is_teensy:
                try:
                    master_fd, slave_fd = pty.openpty()
                    binary_filepath = get_pio_asset(device['pio_target'])
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

            device_session = USBSession(device_name, self.uplink_console, device["http_port"], is_teensy, self.simulation_run_dir, 
                self.tlm_config, device['imei'], device["scrape_uplinks"], device["enable_auto_dbtelem"])

            # Connect to device, failing gracefully if device connection fails
            if device_session.connect(device["port"], device["baud_rate"]):
                self.devices[device_name] = device_session
            else:
                self.stop_all(f"Unable to set up USBSession for {device_name}.")

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

                radio_session = RadioSession(radio_name,
                    imei,
                    self.uplink_console,
                    radio["http_port"],
                    radio["send_queue_duration"],
                    radio["send_lockout_duration"],
                    radio["check_uplink_queue_enable"],
                    self.simulation_run_dir,
                    self.tlm_config)
                self.radios[radio_name] = radio_session

    def set_up_testcase(self):
        """
        Starts up the test case and the MATLAB simulation if it is required by the testcase.
        """

        _ = __import__("ptest.cases")
        testcases = getattr(_, "cases")
        try:
            testcase = getattr(testcases, self.testcase_name)
        except:
            self.stop_all(f"Nonexistent test case: {self.testcase_name}")
        print(f"Running mission testcase {self.testcase_name}.")

        self.testcase = testcase(self.is_interactive, self.random_seed, self.simulation_run_dir, self.device_config)
        self.testcase.setup(self.devices, self.radios)

    def set_up_cmd_prompt(self):
        # Set up user command prompt
        self.cmd_prompt = StateCmdPrompt(self.devices, self.radios, self.stop_all, self.testcase)
        try:
            self.cmd_prompt.cmdloop()
        except (KeyboardInterrupt, SystemExit):
            # Gracefully exit session
            self.cmd_prompt.do_quit(None)
            self.stop_all("Exiting due to keyboard interrupt.", is_error=False)

    def stop_all(self, reason_for_stop, is_error=True):
        """Gracefully ends simulation run."""

        # Prevent multiple threads from trying to stop the simulation at the same time.
        if not self.is_running:
            return
        self.is_running = False

        stop_str = ("Error: " if is_error else "") + reason_for_stop
        print(stop_str)

        print("Stopping binary monitor thread...")
        if hasattr(self, "binary_monitor_thread"):
            self.binary_monitor_thread.join()

        print("Stopping uplink console...")
        self.uplink_console.close()

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
            binary['subprocess'].wait()
            os.close(binary['pty_master_fd'])
            os.close(binary['pty_slave_fd'])

        sys.exit(1 if (is_error or self.testcase.errored) else 0)

def check_system_dependencies():
    if sys.version_info[0] != 3 or sys.version_info[1] < 6:
        print("Running PTest requires Python 3.6 or above.")
        sys.exit(1)

def main(args):
    check_system_dependencies()

    parser = ArgumentParser(description='''
    Interactive console allows sending state commands to PAN Teensy devices, and parses console output 
    from Teensies into human-readable, storable logging information.''', prog="ptest runsim")

    parser.add_argument('-t', '--testcase', action='store', help='Name of mission testcase, specified in cases/.',
                        default = "EmptyCase")

    parser.add_argument('-c', '--conf', action='store', help='JSON file listing serial ports and Teensy computer names.', required=True)
    parser.add_argument('-tlm', '--tlmconfig', action='store', help='Custom secret tlm config, otherwise defaults to secret', required=False, default = 'ptest/configs/tlm_secret.json')

    parser.add_argument('-ni', '--no-interactive', dest='interactive', action='store_false', help='If provided, disables the interactive console.')
    parser.add_argument('-i', '--interactive', dest='interactive', action='store_true', help='If provided, enables the interactive console.')
    parser.add_argument('--clean', dest='clean', action='store_true', help='Starts a fresh run if in HOOTL (deletes the EEPROM file.)')
    parser.set_defaults(interactive=True)

    log_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'logs')
    parser.add_argument('-d', '--data-dir', action='store',
        help='''Directory for storing run data. Must be an absolute path. Default is logs/ relative to this script's location on disk.
                For the current run, a subdirectory of DATA_DIR is created in which the actual data is stored.''', default=log_dir)
    args = parser.parse_args(args)

    if args.clean:
        print("Removing EEPROM file due to user request.")
        try: 
            os.remove("eeprom.json")
        except OSError:
            pass

    try:
        with open(args.conf, 'r') as config_file:
            config_data = json.load(config_file)
            validate_config(config_data, ptest_config_schema)
            
    except json.JSONDecodeError:
        print("Could not load config file. Exiting.")
        sys.exit(1)
    except KeyError:
        print("Malformed config file. Exiting.")
        sys.exit(1)

    try:
        with open(args.tlmconfig, 'r') as config_file:
            tlm_config_data = json.load(config_file)
            config_data = {**tlm_config_data, **config_data}

    except json.JSONDecodeError:
        print("Could not load config file. Exiting.")
        sys.exit(1)
    except KeyError:
        print("Malformed config file. Exiting.")
        sys.exit(1)
    except FileNotFoundError:
        print("WARNING TLM CONFIG NOT FOUND. DEFAULTING TO EMPTY TLM CONFIG")
        with open('ptest/configs/tlm_empty.json', 'r') as config_file:
            tlm_config_data = json.load(config_file)
            config_data = {**tlm_config_data, **config_data}
        
    test = PTest(config_data, args.testcase, args.data_dir, args.interactive)
    test.start()

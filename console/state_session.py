import time
import datetime
import serial
import threading
import json

class StateSession(object):
    '''
    Represents a connection session with a Flight Computer's state system.

    This class is used by the simulation software and user command prompt to read and write to a
    flight computer's state.
    '''

    def __init__(self, data_dir, device_name):
        '''
        Initializes state cmd prompt.

        Args:
        console_port: Serial port of connected Teensy
        data_dir: Directory in which to store the results of the run.
        device_name: Name of device being connected to
        '''
        # Device connection
        self.device_name = device_name

        # Simulation
        self.overriden_variables = set()

        # Telemetry/logging
        self.start_time = datetime.datetime.now()
        self.logfile_name = "{}/{}.log".format(data_dir, device_name)
        self.dbfile_name = "{}/{}.db".format(data_dir, device_name)
        self.logfile = open(self.logfile_name, "w+")
        self.dbfile = open(self.dbfile_name, "w+")
        self.dbfile.write("[")
        self.timeseries_data = {}

    def save_timeseries_data(self):
        '''Write timeseries data to file every 10 seconds.'''
        while self.running_logger:
            self.updating_data_lock.acquire()
            timeseries_data_copy = self.timeseries_data.copy()
            self.timeseries_data = {}
            self.updating_data_lock.release()

            json.dump(timeseries_data_copy, self.dbfile, default=str)
            self.dbfile.write(",")
            time.sleep(10.0)

        self.dbfile.write("}")
        # Clean up data into one nice & neat JSON object
        self.dbfile.seek(0)
        json.dump(json.load(self.dbfile), self.dbfile, default=str)
        self.dbfile.close()

    def connect(self, console_port, baud_rate, msg_check_delay=0.02):
        '''Starts serial connection to the flight computer.'''
        self.console_port = console_port
        self.baud_rate = baud_rate

        try:
            self.console = serial.Serial(self.console_port, self.baud_rate)

            self.updating_data_lock = threading.Lock()
            self.logging_thread = threading.Thread(target=self.check_console_msgs, args=(msg_check_delay))
            self.logging_thread.start()

            # Create thread to periodically save JSON data to disk so that
            # log data isn't lost.
            self.running_logger = True
            self.save_data_thread = threading.Thread(target=self.save_timeseries_data)
            self.save_data_thread.start()

            self.connected = True
            return True
        except serial.SerialException:
            print("Error: unable to open serial port for {}.".format(
                self.device_name))

            self.connected = False
            return False

    def check_console_msgs(self, delay):
        '''
        Read FC output for debug messages and state variable updates. Record debug messages
        to the logging file, and update the console's record of the state.

        Args:
        - delay: Time to wait between executions of the message check loop. Default is 20ms.
        '''

        while self.running_logger:
            logline = ""
            try:
                # Read line coming from device and parse it
                line = self.console.readline().rstrip()
                data = json.loads(line)

                data["time"] = self.start_time + datetime.timedelta(
                    milliseconds=data["t"])

                if "msg" in data:
                    # The logline represents a debugging message created by Flight Software. Report the message to the logger.

                    logline = "[{}] ({}) {}".format(data["time"],
                                                    data["svrty"], data["msg"])
                elif "err" in data:
                    # The log line represents an error in retrieving or writing state data that
                    # was caused by a StateSession client improperly setting/retrieving a value.
                    # Report this failure to the logger.

                    logline = "[{}] (ERROR) Tried to {} state value named '{}' but encountered an error: {}".format(
                        data["time"], data["mode"], data["field"], data["err"])
                    self.awaiting_value = False
                    self.awaited_value = None
                else:
                    # The log line represents a response to a data request from a StateSession client.
                    # Process the response and notify the read_state() function when it is processed.

                    self.updating_data_lock.acquire()

                    if data['field'] not in self.timeseries_data:
                        self.timeseries_data[data['field']] = {"timeseries": []}

                    self.timeseries_data[data['field']]["now"] = data['val']
                    self.timeseries_data[data['field']]["timeseries"].append(
                        (data['time'], data['val']))

                    self.updating_data_lock.release()

                    # If the "read state" command is awaiting the current field's value,
                    # report it!
                    self.awaiting_value = False
                    self.awaited_value = data['val']

            except json.JSONDecodeError:
                logline = "[RAW] {}".format(line)
            except serial.SerialException:
                print("Error: unable to read serial port for {}. Exiting.".
                      format(self.device_name))
                self.disconnect()
            except:
                print("Unspecified error. Exiting.")
                self.disconnect()
            finally:
                self.logfile.write(logline + "\n")

            time.sleep(delay)

    def read_state(self, field_name):
        '''
        Read state.
        
        Read the value of the state field associated with the given field name on the flight controller.
        '''

        print(field_name)
        json_cmd = {"mode": ord('r'), "field": str(field_name)}
        self.awaiting_value = field_name
        self.console.write(json.dumps(json_cmd).encode())

        # Wait for value to be found by check_console_msgs
        while self.awaiting_value:
            pass

        return self.awaited_value

    def _write_state_basic(self, field_name, val):
        '''
        Write state.

        Overwrite the value of the state field with the given state field name on the flight controller.
        '''
        json_cmd = {
            "mode": ord('w'),
            "field": str(field_name),
            "val": str(val)
        }
        self.console.write(json.dumps(json_cmd).encode())

    def write_state(self, field_name, val):
        '''
        Write state.

        Overwrite the value of the state field with the given state field name on the flight computer.
        If the value is being overriden by user input, no changes are applied to computer state.

        This function doesn't check for the value of the state actually getting set. That can be handled by
        wsfb().
        '''
        if field_name in self.overriden_variables:
            return
        self._write_state_basic(field_name, val)

    def write_state_fb(self, field_name, val):
        '''
        Write state and check write operation with feedback.

        Overwrite the value of the state field with the given state field name on the flight computer, and
        then verify (via a read request) that the state was actually set.
        '''

        self.write_state(field_name, val)
        return val == self.read_state(field_name)

    def override_state(self, field_name, val):
        '''
        Override simulation state.

        This function works the same way as ws(), but any fields modified through this function are added
        to a list of overriden variables. Once variables are in this list, ws() cannot be used to modify their
        state. This prevents Simulation, which only uses ws(), from writing state to the device. The reason
        for this function's existence is to allow manual overrides of state variables during operation, e.g.
        via a command prompt or within a test case definition.
        '''
        self.overriden_variables.add(field_name)
        self._write_state_basic(field_name, val)

    def release_override(self, field_name):
        '''
        Release override of simulation state.
        '''
        try:
            self.overriden_variables.remove(field_name)
        except KeyError:
            # It doesn't matter if you try to take the override off of a field that wasn't
            # actually being written.
            return

    def override_state_fb(self, field_name, val):
        '''
        Override state and check write operation with feedback.

        Behaves the same way as wsfb(), but is strictly written for a state variable that is overriden.
        '''

        self.override_state(field_name, val)
        return val == self.read_state(field_name)

    def disconnect(self):
        '''Quits the program and stores message log and field telemetry to file.'''

        print("Terminating console connection to and saving logging/telemetry data for {}.".format(self.device_name))

        # End threads if there was actually a connection to the device
        if self.connected:
            self.running_logger = False
            self.logging_thread.join()
            self.save_data_thread.join()
            self.console.close()
            self.logfile.close()

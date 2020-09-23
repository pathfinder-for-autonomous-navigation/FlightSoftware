import time
import datetime
import serial
import threading
import json
import traceback
import queue
import os
import pty
import subprocess
from multiprocessing import Process
import glob
from elasticsearch import Elasticsearch

from .data_consumers import Datastore, Logger
from .http_cmd import create_usb_session_endpoint

class USBSession(object):
    '''
    Represents a connection session with a Flight Computer's state system.

    This class is used by the simulation software and user command prompt to read and write to a
    flight computer's state.

    This object is thread-safe; if an instance of this class is shared between the MATLAB simulation
    interface (an instance of Simulation) and the user command line (an instance of StateCmdPrompt),
    they won't trip over each other in setting/receiving variables from the connected flight computer.
    '''

    def __init__(self, device_name, uplink_console, port, is_teensy, simulation_run_dir):
        '''
        Initializes state session with a device.
        '''

        # Device connection
        self.device_name = device_name
        self.port = port
        self.is_teensy = is_teensy

        # Uplink console
        self.uplink_console = uplink_console

        # Data logging
        self.datastore = Datastore(device_name, simulation_run_dir)
        self.logger = Logger(device_name, simulation_run_dir)
        self.raw_logger = Logger(device_name + "_raw", simulation_run_dir)
        self.telem_save_dir = simulation_run_dir

        #Start downlink parser. Compile it if it is not available.
        downlink_parser_filepath = ".pio/build/gsw_downlink_parser/program" 
        if not os.path.exists(downlink_parser_filepath):
            print("Compiling the downlink parser.")
            os.system("pio run -e gsw_downlink_parser > /dev/null")

        master_fd, slave_fd = pty.openpty()
        self.downlink_parser = subprocess.Popen([downlink_parser_filepath], stdin=master_fd, stdout=master_fd)
        self.dp_console = serial.Serial(os.ttyname(slave_fd), 9600, timeout=1)
        self.telem_save_dir = simulation_run_dir

        # Open a connection to elasticsearch
        self.es = Elasticsearch([{'host':"127.0.0.1",'port':"9200"}])

        # Simulation
        self.overriden_variables = set()

    def connect(self, console_port, baud_rate):
        '''
        Starts serial connection to the desired device.

        Args:
        - console_port: Serial port to connect to.
        - baud_rate: Baud rate of connection.
        '''
        try:
            self.console = serial.Serial(console_port, baud_rate)
            self.start_time = datetime.datetime.now() # This is t = 0 on the Teensy, +/- a few milliseconds.

            self.device_write_lock = threading.Lock() # Lock to prevent multiple writes to device at the same time.

            # Queues used to manage interface between the check_msgs_thread and calls to read_state or write_state
            self.field_requests = queue.Queue()
            self.field_responses = queue.Queue()

            self.datastore.start()
            self.logger.start()
            self.raw_logger.start()
            self.running_logger = True
            self.check_msgs_thread = threading.Thread(
                name=f"{self.device_name} logger thread",
                target=self.check_console_msgs)
            self.check_msgs_thread.start()

            print(f"Opened connection to {self.device_name}.")
        except serial.SerialException:
            print(f"Unable to open serial port for {self.device_name}.")
            return False

        try:
            self.flask_app = create_usb_session_endpoint(self)
            self.flask_app.config["uplink_console"] = self.uplink_console
            self.flask_app.config["console"] = self.console
            self.http_thread = Process(name=f"{self.device_name} HTTP Command Endpoint", target=self.flask_app.run, kwargs={"port": self.port})
            self.http_thread.start()
            print(f"{self.device_name} HTTP command endpoint is running at http://localhost:{self.port}")
            return True
        except:
            print(f"Unable to start {self.device_name} HTTP command endpoint at http://localhost:{self.port}")
            return False

    def check_console_msgs(self):
        '''
        Read device output for debug messages and state variable updates. Record debug messages
        to the logging file, and update the console's record of the state.
        '''

        while self.running_logger:
            try:
                # Read line coming from device and parse it
                if self.console.inWaiting() > 0:
                    line = self.console.readline().rstrip()
                    self.raw_logger.put("Received: " + line.decode("utf-8"))
                else:
                    continue

                data = json.loads(line)
                data['time'] = str(self.start_time + datetime.timedelta(milliseconds=data['t']))

                if 'msg' in data:
                    # The logline represents a debugging message created by Flight Software. Report the message to the logger.
                    logline = f"[{data['time']}] ({data['svrty']}) {data['msg']}"
                    self.logger.put(logline, add_time = False)
                elif 'telem' in data:
                    logline = f"[{data['time']}] Received requested telemetry from spacecraft.\n"
                    logline += data['telem']
                    print("\n" + logline)
                    self.logger.put(logline, add_time = False)
                    #log data to a timestamped file
                    telem_bytes = data['telem'].split(r'\x')
                    telem_bytes.remove("")
                    telem_file = open(os.path.join(self.telem_save_dir ,f"telem[{data['time']}].txt"), "wb")
                    for byte in telem_bytes:
                        telem_file.write(int(byte, 16).to_bytes(1, byteorder='big'))
                    telem_file.close()
                elif 'uplink' in data:
                    if data['uplink'] and data['len']:
                        logline = f"[{data['time']}] Successfully sent telemetry to FlightSoftware.\n"
                        logline += str(data['uplink'])
                    else:
                        logline = f"[{data['time']}] Failed to send telemetry to FlightSoftware."
                    print("\n" + logline)
                    self.logger.put(logline, add_time = False)
                else:
                    if 'err' in data:
                        # The log line represents an error in retrieving or writing state data that
                        # was caused by a USBSession client improperly setting/retrieving a value.
                        # Report this failure to the logger.

                        logline = f"[{data['time']}] (ERROR) Tried to {data['mode']} state value named \"{data['field']}\" but encountered an error: {data['err']}"
                        self.logger.put(logline, add_time = False)
                        data['val'] = None
                    else:
                        # A valid telemetry field was returned. Manage it.
                        self.datastore.put(data)

                    self.field_responses.put(data)

            except ValueError:
                logline = f'[RAW] {line}'
                self.logger.put(logline)
            except serial.SerialException:
                print('Error: unable to read serial port for {}. Exiting.'.
                      format(self.device_name))
                self.disconnect()
            except:
                traceback.print_exc()
                print('Unspecified error. Exiting.')
                self.disconnect()

    def _wait_for_state(self, field, timeout = None):
        """
        Helper function used by both read_state and write_state to wait for a desired value
        to be reported back by the connected device.
        """
        self.field_requests.put(field)
        try:
            data = self.field_responses.get(True, timeout)
            return data['val']
        except queue.Empty:
            return None

    def read_state(self, field, timeout = None):
        '''
        Read state.
        
        Read the value of the state field associated with the given field name on the flight controller.
        '''
        if not self.running_logger: return

        json_cmd = {'mode': ord('r'), 'field': str(field)}
        json_cmd = json.dumps(json_cmd) + "\n"
        self.device_write_lock.acquire()
        self.console.write(json_cmd.encode())
        self.device_write_lock.release()
        self.raw_logger.put("Sent:     " + json_cmd.rstrip())

        return self._wait_for_state(field)

    def str_to_val(self, field):
        '''
        Automatically detects floats, ints and bools

        Returns a float, int or bool
        '''
        if 'nan' in field:
            return float("NAN")
        elif '.' in field:
            return float(field)
        elif field == 'true':
            return True
        elif field == 'false':
            return False
        else:
            return int(field)

    def smart_read(self, field, **kwargs):
        '''
        Turns a string state field read into the actual desired vals.

        Returns list of vals, or the val itself. Vals can be bools, ints, or floats.
        Raises NameError if no state field was found.
        '''
        
        ret = self.read_state(field, kwargs.get('timeout'))
        if ret is None:
            raise NameError(f"State field: {field} not found.")

        # begin type inference

        if ',' in ret:
            # ret is a list
            list_of_strings = ret.split(',')
            list_of_strings = [x for x in list_of_strings if x is not '']
            list_of_vals = [self.str_to_val(x) for x in list_of_strings]
            return list_of_vals
        else:
            return self.str_to_val(ret)

    def _write_state_basic(self, fields, vals, timeout = None):
        '''
        Write multiple state fields to the device at once.
        '''
        if not self.running_logger: return

        assert len(fields) == len(vals)
        assert len(fields) <= 20, "Flight Software can't handle more than 20 state field writes at a time"

        json_cmds = ""
        for field, val in zip(fields, vals):
            json_cmd = {
                'mode': ord('w'),
                'field': str(field),
                'val': str(val)
            }
            json_cmd = json.dumps(json_cmd) + "\n"
            json_cmds += json_cmd

        if len(json_cmds) >= 512:
            print("Error: Flight Software can't handle input buffers >= 512 bytes.")
            return False

        self.device_write_lock.acquire()
        self.console.write(json_cmds.encode())
        self.device_write_lock.release()
        self.raw_logger.put("Sent:     " + json_cmds)

        returned_vals = []
        for field in fields:
            returned_vals.append(self._wait_for_state(field, timeout))

        if returned_vals[0] is None:
            return False

        returned_vals = returned_vals[0].split(",")
        returned_vals = [x for x in returned_vals if x is not ""]
        
        if (returned_vals[0].replace('.','').replace('-','')).isnumeric():
            numeric_returned_vals = [float(x) for x in returned_vals]
            if type(vals[0]) == str:
                vals = vals[0]
                vals = [float(x) for x in vals.split(",") if x is not '']

            return numeric_returned_vals == vals

        return returned_vals == vals

    def write_multiple_states(self, fields, vals, timeout=None):
        '''
        Write multiple states and check the write operation with feedback.

        Overwrite the value of the state field with the given state field name on the flight computer, and
        then verify that the state was actually set. Do not write the state if the variable is being overriden
        by the user. (This is the function that sim should exclusively use.)
        '''
        # Filter out fields that are being overridden by the user
        field_val_pairs = [
            field_val_pair for field_val_pair in zip(fields, vals)
            if field_val_pair[0] not in self.overriden_variables
        ]
        fields, vals = zip(*field_val_pairs)

        return self._write_state_basic(list(fields), list(vals), timeout)

    def _val_to_str(self, val):
        '''
        Convert a state value or list of values into a single string writable to
        a state.

        Currently, the supported types are integers, doubles, integer vectors,
        double vectors, and booleans.
        '''
        if type(val) not in (list, tuple):
            if type(val) is bool:
                return 'true' if val else 'false'
            else:
                return str(val)
        else:
            val_str = ''
            for _val in val:
                val_str += self._val_to_str(_val) + ', '
            return val_str[:len(val_str) - 2]

    def write_state(self, field, *args, **kwargs):
        '''
        Write state and check write operation with feedback.

        Overwrite the value of the state field with the given state field name on the flight computer, and
        then verify that the state was actually set. Do not write the state if the variable is being overriden
        by the user. (This is a function that sim should exclusively use.)
        '''
        return self.write_multiple_states([field], [self._val_to_str(args)], kwargs.get('timeout'))

    def send_uplink(self, filename):
        '''
        Gets the uplink packet from the given file. Sends the hex 
        representation of the packet and the length of the packet
        to the console to be processed by FlightSoftware
        '''
        # Get the uplink packet from the uplink sbd file
        try:
            file = open(filename, "rb")
        except:
            logline = f"Error:    File {filename} doesn't exist"
            self.raw_logger.put(logline)
            return False

        uplink_packet = file.read()
        uplink_packet_length = len(uplink_packet)
        file.close() 
        uplink_packet = str(''.join(r'\x'+hex(byte)[2:] for byte in uplink_packet)) #get the hex representation of the packet bytes

        # Send a command to the console to process the uplink packet
        json_cmd = {
            'mode': ord('u'),
            'val': uplink_packet,
            'length': uplink_packet_length
        }
        json_cmd = json.dumps(json_cmd) + "\n"

        self.device_write_lock.acquire()
        self.console.write(json_cmd.encode())
        self.device_write_lock.release()
        self.raw_logger.put("Sent:     " + json_cmd)

        return True
    
    def uplink(self, fields, vals, timeout=None):
        '''
        Create an uplink packet from the provided data and save it
        locally to disk.

        The send_uplink function can be used to send this uplink to
        the flight controller.

        Returns: false if the uplink could not be created, true otherwise.
        The uplink might not be possible to create if it uses unrecognized
        state fields or if its size exceeds 70 bytes.
        '''
        if not self.running_logger: return

        # Filter out fields that are being overridden by the user
        field_val_pairs = [
            field_val_pair for field_val_pair in zip(fields, vals)
            if field_val_pair[0] not in self.overriden_variables
        ]
        fields, vals = zip(*field_val_pairs)

        success = self.uplink_console.create_uplink(fields, vals, "uplink.sbd")

        # If the uplink packet exists, send it to the FlightSoftware console
        if success and os.path.exists("uplink.sbd"):
            success &= self.send_uplink("uplink.sbd")
            os.remove("uplink.sbd") 
            os.remove("uplink.json") 
            return success
        else:
            if os.path.exists("uplink.json"): os.remove("uplink.json") 
            return False

    def parsetelem(self):
        '''
        Provide the latest downlink telemetry file that was received from the
        spacecraft to the downlink producer, and then return the parsed value
        of the latest completed downlink frame as a JSON object.
        '''

        #get newest file
        telem_files = glob.iglob(os.path.join(self.telem_save_dir, 'telem*'))
        try:
            newest_telem_file = max(telem_files, key=os.path.basename)
        except ValueError:
            return "No telemetry to parse."
        self.dp_console.write((newest_telem_file+"\n").encode())
        telem_json_data = json.loads(self.dp_console.readline().rstrip())
        if telem_json_data is not None:
                telem_json_data = telem_json_data["data"]
        return telem_json_data

    def dbtelem(self):
        '''
        Run parsetelem(), and dump the results into the Elasticsearch database.

        This function is useful because it allows database-connected technologies,
        such as the telemetry webserver and OpenMCT, to consume downlink data.
        '''

        jsonObj = self.parsetelem()
        if not isinstance(jsonObj, dict):
            print("Error parsing telemetry.")
            return False
        failed = False
        for field in jsonObj:
            value = jsonObj[field]
            data=json.dumps({
            field: value,
                "time": str(datetime.datetime.now().isoformat())
            })
            res = self.es.index(index='statefield_report_'+str(self.device_name.lower()), doc_type='report', body=data)
            if not res['result'] == 'created':
                failed = True
        return not failed

    def override_state(self, field, *args, **kwargs):
        '''
        Override state and check write operation with feedback.

        Behaves the same way as write_state(), but is strictly written for a state variable that is overriden
        by the user, i.e. is no longer set by the simulation.
        '''
        self.overriden_variables.add(field)
        return self._write_state_basic([field], [self._val_to_str(args)], kwargs.get('timeout'))

    def release_override(self, field):
        '''
        Release override of simulation state.

        If the state wasn't currently being overriden, then this functions just
        acts as a no-op.
        '''
        self.overriden_variables.discard(field)

    def disconnect(self):
        '''Quits the program and stores message log and field telemetry to file.'''

        print(f' - Terminating console connection to and saving logging/telemetry data for {self.device_name}.')

        # End threads
        self.running_logger = False
        self.check_msgs_thread.join()
        self.console.close()
        self.dp_console.close()

        self.http_thread.terminate()
        self.http_thread.join()

        self.http_thread.terminate()
        self.http_thread.join()

        self.datastore.stop()
        self.logger.stop()
        self.raw_logger.stop()

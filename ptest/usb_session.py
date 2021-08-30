import datetime
import serial
import threading
import json
import traceback
import queue
import os
import pty
import subprocess
import glob
from elasticsearch import Elasticsearch
import email
import imaplib

from .data_consumers import Datastore, Logger
from . import get_pio_asset
from .cases.utils import str_to_val
import lin

class USBSession(object):
    '''
    Represents a connection session with a Flight Computer's state system.

    This class is used by the simulation software and user command prompt to read and write to a
    flight computer's state.

    This object is thread-safe; if an instance of this class is shared between the MATLAB simulation
    interface (an instance of Simulation) and the user command line (an instance of StateCmdPrompt),
    they won't trip over each other in setting/receiving variables from the connected flight computer.
    '''
    class Request(object):
        """Helper object allows read-state request to USB session to be easily
        synchronized.
        """
        def __init__(self, field: str):
            super(USBSession.Request, self).__init__()

            self.__field = field
            self.__lock = threading.Lock()
            self.__has_reply = threading.Condition(self.__lock)
            self.__data = None

        @property
        def data(self) -> dict:
            with self.__lock:
                while self.__data is None:
                    self.__has_reply.wait()

                return self.__data

        @data.setter
        def data(self, data: dict):
            with self.__lock:
                assert self.__data is None, "Error; the request has already been filled"
                assert data is not None, "Error; a request must be filled with a value other than None"

                self.__data = data
                self.__has_reply.notify_all()

        @property
        def field(self) -> str:
            return self.__field

    def __init__(self, device_name, uplink_console, port, is_teensy, simulation_run_dir, tlm_config, radio_imei, scrape_uplinks, enable_auto_dbtelem):
        '''
        Initializes state session with a device.
        '''

        # Device connection
        self.device_name = device_name
        self.port = port
        self.is_teensy = is_teensy
        self.radio_imei = radio_imei

        # Uplink console
        self.uplink_console = uplink_console

        # Data logging
        self.datastore = Datastore(device_name, simulation_run_dir)
        self.logger = Logger(device_name, simulation_run_dir)
        self.raw_logger = Logger(device_name + "_raw", simulation_run_dir)
        self.telem_save_dir = simulation_run_dir

        downlink_parser_filepath = get_pio_asset("gsw_downlink_parser")
        master_fd, slave_fd = pty.openpty()
        self.downlink_parser = subprocess.Popen([downlink_parser_filepath], stdin=master_fd, stdout=master_fd)
        self.dp_console = serial.Serial(os.ttyname(slave_fd), 9600, timeout=1)
        self.telem_save_dir = simulation_run_dir
        self.uplink_json_name = "uplink"+self.radio_imei+".http"
        self.uplink_sbd_name = "uplink"+self.radio_imei+".sbd"

        # Open a connection to elasticsearch
        self.es = Elasticsearch([{'host':"127.0.0.1",'port':"9200"}])

        #connect to email
        self.scrape = scrape_uplinks
        self.username=tlm_config["email_username"]
        self.password=tlm_config["email_password"]
        self.mail = None

        self.enable_auto_dbtelem = enable_auto_dbtelem

        if self.username != "":
            self.mail = imaplib.IMAP4_SSL("imap.gmail.com", 993)
            self.mail.login(self.username, self.password)
            self.mail.select('"[Gmail]/Sent Mail"')

        self.debug_to_console = None

    def case_interaction_setup(self, _debug_to_console):
        self.debug_to_console = _debug_to_console

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

            # Prevent multiple writes to the device at one time
            self.device_lock = threading.Lock()

            # Prevent multiple requests from being pushed to the queue at the same time
            self.request_lock = threading.Lock()
            self.requests = queue.Queue()

            self.datastore.start()
            self.logger.start()
            self.raw_logger.start()
            self.running_logger = True
            self.check_msgs_thread = threading.Thread(
                name=f"{self.device_name} logger thread",
                target=self.check_console_msgs)
            self.check_msgs_thread.start()

            self.scrape_uplinks_thread = threading.Thread(
                name=f"{self.device_name} uplinks",
                target=self.scrape_uplinks)
            self.scrape_uplinks_thread.start()

            print(f"Opened connection to {self.device_name}.")
        except serial.SerialException:
            print(f"Unable to open serial port for {self.device_name}.")
            return False

        return True

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

                    # If we want debug to go to the console
                    if self.debug_to_console:
                        print(logline)

                elif 'telem' in data:
                    logline = f"[{data['time']}] Received requested telemetry from spacecraft.\n"
                    logline += data['telem']
                    # print("\n" + logline)
                    self.logger.put(logline, add_time = False)
                    #log data to a timestamped file
                    telem_bytes = data['telem'].split(r'\x')
                    telem_bytes.remove("")
                    telem_file = open(os.path.join(self.telem_save_dir ,f"telem{self.radio_imei}[{data['time']}].txt"), "wb")
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

                    request = self.requests.get(block=False).data = data

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

    def read_state(self, field, timeout = None):
        '''
        Read state.
        
        Read the value of the state field associated with the given field name on the flight controller.
        '''
        if not self.running_logger: return

        json_cmd = {
            'mode': ord('r'),
            'field': str(field)
        }
        json_cmd = json.dumps(json_cmd) + "\n"

        request = USBSession.Request(field)
        with self.request_lock:
            self.requests.put(request, block=False)
            with self.device_lock:
                self.console.write(json_cmd.encode())

        self.raw_logger.put("Sent:     " + json_cmd.rstrip())

        return request.data['val']

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
        return str_to_val(ret)

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
                'val': self._val_to_str(val)
            }
            json_cmd = json.dumps(json_cmd) + "\n"
            json_cmds += json_cmd

        if len(json_cmds) >= 512:
            print("Error: Flight Software can't handle input buffers >= 512 bytes.")
            return False

        requests = [USBSession.Request(field) for field in fields]
        with self.request_lock:
            for request in requests:
                self.requests.put(request, block=False)
            with self.device_lock:
                self.console.write(json_cmds.encode())

        self.raw_logger.put("Sent:     " + json_cmds)

        returned_vals = [request.data['val'] for request in requests]
        if returned_vals[0] is None:
            return False

        returned_vals = returned_vals[0].split(",")
        returned_vals = [x for x in returned_vals if x != ""]
        
        if (returned_vals[0].replace('.','').replace('-','')).isnumeric():
            numeric_returned_vals = [float(x) for x in returned_vals]
            if type(vals[0]) == str:
                vals = vals[0]
                vals = [float(x) for x in vals.split(",") if x != '']

            return numeric_returned_vals == vals

        return returned_vals == vals

    def write_multiple_states(self, fields, vals, timeout=None):
        '''
        Write multiple states and check the write operation with feedback.

        Overwrite the value of the state field with the given state field name on the flight computer, and
        then verify that the state was actually set.
        '''
        # Filter out fields that are being overridden by the user
        field_val_pairs = [
            field_val_pair for field_val_pair in zip(fields, vals)
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
        if(type(val) in {lin.Vector2, lin.Vector3, lin.Vector4}):
            val = list(val)
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
        then verify that the state was actually set.
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

        with self.device_lock:
            self.console.write(json_cmd.encode())

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
        ]
        fields, vals = zip(*field_val_pairs)

        success = self.uplink_console.create_uplink(fields, vals, self.uplink_sbd_name, self.uplink_json_name)

        # If the uplink packet exists, send it to the FlightSoftware console
        if success and os.path.exists(self.uplink_sbd_name):
            success &= self.send_uplink(self.uplink_sbd_name)
            os.remove(self.uplink_sbd_name) 
            os.remove(self.uplink_json_name) 
            return success
        else:
            if os.path.exists(self.uplink_json_name): os.remove(self.uplink_json_name) 
            return False

    def parsetelem(self):
        '''
        Provide the latest downlink telemetry file that was received from the
        spacecraft to the downlink producer, and then return the parsed value
        of the latest completed downlink frame as a JSON object.
        '''

        #get newest file
        telem_files = glob.iglob(os.path.join(self.telem_save_dir, f'telem{self.radio_imei}*'))
        try:
            newest_telem_file = max(telem_files, key=os.path.basename)
        except ValueError:
            return "No telemetry to parse."
        self.dp_console.write((newest_telem_file+"\n").encode())
        line = self.dp_console.readline().rstrip()
        if line == b'':
            # TODO A MORE FORMAL FIX
            print("[ WARNING ] USB_SESSION LINE FIX")
            line = b'null'
        telem_json_data = json.loads(line)

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
            # print(f"Error parsing telemetry on {self.device_name}")            
            return False
        failed = False
        for field in jsonObj:
            value = jsonObj[field]
            data=json.dumps({
            field: value,
                "time.downlink_received": str(datetime.datetime.now().isoformat())
            })
            res = self.es.index(index='statefield_report_'+str(self.radio_imei), doc_type='report', body=data)
            if not res['result'] == 'created':
                failed = True
        return not failed

    def scrape_uplinks(self):
        '''
        For the AMC tests, we need the Flight Computer to read sent uplinks
        without actually using Iridium. This method reads from the "sent"
        box in the PAN email account (attempted uplinks) and passes the uplink packet
        directly to the Flight computer.
        '''
        while self.scrape == True and self.mail != None:
            self.scrape_uplink()

    def scrape_uplink(self):
        '''
        Look in the Sent Mail box of the Pan email account and forward all the 
        uplinks directed to this satellite to the Flight Computer
        '''
        #look for all new emails from iridium
        try:
            self.mail.select('"[Gmail]/Sent Mail"')
        except:
            self.mail = imaplib.IMAP4_SSL("imap.gmail.com", 993)
            self.mail.login(self.username, self.password)
            self.mail.select('"[Gmail]/Sent Mail"')
        _, data = self.mail.search(None, '(FROM "pan.ssds.qlocate@gmail.com")', '(UNSEEN)')
        mail_ids = data[0]
        id_list = mail_ids.split()

        for num in id_list:
            #.fetch() fetches the mail for given id where 'RFC822' is an Internet 
            # Message Access Protocol.
            _, data = self.mail.fetch(num,'(RFC822)')

            #go through each component of data
            for response_part in data:
                if isinstance(response_part, tuple):
                    # converts message from byte literal to string removing b''
                    msg = email.message_from_bytes(response_part[1])
                    email_subject = msg['subject']
                    
                    if email_subject.isdigit():
                        # Get imei number of the radio that the uplink was sent to
                        radio_imei = int(email_subject)

                        if self.radio_imei != None and radio_imei == int(self.radio_imei):
                            # Go through the email contents
                            for part in msg.walk():
                                            
                                if part.get_content_maintype() == 'multipart':
                                    continue

                                if part.get('Content-Disposition') is None:
                                    continue

                                # Check if there is an email attachment
                                if part.get_filename() is not None:
                                    # Download uplink packet from email attachment and send it to the Flight Computer
                                    fp = open("new_" + self.uplink_sbd_name, 'wb')
                                    fp.write(part.get_payload(decode=True))
                                    fp.close()
                                    self.send_uplink("new_"+self.uplink_sbd_name)
                                    os.remove("new_"+self.uplink_sbd_name)
                        else:
                            # Mark message as unseen again if it wasn't addressed to this satellite
                            self.mail.store(num, '-FLAGS', '\SEEN')

        return True

    def disconnect(self):
        '''Quits the program and stores message log and field telemetry to file.'''

        print(f' - Terminating console connection to and saving logging/telemetry data for {self.device_name}.')

        # End threads
        self.running_logger = False
        self.check_msgs_thread.join()
        self.scrape = False
        self.scrape_uplinks_thread.join()
        self.console.close()
        self.dp_console.close()

        self.datastore.stop()
        self.logger.stop()
        self.raw_logger.stop()

        print(f' - Finished terminating for {self.device_name}.')

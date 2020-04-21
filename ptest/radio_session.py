import time
import datetime
import serial
import threading
import json
import traceback
import queue
import yagmail
import requests
import subprocess
import glob
import os
import pty

from .data_consumers import Datastore, Logger

class RadioSession(object):
    '''
    Represents a connection session with a Flight Computer's Quake radio.

    This class is used by the simulation software and user command prompt to read and write to a
    flight computer's Quake radio. The connection is fundamentally stateless, since the state of the
    connection (connected or disconnected) is managed by Iridium.

    TODO this class needs to be thread-safe. Protect uplinks via a lock, and protect data sharing
    between the check_for_downlink and the read_state functions.
    '''


    def __init__(self, device_name, port, imei, simulation_run_dir, tlm_config, downlink_parser_filepath):
        '''
        Initializes state session with the Quake radio.
        '''

        # Device connection
        self.device_name = device_name
        self.imei=imei

        #Flask server connection
        self.flask_server=tlm_config["server"]
        self.flask_port=tlm_config["port"]

        #email
        self.username=tlm_config["email_username"]
        self.password=tlm_config["email_password"]

        #start downlink parser
        master_fd, slave_fd = pty.openpty()
        self.downlink_parser = subprocess.Popen([downlink_parser_filepath], stdin=master_fd, stdout=master_fd)
        self.console = serial.Serial(os.ttyname(slave_fd), 9600, timeout=1)
        self.telem_save_dir = simulation_run_dir

    def read_state(self, field, timeout=None):
        '''
        Read state by posting a request for data to the Flask server
        '''
        headers = {
            'Accept': 'text/html',
        }
        payload = {
            "index" : "statefield_report_"+str(self.imei),
            "field" : str(field)
        }

        response = requests.get('http://'+self.flask_server+':'+self.flask_port+'/search-es', params=payload, headers=headers)
        return response.text

    def write_multiple_states(self, fields, vals, timeout=None):
        '''
        Uplink multiple state variables. Return success of write.
        Reads from the most recent Iridium Report whether or
        not RadioSession is able to send uplinks
        '''
        assert len(fields) == len(vals)

        headers = {
            'Accept': 'text/html',
        }
        payload = {
            "index" : "iridium_report_"+str(self.imei),
            "field" : "send-uplinks"
        }

        response = requests.get('http://'+self.flask_server+':'+self.flask_port+'/search-es', params=payload, headers=headers)

        if response.text=="True":
            #create dictionary object with new fields and vals
            updated_fields={}
            for i in range(len(fields)):
                updated_fields[fields[i]]=vals[i]

            #connect to PAN email account
            yag = yagmail.SMTP(self.username, self.password)

            #create a JSON file with the updated statefields and send it to the iridium email
            with open('uplink.json', 'w') as json_uplink:
                json.dump(updated_fields, json_uplink)
            yag.send('sbdservice@sbd.iridium.com', self.imei, 'uplink.json')

            return True
        else:
            return False

    def write_state(self, field, val, timeout=None):
        '''
        Uplink one state variable. Return success of write.
        '''
        return self.write_multiple_states([field], [val], timeout)

    def parsetelem(self):
        #get newest file
        telem_files = glob.iglob(os.path.join(self.telem_save_dir, 'telem*'))
        try:
            newest_telem_file = max(telem_files, key=os.path.basename)
        except ValueError:
            return "No telemetry to parse."
        self.console.write((newest_telem_file+"\n").encode())
        telem_json_data = json.loads(self.console.readline().rstrip())
        if telem_json_data is not None:
                telem_json_data = telem_json_data["data"]
        return telem_json_data

    def disconnect(self):
        '''Quits the Quake connection, and stores message log and field telemetry to file.'''

        print(
            f' - Terminating console connection to and saving logging/telemetry data for radio connection to {self.device_name}.'
        )

        # End threads if there was actually a connection to the radio
        self.console.close()

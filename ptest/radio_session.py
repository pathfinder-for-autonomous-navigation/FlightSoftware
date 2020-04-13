import time
import datetime
from datetime import datetime
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
from elasticsearch import Elasticsearch


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


    def __init__(self, device_name, imei, simulation_run_dir, radio_keys_config, flask_keys_config, downlink_parser_filepath):
        '''
        Initializes state session with the Quake radio.

        Args:
        device_name: Name of device being connected to
        datastore: Datastore to which telemetry data will be published
        logger: Logger to which log lines should be committed
        '''

        # Device connection
        self.device_name = device_name
        self.imei=imei

        #Flask server connection
        self.flask_server=flask_keys_config["server"]
        self.flask_port=flask_keys_config["port"]

        # Data logging
        self.datastore = Datastore(device_name, simulation_run_dir)
        self.logger = Logger(device_name, simulation_run_dir)

        #email
        self.username=radio_keys_config["email_username"]
        self.password=radio_keys_config["email_password"]

        #start downlink parser
        master_fd, slave_fd = pty.openpty()
        self.downlink_parser = subprocess.Popen([downlink_parser_filepath], stdin=master_fd, stdout=master_fd)
        self.console = serial.Serial(os.ttyname(slave_fd), 9600, timeout=1)
        self.telem_save_dir = simulation_run_dir

        # Get keys for connecting to email account and elasticsearch server
        try:
            with open(os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../ptest/configs/radio_keys.json')))as radio_keys_config_file:
                radio_keys_config = json.load(radio_keys_config_file)
            with open(os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../ptest/configs/server_keys.json')))as server_keys_config_file:
                server_keys_config = json.load(server_keys_config_file)
        except json.JSONDecodeError:
            print("Could not load config files. Exiting.")
            sys.exit(1)
        except KeyError:
            print("Malformed config file. Exiting.")
            sys.exit(1)
        # Open a connection to elasticsearch
        es_server = server_keys_config["server"]
        es_port = server_keys_config["port"]
        self.es = Elasticsearch([{'host':es_server,'port':es_port}])

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
        self.logger.put("Send Uplinks: "+str(response.text))

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
            self.logger.put("Wait for confirmation MTMSN")
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
    
    def dbtelem(self):
        jsonObj = self.parsetelem()
        if not isinstance(jsonObj, dict):
            print(jsonObj)
            return False
        failed = False
        print(jsonObj)
        for field in jsonObj:
            value = jsonObj[field]
            data=json.dumps({
            field: value,
            "time": str(datetime.now().isoformat())
            })
            res = self.es.index(index='statefield_report_'+str(self.imei), doc_type='report', body=data)
            if not res['result'] == 'created':
                failed = True
        return not failed 

    def disconnect(self):
        '''Quits the Quake connection, and stores message log and field telemetry to file.'''

        print(
            f' - Terminating console connection to and saving logging/telemetry data for radio connection to {self.device_name}.'
        )

        # End threads if there was actually a connection to the radio
        self.datastore.stop()
        self.logger.stop()
        self.running_logger = False
        self.console.close()

import time
import datetime
from datetime import datetime
import serial
import threading
import json
import traceback
import queue
import requests
import subprocess
import glob
import os
import pty

from .data_consumers import Datastore, Logger
from .http_cmd import create_radio_session_endpoint

class RadioSession(object):
    '''
    Represents a connection session with a Flight Computer's Quake radio.

    This class is used by the simulation software and user command prompt to read and write to a
    flight computer's Quake radio. The connection is fundamentally stateless, since the state of the
    connection (connected or disconnected) is managed by Iridium.

    TODO this class needs to be thread-safe. Protect uplinks via a lock, and protect data sharing
    between the check_for_downlink and the read_state functions.
    '''

    def __init__(self, device_name, imei, uplink_console, port, send_queue_duration,
                    send_lockout_duration, simulation_run_dir, tlm_config):
        '''
        Initializes state session with the Quake radio.
        '''

        # Device connection
        self.device_name = device_name
        self.imei=imei
        self.port=port
        self.flask_app=create_radio_session_endpoint(self)

        try:
            self.http_thread = Process(name=f"{self.device_name} HTTP Command Endpoint", target=self.flask_app.run, kwargs={"port": self.port})
            self.http_thread.start()
            print(f"{self.device_name} HTTP command endpoint is running at http://localhost:{self.port}")
        except:
            print(f"Unable to start {self.device_name} HTTP command endpoint at http://localhost:{self.port}")

        self.send_queue_duration = send_queue_duration
        self.send_lockout_duration = send_lockout_duration
        if send_lockout_duration > send_queue_duration:
            # TODO shift this logic down into write_state.
            print("Error: send_lockout_duration is greater than send_queue_duration.")

        # Uplink console
        self.uplink_console = uplink_console

        #Flask server connection
        self.flask_server=tlm_config["webservice"]["server"]
        self.flask_port=tlm_config["webservice"]["port"]

        #email
        self.username=tlm_config["email_username"]
        self.password=tlm_config["email_password"]

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

        tlm_service_active = self.flask_server != ""
        if tlm_service_active:
            response = requests.get(
                'http://'+self.flask_server+':'+self.flask_port+'/search-es',
                    params=payload, headers=headers)

        if not tlm_service_active or response.text=="True":
            #create dictionary object with new fields and vals
            updated_fields={}
            for i in range(len(fields)):
                updated_fields[fields[i]]=vals[i]

            created_uplink = self.uplink_console.create_uplink(fields, vals, "uplink.sbd")
            if not created_uplink: return False
            os.system("./ptest/send_uplink uplink.sbd")
            os.remove("uplink.sbd") 
            os.remove("uplink.json") 
            return True
        else:
            return False

    def write_state(self, field, val, timeout=None):
        '''
        Uplink one state variable. Return success of write.
        '''
        return self.write_multiple_states([field], [val], timeout) 

    def disconnect(self):
        '''Quits the Quake connection, and stores message log and field telemetry to file.'''

        print(
            f' - Terminating console connection to and saving logging/telemetry data for radio connection to {self.device_name}.'
        )

        self.http_thread.terminate()
        self.http_thread.join()

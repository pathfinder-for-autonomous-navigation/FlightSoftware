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
from multiprocessing import Process, Queue
from elasticsearch import Elasticsearch

from .data_consumers import Datastore, Logger
from .http_cmd import create_radio_session_endpoint
from tlm.oauth2 import *
from .uplink_timer import UplinkTimer

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

        # Uplink timer
        self.timer = UplinkTimer(send_queue_duration, self.send_uplink)

        # Radio session and the http endpoints communicate information about the state of the timer
        # by passing messages over the queue.
        q = Queue()
        self.check_queue_msgs = True
        self.check_queue_thread = threading.Thread(target=self.check_queue, args=(q,))
        self.check_queue_thread.start()

        # HTTP Endpoints
        self.flask_app=create_radio_session_endpoint(self, q)
        self.flask_app.config["uplink_console"] = uplink_console
        self.flask_app.config["imei"] = imei

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

    def check_queue(self, queue):
        '''
        Continuously reads and carries out requests
        from the HTTP endpoints.
        '''
        while self.check_queue_msgs:
            msg = queue.get()

            if msg == "time":
                time_left = self.timer.time_left()
                queue.put(str(time_left))

            elif msg == "pause":
                if not self.timer.is_alive():
                    queue.put("Timer not running")
                elif self.timer.run_time() < self.send_queue_duration-self.send_lockout_duration:
                    if self.timer.pause():
                        queue.put("Paused timer")
                    else:
                        queue.put("Unable to pause timer")
                else:
                    queue.put("Unable to pause timer")

            elif msg == "resume":
                if self.timer.is_alive():
                    queue.put("Timer already running")
                elif self.timer.resume():
                    queue.put("Resumed timer")
                else:
                    queue.put("Unable to resume timer")
            
            elif msg == "view":
                if not os.path.exists("uplink.json"):
                    return "No queued uplink"
                with open('uplink.json', 'r') as telem_file:
                    queued_uplink = json.load(telem_file)
                queue.put(queued_uplink)

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

        response = requests.get('http://'+self.flask_server+':'+str(self.flask_port)+'/search-es', params=payload, headers=headers)
        return response.text

    def write_multiple_states(self, fields, vals, timeout=None):
        '''
        Uplink multiple state variables. Return success of write.
        Reads from the most recent Iridium Report whether or
        not RadioSession is able to send uplinks
     	'''

        assert len(fields) == len(vals)

        if self.uplink_queued():
            return False

        # Create dictionary object with new fields and vals
        updated_fields={}
        for i in range(len(fields)):
            updated_fields[fields[i]]=vals[i]

        # Create a JSON file to hold the uplink
        with open('uplink.json', 'w') as telem_file:
            json.dump(updated_fields, telem_file)

        # Start the timer. Timer will send uplink once after waiting for the
        # configured send queue duration.
        t = threading.Thread(target=self.timer.start)
        t.start()

        return True

    def write_state(self, field, val, timeout=None):
        '''
        Uplink one state variable. Return success of write.
        '''
        return self.write_multiple_states([field], [val], timeout) 

    def uplink_queued(self):
        '''
        Check if an uplink is currently queued to be sent by Iridium
        (i.e. if the most recently sent uplink was confirmed to be 
        received by the spacecraft). Can be used by ptest to determine
        whether or not to send an uplink autonomously.
        '''
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
                'http://'+self.flask_server+':'+str(self.flask_port)+'/search-es',
                    params=payload, headers=headers)

        if tlm_service_active and response.text.lower()=="true": 
            return False
        return True

    def send_uplink(self):
        if not os.path.exists("uplink.json"):
            return False

        # Extract the json telemetry data from the queued uplink json file
        with open("uplink.json", 'r') as uplink:
            queued_uplink = json.load(uplink)

        # Get an updated list of the field and values 
        fields, vals=queued_uplink.keys(), queued_uplink.values()
        
        # Create an uplink packet
        success = self.uplink_console.create_uplink(fields, vals, "uplink.sbd") and os.path.exists("uplink.sbd")

        if success:
            # Send the uplink to Iridium
            to = "data@sbd.iridium.com"
            sender = "pan.ssds.qlocate@gmail.com"
            subject = self.imei
            msgHtml = ""
            msgPlain = ""
            SendMessage(sender, to, subject, msgHtml, msgPlain, 'uplink.sbd')

            # Remove uplink files/cleanup
            os.remove("uplink.sbd") 
            os.remove("uplink.json")

            return True
        else:
            os.remove("uplink.json")
            return False

    def disconnect(self):
        '''Quits the Quake connection, and stores message log and field telemetry to file.'''

        print(
            f' - Terminating console connection to and saving logging/telemetry data for radio connection to {self.device_name}.'
        )

        self.check_queue_msgs = False
        self.check_queue_thread.join()
        self.http_thread.terminate()
        self.http_thread.join()

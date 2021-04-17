import threading
import json
import traceback
import queue
import requests
import os
from multiprocessing import Process, Queue
from elasticsearch import Elasticsearch

from .http_cmd import create_radio_session_endpoint
from tlm.oauth2 import *
from .uplink_timer import UplinkTimer
from .cases.utils import str_to_val

import email
import imaplib

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
                    send_lockout_duration, check_uplink_queue_enable, simulation_run_dir, tlm_config):
        '''
        Initializes state session with the Quake radio.
        '''

        # Device connection
        self.device_name = device_name
        self.imei=imei
        self.port=port

        self.username=tlm_config["email_username"]
        self.password=tlm_config["email_password"]

        # Uplink timer
        self.timer = UplinkTimer(send_queue_duration, self.send_uplink)
        self.check_uplink_queue_enable = check_uplink_queue_enable

        # Radio session and the http endpoints communicate information about the state of the timer
        # by passing messages over the queue.
        q = Queue()
        self.check_queue_msgs = True
        self.check_queue_thread = threading.Thread(target=self.check_queue, args=(q,), name="Uplink queue check thread")
        self.check_queue_thread.start()

        # dictionary of statefields and values to uplink
        self.statefield_dict = {}


        # HTTP Endpoints
        self.flask_app=create_radio_session_endpoint(self, q)
        self.flask_app.config["uplink_console"] = uplink_console
        self.flask_app.config["imei"] = imei

        try:
            self.http_thread = Process(name=f"{self.device_name} HTTP Command Endpoint", target=self.flask_app.run, kwargs={'host':'0.0.0.0', "port":self.port})
            self.http_thread.start()
            print(f"{self.device_name} HTTP command endpoint is running at http://0.0.0.0:{self.port}")
        except:
            print(f"Unable to start {self.device_name} HTTP command endpoint at http://0.0.0.0:{self.port}")
            traceback.print_exc()

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

        # Connect to email
        try:
            self.mail = imaplib.IMAP4_SSL("imap.gmail.com", 993)
            self.mail.login(self.username, self.password)
            self.mail.select('"[Gmail]/Sent Mail"')
        except Exception as e:
            print("Not connected to email")

        if os.path.exists("uplink"+self.imei+".json"):
            os.remove("uplink"+self.imei+".json")
        if os.path.exists("http_uplink"+self.imei+".json"):
            os.remove("http_uplink"+self.imei+".json")

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
                if not os.path.exists("uplink"+self.imei+".json"):
                    queue.put("No queued uplink")
                else:
                    with open('uplink'+self.imei+'.json', 'r') as telem_file:
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
        if self.uplink_queued() and self.check_uplink_queue_enable:
            return False

        assert len(fields) == len(vals)

        for i in range(len(fields)):
            self.statefield_dict[fields[i]] = str_to_val(vals[i]) if type(vals[i])==str else vals[i]

        # Start the timer. Timer will send uplink once after waiting for the
        # configured send queue duration.
        # make the uplink.json file so we know there is data to send and http_cmd
        # can see it as well
        if not os.path.exists("uplink"+self.imei+".json"):   
            with open('uplink'+self.imei+'.json', 'w'): pass 
            t = threading.Thread(target=self.timer.start, name="Uplink timer thread")
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
        # Check there is an uplink to send
        if not os.path.exists("uplink"+self.imei+".json"):
            return False

        if os.path.exists("http_uplink"+self.imei+".json"):
            # Extract the json telemetry data from the queued http uplink json file
            with open("http_uplink"+self.imei+".json", 'r') as http_uplink:
                queued_http_uplink = json.load(http_uplink)

            # Get an updated list of the field and values from http endpoint and update the dictionary
            for field in queued_http_uplink:
                self.statefield_dict[field] = queued_http_uplink[field]

        #merged and updated fields to send
        fields, vals = self.statefield_dict.keys(), self.statefield_dict.values()
        with open('uplink'+self.imei+'.json', 'w') as telem_file:
                    json.dump(self.statefield_dict, telem_file)

        # Create an uplink packet
        success = self.uplink_console.create_uplink(fields, vals, "uplink"+self.imei+".sbd", "uplink"+self.imei+".json") and os.path.exists("uplink"+self.imei+".sbd")

        if success:
            # Send the uplink to Iridium
            to = "pan.ssds.qlocate@gmail.com"
            sender = "pan.ssds.qlocate@gmail.com"
            subject = self.imei
            msgHtml = ""
            msgPlain = ""
            SendMessage(sender, to, subject, msgHtml, msgPlain, 'uplink'+self.imei+'.sbd')

            # Remove uplink files/cleanup
            os.remove("uplink"+self.imei+".sbd")
            os.remove("uplink"+self.imei+".json")
            if os.path.exists("http_uplink"+self.imei+".json"):
                os.remove("http_uplink"+self.imei+".json")
            self.statefield_dict = {}
            return True
        else:
            os.remove("uplink"+self.imei+".json")
            if os.path.exists("http_uplink"+self.imei+".json"):
                os.remove("http_uplink"+self.imei+".json")
            self.statefield_dict = {}
            return False

    def mark_message_unseen(self):
        '''
        Mark most recent message in sent mail box and UNSEEN
        '''
        try:
            self.mail = imaplib.IMAP4_SSL("imap.gmail.com", 993)
            self.mail.login(self.username, self.password)
            self.mail.select('"[Gmail]/Sent Mail"')
        except Exception as e:
            print(e)
        _, data = self.mail.search(None, '(FROM "pan.ssds.qlocate@gmail.com")')
        mail_ids = data[0]
        id_list = mail_ids.split()
        last_id = id_list[len(id_list)-1]
        
        _, data = self.mail.fetch(last_id, "(RFC822)")

        for response_part in data:
            if isinstance(response_part, tuple):
                # converts message from byte literal to string removing b''
                msg = email.message_from_string(response_part[1].decode('utf-8'))
                email_subject = msg['subject']

                if email_subject.isdigit() and int(self.imei) == int(email_subject):
                    # mark message as unseen
                    self.mail.store(last_id, '-FLAGS', '\SEEN')
        

    def disconnect(self):
        '''Quits the Quake connection, and stores message log and field telemetry to file.'''

        print(
            f' - Terminating console connection to and saving logging/telemetry data for radio connection to {self.device_name}.'
        )

        self.check_queue_msgs = False
        self.check_queue_thread.join()
        self.http_thread.terminate()
        self.http_thread.join()

from flask import Flask, request, jsonify
from flasgger import Swagger
from flasgger.utils import swag_from
from elasticsearch import Elasticsearch
from datetime import datetime
import threading
import json
import imaplib
import base64
import os
import email
import time
import logging
import subprocess
import pty
import serial


class read_iridium(object):
    def __init__(self, radio_keys_config, elastic_search):
        # Connection to elasticsearch
        self.es=elastic_search

        #pan email
        self.username=radio_keys_config["email_username"]
        self.password=radio_keys_config["email_password"]

        #updates MOMSN and MTMSN numbers sent/recieved
        self.momsn=-1
        self.mtmsn=-1
        self.confirmation_mtmsn=-1

        #send_uplinks, keeps track of if the ground can send more uplinks. This allows us to make sure we are only sending one uplink at a time.
        self.send_uplinks=False
        self.recieved_uplink_confirmation=False

        #imei number of the radio that sent the most recent email
        self.imei=-1

        #connect to email
        self.mail = imaplib.IMAP4_SSL("imap.gmail.com", 993)
        self.mail.login(self.username, self.password)
        self.mail.select('Inbox')

        #thread
        self.run_email_thread = False

        #set up downlink parser
        filepath = os.path.dirname(os.path.abspath(__file__))
        binary_dir = os.path.join(filepath, "../../.pio/build/gsw_downlink_parser/program")
        master_fd, slave_fd = pty.openpty()
        self.downlink_parser = subprocess.Popen([binary_dir], stdin=master_fd, stdout=master_fd)
        self.console = serial.Serial(os.ttyname(slave_fd), 9600, timeout=1)

    def connect(self):
        '''
        Starts a thread which will continuously
        check the PAN email and post reports to 
        elasticsearch 
        '''
        self.check_email_thread = threading.Thread(target=self.post_to_es)
        self.run_email_thread = True
        self.check_email_thread.start()

    def is_json(self, payload):
        '''
        Returns whether or not an object is a valid JSON string
        '''
        try:
            json_object = json.loads(str(payload.decode('utf8').rstrip("\x00")))
        except ValueError as e:
            return False
        return True

    def process_downlink_packet(self, payload):
        '''
        Converts the email attachment payload into a 
        JSON object. This is because elasticsearch
        only takes in JSON data. If the payload doesn't 
        already contain a valid JSON string, then it runs 
        DownlinkParser
        '''
        if self.is_json(payload):
            # If the attachment is a json string, then return the json
            # as a python dictionary
            data=json.loads(str(payload.decode('utf8').rstrip("\x00")))
            data["time"]=str(datetime.now().isoformat())
        else:
            # Run downlink parser and return json
            f=open("data.sbd", "wb")
            f.write(payload)
            f.close()

            self.console.write(("data.sbd\n").encode())
            data = json.loads(self.console.readline().rstrip())
            if data is not None:
                data=data["data"]
                data["time"]=str(datetime.now().isoformat())
            os.remove("data.sbd")

        return data

    def check_for_email(self):
        '''
        Reads the most recent unread email from the Iridium email account. 
        If it is a downlink, the function returns a statefield report. If it is an uplink 
        confirmation, the function records that we recieved an uplink confirmation and returns
        None. If there is neither a downlink or an uplink, the function returns None.
        If either a downlink or an uplink is recieved, the function will update the most recent MOMSN 
        and MTMSN numbers and check whether or not radioSession can send uplinks.
        '''
        #look for all new emails from iridium
        self.mail.select('Inbox')
        _, data = self.mail.search(None, '(FROM "sbdservice@sbd.iridium.com")', '(UNSEEN)')
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
                    msg = email.message_from_string(response_part[1].decode('utf-8'))
                    email_subject = msg['subject']

                    #handles uplink confirmations
                    if email_subject.find("SBD Mobile Terminated Message Queued for Unit: ")==0:
                        # Get imei number of the radio that recieved the uplink
                        self.imei=int(email_subject[47:])

                        for part in msg.walk():
                                            
                            if part.get_content_maintype() == 'multipart':
                                continue

                            if part.get('Content-Disposition') is None:
                                continue

                            #get the body text of the email and look for the MOMSN/MTMSN number
                            if part.get_content_type() == "text/plain":
                                email_body = part.get_payload(decode=True).decode('utf8')
                                for line in email_body.splitlines():
                                    if line.find("MTMSN")!=-1:
                                        self.mtmsn=int(line[line.find("MTMSN")+9:line.find("MTMSN")+11])

                            #Set whether or not RadioSession can send uplinks
                            if self.confirmation_mtmsn != self.mtmsn:
                                #stop radio session from sending any more uplinks
                                self.send_uplinks=False
                            else:
                                #allow radio session to send more uplinks
                                self.send_uplinks=True
                                    
                        # Record that we just recieved an uplink confirmation
                        self.recieved_uplink_confirmation=True
                        
                    else:
                        # Record that we did not recieve an uplink confirmation
                        self.recieved_uplink_confirmation=False

                    # Handles downlinks
                    if email_subject.find("SBD Msg From Unit:")==0:
                        # Get imei number of the radio that sent the downlink
                        self.imei=int(email_subject[19:])

                        # Go through the email contents
                        for part in msg.walk():
                                        
                            if part.get_content_maintype() == 'multipart':
                                continue

                            if part.get('Content-Disposition') is None:
                                continue
                                            
                            # Get the body text of the email and look for the MOMSN/MTMSN number
                            if part.get_content_type() == "text/plain":
                                email_body = part.get_payload(decode=True).decode('utf8')
                                for line in email_body.splitlines():
                                    if line.find("MOMSN")!=-1:
                                        self.momsn=int(line[7:])
                                    if line.find("MTMSN")!=-1:
                                        self.confirmation_mtmsn=int(line[7:])
                            
                            #Set whether or not radioSession can send uplinks
                            if self.confirmation_mtmsn != self.mtmsn:
                                #stop radio session from sending any more uplinks
                                self.send_uplinks=False
                            else:
                                #allow radio session to send more uplinks
                                self.send_uplinks=True

                            # Check if there is an email attachment
                            if part.get_filename() is not None:
                                # Get data from email attachment
                                statefield_report=self.process_downlink_packet(part.get_payload(decode=True))
                                return statefield_report
                        
                    #if we have not recieved a downlink, return None
                    return None

    def create_iridium_report(self):
        '''
        Creates and indexes an Iridium Report
        '''
        # Create an iridium report 
        ir_report=json.dumps({
            "momsn":self.momsn,
            "mtmsn":self.mtmsn, 
            "confirmation-mtmsn": self.confirmation_mtmsn,
            "send-uplinks": self.send_uplinks,
            "time": str(datetime.now().isoformat())
        })

        # Index iridium report in elasticsearch
        iridium_res = self.es.index(index='iridium_report_'+str(self.imei), doc_type='report', body=ir_report)
        # Print whether or not indexing was successful
        print("Iridium Report Status: "+iridium_res['result']+"\n\n")

    def post_to_es(self):
        '''
        Check for the most recent email from iridium. 
        There are two conditions under which we would create an Iridium report: 
        1) When we recieve a statefield report
        2) when we recieve an uplink confirmation.
        - If there is a statefield report, index the statefield report and 
        create and index an iridium report.
        - If there is not a statefield report but we recently recieved an uplink
        confirmation, then create and index an iridium report.
        '''

        while self.run_email_thread==True:
            #get the most recent statefield report
            sf_report=readIr.check_for_email()

            if sf_report is not None:
                # Print the statefield report recieved
                print("Got report: "+str(sf_report)+"\n")

                # Index statefield report in elasticsearch
                statefield_res = self.es.index(index='statefield_report_'+str(self.imei), doc_type='report', body=sf_report)
                # Print whether or not indexing was successful
                print("Statefield Report Status: "+statefield_res['result'])

                self.create_iridium_report()

            elif self.recieved_uplink_confirmation:
                # Create an iridium report and add that to the iridium_report index in elasticsearch. 
                self.create_iridium_report()
                # Record that we have not recently recieved any uplinks as we just indexed the most recent one
                self.recieved_uplink_confirmation=False

            time.sleep(10)

    def disconnect(self):
        '''
        Stops the thread which is checking emails
        and posting reports to elasticsearch
        '''
        self.run_email_thread=False
        self.check_email_thread.join()

# Get keys for connecting to email account and elasticsearch server
try:
    with open(os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../ptest/configs/radio_keys.json')))as radio_keys_config_file:
        radio_keys_config = json.load(radio_keys_config_file)
    with open(os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../ptest/configs/server_keys.json')))as server_keys_config_file:
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
es = Elasticsearch([{'host':es_server,'port':es_port}])

# Create a read_iridium object
readIr = read_iridium(radio_keys_config, es)
# Start checking emails and posting reports
readIr.connect()

app = Flask(__name__)
app.config["SWAGGER"]={"title": "PAN Ground Software", "uiversion": 2}

# Set up the SwaggerUI API
swagger_config={
    "headers":[],
    "specs":[
        {
            "endpoint":"apispec_1",
            "route":"/apispec_1.json",
            "rule_filter":lambda rule:True,
            "model_filter":lambda tag:True
        }
    ],
    "static_url_path": "/flassger_static",
    "swagger_ui":True,
    "specs_route":"/swagger/"
}
swagger=Swagger(app, config=swagger_config)

# Endpoint for indexing data in elasticsearch
# Mostly for testing purposes. We don't use this to actually index data in elasticsearch
@app.route("/telemetry", methods=["POST"])
@swag_from("endpoint_configs/telemetry_config.yml")
def index_sf_report():
    sf_report=request.get_json()
    imei=sf_report["imei"]
    data=json.dumps({
        sf_report["field"]: sf_report["value"],
        "time": str(datetime.now().isoformat())
    })

    #index statefield report in elasticsearch
    sf_res = es.index(index='statefield_report_'+str(imei), doc_type='report', body=data)
    res={
        "Report Status": sf_res['result'],
        "Report": json.loads(data),
        "Index": 'statefield_report_'+str(imei)
    }
    return res

# Endpoint for getting data from ElasticSearch
@app.route("/search-es", methods=["GET"])
@swag_from("endpoint_configs/search_es_config.yml")
def search_es():
    index = request.args.get('index')
    field = str(request.args.get('field'))

    # Get the most recent document in the given index which has a given statefield in it
    search_object={
        'query': {
            'exists': {
                'field': field
            }
        },
        "sort": [
            {
                "time": {
                    "order": "desc"
                }
            }
        ],
        "size": 1
    }
    # Get the value of that field from the document
    if es.indices.exists(index=index):
        res = es.search(index=index, body=json.dumps(search_object))
        if len(res["hits"]["hits"])!=0:
            most_recent_field=res["hits"]["hits"][0]["_source"][field]
            return str(most_recent_field)
        else:
            return f"Unable to find field: {field} in index: {index}"
    else:
        return f"Unable to find index: {index}"

if __name__ == "__main__":
    app.run(debug=True, host='0.0.0.0')
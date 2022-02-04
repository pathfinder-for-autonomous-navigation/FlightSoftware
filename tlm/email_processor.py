from datetime import datetime
import threading
import json
import imaplib
import os
import email
import email.utils
import time
import subprocess
import pty
import serial
import shutil
from .oauth2 import *
from ptest.uplink_console import UplinkConsole


LEADER = 0
FOLLOWER = 1
TLM_RUN_DIR = 'tlm/'


class IridiumEmailProcessor(object):
    def __init__(self, radio_keys_config, elasticsearch, downlink_parser_path):
        # Connection to elasticsearch
        self.es=elasticsearch

        #pan email
        self.username=radio_keys_config["email_username"]
        self.password=radio_keys_config["email_password"]

        self.leader_imei=radio_keys_config["leader"]
        self.follower_imei=radio_keys_config["follower"]

        #updates MOMSN and MTMSN numbers sent/recieved
        self.momsn=-1
        self.mtmsn=-1
        self.confirmation_mtmsn=-1

        #send_uplinks, keeps track of if the ground can send more uplinks. This allows us to make sure we are only sending one uplink at a time.
        self.send_uplinks=True
        self.recieved_uplink_confirmation=False

        #connect to email
        self.authentication = authenticate()
        self.mail = imaplib.IMAP4_SSL("imap.gmail.com", 993)
        self.mail.login(self.username, self.password)
        self.mail.select('Inbox')

        #thread
        self.run_email_thread = False

        #set up downlink parser
        master_fd, slave_fd = pty.openpty()
        self.downlink_parser = subprocess.Popen([downlink_parser_path], stdin=master_fd, stdout=master_fd)
        self.console = serial.Serial(os.ttyname(slave_fd), 9600, timeout=1)

        self.uplink_console = UplinkConsole('.') # open a new uplink console in the current directory\
        self.enable_leader_goto = True
        self.enable_follower_goto = True

    def connect(self):
        '''
        Starts a thread which will continuously
        check the PAN email and post reports to 
        elasticsearch 
        '''
        self.check_email_thread = threading.Thread(target=self.post_to_es)
        self.run_email_thread = True
        self.check_email_thread.start()
        print("Monitoring GMAIL for Iridium Messages...")

    def is_json(self, payload):
        '''
        Returns whether or not an object is a valid JSON string
        '''
        try:
            json_object = json.loads(str(payload.decode('utf8').rstrip("\x00")))
        except ValueError as e:
            return False
        return True

    def process_downlink_packet(self, payload, downlink_time = None):
        '''
        Converts the email attachment payload into a 
        JSON object. This is because elasticsearch
        only takes in JSON data. If the payload doesn't 
        already contain a valid JSON string, then it runs 
        DownlinkParser
        
        args
            payload
            downlink_time:
                optional time to specify when the downlink was recieved.
                formatted utc z time.
        '''
        
        if downlink_time is None:
            downlink_time = str(datetime.utcnow().isoformat())[:-3]+'Z'

        if self.is_json(payload):
            # If the attachment is a json string, then return the json
            # as a python dictionary
            data=json.loads(str(payload.decode('utf8').rstrip("\x00")))
            data["time.downlink_received"] = downlink_time
        else:
            # Run downlink parser and return json
            f=open("data.sbd", "wb")
            f.write(payload)
            f.close()
            self.console.write(("data.sbd\n").encode())
            console_read = self.console.readline().rstrip()
            data = json.loads(console_read)
            if data is not None:
                try:
                    data = data["data"]
                    data["time.downlink_received"] = downlink_time
                except:
                    # take the except branch when attachment is a first packet
                    data = None
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
        try:
            self.mail.select('Inbox')
            _, data = self.mail.search(None, '(FROM "sbdservice@sbd.iridium.com")', '(UNSEEN)')
        except Exception as e:
            # catch exception and try again shortly.
            print(e)
            return None, None # return None, None to signal error and try again

        # _, data = self.mail.search(None, '(FROM "pan.ssds.qlocate@gmail.com")', '(UNSEEN)') # for testing

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
                        imei=int(email_subject[47:])

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
                                        rest_of_mtmsn_line = line[line.find("MTMSN")+9:]
                                        mtmsn_str = rest_of_mtmsn_line.split(",")[0]
                                        self.mtmsn=int(mtmsn_str)
                                        print(self.mtmsn)

                            self.set_send_uplinks()

                        # Record that we just recieved an uplink confirmation
                        self.recieved_uplink_confirmation=True
                        
                    else:
                        # Record that we did not recieve an uplink confirmation
                        self.recieved_uplink_confirmation=False

                    # Handles downlinks
                    if email_subject.find("SBD Msg From Unit:")==0:
                        # Get imei number of the radio that sent the downlink
                        imei=int(email_subject[19:])

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
                                        mtmsn = int(line[7:])
                                        if mtmsn != 0:
                                            self.confirmation_mtmsn = mtmsn
                                        self.mtmsn_down = mtmsn
                                    session_time_idx = line.find("Time of Session (UTC)")
                                    if session_time_idx!=-1:
                                        time_of_session = line[session_time_idx+23:]
                                        time_tuple = email.utils.parsedate(time_of_session)
                                        utc_time_tuple = tuple(list(time_tuple) + [0]) 
                                        time_stamp = email.utils.mktime_tz(utc_time_tuple)
                                        email_time = datetime.utcfromtimestamp(time_stamp)
                                        formatted_email_time = str(email_time.isoformat())+'Z'

                                        self.formatted_email_time = formatted_email_time

                            self.set_send_uplinks()

                            # Check if there is an email attachment
                            if part.get_filename() is not None:
                                # Get data from email attachment
                                attachment_payload = part.get_payload(decode=True)
                                statefield_report=self.process_downlink_packet(attachment_payload, downlink_time = self.formatted_email_time)
                                return (imei, statefield_report)
                        
                    #if we have not recieved a downlink, return None
                    return imei, None
        return None, None

    def set_send_uplinks(self):
        #Set whether or not radioSession can send uplinks

        if self.confirmation_mtmsn < self.mtmsn:
            #stop radio session from sending any more uplinks
            self.send_uplinks=False
        else:
            #allow radio session to send more uplinks
            self.send_uplinks=True

    def create_iridium_report(self, imei):
        '''
        Creates and indexes an Iridium Report
        '''
        # Create an iridium report 
        ir_report=json.dumps({
            "momsn":self.momsn,
            "mtmsn":self.mtmsn, 
            "confirmation-mtmsn": self.confirmation_mtmsn,
            "send-uplinks": self.send_uplinks,
            "time.downlink_received": str(datetime.utcnow().isoformat())[:-3]+'Z'
        })

        # Index iridium report in elasticsearch
        iridium_res = self.es.index(index='iridium_report_'+str(imei), doc_type='report', body=ir_report)
        # Print whether or not indexing was successful
        print("Iridium Report Status: "+iridium_res['result']+"\n\n")

    def get_go_to_command_name(self, role):
        '''Returns the string name of the proper go to command name'''
        file_ending = ".json"
    
        if role == LEADER:
            return "goto_leader" + file_ending
        if role == FOLLOWER:
            return "goto_follower" + file_ending

    def get_go_to_command_data(self, file_path):
        '''Load the go to command from local file directory. tlm/
        
        returns:
            json of the command'''
            
        with open(file_path, 'r') as json_file:
            data = json.load(json_file)
        return data
   
    def get_sbd_file_name(self, role):
        '''Get the name of the sbd file that should be created for the given role'''
        if role == LEADER:
            target_sbd_file_name = "goto_leader.sbd"
        else:
            target_sbd_file_name = "goto_follower.sbd"
    
        return target_sbd_file_name
    
    def send_sbd_file(self, role, sbd_file_name):
        '''Sends the sbd file to the iridium email'''
        
        if role == LEADER:
            target_imei = self.leader_imei
        else:
            target_imei = self.follower_imei
        
        # Send the uplink to Iridium
        to = "data@sbd.iridium.com"
        sender = "pan.ssds.qlocate@gmail.com"
        subject = target_imei
        msgHtml = ""
        msgPlain = ""

        # ### TEST CODE SECTION ###
        # to = "pan.ssds.qlocate@gmail.com"
        # sender = "pan.ssds.qlocate@gmail.com"
        # subject = "TESTING"
        # ### END TEST CODE SECTION ###

        SendMessage(sender, to, subject, msgHtml, msgPlain, sbd_file_name)

    def clean_up_files(self, files):
        # Remove uplink files/cleanup
        for file in files:
            if os.path.exists(file):
                os.remove(file)

    def comment_to_github(self, role, json_command_data):
        '''
        TODO
        '''
        pass

    def queue_go_to_command(self, role):
        '''With the given role, automatically queue the corresponding go to command for that satellite'''
        json_command_file_name = self.get_go_to_command_name(role)
        json_command_file_path = TLM_RUN_DIR + json_command_file_name
        json_command_data = self.get_go_to_command_data(json_command_file_path)
        list_of_fv_dicts = json_command_data
        fields = [x['field'] for x in list_of_fv_dicts]
        vals = [x['value'] for x in list_of_fv_dicts]

        temp_json_name = "automatic_command.json"
        sbd_file_name = self.get_sbd_file_name(role)

        success = self.uplink_console.create_uplink(fields, vals, sbd_file_name, temp_json_name) and os.path.exists(sbd_file_name)

        if success:
            print("Sending message: ")
            self.send_sbd_file(role, sbd_file_name)
            print(json_command_data)
            print("Successfully Sent Message")

            try:
                self.comment_to_github(role, json_command_data)
            except:
                print("Comment to github failed")

        else:
            print('Failed to generate uplink file')

        # attempt cleanup regardless
        files_to_delete = [sbd_file_name, temp_json_name]
        self.clean_up_files(files_to_delete)

    def is_goto_command_enabled(self, role):
        '''Returns whether or not goto commands are enabled for the given role
        
        TODO CONTROL FROM END POINTS
        '''
        if role == LEADER:
            return self.enable_leader_goto
        if role == FOLLOWER:
            return self.enable_follower_goto

    def handle_downlink_event_for_goto_commands(self, role):
        '''Handles the event for a downlink for goto commands'''
        if self.is_goto_command_enabled(role):
            self.queue_go_to_command(role)

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
            imei, sf_report=self.check_for_email()

            if sf_report is not None:
                if str(imei) == self.leader_imei:
                    downlink_path = 'es_routing/leader_jsons/DOWNLINK_MOMSN' + str(self.momsn) + '_MTMSN' + str(self.mtmsn_down) +'.json'
                    if not os.path.exists(downlink_path):
                        with open(downlink_path, 'x') as f:
                            json.dump(sf_report, f)

                elif str(imei) == self.follower_imei:
                    downlink_path = 'es_routing/follower_jsons/DOWNLINK_MOMSN' + str(self.momsn) + '_MTMSN' + str(self.mtmsn_down) +'.json'
                    if not os.path.exists(downlink_path):
                        with open(downlink_path, 'x') as f:
                            json.dump(sf_report, f)

                if str(imei) == self.leader_imei:
                    role = LEADER
                    self.handle_downlink_event_for_goto_commands(role)

                elif str(imei) == self.follower_imei:
                    role = FOLLOWER
                    self.handle_downlink_event_for_goto_commands(role)

                # Print the statefield report recieved
                print("Got report: "+str(sf_report)+"\n")

                # Index statefield report in elasticsearch
                statefield_res = self.es.index(index='statefield_report_'+str(imei), doc_type='report', body=sf_report)
                # Print whether or not indexing was successful
                print("Statefield Report Status: "+statefield_res['result'])

                self.create_iridium_report(imei)

            elif self.recieved_uplink_confirmation:
                # Create an iridium report and add that to the iridium_report index in elasticsearch. 
                self.create_iridium_report(imei)
                # Record that we have not recently recieved any uplinks as we just indexed the most recent one
                self.recieved_uplink_confirmation=False

            time.sleep(1)

    def disconnect(self):
        '''
        Stops the thread which is checking emails
        and posting reports to elasticsearch
        '''
        self.run_email_thread=False
        self.check_email_thread.join()


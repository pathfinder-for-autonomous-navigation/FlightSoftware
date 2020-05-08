from flask import Flask, request, jsonify
from flasgger import Swagger
from flasgger.utils import swag_from
from datetime import datetime
from argparse import ArgumentParser
import json, sys, logging
import os, threading
from .uplinkTimer import UplinkTimer
from tlm.oauth2 import *

"""
This file contains HTTP endpoint factories for StateSession and RadioSession.
The endpoints can be used by a command client to send uplinks to the satellites.
"""

# Disable startup logging
cli = sys.modules['flask.cli']
cli.show_server_banner = lambda *x: None

# Disable all logging
log = logging.getLogger('werkzeug')
log.disabled = True

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
    "static_url_path": "/flasgger_static",
    "swagger_ui":True,
    "specs_route":"/swagger/"
}

def create_radio_session_endpoint(radio_session):
    app = Flask(__name__)
    app.logger.disabled = True
    app.config["radio_session"] = radio_session

    app.config["SWAGGER"]={"title": "PAN Radio Session Command Endpoint", "uiversion": 2}
    swagger=Swagger(app, config=swagger_config)

    @app.route("/time", methods=["GET"])
    @swag_from("endpoint_configs/radio_session/time.yml")
    def get_time_left():
        timer = radio_session.timer
        if timer.is_alive():
            return "timer running"
            # return str(timer.time_left())
        return "Timer not running"

    @app.route("/pause", methods=["GET"])
    def pause_timer():
        timer = radio_session.timer
        send_queue_duration = app.config["send_queue_duration"]
        send_lockout_duration = app.config["send_lockout_duration"]
        if timer.is_alive():
            if timer.run_time()<send_queue_duration-send_lockout_duration:
                timer.pause()
                return "Paused"
            return "Unable to pause timer"
        return "No timer"

    @app.route("/resume", methods=["GET"])
    def resume_timer():
        timer = radio_session.timer
        if timer.is_alive():
            if timer.resume():
                return "Resumed"
            return "Unable to resume timer"
        return "No timer"


    @app.route("/send-telem", methods=["POST"])
    @swag_from("endpoint_configs/radio_session/request.yml")
    def send_telem():
        uplink=request.get_json()

        uplink_console = app.config["uplink_console"]
        imei = app.config["imei"]
        queued_uplink = app.config["queued_uplink"]

        if queued_uplink is None:
            # Get a list of the field and values 
            fields, vals=list(), list()
            for field_val in uplink:
                fields.append(field_val["field"])
                vals.append(field_val["value"])

            # Create a new uplink packet if there are no autonomous uplinks queued
            success = uplink_console.create_uplink(fields, vals, "uplink.sbd") and os.path.exists("uplink.sbd")

            if not success:
                return "Unable to send telemetry"

            # Send the uplink immediately to Iridium
            to = "fy56@cornell.edu" # data@sbd.iridium.com
            sender = "pan.ssds.qlocate@gmail.com"
            subject = imei
            SendMessage(sender, to, subject, "", "", 'uplink.sbd')

            # Remove uplink files/cleanup
            os.remove("uplink.sbd") 
            #os.remove("uplink.json")

            return "Successfully sent telemetry to Iridium"

        else:
            # Edit the queued uplink.
            queued_uplink.update(uplink)
            with open('uplink.json', 'w') as telem_file:
                json.dump(queued_uplink, telem_file)

            return "Successfully sent telemetry to radio session"

    return app

def create_state_session_endpoint(state_session):
    app = Flask(__name__)
    app.logger.disabled = True
    app.config["state_session"] = state_session

    app.config["SWAGGER"]={"title": "PAN State Session Command Endpoint", "uiversion": 2}
    swagger=Swagger(app, config=swagger_config)

    @app.route("/send-telem", methods=["POST"])
    @swag_from("endpoint_configs/state_session/request.yml")
    def send_telem():
        uplink=request.get_json()

        # Create an uplink packet
        fields, vals=list(), list()
        for field_val in uplink:
            fields.append(field_val["field"])
            vals.append(field_val["value"])

        uplink_console = app.config["uplink_console"]
        console = app.config["console"]
        success = uplink_console.create_uplink(fields, vals, "uplink.sbd") and os.path.exists("uplink.sbd")

        if not success:
            return "Unable to send telemetry"
        
        # Extract uplink data from created sbd file
        try:
            file = open("uplink.sbd", "rb")
        except:
            return "Unable to send telemetry"

        uplink_packet = file.read()
        uplink_packet_length = len(uplink_packet)
        file.close() 
        uplink_packet = str(''.join(r'\x'+hex(byte)[2:] for byte in uplink_packet)) #get the hex representation of the packet bytes

        # Send a command to the Flight Software console to process the uplink packet
        json_cmd = {
            'mode': ord('u'),
            'val': uplink_packet,
            'length': uplink_packet_length
        }
        json_cmd = json.dumps(json_cmd) + "\n"

        device_write_lock = threading.Lock()
        device_write_lock.acquire()
        console.write(json_cmd.encode())
        device_write_lock.release()

        # Get rid of uplink files/cleanup
        os.remove("uplink.sbd") 
        os.remove("uplink.json") 

        return "Successfully sent telemetry to State Session"

    return app
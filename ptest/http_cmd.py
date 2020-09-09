from flask import Flask, request, jsonify
from flasgger import Swagger
from flasgger.utils import swag_from
from datetime import datetime
from argparse import ArgumentParser
import json, sys, logging
import os, threading
from tlm.oauth2 import SendMessage

"""
This file contains HTTP endpoint factories for USBSession and RadioSession.
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

def create_radio_session_endpoint(radio_session, queue):
    app = Flask(__name__)
    app.logger.disabled = True
    app.config["radio_session"] = radio_session

    app.config["SWAGGER"]={"title": "PAN Radio Session Command Endpoint", "uiversion": 2}
    swagger=Swagger(app, config=swagger_config)

    @app.route("/time", methods=["GET"])
    @swag_from("endpoint_configs/radio_session/time.yml")
    def get_time_left():
        queue.put("time")
        return queue.get()

    @app.route("/pause", methods=["GET"])
    @swag_from("endpoint_configs/radio_session/pause.yml")
    def pause_timer():
        queue.put("pause")
        return queue.get()

    @app.route("/resume", methods=["GET"])
    @swag_from("endpoint_configs/radio_session/resume.yml")
    def resume_timer():
        queue.put("resume")
        return queue.get()

    @app.route("/view", methods=["GET"])
    @swag_from("endpoint_configs/radio_session/view.yml")
    def view_queued_uplink():
        queue.put("view")
        return queue.get()

    @app.route("/remove", methods=["POST"])
    @swag_from("endpoint_configs/radio_session/remove.yml")
    def remove_queued_uplink():
        requested_changes = request.get_json()

        # Get the queued uplink
        with open('uplink.json', 'r') as telem_file:
            queued_uplink = json.load(telem_file)

        # Remove listed fields from the queued uplink
        for field_val in requested_changes:
            field = field_val["field"]
            queued_uplink.pop(field)

        # Add the edited telemetry to the queued uplink
        with open('uplink.json', 'w') as telem_file:
            json.dump(queued_uplink, telem_file)
        
        return queued_uplink


    @app.route("/send-telem", methods=["POST"])
    @swag_from("endpoint_configs/radio_session/send-telem.yml")
    def send_telem():
        uplink=request.get_json()
        uplink_console = app.config["uplink_console"]
        imei = app.config["imei"]

        # Check if an uplink is queued
        if os.path.exists("uplink.json"):
            # Organize the requested telemetry into a json object
            requested_telem = {}
            for field_val in uplink:
                requested_telem[field_val["field"]] = field_val["value"]

            # Get the queued uplink
            with open('uplink.json', 'r') as telem_file:
                queued_uplink = json.load(telem_file)

            # Add the requested telemetry to the queued uplink
            with open('uplink.json', 'w') as telem_file:
                queued_uplink.update(requested_telem)
                json.dump(queued_uplink, telem_file)

            return "Added telemetry"
        
        # If there is no uplink queued, send the requested telemetry to Iridium immediately
        fields, vals=list(), list()
        for field_val in uplink:
            fields.append(field_val["field"])
            vals.append(field_val["value"])

         # Create a new uplink packet
        success = uplink_console.create_uplink(fields, vals, "uplink.sbd") and os.path.exists("uplink.sbd")
        if not success:
            return "Unable to send telemetry"

         # Send the uplink immediately to Iridium
        to = "data@sbd.iridium.com"
        sender = "pan.ssds.qlocate@gmail.com"
        subject = imei
        SendMessage(sender, to, subject, "", "", 'uplink.sbd')

         # Remove uplink files/cleanup
        os.remove("uplink.sbd") 
        os.remove("uplink.json")

        return "Successfully sent telemetry to Iridium"

    return app

def create_usb_session_endpoint(usb_session):
    app = Flask(__name__)
    app.logger.disabled = True
    app.config["usb_session"] = usb_session

    app.config["SWAGGER"]={"title": "PAN State Session Command Endpoint", "uiversion": 2}
    swagger=Swagger(app, config=swagger_config)

    @app.route("/send-telem", methods=["POST"])
    @swag_from("endpoint_configs/usb_session/send-telem.yml")
    def send_telem():
        uplink=request.get_json()

        # Create an uplink packet
        fields, vals=list(), list()
        for field_val in uplink:
            fields.append(field_val["field"])
            vals.append(field_val["value"])

        uplink_console = app.config["uplink_console"]
        success = uplink_console.create_uplink(fields, vals, "uplink.sbd") and os.path.exists("uplink.sbd")

        # If the uplink packet is successfully created, then send it to the Flight Computer
        if not success: return "Unable to send telemetry"
        success = usb_session.send_uplink("uplink.sbd")

        # Get rid of uplink files/cleanup
        os.remove("uplink.sbd") 
        os.remove("uplink.json") 

        if success:
            return "Successfully sent telemetry to State Session"
        return "Unable to send telemetry"

    return app
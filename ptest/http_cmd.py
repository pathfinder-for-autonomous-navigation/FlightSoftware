from flask import Flask, request, jsonify
from flasgger import Swagger
from flasgger.utils import swag_from
from datetime import datetime
from argparse import ArgumentParser
import json, sys, logging
import os, threading

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

    @app.route("/request", methods=["GET", "POST"])
    @swag_from("endpoint_configs/radio_session/request.yml")
    def send_telem():
        uplink=request.get_json()

        fields, vals=list(), list()
        for field_val in uplink:
            fields.append(field_val["field"])
            vals.append(field_val["value"])

        uplink_console = app.config["uplink_console"]
        success = uplink_console.create_uplink(fields, vals, "uplink.sbd") and os.path.exists("uplink.sbd")

        # Send the uplink to FlightSoftware


        if success:
            return "Successfully sent telemetry to State Session"
        return "Unable to send telemetry"

    return app

def create_state_session_endpoint(state_session):
    app = Flask(__name__)
    app.logger.disabled = True
    app.config["state_session"] = state_session

    app.config["SWAGGER"]={"title": "PAN State Session Command Endpoint", "uiversion": 2}
    swagger=Swagger(app, config=swagger_config)

    @app.route("/request", methods=["POST"])
    @swag_from("endpoint_configs/state_session/request.yml")
    def send_telem():
        # uplink=request.get_json()

        # # Create an uplink packet
        # fields, vals=list(), list()
        # for field_val in uplink:
        #     fields.append(field_val["field"])
        #     vals.append(field_val["value"])

        # uplink_console = app.config["uplink_console"]
        # console = app.config["console"]
        # success = uplink_console.create_uplink(fields, vals, "uplink.sbd") and os.path.exists("uplink.sbd")

        # if success:
        #     # Extract uplink data from created sbd file
        #     try:
        #         file = open("uplink.sbd", "rb")
        #     except:
        #         return "Unable to send telemetry"

        #     uplink_packet = file.read()
        #     uplink_packet_length = len(uplink_packet)
        #     file.close() 
        #     uplink_packet = str(''.join(r'\x'+hex(byte)[2:] for byte in uplink_packet)) #get the hex representation of the packet bytes

            # Send a command to the Flight Software console to process the uplink packet
            # json_cmd = {
            #     'mode': ord('u'),
            #     'val': uplink_packet,
            #     'length': uplink_packet_length
            # }
            # json_cmd = json.dumps(json_cmd) + "\n"

            # device_write_lock = threading.Lock()
            # device_write_lock.acquire()
            # console.write(json_cmd.encode())
            # device_write_lock.release()

            # return "Successfully sent telemetry to State Session"

        return "Unable to send telemetry"

    return app
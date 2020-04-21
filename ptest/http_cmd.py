from flask import Flask, request, jsonify
from flasgger import Swagger
from flasgger.utils import swag_from
from datetime import datetime
from argparse import ArgumentParser
import json, sys, logging

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
    "static_url_path": "/flassger_static",
    "swagger_ui":True,
    "specs_route":"/swagger/"
}

def create_radio_session_endpoint():
    app = Flask(__name__)
    app.config["SWAGGER"]={"title": "PAN Radio Session Command Endpioint", "uiversion": 2}
    swagger=Swagger(app, config=swagger_config)

    @app.route("/request", methods=["GET"])
    @swag_from("endpoint_configs/common/request.yml")
    def request():
        return "Hello world!"

    return app

def create_state_session_endpoint():
    app = Flask(__name__)
    app.logger.disabled = True

    app.config["SWAGGER"]={"title": "PAN State Session Command Endpioint", "uiversion": 2}
    swagger=Swagger(app, config=swagger_config)

    @app.route("/request", methods=["GET"])
    @swag_from("endpoint_configs/common/request.yml")
    def request():
        return "Hello world!"

    return app

import sys
from cerberus import Validator

ptest_config_schema = {
    "seed" : {"type" : "integer"},
    "single_sat_sim" : {"type": "boolean"},
    "uplink_producer_filepath": {"type": "string"},
    "downlink_parser_filepath": {"type": "string"},
    "devices" : {
        "type" : "list",
        "schema" : {
            "type" : "dict",
            "schema" : {
                "name" : {"type" : "string", "required" : True},
                "run_mode" : {"type" : "string", "allowed" : ["native", "teensy"], "required" : True},
                "binary_filepath" : {
                    "type" : "string", 
                    "dependencies" : {"run_mode" : ["native"]}, 
                    "excludes" : ["port", "baud_rate"]
                },
                "port" : {
                    "type" : "string",
                    "dependencies" : {"run_mode" : ["teensy"]},
                    "excludes" : "binary_filepath"
                },
                "baud_rate" : {
                    "type" : "integer",
                    "dependencies" : {"run_mode" : ["teensy"]},
                    "excludes" : "binary_filepath"
                },
                "http_port" : {"type" : "integer", "required" : True}
            }
        },
        "required" : True
    },
    "radios" : {
        "type" : "list",
        "schema" : {
            "type" : "dict",
            "schema" : {
                "connected_device" : {"type" : "string", "required" : True},
                "imei" : {"type" : "string", "required" : True},
                "connect" : {"type" : "boolean", "required" : True},
                "http_port" : {"type" : "integer", "required" : True},
                "send_queue_duration" : {"type" : "integer", "required" : True},
                "send_lockout_duration" : {"type" : "integer", "required" : True},
            }
        },
        "required" : True
    },
    "tlm" : {
        "type" : "dict",
        "schema" : {
            "email_username" : {"type" : "string", "required" : True},
            "email_password" : {"type" : "string", "required" : True},
            "webservice" : {
                "type" : "dict",
                "schema": {
                    "server": { "type": "string" , "required" : True},
                    "port": { "type": "integer" , "required" : True}
                }
            },
            "elasticsearch": {
                "type" : "dict",
                "schema": {
                    "server": { "type": "string" , "required" : True },
                    "port": { "type": "integer" , "required" : True }
                }
            }
        }
    }
}

def validate_config(data, schema):
    v = Validator(schema)
    if not v.validate(data, schema):
        print("Malformed config file. The following errors were found. Exiting.")
        print(v.errors)
        sys.exit(1)

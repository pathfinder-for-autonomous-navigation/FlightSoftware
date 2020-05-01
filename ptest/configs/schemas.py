import sys
from cerberus import Validator

ptest_config_schema = {
    "seed" : {"type" : "integer"},
    "single_sat_sim" : {"type": "boolean"},
    "devices" : {
        "type" : "list",
        "schema" : {
            "type" : "dict",
            "schema" : {
                "name" : {"type" : "string"},
                "run_mode" : {"type" : "string", "allowed" : ["native", "teensy"]},
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
                "port" : {"type" : "string"}
            }
        }
    },
    "radios" : {
        "type" : "list",
        "schema" : {
            "type" : "dict",
            "schema" : {
                "connected_device" : {"type" : "string"},
                "imei" : {"type" : "string"},
                "connect" : {"type" : "boolean"},
                "port" : {"type" : "string"}
            }
        },
    },
    "tlm" : {
        "type" : "dict",
        "schema" : {
            "email_username" : {"type" : "string"},
            "email_password" : {"type" : "string"},
            "webservice" : {
                "type" : "dict",
                "schema": {
                    "server": { "type": "string" },
                    "port": { "type": "string" }
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

from elasticsearch import Elasticsearch
from argparse import ArgumentParser
import json
import sys

from .email_processor import IridiumEmailProcessor
# from .webservice import app

def main(args):
    parser = ArgumentParser(description='''Webservice for accessing and updating PAN telemetry.''', prog="tlm.py")
    parser.add_argument('-c', '--conf', action='store', 
        help='Location of radio keys configuration file. Defaults to tlm/config.json',
        default="tlm/config.json")
    parser.add_argument('-p', '--port', action='store', 
        help='Port to run this service on. Defaults to 5000.',
        default="5000")
    parser.add_argument('-d', '--downlink-parser-path', action='store', 
        help='Path to the downlink parser. Defaults to .pio/build/gsw_downlink_parser/program',
        default=".pio/build/gsw_downlink_parser/program")
    parser.add_argument('-e', '--elasticsearch-port', action='store',
        help='Port on which the Elasticsearch service is running. Elasticsearch is assumed to be running on localhost. Defaults to 9200.',
        default="9200")
    args = parser.parse_args(args)

    # Get keys for connecting to email account and elasticsearch server
    try:
        with open(args.conf, 'r') as json_config_file:
            radio_keys_config = json.load(json_config_file)
    except json.JSONDecodeError:
        print("Could not load config files. Exiting.")
        sys.exit(1)
    except KeyError:
        print("Malformed config file. Exiting.")
        sys.exit(1)

    # Open a connection to elasticsearch
    es = Elasticsearch([{'host':'127.0.0.1','port':args.elasticsearch_port}])
    
    # Create a read_iridium object
    readIr = IridiumEmailProcessor(radio_keys_config, es, args.downlink_parser_path)
    
    print("Starting Iridium Email Processor...")
    # Start checking emails and posting reports
    readIr.connect()
    
    # MOVED TO /groundsoftware/es_routing
    # app.config["es"] = es
    # app.run(debug=True, host='0.0.0.0')

if __name__ == "__main__":
    main(sys.argv[1:])

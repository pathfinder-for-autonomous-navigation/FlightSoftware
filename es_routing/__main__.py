import sys
from elasticsearch import Elasticsearch
from argparse import ArgumentParser
from .webservice import app

def main(args):
    parser = ArgumentParser(description='''Webservice for accessing and updating PAN telemetry.''', prog="es_routing.py")
    parser.add_argument('-p', '--port', action='store', 
        help='Port to run this service on. Defaults to 5000.',
        default="5000")
    parser.add_argument('-e', '--elasticsearch-port', action='store',
        help='Port on which the Elasticsearch service is running. Elasticsearch is assumed to be running on localhost. Defaults to 9200.',
        default="9200")
    args = parser.parse_args(args)

    # Open a connection to elasticsearch
    es = Elasticsearch([{'host':'127.0.0.1','port':args.elasticsearch_port}])

    app.config["es"] = es
    app.run(debug=True, host='0.0.0.0')

if __name__ == "__main__":
    main(sys.argv[1:])

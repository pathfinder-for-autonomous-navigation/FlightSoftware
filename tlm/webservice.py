from flask import Flask, request, jsonify
from flasgger import Swagger
from flasgger.utils import swag_from
from datetime import datetime
from argparse import ArgumentParser
import json

app = Flask(__name__)
app.config["SWAGGER"]={"title": "PAN Telemetry Software", "uiversion": 2}

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
        "time.downlink_received": str(datetime.now().isoformat())
    })

    #index statefield report in elasticsearch
    sf_res = app.config["es"].index(index='statefield_report_'+str(imei), doc_type='report', body=data)
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
                "time.downlink_received": {
                    "order": "desc"
                }
            }
        ],
        "size": 1
    }

    # Get the value of that field from the document
    if app.config["es"].indices.exists(index=index):
        res = app.config["es"].search(index=index, body=json.dumps(search_object))
        if len(res["hits"]["hits"])!=0:
            most_recent_field=res["hits"]["hits"][0]["_source"][field]
            return str(most_recent_field)
        else:
            return f"Unable to find field: {field} in index: {index}"
    else:
        return f"Unable to find index: {index}"

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
        "time.downlink_received": str(datetime.utcnow().isoformat())[:-3]+'Z'
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


# The function that completes a deep pagination of a Point in Time Snapshot
def pit_search(pit_id, index, start, end, field, values = [], last_search_location = None):
    #core search object for first 10,000 results
    search_object = {
        "size": 10000,
        'query': {
            'exists': {
                'field': field
            }
        },
        "pit": {
            'id': pit_id,
            'keep_alive': '1m'
        },
        "sort": [
            {
                "time.downlink_received": {
                    "order": "desc"
                }
            }
        ]
        
    }
    #checks whether the last search location is not None and therefore isn't the first search
    if last_search_location is not None:
        #sets the starting posisition for the next 10,000 hits of the deep pagination
        search_object['search_after'] = last_search_location
        search_object['track_total_hits'] = False

    #complete the search
    res = app.config["es"].search(body=json.dumps(search_object))

    hit_num = len(res["hits"]["hits"])
    if hit_num == 0:
        #if hits are zero and it isnt the first search return the values
        if last_search_location is not None:
            return json.dumps(values)
        #if hits are zero and it is the first search return a data point saying the field could not be found in the index
        else:
            telempoint = {'timestamp': end, 'id': field, 'value': f"Unable to find field: {field} in index: {index}"}
            values = values + [telempoint]
            return json.dumps(values)
    
    # check through every hit to see if it is between the designated start and end times
    for r in res["hits"]["hits"]:
        time = r["_source"]["time.downlink_received"]
        value = r["_source"][field]
        if time <= end and time >= start:
            telempoint = {'timestamp': time, 'id': field, 'value': value}
            values = values + [telempoint]
        elif time < start:
            return json.dumps(values)
    #set the last location for the start of the next 10,000 hit search
    last_loc = res["hits"]["hits"][hit_num - 1]["sort"]

    # Call the function again to search for the next 10,000 hits
    return pit_search(pit_id,index,start,end,field,values, last_loc)
        

    

# endpoint for getting an array of data points from elasticsearch for a specific field in a particular index    
@app.route("/time-search-es", methods=["GET"])
@swag_from("endpoint_configs/time_search_es_config.yml")
def time_search_es():
    #get parameters
    index = request.args.get('index')
    field = str(request.args.get('field'))
    start = request.args.get('start')
    end = request.args.get('end')
    #convert into proper time format
    start = start.replace('%3A', ':')
    end = end.replace('%3A', ':')
    #if the index exists
    if app.config["es"].indices.exists(index=index):
        #create a point in time snapshot to begin the deep pagination
        pit_id = (app.config["es"].open_point_in_time(index=index,keep_alive='3m'))['id']
        #do deep pagination
        completed_search = pit_search(pit_id, index, start, end, field)
        del_pit = {
            "id" : pit_id
        }
        #delete old snapshot when complete
        app.config["es"].close_point_in_time(body=json.dumps(del_pit))
        return completed_search
    #if the index doesnt exist return one telemetry point saying the index doesnt exist
    else:
        telempoint = {'timestamp': end, 'id': field, 'value': f"Unable to find index: {index}"}
        values = [telempoint]
        return json.dumps(values)
    
# GROUND SOFTWARE

Ground Software will store/index all telemetry coming from the Iridium email account 
using a Flask server connected to ElasticSearch. Telemetry will be stored in two 
different indexes in ElasticSearch

1) Statefield_Reports: Statefield reports contain updated statefields, values, and times
2) Iridium_Reports: Iridium reports include the most recent MOMSN and MTMSN (from both downlinks
and uplink confirmations) numbers. It also includes whether or not the ground is able to send 
uplinks and the time of the report.

The Flask server contains an endpoint for accessing information from either index in ElasticSearch.
This endpoint is used in RadioSession to check whether or not it can send uplinks and for
reading the most updated statefield variables.

# Installing and Running

Flask Server:
1. Create and activate a virtualenv within this directory.
    - For Mac and Linux: `python3 -m virtualenv venv; source venv/bin/activate`
    - For Windows: `python -m virtualenv venv`, followed by `venv\Scripts\activate`.
2. Install the requirements: `pip install -r requirements.txt`
3. To run the server, type `python main.py` in the terminal
4. To view the SwaggerUI API, type `http://127.0.0.1:5000/swagger/` in your browser

ElasticSearch:
1. Install Java and Homebrew on your computer
2. Install ElasticSearch
    - For Mac: `brew install elasticsearch`
3. To run ElasticSearch, open a terminal and type `elasticsearch`
4. Type `http://localhost:9200` in your browser to see if elasticsearch is running

Kibana:
1. Install Java and Homebrew on your computer
2. Install Kibana
    - For Mac: `brew install kibana`
3. To run Kibana, open a terminal and type `kibana`
4. Type `http://localhost:5601` in your browser to view the Kibana interface

Right now, we are running ElasticSearch and Kibana locally. However, in the future,
we will be running ElasticSearch and Kibana on a separate server.
# Telemetry Software

Telemetry Software will store/index all telemetry coming from the Iridium email account 
using a Flask server connected to ElasticSearch. Telemetry will be stored in two 
different indexes in ElasticSearch

1) Statefield_Reports: Statefield reports contain updated statefields, values, and times
2) Iridium_Reports: Iridium reports include the most recent MOMSN and MTMSN (from both downlinks
and uplink confirmations) numbers. It also includes whether or not the ground is able to send 
uplinks and the time of the report.

The Flask server contains an endpoint for accessing information from either index in ElasticSearch.
This endpoint is used in RadioSession to check whether or not it can send uplinks and for
reading the most updated statefield variables.

# Gmail Access

Access to the PAN gmail account is authenticated by the `oauth2.py` file. This requires
a JSON file holding the information of project requesting access to the email account. To get
this file:

1) Sign into the PAN email account. 
2) Go to the Google Developers Console: https://console.developers.google.com/apis/
3) In the top left of the screen, select the RemoteServer project.
4) Go to the credentials page. This should be found in the navigation bar to the left.
5) Underneath the section `OAuth 2.0 Client IDs`, there should be an entry called `TelemetryServer`.
6) Download a JSON file holding the credentials for the Telemetry server (click the downward pointing arrow to the right of the entry).
7) Move the JSON file into the `FlightSoftware/tlm` folder. Rename the file to `client_secret.json`.

# Installing and Running

Flask Server:
Do all of the following from the root FlightSoftware directory.

1. Install and activate the virtual environment.
2. To run the server, type `python -m tlm` in the terminal. The call can be configured;
   see the argument parser in `__main__.py` to see how.
3. To view the SwaggerUI API, type `http://127.0.0.1:5000/swagger/` in your browser

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
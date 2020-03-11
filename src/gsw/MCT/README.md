# Mct-Mission-Control

This Mission control API uses the visual elements of the openMCT data visualization program to display telemetry data for the two leader and follower satellites. Open MCT supports receiving telemetry by requesting data from a telemetry store, and by subscribing to real-time telemetry updates. 
In Open MCT everything is represented as a Domain Object, this includes sources of telemetry, telemetry points, and views for visualizing telemetry. Domain Objects are accessible from the object tree shown on the left side of the 

There are three main plugins that allow the program to analyze realtime and historical telemetry, as well as identify where to display each datum.

# The Realtime Telemetry plugin
(realtime-telemetry-plugin.js)
This script containts two main functions:  RealtimeTelemetryPlugin( ) and subscribe( )
**The supportsSubscribe function maintains the precondition that any input is truly a telemetry object**

RealtimeTelemetryPlugin( ) defines a new WebSocket object and then casts the incoming data into a JSON type that the openmct web service is able to analyze and graph visually.

subscribe( ) pings the server for data and adds any callback into the listener[ ] list for the WebSocket to analyze. It also includes an unsubscribe function that is used to remove data from the listener[ ] list

# The Historical Telemetry plugin
(historical-telemetry-plugin.js)
This script only has one main function (install) that nests two other functions supportsRequest( ) and request( )
**Like supportsSubscribe( ) in the realtime plugin, the supportsRequest( ) acts as a precondition check that the object passed into the plugin is truly a telemetry object**

The request( ) function makes an HTTP request to a historical database and then creates a telemetry point using the addProvider( ) function given by the openmct.telemetry class.

# The Dictionary Plugin
(dictionary-plugin.js)
This script analyzes the json file that holds the domain object for the satellite and the subobjects that describe each telemetry point of the satellite. 
There are three main functions in this file, get( ), load( ), and  DictionaryPlugin( ).

get( ) takes the dictionary.json file as input and gets each identifier inside of the satellite domain object and constructs a new child object for each attribute of the original parent object (the satellite).

load( ) is part of the composition provider which accepts a domain object and then provides identifiers for the children of that domain object. This creates the inheritance within the object tree and its individual telemetry points.

The DictionaryPlugin( ) function defines the type "example-telemetry" and


# Mct-Mission-Control

This Mission control API uses the visual elements of the openMCT data visualization program to display telemetry data for the two leader and follower satellites. Open MCT supports receiving telemetry by requesting data from a telemetry store, and by subscribing to real-time telemetry updates.

The openMCT web server is run using the **"npm start"** command in terminal after navigating to the /MCT directory and installing the node.js modules with **"npm install"**.

In Open MCT everything is represented as a Domain Object, this includes sources of telemetry, telemetry points, and views for visualizing telemetry. Domain Objects are accessible from the object tree shown on the left side of the openMCT display.
![Object Tree](https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/batteryinfo_mct/MCT/images/object-tree.png)

# Setting up a new Taxonomy or Telemetry point
In this implementation a Taxonomy point is defined as an object that containts a collection of telemetry points. (ex. a spacecraft object) A new Taxonomy point is created using the *openmct.objects.addRoot( )* function, which takes a javascript object as a parameter.

This is modeled below with our satellite object with namespace 'sat.taxonomy' and a key 'spacecraft'.
![addRoot function](https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/batteryinfo_mct/MCT/images/add-Root.png)

A new Telemetry point is added after a Taxonomy point is added and serves as the child object to the root (the satellite). This object is added using the openmct.objects.addType( ) function which also takes a JS object as a parameter.

In the example below, we have a name and description field for the object and also a cssClass field which allows it to be displayed on the openmct visualization.
![addType function](https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/batteryinfo_mct/MCT/images/add-Type.png)



There are three main plugins that allow the program to analyze realtime and historical telemetry, as well as identify where to display each datum.

# The Realtime Telemetry plugin
(realtime-telemetry-plugin.js)
This script containts two main functions:  RealtimeTelemetryPlugin( ) and subscribe( )
It also instantiates a new WebSocket object ws.

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

The DictionaryPlugin( ) function defines the type "sat-telemetry" and the root case, "sat-taxonomy" which represents the satellite. The "sat-telemetry" objects represent the specific telemetry points on the satellite.

# The HTML Index File
(index.html)
This file combines all of the plugins together and runs them as scripts and is written in HTML.
It also calls the openmct.install( ) function to instantiate the visual aspect of openMCT (ex. the clock, the three plugins mentioned above, and the node.js library)

# Main method and Server-files

The main method that is called when openMCT is started is **server.js** which is located in the server-files folder which also contains three server files that provide realtime and historical data collection. These are the history-server.js, realtime-server.js, and static-server.js files.

The main script, server.js, instantiates these files and calls them to start each respective server and subscribe to updates from the historical and realtime telemetry plugins.

**realtime-server.js** contains two main functions, RealtimeServer( ) which takes the spacecraft as a parameter and notifySubscribers( ) which takes one point as a parameter and sends it to the WebSocket.
notifySubscribers( ) is defined inside the RealtimeServer( ) function and converts the javascript object of a single telemetry point into a JSON string.

**history-server.js** initiates a new router object and a response object. It uses the node.js module express to create a new Router( ) function. This router then pulls data from the spacecraft.js list history[ ] using the response function.

**static-server.js** This script uses the Router( ) module's router.use( ) function to allow our application to handle static requests for data. This helps handle image data, for example, the CSS image data that is sent in with each telemetry point.



**Spacecraft.js** represents a spacecraft object that is run when the main script is evoked in terminal.
The spacecraft function contains 3 main fields, state, history, and listeners.
The state field contains all of the telemetry objects contained by the spacecraft and is checked periodically for changes to generate telemetry.
The history field contains telemetry data that is input from the generateTelemetry( ) function.
The listener field is notified every time telemetry data is generated.

The generateTelemetry( ) function is the most important method of spacecraft.js as it interacts with each field inside the spacecraft.js file and creates data for the openMCT server to then catch and display.
![Telemetry function](https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/addmct/src/gsw/MCT/images/generate-telemetry.png)
This function takes a measurement of spacecraft state, stores it in history{ }, and notifies listeners[ ] using the *Spacecraft.prototype.notify* function.

**Battery.js** represents the gomspace battery object and creates battery telemetry that is sent to specific telemetry points as defined in telemetry reviews.

#Full telemetry diagram
![Telemetry diagram](https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/batteryinfo_mct/MCT/images/telemetry-diagram.PNG)

# PAN Mission Control (OpenMCT)

<hr style="border:4px solid gray"> </hr>

[Usage Guide]:
1. [Installing Requirements and Running Mission Control]
2. [Setting up a new Subsystem or Domain Object]
   1. [Adding the json file]
   2. [Adding to the Instillation Configuration Object]
   3. [Installing the plugin in index.html]
   4. [Adding entry to state-variables.js to initialize the the variables]


[Code Description]:
1. [The Generic Plugin]
2. [The Realtime Telemetry Plugin option]
3. [The Historical Telemetry Plugin option]
4. [The HTML Index File]
5. [Main method and Server-files]


# Usage Guide:
<hr style="border:2px solid gray"> </hr>

## Installing Requirements and Running Mission Control

This Mission control API uses the visual elements of the openMCT data visualization program to display telemetry data for the two leader and follower satellites. Open MCT supports receiving telemetry by requesting data from a telemetry store, and by subscribing to real-time telemetry updates.

The following steps guide you through starting up PAN Mission Control.

Step 2 needs to be completed any time the package files are updated. 
Step 3 needs to be completed any time the mct code-base has been updated.

1. navigate to the /MCT directory

2. install the node.js modules with **```npm install```**

3. configure the MCT code-base using **```npm run setup```**

4. start the PAN Mission Control server using **```npm start```**

Running **```npm start```** defaults to using server-files/mct_secret.json configuration file. you can specify the configuration file that you want MCT to use by appending an argument containing the path relative to FlightSoftware root directory.

For example:

**```npm start MCT/server-files/mct_single.json```** would start MCT using the configuration file mct_single.json. Make sure the config file is a valid mct config file as defined in ```/MCT/server-files/mct_config_schema.json``` .

In Open MCT everything is represented as a Domain Object, this includes sources of telemetry, telemetry points, and views for visualizing telemetry. Domain Objects are accessible from the object tree shown on the left side of the openMCT display.

## Setting up a new Subsystem or Domain Object

#### There are four main steps to install a new subsystem's domain object:

1. Add the subsystem's json file to the [subsystems] folder
2. Add an entry to the instillation configuration object literal at the top of [generic-plugin.js]
3. Add a line to [index.html] to install the plugin
4. Add an entry to the [state-variables.js] file containing the initial values of the telemetry states contained within the subsystem/domain object

### 1. Adding the json file

After generating the json file, Place it in the [subsystems] folder

More on this can be found in the [MCT/Public Readme File]

### 2. Adding to the Instillation Configuration Object

Add an entry to the instillation configuration object called configs at the top of [generic-plugin.js]

The key of the entry is a name representing the domain object you wish to install such as battery

the value of the entry is an object literal. 

#### The keys and values needed in this object are as follows:

 * **namespace** - where the value is the name of the namespace/taxonomy of the domain object. 
 Ex.: ```namespace: 'bat.taxonomy'```
 * **key** - where the value is the key of the domain object within the namespace. 
 Ex.: ```key: 'spacecraft'```
 * **type** - this contains the type of the telemetry you want to be related to the domain object. 
 Ex.: ```type: 'bat.telemetry'```
 * **typeName** - where the value is the name of the type you want to be related to the domain object. 
 Ex.: ```typeName: 'Battery Telemetry Point'```
 * **typeDescription** - where the value is the description of the type you want to be related to the domain object. 
 Ex.: ```typeDescription: 'Telemetry point on the Gomspace battery'```
 * **typeCssClass** - where the value is the css class used by the type related to the domain object. 
 Ex.: ```typeCssClass: 'icon-telemetry'```
 * **jsonFile** - where the value is the path of the jsonFile for the domain object within the '/public/subsystems' folder. 
 Ex.: ```jsonFile: '/gomspace.json'```

### 3. Installing the plugin in index.html

Next you must include the instillation line for the domain object.

To install the domain object you must use the ```GenericPlugin(config)``` function.
the input for config is the key of the entry in the instilation configuration variable in [generic-plugin.js]'

Example:

```openmct.install(GenericPlugin("battery"));```

### 4. Adding entry to state-variables.js to initialize the variables

In order for [telemetry.js] to properly pull data down from Elastic Search, the fields you want to update must be initialized in [state-variables.js].

the keys of the entry must correspond with the sections of the Elastic Search entry:

Ex.:

if ```a.b```, ```a.c.d```, ```a.c.e```, ```a.f```, and ```a.g``` are fields in Elastic Search this initialization would look like:
```
a: {
    b:___,
    c{
        d:___,
        e:___
    },
    f:___,
    g:___
}
```
where each ```___``` represents the initial value of the field.



# Code Description:
<hr style="border:2px solid gray"> </hr>

## The Generic Plugin



There is the one plugin for the input of telemetry into OpenMCT which takes in config data to specifiy the desired domain object you wish to install

**This script containts 3 main cases:**

1. A Realtime Telemetry Plugin option
2. A Historical Telemetry Plugin option
3. A Genericized Plugin option to install a telemetry plugin for a new Domain Object



### The Realtime Telemetry Plugin option

The Realtime Telemetry Plugin containts two main functions:  ```RealtimeTelemetryPlugin( )``` and ```subscribe( )```. It also instantiates a new WebSocket object ws.

**The supportsSubscribe function maintains the precondition that any input is truly a telemetry object**

```RealtimeTelemetryPlugin( )``` defines a new WebSocket object and then casts the incoming data into a JSON type that the openmct web service is able to analyze and graph visually.

```subscribe( )``` pings the server for data and adds any callback into the ```listener[ ]``` list for the WebSocket to analyze. It also includes an unsubscribe function that is used to remove data from the ```listener[ ]``` list

### The Historical Telemetry Plugin option

This script only has one main function ```(install)``` that nests two other functions ```supportsRequest( )``` and ```request( )```

Like ```supportsSubscribe( )``` in the realtime plugin, the ```supportsRequest( )``` acts as a precondition check that the object passed into the plugin is truly a telemetry object

The ```request( )``` function makes an HTTP request to a historical database and then creates a telemetry point using the ```addProvider( )``` function given by the openmct.telemetry class.

### The Generic Plugin option

This script analyzes the json file that holds the domain object for the satellite and the subobjects that describe each telemetry point of the satellite.
It contains a configuration object literal containing all the information for the different possible domain object instillations.
There are three main functions in this file, ```get( )```, ```load( )```, and  ```GenericPlugin( )```.

```get( )``` takes the json file specified in the config data as input and gets each identifier inside of the domain object and constructs a new child object for each attribute of the original parent object.

```load( )``` is part of the composition provider which accepts a domain object and then provides identifiers for the children of that domain object. This creates the inheritance within the object tree and its individual telemetry points.

The ```GenericPlugin(config)``` function defines the type and the root case, specified in the config data, which represents the domain object. The objects represent the specific telemetry points on the domain object. It takes in a ```config``` variable which matches the key of one of the entries in the ```configs``` object literal defined at the top of the file.

## The HTML Index File

[index.html] combines all of the plugins together and runs them as scripts and is written in HTML.
It also calls the ```openmct.install( )``` function to instantiate the visual aspect of openMCT (ex. the clock, the Generic plugin mentioned above, and the node.js library)

## Main method and Server-files


The main method that is called when openMCT is started is **[server.js]** which is located in the server-files folder which also contains a [boilerplate-mct-servers] folder containing two server files that provide realtime and historical data collection. These are the [history-server.js] and [realtime-server.js] files.

**[server.js]** 

This script runs when calling main from the terminal. It creates a new realtime server and a new historical server from [realtime-server.js] and [history-server.js] files. It also creates the telemetry generator from [telemetry.js]. Finally it starts up the server on port 8080.

The main script, [server.js], instantiates the files and calls them to start each respective server and subscribe to updates from the historical and realtime telemetry plugins.

**[realtime-server.js]** 

contains two main functions, ```RealtimeServer( )``` which takes the spacecraft as a parameter and ```notifySubscribers( )``` which takes one point as a parameter and sends it to the WebSocket.
```notifySubscribers( )``` is defined inside the ```RealtimeServer( )``` function and converts the javascript object of a single telemetry point into a JSON string.

**[history-server.js]** 

initiates a new router object and a response object. It uses the node.js module express to create a new ```Router( )``` function. This router then pulls data from the [telemetry.js] list ```history[ ]``` using the ```response``` function.

**[telemetry.js]** 

represents all the telemetry from all of the domain objects.
The telemetry function contains 3 main fields, state, history, and listeners.
The state field contains all of the telemetry objects contained by the domain objects defined in [state-variables.js] and is checked periodically for changes to generate telemetry.

The history field contains telemetry data that is input from the ```generateTelemetry( )``` function.
The listener field is notified every time telemetry data is generated.

The ```generateTelemetry( )``` function is the most important method of telemetry.js as it interacts with each field inside the telemetry.js file and creates data for the openMCT server to then catch and display. This function takes a measurement of all the domains' states, pulling from elasticsearch, stores it in ```history{ }```, and notifies ```listeners[ ]``` using the ```Telemetry.prototype.notify``` function.

**[state-variables.js]** 

This file contains the intial values of all the state fields that [telemetry.js] will repeatedly update


[Usage Guide]: #usage-guide
[Installing Requirements and Running Mission Control]: #installing-requirements-and-running-mission-control
[Setting up a new Subsystem or Domain Object]: #setting-up-a-new-subsystem-or-domain-object
[Adding the json file]: #adding-the-json-file
[Adding to the Instillation Configuration Object]: #adding-to-the-instillation-configuration-object
[Installing the plugin in index.html]: #installing-the-plugin-in-indexhtml
[Adding entry to state-variables.js to initialize the the variables]: #adding-entry-to-state-variablesjs-to-initialize-the-variables
[Code Description]: #code-description
[The Generic Plugin]: #the-generic-plugin
[The Realtime Telemetry Plugin option]: #the-realtime-telemetry-plugin-option
[The Historical Telemetry Plugin option]: #the-historical-telemetry-plugin-option
[The HTML Index File]: #the-html-index-file
[Main method and Server-files]: #main-method-and-server-files

[history-server.js]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/server-files/boilerplate-mct-servers/history-server.js

[telemetry.js]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/server-files/telemetry.js

[state-variables.js]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/server-files/state-variables.js

[realtime-server.js]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/server-files/boilerplate-mct-servers/realtime-server.js

[server.js]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/server-files/server.js

[generic-plugin.js]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/public/generic-plugin.js

[index.html]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/public/index.html

[boilerplate-mct-servers]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/tree/master/MCT/server-files/boilerplate-mct-servers

[MCT/Public Readme File]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/public/README.md

[subsystems]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/tree/master/MCT/public/subsystems

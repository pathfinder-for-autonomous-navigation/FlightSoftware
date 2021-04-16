/**
 * Configurations for instillation of each supported Domain Object/Subsystem using '/generic-plugin.js'
 */
var configs = {
    battery: {
        namespace: 'bat.taxonomy',
        key: 'spacecraft',
        type: 'bat.telemetry',
        typeName: 'Battery Telemetry Point',
        typeDescription: 'Telemetry point on the Gomspace battery',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'gomspace.json'
    },

    attitude: {
        namespace: 'attitude.taxonomy',
        key: 'spacecraft',
        type: 'attitude.telemetry',
        typeName: 'Attitude',
        typeDescription: 'An Attitude Telemetry Point',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'attitude.json'
    },

    pan:{
        namespace: 'pan.taxonomy',
        key: 'spacecraft',
        type: 'pan.telemetry',
        typeName: 'PAN Telemetry Point',
        typeDescription: 'Telemetry point for PAN namespace',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'pan.json'
    },


    orbit:{
        namespace: 'orb.taxonomy',
        key: 'spacecraft',
        type: 'orb.telemetry',
        typeName: 'Orbit Telemetry Point',
        typeDescription: 'Telemetry point for the Orbit',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'orbit.json'
    },

    piksi:{
        namespace: 'piksi.taxonomy',
        key: 'spacecraft',
        type: 'piksi.telemetry',
        typeName: 'Pikis Telemetry Point',
        typeDescription: 'Telemetry point on the piksi',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'piksi.json'
    },

    attitude_estimator:{
        namespace: 'att.taxonomy',
        key: 'spacecraft',
        type: 'att.telemetry',
        typeName: 'Attitude Estimator Telemetry Point',
        typeDescription: 'Telemetry point for the Attitude Estimator',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'attitude_estimator.json' 
    },

    adcs_monitor:{
        namespace: 'adcsmon.taxonomy',
        key: 'spacecraft',
        type: 'adcsmon.telemetry',
        typeName: 'ADCS Monitor Telemetry Point',
        typeDescription: 'Telemetry point for the ADCS Monitor',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'adcs_monitor.json'
    },

    prop:{
        namespace: 'prop.taxonomy',
        key: 'spacecraft',
        type: 'prop.telemetry',
        typeName: 'Prop Telemetry Point',
        typeDescription: 'Telemetry point for Prop',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'prop.json'
    },

    adcs_command:{
        namespace: 'adcscomm.taxonomy',
        key: 'spacecraft',
        type: 'adcscomm.telemetry',
        typeName: 'ADCS Command Telemetry Point',
        typeDescription: 'Telemetry point for the ADCS Command',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'adcs_cmd.json'
    },

    adcs: {
        namespace: 'adcs.taxonomy',
        key: 'spacecraft',
        type: 'adcs.telemetry',
        typeName: 'ADCS Telemetry Point',
        typeDescription: 'Telemetry point for the ADCS',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'adcs.json'
    },

    radio: {
        namespace: 'radio.taxonomy',
        key: 'spacecraft',
        type: 'radio.telemetry',
        typeName: 'Radio Telemetry Point',
        typeDescription: 'Telemetry point for the Radio',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'radio.json'
    },

    docksys: {
        namespace: 'dock.taxonomy',
        key: 'spacecraft',
        type: 'dock.telemetry',
        typeName: 'Docksys Telemetry Point',
        typeDescription: 'Telemetry point for Docksys',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'docksys.json'
    },

    dcdc: {
        namespace: 'dcdc.taxonomy',
        key: 'spacecraft',
        type: 'dcdc.telemetry',
        typeName: 'DCDC Telemetry Point',
        typeDescription: 'Telemetry point for DCDC',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'dcdc.json'
    },

    fault_handler:{
        namespace: 'fault.taxonomy',
        key: 'spacecraft',
        type: 'fault.telemetry',
        typeName: 'Fault Handler Telemetry Point',
        typeDescription: 'Telemetry point for the Fault Handler',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'fault_handler.json'
    },

    trigger_dist:{
        namespace: 'trigger.taxonomy',
        key: 'spacecraft',
        type: 'trigger.telemetry',
        typeName: 'Trigger Distance Telemetry Point',
        typeDescription: 'Telemetry point for the Trigger Distance',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'trigger_dist.json'
    },

    downlink:{
        namespace: 'down.taxonomy',
        key: 'spacecraft',
        type: 'down.telemetry',
        typeName: 'Downlink Telemetry Point',
        typeDescription: 'Telemetry point for Downlink',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'downlink.json'
    },

    timing: {
        namespace: 'time.taxonomy',
        key: 'spacecraft',
        type: 'time.telemetry',
        typeName: 'Timing Telemetry Point',
        typeDescription: 'Telemetry point for timing',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'timing.json'
    },

    piksi_fh: {
        namespace: 'piksifh.taxonomy',
        key: 'spacecraft',
        type: 'piksifh.telemetry',
        typeName: 'Piksi_fh Telemetry Point',
        typeDescription: 'Telemetry point for the Piksi_fh',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'piksi_fh.json'
    },

    miscellaneous: {
        namespace: 'misc.taxonomy',
        key: 'spacecraft',
        type: 'misc.telemetry',
        typeName: 'Miscellaneous Telemetry Point',
        typeDescription: 'A miscellaneous telemetry point on the satellite',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'miscellaneous.json'
    }
}

/**
 * Uses the './https.js' module to request a json file
 * 
 * @param {*} j the path to the json file
 */
function requestJSON(j) {
    return http.get(j)
        .then(function (result) {
            return result.data;
        });
}

/**
 * The Plugin Function called in index.html to install a plugin taking a configuration input.
 * This enables the plugin to be used generically for all subsystems/domain objects
 * 
 * @param {*} config A string matching the key of one of the config data objects
 * @param {*} satellite A string designating either "Follower" or "Leader"
 */
function GenericPlugin(config, satellite) {
    return  function install(openmct) {

        console.log(configs);

        //if the parameter config is "realtime", it will install the realtime server
        if(config === "realtime"){
            
            //creates the websocket - wss (since the location of the origin is running on https)
            var socket = new WebSocket(location.origin.replace(/^http/, 'ws') + '/realtime/');
            var listener = {};
            
            //adds the event data to the listener
            socket.onmessage = function (event) {
                point = JSON.parse(event.data);
                if (listener[point.id]) {
                    listener[point.id](point);
                }
            };
            
            //creates the realtime telemetry provder
            var provider = {

                //returns whether or not a realtime telem subscription for this domain object is supported
                supportsSubscribe: function (domainObject) {
                    //cycles through each of the configs to see if there is an allowed domain object plugin with matching telemetry type
                    var allowed = false;
                    for (const allowedDomainObject of Object.values(configs)) {
                        if(domainObject.type === allowedDomainObject.type){//set allowed to true if the domain object type is allowed
                            allowed = true;
                        }
                    }

                    return allowed;
                },
                //subscribes to the domain object adding it to listener and setting up the websocket subscription
                subscribe: function (domainObject, callback) {
                    listener[domainObject.identifier.key] = callback;
                    socket.send('subscribe ' + domainObject.identifier.key);
                    //option to unsubscribe
                    return function unsubscribe() {
                        delete listener[domainObject.identifier.key];
                        socket.send('unsubscribe ' + domainObject.identifier.key);
                    };
                }
            };
    
            openmct.telemetry.addProvider(provider);
    
            
        }
        
        //if the parameter config is "historical", it will install the historical server
        else if(config === "historical"){
        
        // returns a provider for historical telemetry
        var provider = {

            //returns whether or not a historical telem request for this domain object is supported
            supportsRequest: function (domainObject) {

                //cycles through each of the configs to see if there is an allowed domain object plugin with matching telemetry type
                var allowed = false;
                for (const allowedDomainObject of Object.values(configs)) {
                    if(domainObject.type === allowedDomainObject.type){//set allowed to true if the domain object type is allowed
                        allowed = true;
                    }
                }

                return allowed;

            },
            //sets up the method of requesting historical telemetry
            request: function (domainObject, options) {
                var url = '/history/' +
                    domainObject.identifier.key +
                    '?start=' + options.start +
                    '&end=' + options.end;

                return requestJSON(url);
            }
        };
        
        //adds the provider to MCT
        openmct.telemetry.addProvider(provider);
        
        }
        
        //if parameter config was neither "historical" or "realtime",
        //install the plugin with the matching config data in the configs variable
        else{

            var generalConfigData;

            //retrieves the config data with the matching key
            for (const key of Object.keys(configs)) {
	            if(config === key){
   		            generalConfigData = configs[key];
                }
            }

            //Adds the taxonomy namespace and key to MCT using the configData
            openmct.objects.addRoot({
                namespace: satellite + '_' + generalConfigData.namespace,
                key: generalConfigData.key
            });

            //creates the object provider from the json file coorisponding with the domain object
            openmct.objects.addProvider(satellite + '_' + generalConfigData.namespace, {
                get: function (identifier) {
                    return requestJSON('/subsystems'  + '/' + satellite + '_' + generalConfigData.jsonFile).then(function (generic) {
                        //Sets up the folder path for the domain object and the Root
                        if (identifier.key === generalConfigData.key) {
                            return {
                                identifier: identifier,
                                name: satellite.charAt(0).toUpperCase() + satellite.slice(1) + ' ' + generic.name,
                                type: 'folder',
                                location: 'ROOT'
                            };
                        } 
                        
                        //returns all the measurement objects in that domain object setting up file tree
                        else {
                            var measurement = generic.measurements.filter(function (m) {
                                return m.key === identifier.key;
                            })[0];
                            return {
                                identifier: identifier,
                                name: measurement.name,
                                type: generalConfigData.type,
                                telemetry: {
                                    values: measurement.values
                                },
                                location: satellite + '_' + generalConfigData.namespace + ':' + generalConfigData.key
                            };
                        }
                    });
                }
            });
    

            //creates the composition provider from the json file coorisponding with the domain object
            openmct.composition.addProvider({
                //checks to see if the provider is able to be used for the domain object
                appliesTo: function (domainObject) {
                    return domainObject.identifier.namespace === (satellite + '_' + generalConfigData.namespace) &&
                           domainObject.type === 'folder';
                },
                //loads in the data from the coorisponding json file for the measurements to load in the composition provider
                load: function (domainObject) {
                    return requestJSON('/subsystems' + '/' + satellite + '_' + generalConfigData.jsonFile)
                        .then(function (generic) {
                            return generic.measurements.map(function (m) {
                                return {
                                    namespace: satellite + '_' + generalConfigData.namespace,
                                    key: m.key
                                };
                            });
                        });
                }
            });
            
            //adds the data for the telemetry type stored in the config data to MCT
            openmct.types.addType(generalConfigData.type, {
                name: generalConfigData.typeName,
                description: generalConfigData.typeDescription + ' on the ' + satellite + ' satellite',
                cssClass: generalConfigData.typeCssClass
            });


        }
    };
};

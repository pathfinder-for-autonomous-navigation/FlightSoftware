/**
 * Configurations for instillation of each supported Domain Object/Subsystem using '/generic-plugin.js'
 */
var configs = {
    follower: {
        namespace: 'follower.taxonomy',
        key: 'spacecraft',
        type: 'follower.telemetry',
        typeName: 'Battery Telemetry Point',
        typeDescription: 'Telemetry point on the Follower',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'follower.json'
    },

    leader: {
        namespace: 'leader.taxonomy',
        key: 'spacecraft',
        type: 'leader.telemetry',
        typeName: 'Battery Telemetry Point',
        typeDescription: 'Telemetry point on the Leader',
        typeCssClass: 'icon-telemetry',
        jsonFile: 'leader.json'
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
 */
function GenericPlugin(config) {
    return function install(openmct) {

        console.log(configs);

        //if the parameter config is "realtime", it will install the realtime server
        if (config === "realtime") {

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
                        if (domainObject.type === allowedDomainObject.type) {//set allowed to true if the domain object type is allowed
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
        else if (config === "historical") {

            // returns a provider for historical telemetry
            var provider = {

                //returns whether or not a historical telem request for this domain object is supported
                supportsRequest: function (domainObject) {

                    //cycles through each of the configs to see if there is an allowed domain object plugin with matching telemetry type
                    var allowed = false;
                    for (const allowedDomainObject of Object.values(configs)) {
                        if (domainObject.type === allowedDomainObject.type) {//set allowed to true if the domain object type is allowed
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
        else {

            var generalConfigData;

            //retrieves the config data with the matching key
            for (const key of Object.keys(configs)) {
                if (config === key) {
                    generalConfigData = configs[key];
                }
            }

            //Adds the taxonomy namespace and key to MCT using the configData
            openmct.objects.addRoot({
                namespace: generalConfigData.namespace,
                key: generalConfigData.key
            });

            //creates the object provider from the json file coorisponding with the domain object
            openmct.objects.addProvider(generalConfigData.namespace, {
                get: function (identifier) {
                    return requestJSON('/satellites' + '/' + generalConfigData.jsonFile).then(function (generic) {
                        //Sets up the folder path for the domain object and the Root
                        if (identifier.key === generalConfigData.key) {
                            return {
                                identifier: identifier,
                                name: generic.name,
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
                                location: generalConfigData.namespace + ':' + generalConfigData.key
                            };
                        }
                    });
                }
            });


            //creates the composition provider from the json file coorisponding with the domain object
            openmct.composition.addProvider({
                //checks to see if the provider is able to be used for the domain object
                appliesTo: function (domainObject) {
                    return domainObject.identifier.namespace === (generalConfigData.namespace) &&
                        domainObject.type === 'folder';
                },
                //loads in the data from the coorisponding json file for the measurements to load in the composition provider
                load: function (domainObject) {
                    return requestJSON('/satellites' + '/' + generalConfigData.jsonFile)
                        .then(function (generic) {
                            return generic.measurements.map(function (m) {
                                return {
                                    namespace: generalConfigData.namespace,
                                    key: m.key
                                };
                            });
                        });
                }
            });

            //adds the data for the telemetry type stored in the config data to MCT
            openmct.types.addType(generalConfigData.type, {
                name: generalConfigData.typeName,
                description: generalConfigData.typeDescription,
                cssClass: generalConfigData.typeCssClass
            });


        }
    };
};

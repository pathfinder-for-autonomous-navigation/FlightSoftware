var configs = {
    battery: {
        namespace: 'bat.taxonomy',
        key: 'spacecraft',
        type: 'bat.telemetry',
        typeName: 'Battery Telemetry Point',
        typeDescription: 'Telemetry point on the Gomspace battery',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/battery.json'
    },

    dictionary:{
        namespace: 'sat.taxonomy',
        key: 'spacecraft',
        type: 'sat.telemetry',
        typeName: 'Satellite Telemetry Point',
        typeDescription: 'Telemetry point on the satellite to send data',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/dictionary.json'
    }

}

function getGeneric(j) {
    return http.get(j)
        .then(function (result) {
            return result.data;
        });
}


function GenericPlugin(config) {
    return  function install(openmct) {


        
        console.log(config);
        console.log(configs);


        if(config === "realtime"){
            
            console.log(configs);

            var socket = new WebSocket(location.origin.replace(/^http/, 'ws') + '/realtime/');
            var listener = {};
    
            socket.onmessage = function (event) {
                point = JSON.parse(event.data);
                if (listener[point.id]) {
                    listener[point.id](point);
                }
            };
    
            var provider = {
                supportsSubscribe: function (domainObject) {
                    var a = false;
                    for (const value of Object.values(configs)) {
                        if(domainObject.type === Object.values(value)[2]){
                            a = true;
                        }
                    }
                    return a;
                },
                subscribe: function (domainObject, callback) {
                    listener[domainObject.identifier.key] = callback;
                    socket.send('subscribe ' + domainObject.identifier.key);
                    return function unsubscribe() {
                        delete listener[domainObject.identifier.key];
                        socket.send('unsubscribe ' + domainObject.identifier.key);
                    };
                }
            };
    
            openmct.telemetry.addProvider(provider);
    
            
        }else if(config === "historical"){
            
        console.log(configs);

        var provider = {
            supportsRequest: function (domainObject) {
                var a = false;
                for (const value of Object.values(configs)) {
                    if(domainObject.type === Object.values(value)[2]){
                        a = true;
                    }
                }

                return a;

            },
            request: function (domainObject, options) {
                var url = '/history/' +
                    domainObject.identifier.key +
                    '?start=' + options.start +
                    '&end=' + options.end;

                return http.get(url)
                    .then(function (resp) {
                        return resp.data;
                    });
            }
        };

        openmct.telemetry.addProvider(provider);
        
        }else{


            var configData;
            var i = 0;
            for (const key of Object.keys(configs)) {
                console.log("entered for loop" + i+1 + "times");
	            console.log(key);
	            if(config === key){
                    console.log(Object.values(configs)[i]);
   		            configData = Object.values(configs)[i];
                }
  	            i++
            }

                console.log(configData);
                console.log(configData.namespace);

                openmct.objects.addRoot({
                    namespace: configData.namespace,
                    key: configData.key
                });
    
                openmct.objects.addProvider(configData.namespace, {
                    get: function (identifier) {
                        return getGeneric(configData.jsonFile).then(function (general) {
                            if (identifier.key ===  configData.key) {
                                return {
                                    identifier: identifier,
                                    name: general.name,
                                    type: 'folder',
                                    location: 'ROOT'
                                };
                            } else {
                                var measurement = general.measurements.filter(function (m) {
                                    return m.key === identifier.key;
                                })[0];
                                return {
                                    identifier: identifier,
                                    name: measurement.name,
                                    type: configData.type,
                                    telemetry: {
                                        values: measurement.values
                                    },
                                    location: configData.namespace + ':' + configData.key
                                };
                            }
                        });
                    }
                });
        
                openmct.composition.addProvider({
                    appliesTo: async function (domainObject) {
                        return domainObject.identifier.namespace === configData.namespace &&
                               domainObject.type === 'folder';
                    },
                    load: function (domainObject) {
                        return getGeneric(configData.jsonFile)
                            .then(function (general) {
                                return general.measurements.map(function (m) {
                                    return {
                                        namespace: configData.namespace,
                                        key: m.key
                                    };
                                });
                            });
                    }
                });
        
                openmct.types.addType(configData.type, {
                    name: configData.typeName,
                    description: configData.typeDescription,
                    cssClass: configData.typeCssClass
                });



        }
    };
};

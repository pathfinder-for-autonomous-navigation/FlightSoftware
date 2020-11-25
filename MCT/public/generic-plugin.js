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
//var configs = require('./plugin-configuration.js')
function getGeneric(j) {
    return http.get(j)
        .then(function (result) {
            return result.data;
        });
}


function GenericPlugin(config) {
    return  function install(openmct) {

        if(config === "realtime"){
            
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

            var generalConfigData;
            var i = 0;
            for (const key of Object.keys(configs)) {
	            if(config === key){
   		            generalConfigData = Object.values(configs)[i];
                }
  	            i++
            }

            openmct.objects.addRoot({
                namespace: generalConfigData.namespace,
                key: generalConfigData.key
            });
            openmct.objects.addProvider(generalConfigData.namespace, {
                get: function (identifier) {
                    return getGeneric(generalConfigData.jsonFile).then(function (generic) {
                        if (identifier.key === generalConfigData.key) {
                            return {
                                identifier: identifier,
                                name: generic.name,
                                type: 'folder',
                                location: 'ROOT'
                            };
                        } else {
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
    
            openmct.composition.addProvider({
                appliesTo: function (domainObject) {
                    return domainObject.identifier.namespace === generalConfigData.namespace &&
                           domainObject.type === 'folder';
                },
                load: function (domainObject) {
                    return getGeneric(generalConfigData.jsonFile)
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
    
            openmct.types.addType(generalConfigData.type, {
                name: generalConfigData.typeName,
                description: generalConfigData.typeDescription,
                cssClass: generalConfigData.typeCssClass
            });


        }
    };
};

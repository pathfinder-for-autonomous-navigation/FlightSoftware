var configs = {
    gomspace: {
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
    },

/*    pan:{
        namespace: 'pan.taxonomy',
        key: 'spacecraft',
        type: 'pan.telemetry',
        typeName: 'PAN Telemetry Point',
        typeDescription: 'Telemetry point for PAN namespace',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/pan.json'
    },
*/
    orbit:{
        namespace: 'orb.taxonomy',
        key: 'spacecraft',
        type: 'orb.telemetry',
        typeName: 'Orbit Telemetry Point',
        typeDescription: 'Telemetry point for the Orbit',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/orbit.json'
    },

    piksi:{
        namespace: 'piksi.taxonomy',
        key: 'spacecraft',
        type: 'piksi.telemetry',
        typeName: 'Pikis Telemetry Point',
        typeDescription: 'Telemetry point on the piksi',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/piksi.json'
    },

    attitude_estimator:{
        namespace: 'att.taxonomy',
        key: 'spacecraft',
        type: 'att.telemetry',
        typeName: 'Attitude Estimator Telemetry Point',
        typeDescription: 'Telemetry point for the Attitude Estimator',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/attitude_estimator.json' 
    },

    adcs_monitor:{
        namespace: 'adcsmon.taxonomy',
        key: 'spacecraft',
        type: 'adcsmon.telemetry',
        typeName: 'ADCS Monitor Telemetry Point',
        typeDescription: 'Telemetry point for the ADCS Monitor',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/adcs_monitor.json'
    },

    prop:{
        namespace: 'prop.taxonomy',
        key: 'spacecraft',
        type: 'prop.telemetry',
        typeName: 'Prop Telemetry Point',
        typeDescription: 'Telemetry point for Prop',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/prop.json'
    },

    adcs_command:{
        namespace: 'adcscomm.taxonomy',
        key: 'spacecraft',
        type: 'adcscomm.telemetry',
        typeName: 'ADCS Command Telemetry Point',
        typeDescription: 'Telemetry point for the ADCS Command',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/adcs_cmd.json'
    },

    adcs: {
        namespace: 'adcs.taxonomy',
        key: 'spacecraft',
        type: 'adcs.telemetry',
        typeName: 'ADCS Telemetry Point',
        typeDescription: 'Telemetry point for the ADCS',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/adcs.json'
    },

    radio: {
        namespace: 'radio.taxonomy',
        key: 'spacecraft',
        type: 'radio.telemetry',
        typeName: 'Radio Telemetry Point',
        typeDescription: 'Telemetry point for the Radio',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/radio.json'
    },

    docksys: {
        namespace: 'dock.taxonomy',
        key: 'spacecraft',
        type: 'dock.telemetry',
        typeName: 'Docksys Telemetry Point',
        typeDescription: 'Telemetry point for Docksys',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/docksys.json'
    },

    dcdc: {
        namespace: 'dcdc.taxonomy',
        key: 'spacecraft',
        type: 'dcdc.telemetry',
        typeName: 'DCDC Telemetry Point',
        typeDescription: 'Telemetry point for DCDC',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/dcdc.json'
    },
/*
    detumble_safety_factor: {
        namespace: 'detumble.taxonomy',
        key: 'spacecraft',
        type: 'detumble.telemetry',
        typeName: 'Detumbler Telemetry Point',
        typeDescription: 'Telemetry point for the Detumbler',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/detumble.json'
    },

    docking_timeout_limit:{
        namespace: 'docktimeout.taxonomy',
        key: 'spacecraft',
        type: 'docktimeout.telemetry',
        typeName: 'Dock Timeout Telemetry Point',
        typeDescription: 'Telemetry point for a Dock Timeout',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/docktimeout.json'
    },
*/
    fault_handler:{
        namespace: 'fault.taxonomy',
        key: 'spacecraft',
        type: 'fault.telemetry',
        typeName: 'Fault Handler Telemetry Point',
        typeDescription: 'Telemetry point for the Fault Handler',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/fault_handler.json'
    },

    trigger_dist:{
        namespace: 'trigger.taxonomy',
        key: 'spacecraft',
        type: 'trigger.telemetry',
        typeName: 'Trigger Distance Telemetry Point',
        typeDescription: 'Telemetry point for the Trigger Distance',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/trigger_dist.json'
    },

    downlink:{
        namespace: 'down.taxonomy',
        key: 'spacecraft',
        type: 'down.telemetry',
        typeName: 'Downlink Telemetry Point',
        typeDescription: 'Telemetry point for Downlink',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/downlink.json'
    },

    timing: {
        namespace: 'time.taxonomy',
        key: 'spacecraft',
        type: 'time.telemetry',
        typeName: 'Timing Telemetry Point',
        typeDescription: 'Telemetry point for timing',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/timing.json'
    },

    piksi_fh: {
        namespace: 'piksifh.taxonomy',
        key: 'spacecraft',
        type: 'piksifh.telemetry',
        typeName: 'Piksi_fh Telemetry Point',
        typeDescription: 'Telemetry point for the Piksi_fh',
        typeCssClass: 'icon-telemetry',
        jsonFile: '/piksi_fh.json'
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
                    return getGeneric('subsystems/'+generalConfigData.jsonFile).then(function (generic) {
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
                    return getGeneric('subsystems/'+generalConfigData.jsonFile)
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

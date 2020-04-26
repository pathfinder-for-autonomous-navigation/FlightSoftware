//This file contains all the plugins necessary for openMCT to handle our telemetry values

function getDictionary() {
    return http.get('/dictionary.json')
        .then(function (result) {
            return result.data;
        });
}

var objectProvider = {
    get: function (identifier) {
        return getDictionary().then(function (dictionary) {
            if (identifier.key === 'spacecraft') {
                return {
                    identifier: identifier,
                    name: dictionary.name,
                    type: 'folder',
                    location: 'ROOT'
                };
            } else {
                var measurement = dictionary.measurements.filter(function (m) {
                    return m.key === identifier.key;
                })[0];
                return {
                    identifier: identifier,
                    name: measurement.name,
                    type: 'sat.telemetry',
                    telemetry: {
                        values: measurement.values
                    },
                    location: 'sat.taxonomy:spacecraft'
                };
            }
        });
    }
};

var compositionProvider = {
    appliesTo: function (domainObject) {
        return domainObject.identifier.namespace === 'sat.taxonomy' &&
               domainObject.type === 'folder';
    },
    load: function (domainObject) {
        return getDictionary()
            .then(function (dictionary) {
                return dictionary.measurements.map(function (m) {
                    return {
                        namespace: 'sat.taxonomy',
                        key: m.key
                    };
                });
            });
    }
};

function DictionaryPlugin() {
    return function install(openmct) {
        openmct.objects.addRoot({
            namespace: 'sat.taxonomy',
            key: 'spacecraft'
        });

        openmct.objects.addProvider('sat.taxonomy', objectProvider);

        openmct.composition.addProvider(compositionProvider);

        openmct.types.addType('sat.telemetry', {
            name: 'Satellite Telemetry Point',
            description: 'Telemetry point on the satellite to send data',
            cssClass: 'icon-telemetry'
        });
    };
};

function getBattery() {
    return http.get('/battery.json')
        .then(function (result) {
            return result.data;
        });
}

var objectProvider1 = {
    get: function (identifier) {
        return getBattery().then(function (battery) {
            if (identifier.key === 'spacecraft') {
                return {
                    identifier: identifier,
                    name: battery.name,
                    type: 'folder',
                    location: 'ROOT'
                };
            } else {
                var measurement = battery.measurements.filter(function (m) {
                    return m.key === identifier.key;
                })[0];
                return {
                    identifier: identifier,
                    name: measurement.name,
                    type: 'bat.telemetry',
                    telemetry: {
                        values: measurement.values
                    },
                    location: 'bat.taxonomy:spacecraft'
                };
            }
        });
    }
};

var compositionProvider1 = {
    appliesTo: function (domainObject) {
        return domainObject.identifier.namespace === 'bat.taxonomy' &&
               domainObject.type === 'folder';
    },
    load: function (domainObject) {
        return getBattery()
            .then(function (battery) {
                return battery.measurements.map(function (m) {
                    return {
                        namespace: 'bat.taxonomy',
                        key: m.key
                    };
                });
            });
    }
};



function BatteryPlugin() {
    return function install(openmct) {
        openmct.objects.addRoot({
            namespace: 'bat.taxonomy',
            key: 'spacecraft'
        });
        openmct.objects.addProvider('bat.taxonomy', objectProvider1);

        openmct.composition.addProvider(compositionProvider1);

        openmct.types.addType('bat.telemetry', {
            name: 'Battery Telemetry Point',
            description: 'Telemetry point on the Gomspace battery',
            cssClass: 'icon-telemetry'
        });

      }
    }

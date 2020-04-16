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

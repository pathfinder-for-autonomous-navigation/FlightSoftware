function getGeneric(s) {
    return http.get(s)
        .then(function (result) {
            return result.data;
        });
}

function GenericPlugin(config) {
    return function install(openmct) {
        openmct.objects.addRoot({
            namespace: config.namespace,
            key: config.key
        });
        

        openmct.objects.addProvider(config.namespace, {
            appliesTo: function (domainObject) {
                return domainObject.identifier.namespace === 'sat.taxonomy' &&
                       domainObject.type === 'folder';
            },
            load: function (domainObject) {
                return getGeneric(config.jsonFile)
                    .then(function (generic) {
                        return generic.measurements.map(function (m) {
                            return {
                                namespace: config.namespace,
                                key: m.key
                            };
                        });
                    });
            }

        });

        openmct.composition.addProvider({
            get: function (identifier) {
                return getGeneric(config.jsonFile).then(function (generic) {
                    if (identifier.key === config.key) {
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
                            type: config.namespace,
                            telemetry: {
                                values: measurement.values
                            },
                            location: config.namespace + ':' + config.key
                        };
                    }
                });
            }
        });

        openmct.types.addType(config.type, {
            name: config.typeName,
            description: config.typeDescription,
            cssClass: config.typeCssClass
        });
    };
};
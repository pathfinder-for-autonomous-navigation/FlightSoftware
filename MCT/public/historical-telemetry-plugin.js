/**
 * Basic historical telemetry plugin.
 */

function HistoricalTelemetryPlugin(configs) {
    return function install (openmct) {
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
    }
}

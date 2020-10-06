/**
 * Basic Realtime telemetry plugin using websockets.
 */
function RealtimeTelemetryPlugin(configs) {
    return function (openmct) {
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
    }
}

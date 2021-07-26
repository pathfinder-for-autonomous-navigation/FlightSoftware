var express = require('express');

function RealtimeServer(spacecraft) {
    console.log("REALTIME")
    var router = express.Router();

    router.ws('/', function (ws) {
        console.log("hi")
        console.log(ws)
        var subscribed = {}; // Active subscriptions for this connection
        function notifySubscribers(point) {
            if (subscribed[point.id]) {
                ws.send(JSON.stringify(point));
            }
        }
        var unlisten = spacecraft.listen(notifySubscribers);
        
        var handlers = { // Handlers for specific requests
                subscribe: function (id) {
                    subscribed[id] = true;
                },
                unsubscribe: function (id) {
                    delete subscribed[id];
                }
            };

        

        // Listen for requests
        ws.on('message', function (message) {
            var parts = message.split(' '),
                handler = handlers[parts[0]];
            if (handler) {
                handler.apply(handlers, parts.slice(1));
            }
        });

        // Stop sending telemetry updates for this connection when closed
        ws.on('close', unlisten);
    });

    return router;
};

module.exports = RealtimeServer;

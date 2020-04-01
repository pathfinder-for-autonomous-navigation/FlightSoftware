/**
 * Basic implementation of a history and realtime server.
 */

var Spacecraft = require('./example-server/spacecraft');
var RealtimeServer = require('./example-server/realtime-server');
var HistoryServer = require('./example-server/history-server');
var StaticServer = require('./example-server/static-server');

var expressWs = require('express-ws');
var app = require('express')();
expressWs(app);

var spacecraft = new Spacecraft();
var realtimeServer = new RealtimeServer(spacecraft);
var historyServer = new HistoryServer(spacecraft);
var staticServer = new StaticServer();

app.use('/realtime', realtimeServer);
app.use('/history', historyServer);
app.use('/', staticServer);

var port = process.env.PORT || 9090

app.listen(port, function () {
    console.log('Open MCT hosted at http://localhost:' + port);
    console.log('History hosted at http://localhost:' + port + '/history');
    console.log('Realtime hosted at ws://localhost:' + port + '/realtime');
});

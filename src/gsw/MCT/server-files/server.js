/**
 * Basic implementation of a history and realtime server.
 */

var DummySpacecraft = require('./dummy_spacecraft');
var Spacecraft = require('./spacecraft');
var RealtimeServer = require('./realtime-server');
var HistoryServer = require('./history-server');

var expressWs = require('express-ws');
var express = require('express');
var app = require('express')();

expressWs(app);

if (process.argv[2] == "test") {
    var spacecraft = new DummySpacecraft();
}
else {
    var spacecraft = new Spacecraft();
}
var realtimeServer = new RealtimeServer(spacecraft);
var historyServer = new HistoryServer(spacecraft);

app.use('/realtime', realtimeServer);
app.use('/history', historyServer);
app.use('/openmct', express.static('node_modules/openmct/dist/'));
app.use('/', express.static("public"));

var port = process.env.PORT || 8080

app.listen(port, function () {
    console.log('Open MCT hosted at http://localhost:' + port);
    console.log('History hosted at http://localhost:' + port + '/history');
    console.log('Realtime hosted at ws://localhost:' + port + '/realtime');
});

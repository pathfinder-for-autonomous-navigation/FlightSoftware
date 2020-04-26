/**
 * Basic implementation of a history and realtime server.
 */

var Spacecraft = require('../server-files/spacecraft');
var RealtimeServer = require('./example-server/realtime-server');
var HistoryServer = require('./example-server/history-server');
var StaticServer = require('./example-server/static-server.js');

var expressWs = require('express-ws');
var express = require('express');
var app = require('express')();
expressWs(app);

var spacecraft = new Spacecraft();
var realtimeServer = new RealtimeServer(spacecraft);
var historyServer = new HistoryServer(spacecraft);
var staticServer = new StaticServer();


app.use('/history', historyServer);

app.use('/', staticServer);
app.use('/realtime', realtimeServer);

var port = process.env.PORT || 9090

app.listen(port, function () {
    console.log('Open MCT hosted at http://localhost:' + port);
    console.log('History hosted at http://localhost:' + port + '/history');
    console.log('Realtime hosted at ws://localhost:' + port + '/realtime');
});

/**
 * Basic implementation of a history and realtime server for MCT.
 */
var Telemetry= require('./telemetry');
var RealtimeServer = require('./boilerplate-mct-servers/realtime-server');
var HistoryServer = require('./boilerplate-mct-servers/history-server');
var expressWs = require('express-ws'); 
var express = require('express');
var fs = require('fs');
var https = require('https');

var privateKey  = fs.readFileSync('./key.pem', 'utf8');
var certificate = fs.readFileSync('./cert.pem', 'utf8');
var credentials = {key: privateKey, cert: certificate};

var app = express()
expressWs(app);

var spacecraft = new Telemetry();
var realtimeServer = new RealtimeServer(spacecraft);
var historyServer = new HistoryServer(spacecraft);

app.use('/realtime', realtimeServer);
app.use('/history', historyServer);
app.use('/openmct', express.static('node_modules/openmct/dist/'));
app.use('/', express.static("public"));

var httpsServer = https.createServer(credentials, app);
var port = process.env.PORT || 8080

httpsServer.listen(port, function () {
    console.log('Open MCT hosted at https://localhost:' + port);
    console.log('History hosted at https://localhost:' + port + '/history');
    console.log('Realtime hosted at wss://localhost:' + port + '/realtime');
});

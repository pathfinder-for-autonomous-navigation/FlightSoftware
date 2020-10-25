/**
 * Basic implementation of a history and realtime server.
 */
var Battery = require('./battery-telemetry');
//var DummySpacecraft = require('./dummy_spacecraft');
var Spacecraft = require('./spacecraft');
var RealtimeServer = require('./realtime-server');
var HistoryServer = require('./history-server');
var expressWs = require('express-ws');
var express = require('express');
var fs = require('fs');
var https = require('https');

var privateKey  = fs.readFileSync('./key.pem', 'utf8');
var certificate = fs.readFileSync('./cert.pem', 'utf8');
var credentials = {key: privateKey, cert: certificate};
var app = express()

expressWs(app);

if (process.argv[2] == "test") {
    var spacecraft = new Battery();
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

var httpsServer = https.createServer(credentials, app);
var port = process.env.PORT || 8080

httpsServer.listen(port, function () {
    console.log('Open MCT hosted at https://localhost:' + port);
    console.log('History hosted at https://localhost:' + port + '/history');
    console.log('Realtime hosted at ws://localhost:' + port + '/realtime');
});

const path = require('path')
const mct = require(path.join( __dirname, 'mct_secret.json'))
/**
 * Basic implementation of a history and realtime server for MCT.
 */

//get arguments
var myArgs = process.argv.slice(2);

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

//sets default config file to ci.json
if(myArgs[0] == undefined){
  myArgs[0] = "ptest/configs/hootl.json"
}

var spacecraft = new Telemetry(myArgs[0]);
var realtimeServer = new RealtimeServer(spacecraft);
var historyServer = new HistoryServer(spacecraft);

app.use((req, res, next) => {

  const auth = {login: mct.login.username, password: mct.login.password}

  const b64auth = (req.headers.authorization || '').split(' ')[1] || ''
  const [login, password] = Buffer.from(b64auth, 'base64').toString().split(':')

  if (login && password && login === auth.login && password === auth.password) {
    return next()
  }

  res.set('WWW-Authenticate', 'Basic realm="401"')
  res.status(401).send('Authentication required.')

})
app.use('/realtime', realtimeServer);
app.use('/history', historyServer);
app.use('/openmct', express.static('node_modules/openmct/dist/'));
app.use('/', express.static("public"));
if (mct.devices.follower.enabled == true && mct.devices.leader.enabled == true){
  app.get('/',function(req,res) {
    res.sendFile(path.join(__dirname, '../public/views/dual.html'));
  });
}else if (mct.devices.follower.enabled == false && mct.devices.leader.enabled == true){
  app.get('/',function(req,res) {
    res.sendFile(path.join(__dirname, '../public/views/leader.html'));
  });
}else if (mct.devices.follower.enabled == true && mct.devices.leader.enabled == false){
  app.get('/',function(req,res) {
    res.sendFile(path.join(__dirname, '../public/views/follower.html'));
  });
}else {
  app.use('/', express.static("public"));
}




var httpsServer = https.createServer(credentials, app);
var port = process.env.PORT || 8080

httpsServer.listen(port, function () {
    console.log('Open MCT hosted at https://localhost:' + port);
    console.log('History hosted at https://localhost:' + port + '/history');
    console.log('Realtime hosted at wss://localhost:' + port + '/realtime');
});

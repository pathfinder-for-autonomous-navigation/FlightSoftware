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
var ws = require('ws');

var privateKey  = fs.readFileSync('./key.pem', 'utf8');
var certificate = fs.readFileSync('./cert.pem', 'utf8');
var credentials = {key: privateKey, cert: certificate};

var app = express()

//sets default config file to mct_secret.json
if(myArgs[0] == undefined){
  myArgs[0] = "MCT/server-files/mct_secret.json"
}

var spacecraft = new Telemetry(myArgs[0]);
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

  


app.use('/history', historyServer);
app.use('/openmct', express.static('node_modules/openmct/dist/'));
app.use('/', express.static("public"));


if (mct.devices.follower.enabled == true && mct.devices.leader.enabled == true){
  app.use(express.static('public/', {index: 'dual.html'}))
}else if (mct.devices.follower.enabled == false && mct.devices.leader.enabled == true){
  app.use(express.static('public/', {index: 'leader.html'}))
}else if (mct.devices.follower.enabled == true && mct.devices.leader.enabled == false){
  app.use(express.static('public/', {index: 'follower.html'}))
}else{
  app.use(express.static('public/', {index: 'empty.html'}))
}




var httpsServer = https.createServer(credentials, app);
const socket = new ws.Server({server: httpsServer})

socket.on('connection', (ws, req) => {
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
})




var port = process.env.PORT || 8080

httpsServer.listen(port, function () {
    console.log('Open MCT hosted at https://localhost:' + port);
    console.log('History hosted at https://localhost:' + port + '/history');
    console.log('Realtime hosted at wss://localhost:' + port);
});

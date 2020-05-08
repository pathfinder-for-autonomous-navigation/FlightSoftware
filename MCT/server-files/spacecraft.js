/*
 Spacecraft.js simulates a small spacecraft generating telemetry.
*/
const url = require('url');
var request = require('request');
const axios = require('axios');

function Spacecraft() {
    this.state = {
        "batt.lvl": 77,
        "incoming": 0
    };
    this.history = {};
    this.listeners = [];
    Object.keys(this.state).forEach(function (k) {
        this.history[k] = [];
    }, this);

    setInterval(function () {
        this.updateState();
        this.generateTelemetry();
    }.bind(this), 1000);
    console.log("Spacecraft Launched")
    process.stdin.on('data', function () {
          this.generateTelemetry();
    }.bind(this));
};


/*  Posts example telemetry using HTTP post request to demonstrate
* elasticsearch integration
*
* if an error with the request is raised, prints the error in console
*/
axios.post('http://localhost:5000/telemetry', {
  field: "telemetry",
  imei: 123,
  value: 15
})
.catch((error) => {
  console.error("error in http POST")
})
//initializes global variable value to hold the numerical value of gsw telemetry
var value = "0";
Spacecraft.prototype.updateState = function () {
/*  HTTP get request accesses the example telemetry and sets spacecraft state to
 *   the value recieved by gsw
 */
/**NOTE: if console is printing out error code that means gsw server has not been
started correctly **/

  const myurl = url.parse('http://localhost:5000/search-es');
  var propertiesObject = { index:'statefield_report_123', field:'telemetry' };
  request({url:myurl, qs:propertiesObject}, function(err, response, body) {//make anonymous function part of the class
    if(err) { console.log("error in http GET"); return; }
    value = body;
  });

  this.state["incoming"] = parseInt(value,10);
  this.state["batt.lvl"] = Math.max(0,this.state["batt.lvl"] - 1);
};

/**
 * Takes a measurement of spacecraft state, stores in history, and notifies
 * listeners.
 */
Spacecraft.prototype.generateTelemetry = function () {
    var timestamp = Date.now(), sent = 0;
    Object.keys(this.state).forEach(function (id) {
        var state = { timestamp: timestamp, value: this.state[id], id: id};
        this.notify(state);
        this.history[id].push(state);
    }, this);
};

Spacecraft.prototype.notify = function (point) {
    this.listeners.forEach(function (l) {
        l(point);
    });
};

Spacecraft.prototype.listen = function (listener) {
    this.listeners.push(listener);
    return function () {
        this.listeners = this.listeners.filter(function (l) {
            return l !== listener;
        });
    }.bind(this);
};

module.exports = function () {
    return new Spacecraft()
};

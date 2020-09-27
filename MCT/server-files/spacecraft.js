/*
 Spacecraft.js simulates a small spacecraft generating telemetry.
*/
const url = require('url');
var request = require('request');
const axios = require('axios')



function Spacecraft(){
    this.state = {
        "batt.lvl": 77,
        "incoming": 0
    };
    this.history = {};
    this.listeners = [];
    Object.keys(this.state).forEach(function (k) {
        this.history[k] = [];
    }, this);
    console.log(this.history);

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

})
//initializes global variable value to hold the numerical value of gsw telemetry
var value = "0";
Spacecraft.prototype.updateState = function () {
/*  HTTP get request accesses the example telemetry and sets spacecraft state to
 *   the value recieved by gsw
 */
/**NOTE: if console is printing out error code that means gsw server has not been
started correctly **/
axios.get('http://localhost:5000/search-es',{
  params:{
    index:'statefield_report_123',
    field: 'telemetry'
    }
})
.then( function (response) {
  value = response.data;
})
.catch( function (error){
  console.log("error in http GET");
})
  this.state["incoming"] = parseInt(value,10);
  this.state["batt.lvl"] = Math.max(0,this.state["batt.lvl"] - 1);
};
function notify (point,listener){
    listener.forEach(function (l) {
        l(point);
  });
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

Spacecraft.prototype.notify = notify(point,this.listeners);


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
};}

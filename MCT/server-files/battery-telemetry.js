/*
 battery-telemetry.js simulates the gomspace generating telemetry.
*/
const variables = require('./state-variables.js')

const axios = require('axios');
function Battery() {

    this.state = {
        "gomspace.modes": variables.modes,
        "gomspace.counters": variables.counters,
        "gomspace.commands": variables.commands,
        "gomspace.curin": variables.curin,
        "gomspace.curout": variables.curout,
        "gomspace.get_hk": variables.get_hk,
        "gomspace.low_batt": variables.low_batt,
        "gomspace.output": variables.output,
        "gomspace.power_cycle_output": variables.power_cycle_output,
        "gomspace.pv": variables.pv,
        "gomspace.temp": variables.temp,
        "gomspace.vboost": variables.vboost,
        "gomspace.vbatt": variables.vbatt,
        "gomspace.batt_threshold": variables.batt_threshold,
        "gomspace.cursun": variables.cursun,
        "gomspace.cursys": variables.cursys,
        "gomspace.bootcause": variables.bootcause
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

};

Battery.prototype.updateState = function () {
  this.state["gomspace.counters"].counter_boot = Math.min(4294967295,this.state["gomspace.counters"].counter_boot+1);
  this.state["gomspace.vbatt"].output1 = 24;
};

/**
 * Takes a measurement of spacecraft state, stores in history, and notifies
 * listeners.
 */
Battery.prototype.generateTelemetry = function () {
    var timestamp = Date.now(), sent = 0;
    //make two cases one that updates objects and one that directly updates field
    Object.keys(this.state).forEach(function (id) {//for each telemetry point in array this.state execute the following function
      if(typeof(this.state[id]) == 'object'){
        var state = { timestamp: timestamp, id: id};//for each field in the telemetry point


        for (const output in this.state[id]){
          state[output] = this.state[id][output];
        }
        this.notify(state);
        this.history[id].push(state);
      }else{
        var state = { timestamp: timestamp, value: this.state[id], id: id};
        this.notify(state);
        this.history[id].push(state);
      }
    }, this);

};

Battery.prototype.notify = function (point) {
    this.listeners.forEach(function (l) {
        l(point);
    });
};

Battery.prototype.listen = function (listener) {
    this.listeners.push(listener);
    return function () {
        this.listeners = this.listeners.filter(function (l) {
            return l !== listener;
        });
    }.bind(this);
};

module.exports = function () {
    return new Battery()
};

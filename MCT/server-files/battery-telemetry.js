/*
 battery-telemetry.js simulates the gomspace generating telemetry.
*/
const variables = require('./state-variables.js')
const url = require('url');
const request = require('request');
const axios = require('axios')
var searchURl = 'http://localhost:5000/search-es';
var searchIndex = 'statefield_report_123';
function Battery() {
    //This state function takes in initial values from the state-variables.js file
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
    /*function addtoHistory(key){
      console.log(this.state.key)

    }*/

    setInterval(function () {
        this.updateState();
        this.generateTelemetry();
    }.bind(this), 1000);
    console.log("Spacecraft Launched")

};
/**
*   Tests the functionality of updateState by incrementing the counter boot time
*   and setting the battery voltage to 24
**/
Battery.prototype.updateState = function () {
  
  Object.keys(this.state).forEach(function (id) {

    if(typeof(this.state[id]) == 'object'){//if state is an object
      
      Object.keys(this.state.id).forEach(function (subId){
        this.state.id.subId = getValue(searchURl, searchIndex, subId);
      })



    }else{// if state is a primitive type
      this.state.id = getValue(searchURl, searchIndex, id);
    }

  }, this);

};

Battery.prototype.getValue = function(url, i, f){
  request(url, {
    params: {
      index: i, 
      field: f
    }
  }, (error, response, body) =>{
    if(error){console.log("error in http GET");}
    if(response.statusCode == 200){
      return body;
    }
  });
};


/**
 * Takes a measurement of battery state, determines its type, stores in history, and notifies
 * listeners.
 *
 *This method has two cases:
 * - if the state is an object, meaning it has substates (output1, output2, etc.)
 *   then generate telemetry for each substate
 * - if the state is a primitive type, int,bool, or char then generate Telemetry
 *   for the state value directly
 */
Battery.prototype.generateTelemetry = function () {
    var timestamp = Date.now(), sent = 0;
    //make two cases one that updates objects and one that directly updates field
    Object.keys(this.state).forEach(function (id) {

      if(typeof(this.state[id]) == 'object'){//if state is an object

        var telempoint = { timestamp: timestamp, id: id};
        for (const output in this.state[id]){
          telempoint[output] = this.state[id][output];
        }
        this.notify(telempoint);
        this.history[id].push(telempoint);

      }else{// if state is a primitive type

        var telempoint = { timestamp: timestamp, value: this.state[id], id: id};

        this.notify(telempoint);
        this.history[id].push(telempoint);
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

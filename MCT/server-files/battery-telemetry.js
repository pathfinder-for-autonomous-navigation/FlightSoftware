/*
 battery-telemetry.js simulates the gomspace generating telemetry.
*/
const variables = require('./state-variables.js')
const url = require('url');
const request = require('request');
const { isObject } = require('util');
var searchURl = 'http://localhost:5000/search-es';
var searchIndex = 'statefield_report_flightcontroller';
function Battery() {
    //This state function takes in initial values from the state-variables.js file
    this.state = {
          //"gomspace.modes": variables.modes,
        "gomspace.battmode": variables.modes.battmode,
        "gomspace.heater": variables.modes.heater,
        "gomspace.pptmode": variables.modes.pptmode,
          //"gomspace.counters": variables.counters,
        "gomspace.counter_boot": variables.counters.counter_boot,
        "gomspace.counter_wdt_i2c": variables.counters.counter_wdt_i2c,
        "gomspace.wdt_i2c_time_left": variables.counters.wdt_i2c_time_left,
          //"gomspace.commands": variables.commands,
        "gomspace.counter_reset_cmd": variables.commands.counter_reset_cmd,
        "gomspace.gs_reboot_cmd": variables.commands.gs_reboot_cmd,
        "gomspace.gs_reset_cmd": variables.commands.gs_reset_cmd,
        "gomspace.heater_cmd": variables.commands.heater_cmd,
        "gomspace.pptmode_cmd": variables.commands.pptmode_cmd,
        "gomspace.curin": variables.curin,
        "gomspace.curout": variables.curout,
          //"gomspace.get_hk": variables.get_hk,
        //"gomspace.get_hk":variables.get_hk.get_hk,
        //"gomspace.override":variables.get_hk.override,
        //"gomspace.persistence":variables.get_hk.persistence,
        //"gomspace.suppress":variables.get_hk.suppress,
        //"gomspace.unsignal":variables.get_hk.unsignal,
          //"gomspace.low_batt": variables.low_batt,
        //"gomspace.low_batt":variables.low_batt.low_batt,
        //"gomspace.override":variables.low_batt.override,
        //"gomspace.persistence":variables.low_batt.persistence,
        //"gomspace.suppress":variables.low_batt.suppress,
        //"gomspace.unsignal":variables.low_batt.unsignal,
        //"gomspace.output": variables.output,
          //"gomspace.power_cycle_output": variables.power_cycle_output,
        "gomspace.power_cycle_output1_cmd": variables.power_cycle_output.power_cycle_output1_cmd,
        "gomspace.power_cycle_output2_cmd": variables.power_cycle_output.power_cycle_output2_cmd,
        "gomspace.power_cycle_output3_cmd": variables.power_cycle_output.power_cycle_output3_cmd,
        "gomspace.power_cycle_output4_cmd": variables.power_cycle_output.power_cycle_output4_cmd,
        "gomspace.power_cycle_output5_cmd": variables.power_cycle_output.power_cycle_output5_cmd,
        "gomspace.power_cycle_output6_cmd": variables.power_cycle_output.power_cycle_output6_cmd,
          //"gomspace.pv": variables.pv,
        "gomspace.pv1_cmd": variables.pv.pv1_cmd,
        "gomspace.pv2_cmd": variables.pv.pv2_cmd,
        "gomspace.pv3_cmd": variables.pv.pv3_cmd,
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
Battery.prototype.updateState = async function () {
  
  Object.keys(this.state).forEach(async function (id) {

    if(typeof(this.state[id]) == 'object'){//if state is an object
      
      Object.keys(this.state[id]).forEach(async function (subId){
        let res = await this.getValue(searchURl, searchIndex, id + '.' + subId);
        console.log("test");
        console.log(res);
        (this.state[id])[subId] = res;
      },this)



    }else{// if state is a primitive type
      let res = await this.getValue(searchURl, searchIndex, id);
      console.log("test non object");
      console.log(res);
      this.state[id] = res;
    }

  }, this);

};

Battery.prototype.getValue = async function(myUrl, i, f){
  
  var propertiesObject = { index: i, field:f };

  let p = new Promise(function(resolve, reject){
    request({url: myUrl, qs:propertiesObject}, function(err, response, body) {//make anonymous function part of the class
      console.log("Get response: " + response.statusCode);
      console.log(myUrl)
      console.log(f)
      console.log(body);
      if(!err && response.statusCode == 200) { resolve(body);}
      else{ reject(err); }
    });
  });
  return await p;
  }



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

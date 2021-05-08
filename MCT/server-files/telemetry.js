/*
telemetry.js pulls data for every field initialized in './state-variables.js'
It then generates it into a form that can be understood by OpenMCT
Then it adds it to the hisorical and realtime servers through the history and listener fields.
*/

/**
 * This object, imported from './state-variables.js', holds all of the inital values of telemetry points for every subsystem/domain object
 */
const variables = require('./state-variables.js')
const request = require('request');
var path = require('path');
const { config } = require('process');
const { stat } = require('fs');

/**
 * The URL of the Elastic Search database
 */
var searchURl = 'http://localhost:5000/search-es';

/**
 *  constructor initializing and then calling for the generation and updating of all telemetry points from all subsytems and domains
 */
function Telemetry(configuration) {

  var FlightSoftware = path.resolve(__dirname, '../..')
  var config_file = FlightSoftware + "/" + configuration
  var config_json = require(config_file);
  /**
  * The index of the Elastic Search database
  */
  if (config_json.devices.length == 1) {
    this.followerIndex = 'statefield_report_' + config_json.devices[0].imei
    this.singleSat = true
  }
  else if (config_json.devices.length > 1) {
    this.singleSat = false
    let deviceOneFilled = false
    let deviceTwoFilled = false
    if (config_json.devices[0].name.indexOf("Leader") != -1) {
      this.leaderIndex = 'statefield_report_' + config_json.devices[0].imei
    }
    else if (config_json.devices[0].name.indexOf("Follower") != -1) {
      this.followerIndex = 'statefield_report_' + config_json.devices[0].imei
    }
    if (config_json.devices[1].name.indexOf("Leader") != -1 && this.leaderIndex == undefined) {
      this.leaderIndex = 'statefield_report_' + config_json.devices[1].imei
    }
    else if (config_json.devices[1].name.indexOf("Follower") != -1 && this.followerIndex == undefined) {
      this.followerIndex = 'statefield_report_' + config_json.devices[1].imei
    }

  }
  else {
    throw "Malformed: There are no devices or radios in this config file"
  }
  //This state function takes in initial values from the state-variables.js file
  this.initialState = variables;

  //all of the current values for the telemetry
  this.follower_state = {};
  this.leader_state = {};

  //creates an entry in state for every variable in './state-variables'
  Object.entries(this.initialState).forEach(function ([statesSubsystem, v]) {
    if (typeof v === 'object' && v !== null) {
      Object.entries(v).forEach(function ([k, v]) {
        if (typeof v === 'object' && v !== null) {
          Object.entries(v).forEach(function ([field, v]) {
            let key = statesSubsystem + '.' + k + '.' + field;
            this.follower_state[('follower_' + key)] = v;
            this.leader_state[('leader_' + key)] = v;
          }, this);
        }
        else {
          let key = statesSubsystem + '.' + k;
          this.follower_state[('follower_' + key)] = v;
          this.leader_state[('leader_' + key)] = v;
        }
      }, this);
    }
    else {
      let key = statesSubsystem;
      this.follower_state[('follower_' + key)] = v;
      this.leader_state[('leader_' + key)] = v;
    }
  }, this);

  //all of the historical telemetry data
  this.history = {};

  //the listeners for the real time telemetry
  this.listeners = [];

  //adds all the initial follower values to history
  Object.entries(this.follower_state).forEach(function ([k, v]) {
    this.history[k] = [];
  }, this);

  //adds all the initial leader values to history
  Object.entries(this.leader_state).forEach(function ([k, v]) {
    this.history[k] = [];
  }, this);


  //updates the states, generates the realtime listers/notifications and historical telemetry ever 1 second.
  setInterval(function () {
    this.updateState();
    this.generateTelemetry();
  }.bind(this), 5000);

  console.log("Now reading spacecraft telemetry from leader and follower")

};

function numberCommas(s) {
  if (typeof s == "string") {
    return s.split(",").length - 1
  }
  else {
    return 0
  }
}

function getCoord(s, num) {
  if (num == 1) {
    let i = s.indexOf(',')
    if (i == -1) {
      return s
    } else {
      return s.substring(0, i)
    }
  }
  else {
    let i = s.indexOf(',')
    if (i == -1) {
      return s
    } else {
      return getCoord(s.substring(i + 1), num - 1)
    }
  }
}

/**
*   Updates the state fields with the new values in the Elastic Search database.<br>
*
*   This method has two cases:
* - if the state is an object, meaning it has substates (output1, output2, etc.)
*   then update the states object for each substate
* - if the state is a primitive type, int,bool, or char then update the state
*   for the state value directly
**/
Telemetry.prototype.updateState = async function () {
  Object.keys(this.follower_state).forEach(async function (id) {

    //if the value for the key of the state entry is an object
    if (typeof (this.follower_state[id]) == 'object') {

      Object.keys(this.follower_state[id]).forEach(async function (subId) {
        new_id = id.substr(id.indexOf('_') + 1);
        //send a request to Elastic Search for the field
        let res = await this.getValue(searchURl, this.followerIndex, new_id + '.' + subId);
        (this.follower_state[id])[subId] = res;//update state
      }, this)

    }
    //if the value for the key of the state entry is a primitive
    else {
      new_id = id.substr(id.indexOf('_') + 1);
      //send a request to Elastic Search for the field
      let res = await this.getValue(searchURl, this.followerIndex, new_id);
      this.follower_state[id] = res;//update state
    }

  }, this);
  if (this.singleSat == false) {
    Object.keys(this.leader_state).forEach(async function (id) {

      //if the value for the key of the state entry is an object
      if (typeof (this.leader_state[id]) == 'object') {

        Object.keys(this.leader_state[id]).forEach(async function (subId) {
          new_id = id.substr(id.indexOf('_') + 1);
          //send a request to Elastic Search for the field
          let res = await this.getValue(searchURl, this.leaderIndex, new_id + '.' + subId);
          (this.leader_state[id])[subId] = res;//update state
        }, this)

      }
      //if the value for the key of the state entry is a primitive
      else {
        new_id = id.substr(id.indexOf('_') + 1);
        //send a request to Elastic Search for the field 
        let res = await this.getValue(searchURl, this.leaderIndex, new_id);
        this.leader_state[id] = res;//update state
      }

    }, this);
  }

};

/**
 * 
 * Requests the data from Elastic Search for field f
 * 
 * @param {*} myUrl the URL for the Elastic Search database
 * @param {*} i  the index of the Elastic Search database
 * @param {*} f the field that's value is being requested
 */
Telemetry.prototype.getValue = async function (myUrl, i, f) {

  //properties of the search
  var propertiesObject = { index: i, field: f };

  let p = new Promise(function (resolve, reject) {

    //requests URL based on properties
    request({ url: myUrl, qs: propertiesObject }, function (err, response, body) {
      if (!err && response.statusCode == 200) { resolve(body); }
      else { reject(err); }
    });
  });
  return await p;
};



/**
 * Takes a measurement of all domain object's states
, determines its type, stores in history, and notifies
 * listeners.
 *
 *This method has two cases:
 * - if the state is an object, meaning it has substates (output1, output2, etc.)
 *   then generate telemetry for each substate
 * - if the state is a primitive type, int,bool, or char then generate Telemetry
 *   for the state value directly
 */
Telemetry.prototype.generateTelemetry = function () {
  var timestamp = Date.now(), sent = 0;
  //make two cases one that updates objects and one that directly updates field

  Object.keys(this.follower_state).forEach(function (id) {

      
      //generate telemetry point primitve state
      let answer = this.follower_state[id]
      if (answer == 'false') {
        var telempoint = { timestamp: timestamp, id: id };
        telempoint['value'] = 0
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);
      } else if (answer == 'true') {
        var telempoint = { timestamp: timestamp, id: id };
        telempoint['value'] = 1
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);id
      } else if (numberCommas(answer) == 3) {
        var telempoint = { timestamp: timestamp, id: id };
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);

        var telempointx = { timestamp: timestamp, id: 'follower_x_' + id.substring(9) };
        telempointx['value'] = getCoord(answer, 1)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempointx);
        if(this.history['follower_x_' + id.substring(9)] == undefined){
        this.history['follower_x_' + id.substring(9)] = []
        }
        this.history['follower_x_' + id.substring(9)].push(telempointx);

        var telempointy = { timestamp: timestamp, id: 'follower_y_' + id.substring(9) };
        telempointy['value'] = getCoord(answer, 2)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempointy);
        if(this.history['follower_y_' + id.substring(9)] == undefined){
        this.history['follower_y_' + id.substring(9)] = []
        }
        this.history['follower_y_' + id.substring(9)].push(telempointy);

        var telempointz = { timestamp: timestamp, id: 'follower_z_' + id.substring(9) };
        telempointz['value'] = getCoord(answer, 3)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempointz);
        if(this.history['follower_z_' + id.substring(9)] == undefined){
        this.history['follower_z_' + id.substring(9)] = []
        }
        this.history['follower_z_' + id.substring(9)].push(telempointz);

      } else if (numberCommas(answer) == 4) {
        var telempoint = { timestamp: timestamp, id: id };
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);

        var telempointa = { timestamp: timestamp, id: 'follower_a_' + id.substring(9) };
        telempointa['value'] = getCoord(answer, 1)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempointa);
        if(this.history['follower_a_' + id.substring(9)] == undefined){
        this.history['follower_a_' + id.substring(9)] = []
        }
        this.history['follower_a_' + id.substring(9)].push(telempointa);

        var telempointb = { timestamp: timestamp, id: 'follower_b_' + id.substring(9) };
        telempointb['value'] = getCoord(answer, 2)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempointb);
        if(this.history['follower_b_' + id.substring(9)] == undefined){
        this.history['follower_b_' + id.substring(9)] = []
        }
        this.history['follower_b_' + id.substring(9)].push(telempointb);

        var telempointc = { timestamp: timestamp, id: 'follower_c_' + id.substring(9) };
        telempointc['value'] = getCoord(answer, 3)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempointc);
        if(this.history['follower_c_' + id.substring(9)] == undefined){
        this.history['follower_c_' + id.substring(9)] = []
        }
        this.history['follower_c_' + id.substring(9)].push(telempointc);
        
        var telempointd = { timestamp: timestamp, id: 'follower_d_' + id.substring(9) };
        telempointd['value'] = getCoord(answer, 4)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempointd);
        if(this.history['follower_d_' + id.substring(9)] == undefined){
        this.history['follower_d_' + id.substring(9)] = []
        }
        this.history['follower_d_' + id.substring(9)].push(telempointd);

      } else {
        var telempoint = { timestamp: timestamp, id: id };
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);
      }


      

  }, this);

  if (this.singleSat == false) {
    Object.keys(this.leader_state).forEach(function (id) {

      //generate telemetry point primitve state
      let answer = this.leader_state[id]
      if (answer == 'false') {
        var telempoint = { timestamp: timestamp, id: id };
        telempoint['value'] = 0
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);
      } else if (answer == 'true') {
        var telempoint = { timestamp: timestamp, id: id };
        telempoint['value'] = 1
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);id
      } else if (numberCommas(answer) == 3) {
        var telempoint = { timestamp: timestamp, id: id };
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);

        var telempointx = { timestamp: timestamp, id: 'leader_x_' + id.substring(7) };
        telempointx['value'] = getCoord(answer, 1)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempointx);
        if(this.history['leader_x_' + id.substring(7)] == undefined){
          
        this.history['leader_x_' + id.substring(7)] = []
        }
        this.history['leader_x_' + id.substring(7)].push(telempointx);

        var telempointy = { timestamp: timestamp, id: 'leader_y_' + id.substring(7) };
        telempointy['value'] = getCoord(answer, 2)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempointy);
        if(this.history['leader_y_' + id.substring(7)] == undefined){
        this.history['leader_y_' + id.substring(7)] = []
        }
        this.history['leader_y_' + id.substring(7)].push(telempointy);

        var telempointz = { timestamp: timestamp, id: 'leader_z_' + id.substring(7) };
        telempointz['value'] = getCoord(answer, 3)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempointz);
        if(this.history['leader_z_' + id.substring(7)] == undefined){
        this.history['leader_z_' + id.substring(7)] = []
        }
        this.history['leader_z_' + id.substring(7)].push(telempointz);

      } else if (numberCommas(answer) == 4) {
        var telempoint = { timestamp: timestamp, id: id };
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);

        var telempointa = { timestamp: timestamp, id: 'leader_a_' + id.substring(7) };
        telempointa['value'] = getCoord(answer, 1)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempointa);
        if(this.history['leader_a_' + id.substring(7)] == undefined){
        this.history['leader_a_' + id.substring(7)] = []
        }
        this.history['leader_a_' + id.substring(7)].push(telempointa);

        var telempointb = { timestamp: timestamp, id: 'leader_b_' + id.substring(7) };
        telempointb['value'] = getCoord(answer, 2)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempointb);
        if(this.history['leader_b_' + id.substring(7)] == undefined){
        this.history['leader_b_' + id.substring(7)] = []
        }
        this.history['leader_b_' + id.substring(7)].push(telempointb);

        var telempointc = { timestamp: timestamp, id: 'leader_c_' + id.substring(7) };
        telempointc['value'] = getCoord(answer, 3)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempointc);
        if(this.history['leader_c_' + id.substring(7)] == undefined){
        this.history['leader_c_' + id.substring(7)] = []
        }
        this.history['leader_c_' + id.substring(7)].push(telempointc);
        
        var telempointd = { timestamp: timestamp, id: 'leader_d_' + id.substring(7) };
        telempointd['value'] = getCoord(answer, 4)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempointd);
        if(this.history['leader_d_' + id.substring(7)] == undefined){
        this.history['leader_d_' + id.substring(7)] = []
        }
        this.history['leader_d_' + id.substring(7)].push(telempointd);

      } else {
        var telempoint = { timestamp: timestamp, id: id };
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);
      }

    }, this);
  }
};

/**
 * Notifies the realtime server of a new telemetry point
 * 
 * @param {*} point The telelmetry point to notify an update to to the realtime server for
 */
Telemetry.prototype.notify = function (point) {
  this.listeners.forEach(function (l) {
    l(point);
  });
};

/**
 * Adds and sets up the listener for realtime telemetry
 * 
 * @param {*} listener The lister being added
 */
Telemetry.prototype.listen = function (listener) {
  //adds the listener
  this.listeners.push(listener);
  return function () {
    this.listeners = this.listeners.filter(function (l) {
      return l !== listener;
    });
  }.bind(this);
};

//exports the telemetry function for use in './server.js'
module.exports = function (config) {
  return new Telemetry(config)
};

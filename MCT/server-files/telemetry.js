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
  * It uses ptest configs currently to get the data necessary to know the imei number
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

/**
 * 
 * @param {*} s a string
 * @returns the number of commas in string [s]
 */
function numberCommas(s) {
  if (typeof s == "string") {
    return s.split(",").length - 1
  }
  else {
    return 0
  }
}

/**
 * 
 * @param {*} s a string
 * @param {*} num an integer
 * @returns the [num]'th term of [s]
 */
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
  //follower value updater
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
  //leader value updater
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
 * Adds the resultant coordinate to history and realtime 
 * 
 * @param {*} id the id of the telempoint
 * @param {*} answer the resultant value recieved from elasticsearch
 * @param {*} satellite the satellite name in a string form 
 * @param {*} coord_name the name of the coordinate point
 * @param {*} coord the coordinate location that we want to get from the elasticsearch value
 */
Telemetry.prototype.generateTelemetryCoordinate = function(id, answer, satellite, coord_name, coord){
        var timestamp = Date.now(), sent = 0;
        
        substringid = id.substring(satellite.length + 1)
        new_id = satellite + '_' + coord_name + '_' + substringid
        var telempoint = { timestamp: timestamp, id: new_id };
        //set value follower_
        telempoint['value'] = getCoord(answer, coord)
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        if(this.history[new_id] == undefined){ //makes sure it is not undefined
        this.history[new_id] = []
        }
        this.history[new_id].push(telempoint);

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

  //follower telemetry generation
  Object.keys(this.follower_state).forEach(function (id) {

      
      //generate telemetry point primitve state
      let answer = this.follower_state[id]

      //test if boolean false
      if (answer == 'false') {
        //create telem point
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = 0
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);

        
      } 
      //test if boolean false
      else if (answer == 'true') {
        //create telem point
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = 1
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);id
      } 
      //test if vector
      else if (numberCommas(answer) == 3) {
        //create raw vector telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);

        this.generateTelemetryCoordinate(id, answer, 'follower', 'x', 1)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'y', 2)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'z', 3)

      } 
      //check if quaternion
      else if (numberCommas(answer) == 4) {
        //create raw Quaternion telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);


        this.generateTelemetryCoordinate(id, answer, 'follower', 'a', 1)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'b', 2)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'c', 3)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'd', 4)

      } 
      //regular numerical data
      else {
        //create telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);
      }


      

  }, this);

  //leader telemetry generation
  if (this.singleSat == false) {
    Object.keys(this.leader_state).forEach(function (id) {

      //recieve current raw data value
      let answer = this.leader_state[id]

      //check if bool false
      if (answer == 'false') {

        //create telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = 0
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);
      } 
      //check if bool true
      else if (answer == 'true') {
        //create telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = 1
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);id
      } 
      //check if vector
      else if (numberCommas(answer) == 3) {
        //create raw vector telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);

        this.generateTelemetryCoordinate(id, answer, 'leader', 'x', 1)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'y', 2)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'z', 3)

      } 
      //check if quaternion
      else if (numberCommas(answer) == 4) {

        //create raw Quaternion telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        this.history[id].push(telempoint);

        this.generateTelemetryCoordinate(id, answer, 'leader', 'a', 1)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'b', 2)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'c', 3)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'd', 4)

      } 
      //regular numerical data
      else {
        //create telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
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

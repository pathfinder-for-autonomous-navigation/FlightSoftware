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

var timeSearchURL = 'http://localhost:5000/time-search-es'

/**
 *  constructor initializing and then calling for the generation and updating of all telemetry points from all subsytems and domains
 */
function Telemetry(configuration) {

  /**
   * The config file (defaults to mct_secret.json)
   */
  var FlightSoftware = path.resolve(__dirname, '../..')
  var config_file = FlightSoftware + "/" + configuration
  var config_json = require(config_file);

  /**
  * The indexes of the Elastic Search database
  */
  try{
  this.leaderIndex = 'statefield_report_' + config_json.devices.leader.imei
  this.followerIndex = 'statefield_report_' + config_json.devices.follower.imei
  this.leader_enabled = config_json.devices.leader.enabled
  this.follower_enabled = config_json.devices.follower.enabled
  } catch {
    console.log("Invalid MCT Configuration File")
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

  //the listeners for the real time telemetry
  this.listeners = [];


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
 * This Function recieves an array of MCT ready data processed to be directly inputted into the webclient. 
 * This data comes from between the starting and ending points and is the id parameter
 * @param {*} start starting point of the time range
 * @param {*} end ending point of the time range
 * @param {*} id the field being searched for
 * @returns 
 */
Telemetry.prototype.ReceiveTelemetry = async function(start, end, id){
  let new_id = ''
  let receiverIndex = ''

  //checks if the object is part of the follower and sets the corrisponding index
  if(id.startsWith("follower_")){
    new_id = id.substring(9, id.length)
    receiverIndex = this.followerIndex

  }
  //checks if the object is part of the leader and sets the corrisponding index
  else if(id.includes("leader_")){
    new_id = id.substring(7, id.length)
    receiverIndex = this.leaderIndex
  }

  let commas = -1
  let position = -1

  //checks if the object is a quaternion and if so sets the specific number of commas and position of element for later parsing
  if(new_id.startsWith("a_")){
    commas = 4
    position = 1
    new_id = new_id.substring(2, new_id.length)
  } 
  else if(new_id.startsWith("b_")){
    commas = 4
    position = 2
    new_id = new_id.substring(2, new_id.length)
  }
  else if(new_id.startsWith("c_")){
    commas = 4
    position = 3
    new_id = new_id.substring(2, new_id.length)
  }
  else if(new_id.startsWith("d_")){
    commas = 4
    position = 4
    new_id = new_id.substring(2, new_id.length)
  }

  //checks if the object is a vector and if so sets the specific number of commas and position of element for later parsing
  else if(new_id.startsWith("x_")){
    commas = 3
    position = 1
    new_id = new_id.substring(2, new_id.length)
  }
  else if(new_id.startsWith("y_")){
    commas = 3
    position = 2
    new_id = new_id.substring(2, new_id.length)
  }
  else if(new_id.startsWith("z_")){
    commas = 3
    position = 3
    new_id = new_id.substring(2, new_id.length)
  }

  //Request the data with the specified starting and ending time from the time-search endpoint on the webservice
  let valueArray = await this.getValue(timeSearchURL, {
    index: receiverIndex, 
    field: new_id, 
    start: (new Date(start)).toISOString().split(':').join('%3A'), 
    end: (new Date(end)).toISOString().split(':').join('%3A')
  })
  valueArray = JSON.parse(valueArray)

  //process the raw data to convert it into the correct coordinate or MCT interpreted value
  let processedArray = valueArray.map( point =>{
    
    let id_val = point['value']

    //converts booleans to 1 or 0 for graphing ability
    if (id_val == 'true') {
      id_val = 1
    }
    else if (id_val == 'false'){
      id_val = 0
    }

    //converts vectors and quaternians from raw data to the coorisponding coordinate requested
    if (numberCommas(id_val) == commas){
      id_val = getCoord(id_val, position);
    }

    //convert time to correct format
    let new_timestamp = Date.parse(point['timestamp'])
    
    return {timestamp: new_timestamp, id: id, value: id_val}
  });

  //returns the processed array
  return processedArray


};




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
  if (this.follower_enabled){
  //follower value updater
  Object.keys(this.follower_state).forEach(async function (id) {

    //if the value for the key of the state entry is an object
    if (typeof (this.follower_state[id]) == 'object') {

      Object.keys(this.follower_state[id]).forEach(async function (subId) {
        new_id = id.substr(id.indexOf('_') + 1);
        //send a request to Elastic Search for the field
        let res = await this.getValue(searchURl, {index: this.followerIndex, field: new_id + '.' + subId});
        (this.follower_state[id])[subId] = res;//update state
      }, this)

    }
    //if the value for the key of the state entry is a primitive
    else {
      new_id = id.substr(id.indexOf('_') + 1);
      //send a request to Elastic Search for the field
      let res = await this.getValue(searchURl, {index: this.followerIndex, field: new_id});
      this.follower_state[id] = res;//update state
    }

  }, this);
  }
  if (this.leader_enabled){
  Object.keys(this.leader_state).forEach(async function (id) {

      //if the value for the key of the state entry is an object
      if (typeof (this.leader_state[id]) == 'object') {

        Object.keys(this.leader_state[id]).forEach(async function (subId) {
          new_id = id.substr(id.indexOf('_') + 1);
          //send a request to Elastic Search for the field
          let res = await this.getValue(searchURl, {index: this.leaderIndex, field: new_id + '.' + subId});
          (this.leader_state[id])[subId] = res;//update state
        }, this)

      }
      //if the value for the key of the state entry is a primitive
      else {
        new_id = id.substr(id.indexOf('_') + 1);
        //send a request to Elastic Search for the field 
        let res = await this.getValue(searchURl, {index: this.leaderIndex, field: new_id});
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
Telemetry.prototype.getValue = async function (myUrl, props) {
  
  let p = new Promise(function (resolve, reject) {

    //requests URL based on properties
    request({ url: myUrl, qs: props }, function (err, response, body) {
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
Telemetry.prototype.generateTelemetryCoordinate = function(id, answer, satellite, coord_name, coord, exists){

        var timestamp = Date.now(), sent = 0;
        
        substringid = id.substring(satellite.length + 1)
        new_id = satellite + '_' + coord_name + '_' + substringid
        var telempoint = { timestamp: timestamp, id: new_id };
        if (exists){
          //set value if parent exists in elasticsearch
          telempoint['value'] = getCoord(answer, coord)

        }else{
          //set value if parent doesn't exist in elasticsearch
          telempoint['value'] = "Data not found"
        }
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
        

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
    if(this.follower_enabled){
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

        
      } 
      //test if boolean false
      else if (answer == 'true') {
        //create telem point
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = 1
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
      } 
      //test if vector
      else if (numberCommas(answer) == 3) {
        //create raw vector telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);

        this.generateTelemetryCoordinate(id, answer, 'follower', 'x', 1, true)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'y', 2, true)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'z', 3, true)

      } 
      //check if quaternion
      else if (numberCommas(answer) == 4) {
        //create raw Quaternion telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);

        this.generateTelemetryCoordinate(id, answer, 'follower', 'a', 1, true)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'b', 2, true)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'c', 3, true)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'd', 4, true)
      }
      //regular numerical data
      else {
        //create telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);

        //handle data missing from elasticsearch
        this.generateTelemetryCoordinate(id, answer, 'follower', 'a', 1, false)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'b', 2, false)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'c', 3, false)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'd', 4, false)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'x', 1, false)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'y', 2, false)

        this.generateTelemetryCoordinate(id, answer, 'follower', 'z', 3, false)
      }
    }, this);
  }

  //leader telemetry generation
  if (this.leader_enabled) {
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
      } 
      //check if bool true
      else if (answer == 'true') {
        //create telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = 1
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);
      } 
      //check if vector
      else if (numberCommas(answer) == 3) {
        //create raw vector telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);

        this.generateTelemetryCoordinate(id, answer, 'leader', 'x', 1, true)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'y', 2, true)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'z', 3, true)

      } 
      //check if quaternion
      else if (numberCommas(answer) == 4) {

        //create raw Quaternion telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);

        this.generateTelemetryCoordinate(id, answer, 'leader', 'a', 1, true)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'b', 2, true)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'c', 3, true)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'd', 4, true)

      } 
      //regular numerical data
      else {
        //create telempoint
        var telempoint = { timestamp: timestamp, id: id };
        //set value
        telempoint['value'] = answer
        //notify the realtime server and push the datapoint to the history server
        this.notify(telempoint);


        //handle data missing from elasticsearch
        this.generateTelemetryCoordinate(id, answer, 'leader', 'x', 1, false)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'y', 2, false)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'z', 3, false)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'a', 1, false)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'b', 2, false)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'c', 3, false)

        this.generateTelemetryCoordinate(id, answer, 'leader', 'd', 4, false)
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

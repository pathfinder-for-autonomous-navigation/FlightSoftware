/*
 Spacecraft.js simulates a small spacecraft generating telemetry.
*/
function DummySpacecraft() {
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

//initializes global variable value to hold the numerical value of gsw telemetry
var value = "0";
DummySpacecraft.prototype.updateState = function () {
  this.state["incoming"] = 10;
  this.state["batt.lvl"] = Math.max(0,this.state["batt.lvl"] - 1);
};

/**
 * Takes a measurement of spacecraft state, stores in history, and notifies
 * listeners.
 */
DummySpacecraft.prototype.generateTelemetry = function () {
    var timestamp = Date.now(), sent = 0;
    Object.keys(this.state).forEach(function (id) {
        var state = { timestamp: timestamp, value: this.state[id], id: id};
        this.notify(state);
        this.history[id].push(state);
    }, this);
};

DummySpacecraft.prototype.notify = function (point) {
    this.listeners.forEach(function (l) {
        l(point);
    });
};

DummySpacecraft.prototype.listen = function (listener) {
    this.listeners.push(listener);
    return function () {
        this.listeners = this.listeners.filter(function (l) {
            return l !== listener;
        });
    }.bind(this);
};

module.exports = function () {
    return new DummySpacecraft()
};

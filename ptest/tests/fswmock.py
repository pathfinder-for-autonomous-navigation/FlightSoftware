import json, sys, time, signal

class FlightSoftwareMock(object):
    """
    This class creates a copy of the execute() function in debug_console.cpp
    to create a Python-based copy of the Flight Software communication interface.

    This "mocked" flight software only contains two fields:
        "foo" : of type int
        "bar" : of type bool
    """

    def __init__(self):
        self.fswdata = {
            "foo" : 0,
            "bar" : False
        }
        self.running = True

    def loop(self):
        while self.running:
            time.sleep(0.5)
            line = sys.stdin.readline()
            try:
                data = json.loads(line)
            except json.JSONDecodeError:
                continue

            # Assess validity of data
            if not "field" in data.keys():
                continue

            err = self._getErr(data)

            # Act on data
            if data["mode"] == ord('w'):
                field_name = data["field"]
                val = data["val"]

                try:
                    casted_val = (type(self.fswdata[field_name]))(val)
                    self.fswdata[field_name] = casted_val
                except:
                    err["err"] == "field value was invalid"

            if "err" in err.keys():
                json.dump(err, sys.stdout)
                sys.stdout.flush()
                continue
            else:
                json.dump({
                    "t" : 0,
                    "field" : data["field"],
                    "val" : self.fswdata[data["field"]]
                }, sys.stdout)
                sys.stdout.flush()

    def _getErr(self, data):
        err = {
            "t" : 0,
            "field" : data["field"],
            "mode" : data["mode"],
        }
        if not "mode" in data.keys():
            err["err"] = "missing mode specification"
        elif not data["mode"] in [ord('r'), ord('w')]:
            if len(data["mode"]) > 1:
                err["err"] = "mode value is not a character"
            else:
                err["err"] = "mode value is not 'r' or 'w'"
        elif not data["field"] in self.fswdata.keys():
            err["err"] = "invalid field name"
        elif data["mode"] == ord('w') and not "val" in data.keys():
            err["err"] = "missing value of field to be written"
        
        return err

fswmock = FlightSoftwareMock()

def signal_handler(sig, frame):
    fswmock.running = False

if __name__ == "__main__":
    signal.signal(signal.SIGABRT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    fswmock.loop()

import json, pty, subprocess, serial, os
from .data_consumers import Logger

class UplinkConsole(object):
    """
    Class that interfaces with gsw_uplink_producer to create uplink packets
    from JSON descriptions.
    """

    def __init__(self, data_dir):
        # Open a subprocess to Uplink Producer. Compile it if it is not available.
        uplink_producer_filepath = ".pio/build/gsw_uplink_producer/program" 
        if not os.path.exists(uplink_producer_filepath):
            print("Compiling the uplink producer.")
            os.system("pio run -e gsw_uplink_producer > /dev/null")

        master_fd, slave_fd = pty.openpty()
        self.uplink_producer = subprocess.Popen([uplink_producer_filepath], stdin=master_fd, stdout=master_fd)
        self.uplink_console = serial.Serial(os.ttyname(slave_fd), 9600, timeout=1)

        self.logger = Logger("UplinkConsole", data_dir)

    def get_val(self, val):
        '''
        Recives string representation of a value and returns the value as a bool, int, or float
        If the value can't be determined, returns None;
        "true" --> true (bool)
        "5" --> 5 (int)
        "0.05" --> 0.05 (float)
        "dchkjdda" --> None
        '''
        try:
            f=float(val)
            i=int(f)
            if f!=i: 
                return f
            return i
        except:
            if val == "true": return True
            if val == "false": return False
        return None

    def create_uplink(self, fields, vals, filename):
        """
        Puts fields and values in a JSON document and sends the JSON 
        object to the uplink producer console. This results in the creation
        of an SBD file with the given filename holding an uplink packet
        """

        # Create a JSON file with all the fields and values
        telem_json={}
        for field, val in zip(fields, vals):
            value = self.get_val(val)
            if value is not None:
                telem_json[field]=value
            else:
                logline = "Failed:   " + json.dumps(telem_json) + "\n"
                logline += f"Error:    Unable to add {field}: {val} to uplink JSON file"
                self.logger.put(logline)
                return False
        with open('uplink.json', 'w') as telem_file:
            json.dump(telem_json, telem_file)

        # Write the JSON file into Uplink Producer - should result in the creation of an sbd file
        # holding the uplink packet.
        self.uplink_console.write(("uplink.json\n").encode())
        self.uplink_console.write((str(filename)+"\n").encode())
        response = json.loads(self.uplink_console.readline().rstrip())

        # Check that the uplink was successfully created
        if 'error' in response:
            logline = "Failed:   " + json.dumps(telem_json) + "\n"
            logline += "Error:    "+response['error']
            self.logger.put(logline)
            return False

        self.logger.put("Uplink:   " + json.dumps(telem_json))
        return True

    def close(self):
        if not hasattr(self, "stopped"):
            self.stopped = False

        if self.stopped: return

        self.uplink_producer.kill()
        self.uplink_console.close()
        self.logger.stop()
        self.stopped = True

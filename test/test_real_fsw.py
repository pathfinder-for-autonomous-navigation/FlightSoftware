# Tests the native_ci target output to ensure that input and output 
# to the Flight Software work as intended.

import subprocess
import pty
import json
import os
import time
import serial
import unittest

class TestFlightSoftwareBinary(unittest.TestCase):
    """
    Ensures that basic state field read-write functionality works as expected within
    Flight Software.
    """

    binary_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)),
        "../.pio/build/fsw_native_leader/program")

    def setUp(self):
        master_fd, slave_fd = pty.openpty()
        self.fsw = subprocess.Popen([self.binary_dir], stdin=master_fd, stdout=master_fd)
        self.console = serial.Serial(os.ttyname(slave_fd), 9600, timeout=1)

    def readCycleNumber(self):
        # Helper method to read the cycle count.

        input = json.dumps({"field": "pan.cycle_no", "mode" : ord('r')}) + "\n"
        self.console.write(input.encode())
        time.sleep(0.200)
        
        while self.console.in_waiting > 0:
            response = json.loads(self.console.readline().rstrip())
            if 'field' in response.keys():
                self.assertEqual(response['field'], "pan.cycle_no")
                return int(response['val'])

    def startNextCycle(self):
        # Send a signal to start the next cycle, like the simulation would.

        input = json.dumps({"field": "cycle.start", "mode" : ord('w'), "val" : "true"}) + "\n"
        self.console.write(input.encode())
        self.console.readline() # Throw away next line

    def testValidRead(self):
        """Test that the cycle count increments on every control cycle."""

        cycleNumber = self.readCycleNumber()
        self.assertGreaterEqual(cycleNumber, 0)

        self.startNextCycle()
        newCycleNumber = self.readCycleNumber()
        self.assertEqual(newCycleNumber, cycleNumber + 1)
        
        self.startNextCycle()
        newCycleNumber = self.readCycleNumber()
        self.assertEqual(newCycleNumber, cycleNumber + 2)

    def tearDown(self):
        self.fsw.kill()
        self.console.close()

if __name__ == '__main__':
    unittest.main()

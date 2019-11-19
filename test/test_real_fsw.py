# Tests the native_ci target output to ensure that input and output 
# to the Flight Software work as intended.

import subprocess
import pty
import json
import os
import time
import serial
import unittest

class TestDummyFlightSoftwareBinary(unittest.TestCase):
    """
    Ensures that basic state field read-write functionality works as expected within
    Flight Software.
    """

    binary_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)),
        "../.pio/build/native/program")

    def setUp(self):
        master_fd, slave_fd = pty.openpty()
        self.fsw = subprocess.Popen([self.binary_dir], stdin=master_fd,
            stdout=master_fd)
        self.console = serial.Serial(os.ttyname(slave_fd), 9600, timeout=1)

    def read_cycle_no(self):
        input = json.dumps({"field": "pan.cycle_no", "mode" : ord('r')}) + "\n"
        self.console.write(input.encode())
        response = json.loads(self.console.readline().rstrip())
        self.assertEqual(response['field'], "pan.cycle_no")
        return int(response['val'])

    def testValidRead(self):
        cycle_no = self.read_cycle_no()
        self.assertGreaterEqual(cycle_no, 0)
        time.sleep(0.5)
        new_cycle_no = self.read_cycle_no()
        self.assertGreater(new_cycle_no, cycle_no)

    def tearDown(self):
        self.fsw.kill()
        self.console.close()

if __name__ == '__main__':
    unittest.main()

# Tests the native_ci target output to ensure that input and output 
# to the Flight Software work as intended.

import subprocess
import pty
import json
import os
import serial
import unittest

class TestDummyFlightSoftwareBinary(unittest.TestCase):
    """
    Ensures that basic state field read-write functionality works as expected within
    Flight Software.
    """

    binary_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)),
        "../.pio/build/fsw_native_ci/program")

    def setUp(self):
        master_fd, slave_fd = pty.openpty()
        self.dummy_fsw = subprocess.Popen([self.binary_dir], stdin=master_fd,
            stdout=master_fd)
        self.console = serial.Serial(os.ttyname(slave_fd), 9600, timeout=1)

    def testInvalidCmd(self):
        """An invalid command to flight software is caught."""

        input = json.dumps({"field": "readable_field"}) + "\n"
        self.console.write(input.encode())
        response = json.loads(self.console.readline().rstrip())
        self.assertEqual(response['field'], "readable_field")
        self.assertEqual(response['mode'], "perform unspecified operation with")
        self.assertEqual(response['err'], "missing mode specification")

    def testInvalidRead(self):
        """An attempt to read a nonexistent field is caught."""

        input = json.dumps({"field": "nonexistent_field", "mode" : ord('r')}) + "\n"
        self.console.write(input.encode())
        response = json.loads(self.console.readline().rstrip())
        self.assertEqual(response['field'], "nonexistent_field")
        self.assertEqual(response['err'], "invalid field name")

    def testValidRead(self):
        """A request to read a readable field is serviced."""

        input = json.dumps({"field": "readable_field", "mode" : ord('r')}) + "\n"
        self.console.write(input.encode())
        response = json.loads(self.console.readline().rstrip())
        self.assertEqual(response['field'], "readable_field")
        self.assertEqual(response['val'], "0")

    def testValidReadOfWritableField(self):
        """A request to read a writable field is serviced."""

        input = json.dumps({"field": "writable_field", "mode": ord('r')}) + "\n"
        self.console.write(input.encode())
        response = json.loads(self.console.readline().rstrip())
        self.assertEqual(response['field'], "writable_field")
        self.assertEqual(response['val'], "0")

    def testInvalidWrite(self):
        """An invalid write command that's missing a value to be written is caught."""

        input = json.dumps({"field": "writable_field", "mode": ord('w')}) + "\n"
        self.console.write(input.encode())
        response = json.loads(self.console.readline().rstrip())
        self.assertEqual(response['field'], "writable_field")
        self.assertEqual(response['err'], "missing value of field to be written")

    def testValidWrite(self):
        """An valid write command to a writable field is serviced."""

        # Ensure OK write.
        input = json.dumps({"field": "writable_field", "mode": ord('w'), "val" : "2"}) + "\n"
        self.console.write(input.encode())
        response = json.loads(self.console.readline().rstrip())
        self.assertEqual(response['field'], "writable_field")
        self.assertEqual(response['val'], "2")

        # Ensure the write is persistent on the device.
        input = json.dumps({"field": "writable_field", "mode": ord('r')}) + "\n"
        self.console.write(input.encode())
        response = json.loads(self.console.readline().rstrip())
        self.assertEqual(response['field'], "writable_field")
        self.assertEqual(response['val'], "2")

    def tearDown(self):
        self.dummy_fsw.kill()
        self.console.close()

if __name__ == '__main__':
    unittest.main()

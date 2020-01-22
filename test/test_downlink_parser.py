import subprocess
import pty
import json
import os
import serial
import unittest

class TestDownlinkParser(unittest.TestCase):
    """
    Ensures that the downlink parser accumulates downlink packets and dumps the
    data contained in a downlink frame once no more packets are available.
    """

    filepath = os.path.dirname(os.path.abspath(__file__))
    binary_dir = os.path.join(filepath, "../.pio/build/gsw_downlink_parser/program")

    def setUp(self):
        master_fd, slave_fd = pty.openpty()
        self.downlink_parser = subprocess.Popen([self.binary_dir], stdin=master_fd,
            stdout=master_fd)
        self.console = serial.Serial(os.ttyname(slave_fd), 9600, timeout=1)

    def getFilepath(self, filename):
        # Get full path of file specified by the argument.
        data_dir = os.path.join(self.filepath, "dat/DownlinkParser")
        return os.path.join(data_dir, filename)

    def testAddInvalidDownlink(self):
        """The parser produces an error if you feed it a nonexistent file. """
        self.console.write((self.getFilepath("nonexistent_downlink") + "\n").encode())
        response = self.console.readline().rstrip()
        self.assertEqual(response.decode(), "Error: file not found.")

    def testValidDownlink(self):
        """Test valid downlink reading."""

        # Adding one downlink packet results in the parser returning data from
        # the last collected downlink frame. There is no such frame, so the
        # parser returns back an empty JSON string.
        self.console.write((self.getFilepath("downlink1") + "\n").encode())
        response = json.loads(self.console.readline().rstrip())
        self.assertIsNone(response)

        # Sending downlink packet 1 again tells the parser that a downlink frame
        # has been completed, so it dumps the data contained in the previously
        # sent downlink frame.
        self.console.write((self.getFilepath("downlink1") + "\n").encode())
        response = json.loads(self.console.readline().rstrip())
        expectedResponse = json.load(open(self.getFilepath("expected_output.json")))
        self.assertDictEqual(response, expectedResponse)

    def tearDown(self):
        self.downlink_parser.kill()
        self.console.close()

if __name__ == '__main__':
    unittest.main()

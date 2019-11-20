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
    binary_dir = os.path.join(filepath, "../.pio/build/downlink_parser/program")
    data_dir = os.path.join(filepath, "dat/DownlinkParser")

    def setUp(self):
        master_fd, slave_fd = pty.openpty()
        self.downlink_parser = subprocess.Popen([self.binary_dir], stdin=master_fd,
            stdout=master_fd)
        self.console = serial.Serial(os.ttyname(slave_fd), 9600, timeout=1)

    def testAddInvalidDownlink(self):
        """The parser produces an error if you feed it a nonexistent file. """
        self.console.write(os.path.join(data_dir,"nonexistent_downlink").encode())
        response = self.console.readline().rstrip()
        assert response == "Error"

    def testAddDownlink1(self):
        """
        Adding one downlink packet results in the parser returning data from
        the last collected downlink frame. There is no such frame, so the
        parser returns back an empty JSON string.
        """
        self.console.write(os.path.join(data_dir,"downlink1").encode())
        response = json.loads(self.console.readline().rstrip())
        assert response == {}

    def testAddDownlink2And3(self):
        """
        Adding another few downlink packets results in the parser returning
        data from the last collected downlink frame. There is no such frame,
        so the parser returns back an empty JSON string.
        """
        self.console.write(os.path.join(data_dir,"downlink2").encode())
        response = json.loads(self.console.readline().rstrip())
        assert response == {}

        self.console.write(os.path.join(data_dir,"downlink3").encode())
        response = json.loads(self.console.readline().rstrip())
        assert response == {}

    def testAddDownlink1Again(self):
        """
        Sending downlink packet 1 again tells the parser that a downlink frame
        has been completed, so it dumps the data contained in packets 1, 2, and 3.
        """
        self.console.write(os.path.join(data_dir,"downlink1").encode())
        response = json.loads(self.console.readline().rstrip())
        assert response == json.load(open(os.path.join(data_dir, "expected_parser_output.json")))

    def tearDown(self):
        self.downlink_parser.kill()
        self.console.close()

if __name__ == '__main__':
    unittest.main()

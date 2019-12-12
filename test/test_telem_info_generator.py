import subprocess
import json
import os
import unittest

class TestTelemInfoGenerator(unittest.TestCase):
    """
    Ensures that the telemetry information generator successfully generates a description
    of the state fields onboard the spacecraft.
    """

    filepath = os.path.dirname(os.path.abspath(__file__))
    binary_dir = os.path.join(filepath, "../.pio/build/telem_info_generator/program")

    def testTelemInfoGeneratorRequiresOneArgument(self):
        """Tests that the telemetry info generator requires exactly one argument, the filename."""

        # Zero arguments doesn't work
        proc = subprocess.Popen([self.binary_dir], stdin=subprocess.PIPE,
            stdout=subprocess.PIPE)
        output, err = proc.communicate()
        returncode = proc.wait()
        self.assertEqual(returncode, 1)
        self.assertEqual(output, "Need to specify a filename.")

        # Two arguments doesn't work
        proc = subprocess.Popen([self.binary_dir, "x", "x"], stdin=subprocess.PIPE,
            stdout=subprocess.PIPE)
        output, err = proc.communicate()
        returncode = proc.wait()
        self.assertEqual(returncode, 1)
        self.assertEqual(output, "Too many arguments.")

    def testTelemInfoGeneratorWorks(self):
        """Tests that the telemetry info generator produces a meaningful, intelligible file."""

        proc = subprocess.run([self.binary_dir, "telem_output.json"], stdin=subprocess.PIPE,
            stdout=subprocess.PIPE)
        self.assertEqual(proc.returncode, 0)

        with open("telem_output.json", "r") as f:
            telem_info = json.load(f)
            self.assertNotIn("flow_id", telem_info["fields"]["pan.cycle_no"].keys())
            self.assertEqual(telem_info["fields"]["pan.cycle_no"]["min"], 0)
            self.assertEqual(telem_info["fields"]["pan.cycle_no"]["max"], 4294967295)
            self.assertEqual(telem_info["fields"]["pan.cycle_no"]["bitsize"], 32)

        # Clean up
        os.remove("telem_output.json")

if __name__ == '__main__':
    unittest.main()

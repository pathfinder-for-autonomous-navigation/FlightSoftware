# Propulsion test case. It can check the sensor values and fire valves. The
# user must verify that the values and valve firings are correct.
from .base import SingleSatOnlyCase
import time

class PropCheckoutCase(SingleSatOnlyCase):
    def setup_case_singlesat(self):
        self.sim.flight_controller.write_state(
            "pan.state", self.mission_states.get_by_name("manual"))
        self.sim.flight_controller.write_state(
            "prop.state", self.prop_states.get_by_name("manual"))

    @property
    def pressure(self):
        return self.read_state("prop.tank2.pressure")

    @property
    def temp1(self):
        return self.read_state("prop.tank1.temp")

    @property
    def temp2(self):
        return self.read_state("prop.tank2.temp")

    @property
    def intertank1_schedule(self):
        return None

    @property
    def intertank2_schedule(self):
        return None

    @intertank1_schedule.setter
    def intertank1_schedule(self, sched):
        self.write_state("prop.sched_intertank1", str(sched))

    @intertank2_schedule.setter
    def intertank2_schedule(self, sched):
        self.write_state("prop.sched_intertank2", str(sched))

    def set_tvalve_schedule(self, sched):
        self.write_state("prop.sched_valve1", str(sched[0]))
        self.write_state("prop.sched_valve2", str(sched[1]))
        self.write_state("prop.sched_valve3", str(sched[2]))
        self.write_state("prop.sched_valve4", str(sched[3]))

    def print_all_sensor_values(self):
        print(f"[TESTCASE] Tank 1 temperature: {self.temp1}")
        print(f"[TESTCASE] Tank 2 temperature: {self.temp2}")
        print(f"[TESTCASE] Tank 2 pressure: {self.pressure}")

    def run_case_singlesat(self):
        print("------------------------------------------------")
        self.test_pressurization()
        print("------------------------------------------------")
        self.test_thruster_firings()
        print("------------------------------------------------")
        self.test_venting_response()
        print("------------------------------------------------")
        print("[TESTCASE] Propulsion testing finished.")

    def print_initial_sensor_values(self):
        print(f"[TESTCASE] Initial tank 1 temperature: {self.temp1}")
        print(f"[TESTCASE] Initial tank 2 temperature: {self.temp2}")
        print(f"[TESTCASE] Initial tank 2 pressure: {self.pressure}")

    def test_pressurization(self):
        print("[TESTCASE] Simulating pressurization of tank.")
        self.print_initial_sensor_values()

        print("[TESTCASE] Opening intertank valve 1 for 500ms and allowing pressure to settle for 10 seconds.")
        self.intertank1_schedule = 500
        self.cycle()
        time.sleep(10)
        self.print_all_sensor_values()

        print("[TESTCASE] Opening intertank valve 2 for 500ms and allowing pressure to settle for 10 seconds.")
        self.intertank2_schedule = 500
        self.cycle()
        time.sleep(10)
        self.print_all_sensor_values()

    def test_thruster_firings(self):
        print("[TESTCASE] Testing thruster firings.")
        self.print_initial_sensor_values()

        print("[TESTCASE] Firing each thrust valve for 0.2 s, spaced apart by 1 second.")
        firings = [[0,0,0,200], [0,0,200,0], [0,200,0,0], [200,0,0,0]]
        for firing in firings:
            self.set_tvalve_schedule(firing)
            self.cycle()
            time.sleep(1)
            self.print_all_sensor_values()

        print("[TESTCASE] Firing each combination of two thrust valves for 0.15 s, spaced apart by 1 second.")
        firings = [[0,0,150,150], [0,150,150,0], [150,150,0,0], [150,0,150,0], [150,0,0,150], [150,0,0,150]]
        for firing in firings:
            self.set_tvalve_schedule(firing)
            self.cycle()
            time.sleep(1)
            self.print_all_sensor_values()

        print("[TESTCASE] Firing each combination of three thrust valves for 0.1 s, spaced apart by 1 second.")
        firings = [[0,100,100,100], [100,0,100,100], [100,100,0,100], [100,100,100,0]]
        for firing in firings:
            self.set_tvalve_schedule(firing)
            self.cycle()
            time.sleep(1)
            self.print_all_sensor_values()

        print("[TESTCASE] Firing all four thrust valves for 0.05 s.")
        firing = [50,50,50,50]
        self.set_tvalve_schedule(firing)
        self.cycle()
        self.print_all_sensor_values()

    def test_venting_response(self):
        print("[TESTCASE] Testing venting response.")
        self.print_initial_sensor_values()

        self.intertank1_schedule = 1000
        self.cycle()

        self.cycle()

        self.set_tvalve_schedule([1000,1000,1000,1000])
        self.cycle()

        self.print_all_sensor_values()

# Propulsion test case. It can check the sensor values and fire valves. The
# user must verify that the values and valve firings are correct.
from .base import SingleSatOnlyCase
import time

class PropulsionCheckoutCase(SingleSatOnlyCase):
    def setup_case_singlesat(self):
        self.sim.flight_controller.write_state(
            "pan.state", self.mission_states.get_by_name["manual"])  # Manual state
        self.run_case_singlesat()
        print("Propulsion testing finished.")

    @property
    def pressure(self):
        return self.read_state("prop.tank2.pressure")

    @property
    def temp1(self):
        return self.read_state("prop.tank1.temp")

    @property
    def temp2(self):
        return self.read_state("prop.tank2.temp")

    def print_all_sensor_values(self):
        print(f"Tank 1 temperature: {self.temp1}")
        print(f"Tank 2 temperature: {self.temp2}")
        print(f"Tank 2 pressure: {self.pressure}")

    def run_case_singlesat(self):
        self.print_initial_sensor_values()
        print("------------------------------------------------")
        self.test_pressurization()
        print("------------------------------------------------")
        self.test_thruster_firings()
        print("------------------------------------------------")
        self.test_venting_response()
        print("------------------------------------------------")

    def print_initial_sensor_values(self):
        print(f"Initial tank 1 temperature: {self.temp1}")
        print(f"Initial tank 2 temperature: {self.temp2}")
        print(f"Initial tank 2 pressure: {self.pressure}")

    def test_pressurization(self):
        print("Simulating pressurization of tank.")
        print("Opening intertank valve 1 for 1 second and allowing pressure to settle for 10 seconds.")
        time.sleep(10)
        self.print_all_sensor_values()
        print("Opening intertank valve 2 for 1 second and allowing pressure to settle for 10 seconds.")
        time.sleep(10)
        self.print_all_sensor_values()

    def test_thruster_firings(self):
        print("Firing each thrust valve for 0.2 s, spaced apart by 1 second.")
        firings = [[0,0,0,200], [0,0,200,0], [0,200,0,0], [200,0,0,0]]
        for firing in firings:
            self.execute_thrust_valve_firing(firing)
            time.sleep(1)
            self.print_all_sensor_values()

        print("Firing each combination of two thrust valves for 0.15 s, spaced apart by 1 second.")
        firings = [[0,0,150,150], [0,150,150,0], [150,150,0,0], [150,0,150,0], [150,0,0,150], [150,0,0,150]]
        for firing in firings:
            self.execute_thrust_valve_firing(firing)
            time.sleep(1)
            self.print_all_sensor_values()

        print("Firing each combination of three thrust valves for 0.1 s, spaced apart by 1 second.")
        firings = [[0,100,100,100], [100,0,100,100], [100,100,0,100], [100,100,100,0]]
        for firing in firings:
            self.execute_thrust_valve_firing(firing)
            time.sleep(1)
            self.print_all_sensor_values()

        print("Firing all four thrust valves for 0.05 s.")
        firing = [50,50,50,50]
        self.execute_thrust_valve_firing(firing)
        self.print_all_sensor_values()

    def execute_thrust_valve_firing(self, firing):
        # TODO
        pass

    def test_venting_response(self):
        print("Testing venting response.")
        # TODO
        pass

# ADCSCheckoutCase. Verifies the functionality of the ADCS.
from .base import SingleSatOnlyCase

class ADCSCheckoutCase(SingleSatOnlyCase):

    @property
    def adcs_func(self):
        return self.sim.flight_controller.read_state("adcs_monitor.functional")
    
    @property
    def cycle_no(self):
        return self.sim.flight_controller.read_state("pan.cycle_no")

    @property
    def havt_read(self):
        read_list = [False for x in range(self.havt_length)]
        for x in range(self.havt_length):
            read_list[x] = self.sim.flight_controller.read_state("adcs_monitor.havt_device"+str(x))
        read_list = [True if x == "true" else False for x in read_list]
        return read_list

    @property
    def rwa_speed_cmd(self):
        print("RWA SPEED CMD GETTER CALLED\n")
        return self.sim.flight_controller.read_state("adcs_cmd.rwa_speed_cmd")
    
    @property
    def rwa_torque_cmd(self):
        return self.sim.flight_controller.read_state("adcs_cmd.rwa_torque_cmd")

    @property
    def rwa_mode_cmd(self):
        print("MODE GETTER\n")
        return self.sim.flight_controller.read_state("adcs_cmd.rwa_mode")

    @rwa_speed_cmd.setter
    def rwa_speed_cmd(self, rwa_list):
        # assert( len(rwa_list) == 3 and type(rwa_list[0]) == float)
        # rwa_list = [str(x) for x in rwa_list]
        # rwa_list = ', '.join(rwa_list)
        # self.sim.flight_controller.write_state("adcs_cmd.rwa_speed_cmd",rwa_list)
        self.sim.flight_controller.write_state("adcs_cmd.rwa_speed_cmd", *rwa_list)

    @rwa_torque_cmd.setter
    def rwa_torque_cmd(self, rwa_list):
    
        # assert( len(rwa_list) == 3 and type(rwa_list[0]) == float)
        # rwa_list = [str(x) for x in rwa_list]
        # rwa_list = ', '.join(rwa_list)
        # self.sim.flight_controller.write_state("adcs_cmd.rwa_torque_cmd",rwa_list)
        
        self.sim.flight_controller.write_state("adcs_cmd.rwa_torque_cmd", *rwa_list)

    @rwa_mode_cmd.setter
    def rwa_mode_cmd(self, val):
        self.sim.flight_controller.write_state("adcs_cmd.rwa_mode", val)

    def setup_case_singlesat(self):
        self.havt_length = 18 # _LENGTH in havt_devices.hpp

        self.sim.flight_controller.write_state("pan.state", 11) # Mission State = Manual
        self.sim.flight_controller.write_state("adcs.state", 5) # ADCS State = Manual
        self.sim.flight_controller.write_state("adcs_cmd.rwa_mode", 1) # Speed Control
        self.sim.flight_controller.write_state("adcs_cmd.rwa_speed_cmd", 0.0,0.0,0.0) # 0 speed to begin with
        self.sim.flight_controller.write_state("dcdc.ADCSMotor_cmd", "true")

    """Steps the FC forward by one step"""
    def step(self):
        self.sim.flight_controller.write_state("cycle.start", "true")

    def print_havt_read(self):
        binary_list = [1 if x else 0 for x in self.havt_read]

        string_of_binary_list = [str(x) for x in binary_list]
        
        # Reverse the list so it prints as it does in ADCSSoftware
        string_of_binary_list.reverse()

        list_of_list = [string_of_binary_list[4*i:(4*i)+4] for i in range((int)(self.havt_length/4)+1)]
        final = [x + [" "] for x in list_of_list]

        final_string = ''.join([''.join(x) for x in final])
        print("HAVT Read: "+str(final_string))

    def run_case_singlesat(self):
        self.step()

        print("Cycle Number: "+str(self.cycle_no))
        print("ADCS Functional: "+str(type(self.adcs_func)))
        print("Cycle Number: "+str(self.cycle_no))
        self.print_havt_read()

        for x in range(self.havt_length):
            if not self.havt_read[x]:
                print("Device #"+str(x)+": Not Functional")
                
        one_thou = [1000.0,1000.0,1000.0]
        self.rwa_speed_cmd = one_thou

        print(self.rwa_speed_cmd)
        self.rwa_speed_cmd = (400,500,600)
        print(self.rwa_speed_cmd)
        self.sim.flight_controller.write_state("adcs_cmd.rwa_speed_cmd", 400,400,500)
        print(self.rwa_speed_cmd)
        self.rwa_mode_cmd = [250,260.0,300.00000]
        print(self.rwa_speed_cmd) # TODO NEED ASSERT GUARDS
        print(self.rwa_mode_cmd)

        self.rwa_speed_cmd = [250,260.0,300.00000]
        print(self.rwa_speed_cmd)

        self.print_havt_read()
        self.sim.flight_controller.write_state("adcs_cmd.havt_disable0", True)
        self.step()
        self.print_havt_read()

        self.rwa_mode_cmd = 2
        print(self.rwa_mode_cmd)
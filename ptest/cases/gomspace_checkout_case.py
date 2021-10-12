# Gomspace test case. Gets cycle count purely for diagnostic purposes and logs
# any other Gomspace state fields.

from typing import List
import time

from .base import SingleSatCase
from .utils import Enums, TestCaseFailure

# DO NOT USE AS A REFRENCE TO WRITE OTHER PTEST CASES
#
# This testcase is basically a dinosaur among the other testcases and using many
# features that are considered "deprecated".

class GomspaceCheckoutCase(SingleSatCase):

    def __init__(self, *args, **kwargs):
        super(GomspaceCheckoutCase, self).__init__(*args, **kwargs)

        self.debug_to_console = True
        self.check_initial_state = False

    def read_state(self, string_state):
        return self.flight_controller.read_state(string_state)

    def write_state(self, string_state, state_value):
        self.flight_controller.write_state(string_state, state_value)
        return self.read_state(string_state)

    def str_to_bool(self, string):
        if string == "true":
            return True
        elif string == "false":
            return False
        else:
            raise ValueError

    def print_and_get_output_outputs(self) -> List[bool]:
        '''Get all the current output states, and return a list of boolean current states'''
        outputs = [self.read_state('gomspace.output.output' + str(i)) for i in range(1, 7)]
        outputs = [self.str_to_bool(x) for x in outputs]
        self.logger.put(f'Outputs: {outputs}')
        return outputs

    def print_cycle_cmds(self) -> List[bool]:
        '''Get all the current output states, and return a list of boolean current states'''
        outputs = [self.read_state(f'gomspace.power_cycle_output' + str(i) + '_cmd') for i in range(1, 7)]
        outputs = [self.str_to_bool(x) for x in outputs]
        self.logger.put(f'Outputs Commands: {outputs}')
        return outputs

    def run(self):
        self.ws('pan.state', 0)
        self.ws('pan.deployment.elapsed', 106)
        self.cycle()
        now = time.time()
        
        while time.time() - now < 10:
            self.cycle()

        # self.flight_controller.write_state('gomspace.piksi_off', False)
        self.failed = False
        self.cycle_no = self.flight_controller.read_state("pan.cycle_no")
        # self.write_state("gomspace.piksi_off", "false")

        # readable fields
        vboost = [int(self.read_state("gomspace.vboost.output" + str(i)))
                  for i in range(1, 4)]
        for n in range(0, len(vboost)):
            self.logger.put("vboost" + str(n) + " is: " + str(vboost[n]) + " mV")

        vbatt = int(self.read_state("gomspace.vbatt"))
        if vbatt < 6000 or vbatt > 8400:
            self.logger.put(
                "Vbatt is out of expected range [6000, 8400] mV at: " + str(vbatt))

        curin = [int(self.read_state("gomspace.curin.output" + str(i)))
                 for i in range(1, 4)]
        for n in range(0, len(curin)):
            self.logger.put("curin" + str(n) + " is: " + str(curin[n]) + " mA")
        cursun = int(self.read_state("gomspace.cursun"))
        self.logger.put("cursun is: " + str(cursun) + " mA")
        cursys = int(self.read_state("gomspace.cursys"))
        self.logger.put("cursys is: " + str(cursys) + " mA")

        curout = [int(self.read_state("gomspace.curout.output" + str(i)))
                  for i in range(1, 7)]
        for n in range(0, len(curout)):
            self.logger.put("curout" + str(n) + " is: " + str(curout[n]) + " mA")

        outputs = self.print_and_get_output_outputs()

        for n in range(0, len(outputs)):
            out_n = outputs[n]
            # checked umbilical for which outputs should be 0 and 1:
            # 1-5 are all 5V, 6 is 3.3V
            if out_n is False:
                self.logger.put("Output-" + str(n) + " is not on")

        wdt_i2c_time_left = int(self.read_state("gomspace.wdt_i2c_time_left"))
        if wdt_i2c_time_left < 99:
            self.logger.put("wdt_i2c_time_left is less than 99 seconds at: " +
                  str(wdt_i2c_time_left))

        counter_wdt_i2c = self.read_state("gomspace.counter_wdt_i2c")
        self.logger.put("counter_wdt_i2c is: " + str(counter_wdt_i2c))
        counter_boot = self.read_state("gomspace.counter_boot")
        self.logger.put("counter_wdt_i2c is: " + str(counter_boot))

        temp = [int(self.read_state("gomspace.temp.output" + str(i)))
                for i in range(1, 5)]
        for n in range(0, len(temp)):
            temp_n = temp[n]
            if temp_n < 20 or out_n > 25:
                self.logger.put("Temp-" + str(n) +
                      " is out of room temperature range [20, 25] degC at: " + str(temp_n))

        bootcause = self.read_state("gomspace.bootcause")
        self.logger.put("bootcause is: " + str(bootcause))
        battmode = self.read_state("gomspace.battmode")
        self.logger.put("battmode is: " + str(battmode))
        pptmode = self.read_state("gomspace.pptmode")
        self.logger.put("pptmode is: " + str(pptmode))

        outputs = self.print_and_get_output_outputs()

        self.print_header("Waiting for all outputs to be on")
        while (not all(outputs)):
            outputs = self.print_and_get_output_outputs()
            self.cycle()

        # writable fields
        cycle_no_init = int(self.read_state("pan.cycle_no"))
        cycle_no = cycle_no_init

        # power cycling
        # start power cycle
        for i in range(1,7):
            self.write_state(f'gomspace.power_cycle_output{i}_cmd', True)
                                  
        self.cycle()
        self.print_header('Commanded all outputs to be off')
        self.cycle()
        self.print_cycle_cmds()
        self.print_and_get_output_outputs()

        # wait for outputs to be off
        while (not all(out == False for out in outputs)):
            outputs = self.print_and_get_output_outputs()
            self.cycle()
            cycle_no = int(self.read_state("pan.cycle_no"))
            if cycle_no - cycle_no_init == 600:
                cycle_no = int(self.read_state("pan.cycle_no"))
                self.logger.put("failed on cycle: " + str(cycle_no))
                raise TestCaseFailure("Power cycled outputs could not turn off after 600 cycles (1 minute)")
            
        self.print_header('Waiting for all outputs to be on.')            

        # wait for outputs to turn on again
        while (not all(out == True for out in outputs)):
            outputs = self.print_and_get_output_outputs()
            self.cycle()
            cycle_no = int(self.read_state("pan.cycle_no"))
            if cycle_no - cycle_no_init == 600:
                cycle_no = int(self.read_state("pan.cycle_no"))
                self.logger.put("failed on cycle: " + str(cycle_no))
                raise TestCaseFailure("Power cycled outputs could not turn on after 600 cycles (1 minute)")

        # check if finished power cycling
        power_cycle_output_cmd = self.print_cycle_cmds()
        for n in range(0, len(power_cycle_output_cmd)):
            if power_cycle_output_cmd[n] == True:
                self.logger.put("Could not update power_cycle_output" + str(n))
                self.failed = True

        self.print_header('Checking can update command fields.')            

        ppt_mode_cmd = int(self.read_state("gomspace.pptmode_cmd"))
        ppt_mode_updated = int(self.write_state(
            "gomspace.pptmode_cmd", (int(ppt_mode_cmd) + 1) % 2))
        if ppt_mode_cmd == ppt_mode_updated:
            self.logger.put("Could not update pptmode")
            self.failed = True

        heater_cmd = self.str_to_bool(self.read_state("gomspace.heater_cmd"))
        heater_cmd_updated = self.str_to_bool(self.write_state(
            "gomspace.heater_cmd", not heater_cmd))
        if heater_cmd == heater_cmd_updated:
            self.logger.put("Could not update heater")
            self.failed = True

        counter_reset_cmd = self.str_to_bool(self.read_state(
            "gomspace.counter_reset_cmd"))
        counter_reset_cmd_updated = self.str_to_bool(self.write_state(
            "gomspace.counter_reset_cmd", not counter_reset_cmd))
        if counter_reset_cmd == counter_reset_cmd_updated:
            self.logger.put("Could not update counter_reset")
            self.failed = True

        # gs_reset_cmd = self.str_to_bool(
        #     self.read_state("gomspace.gs_reset_cmd"))
        # gs_reset_cmd_updated = self.str_to_bool(self.write_state(
        #     "gomspace.gs_reset_cmd", not gs_reset_cmd))
        # if gs_reset_cmd == gs_reset_cmd_updated:
        #     self.logger.put("Could not update gs_reset")
        #     self.failed = True

        gs_reboot_cmd = self.str_to_bool(
            self.read_state("gomspace.gs_reboot_cmd"))
        gs_reboot_cmd_updated = self.str_to_bool(self.write_state(
            "gomspace.gs_reboot_cmd", not gs_reboot_cmd))
        if gs_reboot_cmd == gs_reboot_cmd_updated:
            self.logger.put("Could not update gs_reboot")
            self.failed = True

        for _ in range(10):
            self.cycle()

        if self.failed: 
            raise TestCaseFailure("Failed a step in Gomspace checkout: see log above.")

        self.print_header("Gomspace Checkout Case Successful.")

        self.finish()

class CheckBatteryLevel(SingleSatCase):
    def run(self):
        voltage = float(self.rs("gomspace.vbatt"))
        self.logger.put("                                 ")
        self.logger.put("=================================")
        self.logger.put("=================================")
        self.logger.put(f"Satellite battery level: {voltage/1000} volts")
        self.logger.put("=================================")
        self.logger.put("=================================")
        self.logger.put("                                 ")
        self.finish()

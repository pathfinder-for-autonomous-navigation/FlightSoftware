# Gomspace test case. Gets cycle count purely for diagnostic purposes and logs
# any other Gomspace state fields.
from .base import SingleSatOnlyCase


class GomspaceCheckoutCase(SingleSatOnlyCase):

    def setup_case_singlesat(self):
        self.sim.flight_controller.write_state(
            "pan.state", self.mission_states.get_by_name("manual"))

    def str_to_bool(self, str):
        if str == "true":
            return True
        return False

    def read_state(self, string_state):
        return self.sim.flight_controller.read_state(string_state)

    def write_state(self, string_state, state_value):
        self.sim.flight_controller.write_state(string_state, state_value)
        return self.read_state(string_state)

    def run_case_singlesat(self):
        self.sim.cycle_no = self.sim.flight_controller.read_state(
            "pan.cycle_no")

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

        output = [self.str_to_bool(self.read_state("gomspace.output.output" + str(i)))
                  for i in range(1, 7)]

        for n in range(0, len(output)):
            out_n = output[n]
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

        # writable fields
        power_cycle_output_cmd = [self.str_to_bool(self.read_state("gomspace.power_cycle_output" + str(i) + "_cmd"))
                                  for i in range(1, 7)]
        cycle_no_init = int(self.read_state("pan.cycle_no"))
        cycle_no = cycle_no_init

        # power cycling
        # start power cycle
        power_cycle_output_cmd = [self.str_to_bool(self.write_state("gomspace.power_cycle_output"
                                                               + str(i) + "_cmd", "true"))
                                  for i in range(1, 7)]
        # wait for outputs to be off
        while (not all(out == False for out in output)) and cycle_no - cycle_no_init < 600:
            output = [self.str_to_bool(self.read_state("gomspace.output.output" + str(i)))
                      for i in range(1, 7)]
            self.write_state("pan.cycle_no", cycle_no + 1)
            cycle_no = int(self.read_state("pan.cycle_no"))
            if cycle_no - cycle_no_init == 600:
                self.logger.put(
                    "Power cycled outputs could not turn off after 600 cycles (1 minute)")
        # wait for outputs to turn on again
        while (not all(out == True for out in output)) and cycle_no - cycle_no_init < 600:
            output = [self.str_to_bool(self.read_state("gomspace.output.output" + str(i)))
                      for i in range(1, 7)]
            self.write_state("pan.cycle_no", cycle_no + 1)
            cycle_no = int(self.read_state("pan.cycle_no"))
            if cycle_no - cycle_no_init == 600:
                self.logger.put(
                    "Power cycled outputs could not turn on after 600 cycles (1 minute)")
        # check if finished power cycling
        power_cycle_output_cmd = [self.str_to_bool(self.read_state("gomspace.power_cycle_output"
                                                              + str(i) + "_cmd"))
                                  for i in range(1, 7)]
        for n in range(0, len(power_cycle_output_cmd)):
            if power_cycle_output_cmd[n] == True:
                self.logger.put("Could not update power_cycle_output" + str(n))

        ppt_mode_cmd = int(self.read_state("gomspace.pptmode_cmd"))
        ppt_mode_updated = int(self.write_state(
            "gomspace.pptmode_cmd", (int(ppt_mode_cmd) + 1) % 2))
        if ppt_mode_cmd == ppt_mode_updated:
            self.logger.put("Could not update pptmode")

        heater_cmd = self.str_to_bool(self.read_state("gomspace.heater_cmd"))
        heater_cmd_updated = self.str_to_bool(self.write_state(
            "gomspace.heater_cmd", not heater_cmd))
        if heater_cmd == heater_cmd_updated:
            self.logger.put("Could not update heater")

        counter_reset_cmd = self.str_to_bool(self.read_state(
            "gomspace.counter_reset_cmd"))
        counter_reset_cmd_updated = self.str_to_bool(self.write_state(
            "gomspace.counter_reset_cmd", not counter_reset_cmd))
        if counter_reset_cmd == counter_reset_cmd_updated:
            self.logger.put("Could not update counter_reset")

        gs_reset_cmd = self.str_to_bool(
            self.read_state("gomspace.gs_reset_cmd"))
        gs_reset_cmd_updated = self.str_to_bool(self.write_state(
            "gomspace.gs_reset_cmd", not gs_reset_cmd))
        if gs_reset_cmd == gs_reset_cmd_updated:
            self.logger.put("Could not update gs_reset")

        gs_reboot_cmd = self.str_to_bool(
            self.read_state("gomspace.gs_reboot_cmd"))
        gs_reboot_cmd_updated = self.str_to_bool(self.write_state(
            "gomspace.gs_reboot_cmd", not gs_reboot_cmd))
        if gs_reboot_cmd == gs_reboot_cmd_updated:
            self.logger.put("Could not update gs_reboot")

        self.finish()

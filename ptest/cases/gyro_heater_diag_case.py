# GyroHeaterDiagCase - A manual diagonstic case to checkout the behavior of the gyro heater
from .base import SingleSatCase
from .utils import TestCaseFailure, Enums
import time
    
class GyroHeaterDiagCase(SingleSatCase):

    def setup_post_bootsetup(self):
        self.print_header("Begin Gyro Heater Diagnostic Case")

        self.ws("pan.state", Enums.mission_states["manual"])
        self.ws("adcs.state", Enums.adcs_states["manual"])
        
        self.ws("cycle.auto", False) # turn off auto cycle in case it was on
        self.cycle()

    def shutdown(self):

        # Shutdown
        self.print_ws("adcs_cmd.imu_gyr_temp_pwm", 24) # Arbitrary non default
        self.print_ws("adcs_cmd.imu_gyr_temp_desired", 20) # Back to low temp
        self.print_ws("adcs_cmd.havt_disable18", True) # disable heater
        self.cycle()

    def run_case_singlesat(self):

        self.print_rs("adcs_monitor.functional")

        if not self.rs("adcs_monitor.functional"):
            raise TestCaseFailure("ADCS Not Functional")

        self.print_havt_read()
        self.print_non_functional_adcs_havt()

        # turn on heater
        self.ws("adcs_cmd.havt_reset18", True)
        self.cycle()

        self.print_havt_read()
        self.print_non_functional_adcs_havt()

        if not self.rs("adcs_monitor.havt_device18"): # gyro heater
            raise TestCaseFailure("GYRO HEATER not functional")

        if not self.rs("adcs_monitor.havt_device0"): # IMU GYR
            raise TestCaseFailure("GYRO not functional")
        
        init_temp = self.print_rs("adcs_monitor.gyr_temp")
        init_cursys = self.print_rs("gomspace.cursys")

        init_curout_arr = [self.rs(f"gomspace.curout.output{x}") for x in range(1,7)]
        self.logger.put("INIT_CUROUT ARR: "+str(init_curout_arr))

        self.target = 40

        self.ws("adcs_cmd.imu_gyr_temp_pwm", 255) # Arbitrary non default
        self.ws("adcs_cmd.imu_gyr_temp_desired", 40) # High temp to let rise and fall
        self.print_rs("adcs_cmd.imu_gyr_temp_pwm")
        self.print_rs("adcs_cmd.imu_gyr_temp_desired")
        
        self.logger.put("")
        self.cycle()

        self.print_header("ENTERING HEATING CONTROL")

        # User please watch screen, and observe temp
        start_time = time.time()
        start_cycle = self.print_rs("pan.cycle_no")
        last_cycle = 0

        while time.time() - start_time < 60*2: # 2 mins
            elapse = time.time() - start_time
            self.logger.put(f"TIME ELAPSE (s): {elapse}")
            last_cycle = self.print_rs("pan.cycle_no")
            self.print_rs("gomspace.vbatt")
            self.print_rs("gomspace.cursys")
            
            self.logger.put("INIT_CUROUT ARR: "+str(init_curout_arr))
            curout_arr = [self.rs(f"gomspace.curout.output{x}") for x in range(1,7)]
            self.logger.put("CUROUT ARR: "+str(curout_arr))

            self.print_rs("adcs_cmd.imu_gyr_temp_pwm")
            self.print_rs("adcs_cmd.imu_gyr_temp_desired")

            self.logger.put(f"INITIAL TEMP: {init_temp}")
            self.logger.put(f"INIT CUR: {init_cursys}")
            
            heater = self.print_rs("adcs_monitor.havt_device18")
            imu_gyr = self.print_rs("adcs_monitor.havt_device0")

            temp_reading = self.print_rs("adcs_monitor.gyr_temp")

            if not heater:
                self.shutdown()
                raise TestCaseFailure("HEATER DEVICE REPORTING AS NON FUNCTIONAL, SHUTTING DOWN.")

            if not imu_gyr:
                self.shutdown()
                raise TestCaseFailure("HEATER DEVICE REPORTING AS NON FUNCTIONAL, SHUTTING DOWN.")

            if temp_reading < 18 or temp_reading > self.target + 5:
                self.shutdown()
                raise TestCaseFailure("TEMP READING OUTTA BOUNDS")

            self.logger.put("")
            self.cycle()

        self.print_header(f"DONE WITH {last_cycle-start_cycle} CYCLES")

        self.print_header("FINAL TEMP: ")
        final_temp = self.print_rs("adcs_monitor.gyr_temp")
        
        target_temp = self.print_rs("adcs_cmd.imu_gyr_temp_desired")

        self.shutdown()

        delta = 1 # 1 degree of delta accepted
        
        if target_temp - delta > final_temp or target_temp + delta < final_temp:
            self.logger.put("") 
            raise TestCaseFailure(f"Final Temp: {final_temp} NOT within {delta} of {target_temp}")

        self.print_header(f"Temp within delta of {delta}")
        self.print_header("TEST PASSED")
        self.finish()
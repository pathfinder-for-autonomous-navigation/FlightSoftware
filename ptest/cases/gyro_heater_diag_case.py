# GyroHeaterDiagCase - A manual diagonstic case to checkout the behavior of the gyro heater
from .base import SingleSatOnlyCase, TestCaseFailure
import time
    
class GyroHeaterDiagCase(SingleSatOnlyCase):

    def setup_case_singlesat(self):
        self.print_header("Begin Gyro Heater Diagnostic Case")

        self.ws("pan.state", self.mission_states.get_by_name("manual"))
        self.ws("adcs.state", self.adcs_states.get_by_name("point_manual"))
        
        self.ws("adcs_cmd.imu_gyr_temp_pwm", 254) # Arbitrary non default
        self.ws("adcs_cmd.imu_gyr_temp_desired", 30) # High temp to let rise and fall

        self.ws("cycle.auto", False) # turn off auto cycle in case it was on
        self.cycle()

    def shutdown(self):

        # Shutdown
        self.ws("adcs_cmd.imu_gyr_temp_pwm", 88) # Arbitrary non default
        self.ws("adcs_cmd.imu_gyr_temp_desired", 20) # Back to low temp
        self.ws("adcs_cmd.havt_disable18", True) # disable heater

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

        self.print_rs("adcs_cmd.imu_gyr_temp_pwm")
        self.print_rs("adcs_cmd.imu_gyr_temp_desired")
        
        init_temp = self.print_rs("adcs_monitor.gyr_temp")
        init_cursys = self.print_rs("gomspace.cursys")

        curout_arr = [self.rs(f"gomspace.curout.output{x}") for x in range(1,7)]
        self.logger.put("CUROUT ARR: "+str(curout_arr))

        self.logger.put("")
        self.cycle()

        self.print_header("ENTERING HEATING CONTROL")

        # User please watch screen, and observe temp
        start_time = time.time()

        while time.time() - start_time < 60*5: # 5 mins
            elapse = time.time() - start_time
            self.logger.put(f"TIME ELAPSE (s): {elapse}")
            self.print_rs("pan.cycle_no")
            self.print_rs("gomspace.vbatt")
            self.print_rs("gomspace.cursys")
            self.logger.put("CUROUT ARR: "+str(curout_arr))

            self.print_rs("adcs_cmd.imu_gyr_temp_pwm")
            self.print_rs("adcs_cmd.imu_gyr_temp_desired")

            self.logger.put(f"INITIAL TEMP: {init_temp}")
            self.logger.put(f"INIT CUR: {init_cursys}")
            
            heater = self.print_rs("adcs_monitor.havt_device18")
            imu_gyr = self.print_rs("adcs_monitor.havt_device0")

            temp_reading = self.print_rs("adcs_monitor.gyr_temp")

            if not heater or not imu_gyr:
                self.shutdown()
                raise TestCaseFailure("DEVICE FAILED MID TEST")

            if temp_reading < 18 or temp_reading > 38:
                self.shuutdown()
                raise TestCaseFailure("TEMP READING OUTTA BOUNDS")

            self.logger.put("")
            self.cycle()

        self.print_header("DONE WITH N CYCLES")

        self.print_header("FINAL TEMP: ")
        final_temp = self.print_rs("adcs_monitor.gyr_temp")
        
        target_temp = self.print_rs("adcs_cmd.imu_gyr_temp_desired")

        self.shutdown()

        delta = 5 # 5 deg diff accepted?
        
        if target_temp - delta > final_temp or target_temp + delta < final_temp:
            self.logger.put("") 
            raise TestCaseFailure(f"Final Temp: {final_temp} NOT within {delta} of {target_temp}")

        self.print_header(f"Temp within delta of {delta}")
        self.print_header("TEST PASSED")
        self.finish()
# GyroHeaterDiagCase - A manual diagonstic case to checkout the behavior of the gyro heater
from .base import SingleSatOnlyCase, TestCaseFailure

    
class GyroHeaterDiagCase(SingleSatOnlyCase):

    def setup_case_singlesat(self):
        self.print_header("Begin Gyro Heater Diagnostic Case")

        self.ws("pan.state", self.mission_states.get_by_name("manual"))
        self.ws("adcs.state", self.adcs_states.get_by_name("point_manual"))
        
        self.ws("adcs_cmd.imu_gyr_temp_pwm", 100) # Arbitrary non default
        self.ws("adcs_cmd.imu_gyr_temp_desired", 35) # High temp to let rise and fall

        self.ws("cycle.auto", False) # turn off auto cycle in case it was on
        self.cycle()
    def run_case_singlesat(self):

        self.print_rs("adcs_monitor.functional")

        if not self.rs("adcs_monitor.functional"):
            raise TestCaseFailure("ADCS Not Functional")

        self.print_havt_read()
        self.print_non_functional_adcs_havt()

        if not self.rs("adcs_monitor.havt_device18"): # gyro heater
            raise TestCaseFailure("GYRO HEATER not functional")

        if not self.rs("adcs_monitor.havt_device0"): # IMU GYR
            raise TestCaseFailure("GYRO not functional")

        self.print_header("ENTERING HEATING CONTROL")

        self.print_rs("adcs_cmd.imu_gyr_temp_pwm")
        self.print_rs("adcs_cmd.imu_gyr_temp_desired")
        self.logger.put("")
        self.cycle()

        # 1000 cycle heater diagnostic test

        # User please watch screen, and observe temp
        for i in range(1000):
            self.logger.put(f"Reading #{i}")
            self.print_rs("pan.cycle_no")
            self.print_rs("gomspace.vbatt")
            self.print_rs("adcs_monitor.gyr_temp")
            self.logger.put("")
            self.cycle()

        self.print_header("DONE WITH N CYCLES")

        self.print_header("FINAL TEMP: ")
        final_temp = self.print_rs("adcs_monitor.gyr_temp")
        
        target_temp = self.print_rs("adcs_cmd.imu_gyr_temp_desired")

        # Shutdown
        self.ws("adcs_cmd.imu_gyr_temp_pwm", 88) # Arbitrary non default
        self.ws("adcs_cmd.imu_gyr_temp_desired", 20) # Back to low temp

        delta = 5 # 5 deg diff accepted?
        
        if target_temp - delta > final_temp or target_temp + delta < final_temp:
            self.logger.put("") 
            raise TestCaseFailure(f"Final Temp: {final_temp} NOT within {delta} of {target_temp}")

        self.print_header(f"Temp within delta of {delta}")
        self.print_header("TEST PASSED")
        self.finish()
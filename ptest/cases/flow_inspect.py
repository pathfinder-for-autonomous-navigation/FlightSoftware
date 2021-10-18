# Empty test case. Gets cycle count purely for diagnostic purposes
import time

from .base import SingleSatCase, PSimCase
from .utils import Enums, TestCaseFailure, str_to_val
from psim.sims import SingleAttitudeOrbitGnc

class FlowInspect(SingleSatCase):
    
    # all_state_fields = ['adcs.state', 'adcs_monitor.functional_fault.base', 'adcs_monitor.gyr_vec.x', 'adcs_monitor.gyr_vec.y', 'adcs_monitor.gyr_vec.z', 'adcs_monitor.havt_device0', 'adcs_monitor.havt_device1', 'adcs_monitor.havt_device10', 'adcs_monitor.havt_device11', 'adcs_monitor.havt_device12', 'adcs_monitor.havt_device13', 'adcs_monitor.havt_device14', 'adcs_monitor.havt_device15', 'adcs_monitor.havt_device16', 'adcs_monitor.havt_device17', 'adcs_monitor.havt_device2', 'adcs_monitor.havt_device3', 'adcs_monitor.havt_device4', 'adcs_monitor.havt_device5', 'adcs_monitor.havt_device6', 'adcs_monitor.havt_device7', 'adcs_monitor.havt_device8', 'adcs_monitor.havt_device9', 'adcs_monitor.mag1_vec.x', 'adcs_monitor.mag1_vec.y', 'adcs_monitor.mag1_vec.z', 'adcs_monitor.mag2_vec.x', 'adcs_monitor.mag2_vec.y', 'adcs_monitor.mag2_vec.z', 'adcs_monitor.rwa_speed_rd.x', 'adcs_monitor.rwa_speed_rd.y', 'adcs_monitor.rwa_speed_rd.z', 'adcs_monitor.rwa_torque_rd.x', 'adcs_monitor.rwa_torque_rd.y', 'adcs_monitor.rwa_torque_rd.z', 'adcs_monitor.ssa_mode', 'adcs_monitor.ssa_vec', 'adcs_monitor.wheel1_fault.base', 'adcs_monitor.wheel2_fault.base', 'adcs_monitor.wheel3_fault.base', 'adcs_monitor.wheel_pot_fault.base', 'attitude.pointer_vec1_current', 'attitude.pointer_vec1_desired', 'attitude.pointer_vec2_current', 'attitude.pointer_vec2_desired', 'attitude_estimator.L_body', 'attitude_estimator.fault.base', 'attitude_estimator.ignore_sun_vectors', 'attitude_estimator.q_body_eci', 'attitude_estimator.valid', 'docksys.docked', 'gomspace.cursun', 'gomspace.cursys', 'gomspace.get_hk.base', 'gomspace.low_batt.base', 'gomspace.temp.output1', 'gomspace.temp.output2', 'gomspace.temp.output3', 'gomspace.temp.output4', 'gomspace.vbatt', 'orbit.control.J_ecef', 'orbit.control.alpha', 'orbit.control.num_near_field_nodes', 'orbit.control.valve1', 'orbit.control.valve2', 'orbit.control.valve3', 'orbit.control.valve4', 'orbit.pos', 'orbit.valid', 'orbit.vel', 'pan.bootcount', 'pan.cycle_no', 'pan.deployed', 'pan.deployment.elapsed', 'pan.sat_designation', 'pan.state', 'piksi.state', 'piksi_fh.dead.base', 'prop.overpressured.base', 'prop.pressurize_fail.base', 'prop.state', 'prop.tank1_temp_high.base', 'prop.tank2_temp_high.base', 'qfh.state', 'radio.err', 'radio.last_comms_ccno', 'radio.state', 'rel_orbit.state', 'time.gps', 'time.valid']
    
    def __init__(self, *args, **kwargs):
        super(FlowInspect, self).__init__(*args, **kwargs)
        self.check_initial_state = False

    def special_function(self):
        self.ws('telem.dump', True)
        self.cycle()
        self.cycle()
        garb = self.flight_controller.parsetelem()
        garb = self.flight_controller.parsetelem()
        
        self.all_state_fields = [k for k in garb]
        
        self.cycle()
        self.cycle()
        
        rs_results = {}
        for sf_name in self.all_state_fields:
            rs_results[sf_name] = self.rs(sf_name)

        self.ws('telem.dump', True)
        self.cycle()
        time.sleep(0.5)

        # garbage parsetelem
        garb = self.flight_controller.parsetelem()
        
        parsetelem_results = self.flight_controller.parsetelem()
        print(parsetelem_results)
        parsetelem_results ={k:str_to_val(v) for k,v in parsetelem_results.items()}
        
        # print('PARSETELEM RESULTS: ')
        # print(parsetelem_results)
     
        # print('RS RESULTS: ')       
        # print(rs_results)

        diffs = [sf_name for sf_name in self.all_state_fields if parsetelem_results[sf_name] != rs_results[sf_name]]
        
        print('DIFFS')
        
        for diff in diffs:
            print(f'{diff} {rs_results[diff]} {parsetelem_results[diff]}')
        

    def run(self):
        self.cycle()
        self.cycle_no = self.flight_controller.read_state("pan.cycle_no")
        
        self.cycle()
        self.cycle()
        self.ws('time.gps', '2047,125,-233')        
        self.ws('orbit.pos', '6000000,2500000,2000000')
        self.ws('orbit.vel', '5700,4100,2000')
        self.ws('attitude_estimator.q_body_eci', '0.221896,0.554252,0.442815,0.668940')
        self.ws('gomspace.vbatt', 6900)
        self.ws('time.valid', True)   
        self.ws('orbit.valid', True)
        self.ws('attitude_estimator.ignore_sun_vectors', True)   
        self.ws('attitude_estimator.valid', True)
        for i in range(9):
            self.ws(f'a.{i}', True)
            
        self.ws('cursed', True)

        # get every single statefield in ptest
        # build a dict of the results

        self.special_function()
        # get the parsetelem
        
        # compare the results
        # return a dict of all the diffs
        
        # for diff in diffs, print diff
        
        self.logger.put("END")
        self.finish()
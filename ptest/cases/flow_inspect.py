# Empty test case. Gets cycle count purely for diagnostic purposes
from termios import PARENB
from .base import SingleSatCase, PSimCase
from .utils import Enums, TestCaseFailure, str_to_val
from psim.sims import SingleAttitudeOrbitGnc

class FlowInspect(SingleSatCase):
    def __init__(self, *args, **kwargs):
        super(FlowInspect, self).__init__(*args, **kwargs)
        self.check_initial_state = False

    def special_function(self):
        self.cycle()
        self.ws('telem.dump', True)
        self.cycle()
        self.cycle()
        self.cycle()
        
        # garbage parsetelem
        garb = self.flight_controller.parsetelem()
        
        parsetelem_results = self.flight_controller.parsetelem()
        parsetelem_results ={k:str_to_val(v) for k,v in parsetelem_results.items()}
        
        print('PARSETELEM RESULTS: ')
        print(parsetelem_results)
        
        all_the_statefields = [key for key in parsetelem_results]
        rs_results = {}
        
        for sf_name in all_the_statefields:
            rs_results[sf_name] = self.rs(sf_name)
     
        print('RS RESULTS: ')       
        print(rs_results)

        diffs = [sf_name for sf_name in all_the_statefields if parsetelem_results[sf_name] != rs_results[sf_name]]
        
        print('DIFFS')
        
        for diff in diffs:
            print(f'{diff} {rs_results[diff]} {parsetelem_results[diff]}')
        

    def run(self):
        self.cycle()
        self.cycle_no = self.flight_controller.read_state("pan.cycle_no")
        

        # get every single statefield in ptest
        # build a dict of the results

        self.special_function()
        # get the parsetelem
        
        # compare the results
        # return a dict of all the diffs
        
        # for diff in diffs, print diff
        
        self.logger.put("END")
        self.finish()
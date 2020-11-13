from .base import SingleSatOnlyCase, TestCaseFailure
from .utils import Enums
import math

class ResetforFlight(SingleSatOnlyCase):

  #Sets gomspace ppt mode to MPPT(1) and fails if it is not successfully updated
  def set_ppt(self):
    ppt_mode_cmd = int(self.rs("gomspace.pptmode_cmd"))
    self.ws("gomspace.pptmode_cmd", 1)
    self.cycle()
    ppt_mode_updated = int(self.rs("gomspace.pptmode_cmd"))
    if ppt_mode_cmd == ppt_mode_updated:
      self.logger.put("Could not update pptmode")
      self.failed = True

  #sets the bootcount to 0 and confirms it was changed correctly
  def set_bootcount(self):
    bootcount = self.rs("pan.bootcount")
    self.ws("pan.bootcount", 0)
    self.cycle()
    bootcount_updated = self.rs("pan.bootcount")

    if bootcount == bootcount_updated:
      self.logger.put("Could not update bootcount")
      self.failed = True
    
  def run_case_singlesat(self):
    self.print_header("Starting Reset")
    self.ws( "cycle.auto", False )
    self.set_ppt()
    self.set_bootcount()

    #ensure autocycling is happening 
    #make sure spacecraft is shut down right after this test case so mission doesnt start executing
    self.ws( "cycle.auto", True )
    self.print_rs("cycle.auto")

    self.print_header("Flight Reset Complete")
    self.finish()
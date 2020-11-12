from .base import SingleSatOnlyCase, TestCaseFailure
from .utils import Enums
import math

class ResetforFlight(SingleSatOnlyCase):

  #Sets gomspace ppt mode to MPPT(1) and fails if it is not successfully updated
  def set_ppt(self):
    ppt_mode_cmd = int(self.read_state("gomspace.pptmode_cmd"))
    ppt_mode_updated = int(self.ws("gomspace.pptmode_cmd", 1))
    if ppt_mode_cmd == ppt_mode_updated:
      self.logger.put("Could not update pptmode")
      self.failed = True

  def run_case_singlesat(self):
      self.set_ppt()

      #ensure autocycling is happening
      self.ws( "cycle.auto", True )

      #set bootcount to zero
      self.ws("pan.bootcount", 0)
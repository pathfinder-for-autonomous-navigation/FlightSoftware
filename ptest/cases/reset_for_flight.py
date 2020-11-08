from .base import SingleSatOnlyCase, TestCaseFailure
from .utils import Enums
import math

class ResetforFlight(SingleSatOnlyCase):

  def run_case_singlesat(self):
      self.ws("gomspace.ppt_mode_cmd", 1)#max ppt
      self.ws( "cycle.auto", True )
      self.ws("pan.bootcount", 0)
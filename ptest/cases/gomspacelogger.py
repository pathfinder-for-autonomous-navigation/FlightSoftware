# Runs mission from startup state to standby state.
from .base import SingleSatCase
from psim.sims import SingleAttitudeOrbitGnc
from .utils import Enums, mag_of, sum_of_differentials
import time

class GomspaceLog(SingleSatCase):

    def data_logs(self):
      pass
      self.rs("gomspace.vboost.output1")
      self.rs("gomspace.vboost.output2")
      self.rs("gomspace.vboost.output3")

      self.rs("gomspace.vbatt")

      self.rs("gomspace.curout.output1")
      self.rs("gomspace.curout.output2")
      self.rs("gomspace.curout.output3")
      self.rs("gomspace.curout.output4")
      self.rs("gomspace.curout.output5")
      self.rs("gomspace.curout.output6")

      self.rs("gomspace.curin.output1")
      self.rs("gomspace.curin.output2")
      self.rs("gomspace.curin.output3")

      self.rs("gomspace.cursun")
      self.rs("gomspace.cursys")
      self.rs("gomspace.battmode")
      self.rs("gomspace.pptmode")

    def run(self):
        
        self.ws("cycle.auto", False)
        self.cycle()
        while not self.finished: 
          self.rs("pan.cycle_no")
          self.data_logs()
          self.cycle()
        self.cycle()
        self.finish()


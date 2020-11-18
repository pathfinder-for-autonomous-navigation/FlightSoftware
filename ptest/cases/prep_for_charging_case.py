# Pre-integration procedure for battery charging.
from .base import SingleSatOnlyCase


class PrepForCharging(SingleSatOnlyCase):

  def run_case_singlesat(self):
    # Set PPT mode to 2 while charging the battery.
    self.ws("gomspace.pptmode",2)
    pptmode = self.rs("gomspace.pptmode")
    self.cycle()
    
    self.logger.put("                                 ")
    self.logger.put("=================================")
    self.logger.put("=================================")
    self.logger.put("Satellite is prepped for charging.")
    self.logger.put("=================================")
    self.logger.put("pptmode was set to: " + str(pptmode))
    self.logger.put("=================================")
    self.logger.put("=================================")
    self.logger.put("                                 ")
    self.finish()
    
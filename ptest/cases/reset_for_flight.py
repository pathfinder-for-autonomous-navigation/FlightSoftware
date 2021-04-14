from .base import SingleSatCase
from .utils import Enums, TestCaseFailure
import math

class ResetforFlight(SingleSatCase):

  #takes in field name and value to set, then confirms the value was correctly
  #set 
  # @param field: state field to update
  # @param value: value to update field with
  def set_confirm(self, field, value):
    initialstate = self.rs( field )
    self.ws( field, value )
    self.cycle()
    updatestate = self.rs( field )
    if ( updatestate == initialstate ):
      self.logger.put("Could not update" + str(field))
      self.failed = True
    self.logger.put("Reset " + str(field) + " to " + str(value))
    
  def run(self):
    self.print_header("Starting Reset")
    self.ws( "cycle.auto", False )
    self.set_confirm("gomspace.pptmode_cmd", 1)
    self.set_confirm("pan.bootcount", 0)
    self.set_confirm("pan.deployment.elapsed", 0)
    self.set_confirm("pan.deployed", False)

    #ensure autocycling is happening 
    #make sure spacecraft is shut down right after this test case so mission doesnt start executing
    self.ws( "cycle.auto", True )
    self.cycle()
    self.print_rs("cycle.auto")

    self.print_header("Flight Reset Complete")
    self.finish()
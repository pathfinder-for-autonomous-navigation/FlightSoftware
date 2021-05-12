# Runs mission from startup state to standby state.
from psim.sims import SingleAttitudeOrbitGnc
from .utils import Enums, mag_of, sum_of_differentials
from .mission.standby import SingleSatStandbyCase

class TaskTime(SingleSatStandbyCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def post_boot(self):
      self.ws("dcdc.ADCSMotor_cmd", False)
    def data_logs(self):
      self.rs("timing.debug.duration")
      self.rs("timing.eeprom_ct.duration")
      self.rs("timing.piksi.duration")
      self.rs("timing.adcs_monitor.duration")
      self.rs("timing.gomspace_rd.duration")
      self.rs("timing.uplink_ct.duration")
      self.rs("timing.mission_ct.duration")
      self.rs("timing.dcdc_ct.duration")
      self.rs("timing.attitude_controller.duration")
      self.rs("timing.adcs_commander.duration")
      self.rs("timing.orbit_control_ct.duration")
      self.rs("timing.prop.duration")
      self.rs("timing.docking_ct.duration")
      self.rs("timing.downlink_ct.duration")
      self.rs("timing.quake.duration")
      self.rs("timing.estimators.duration")
      self.rs("timing.adcs_controller.duration")


    def run(self):
        self.cycle()
        self.data_logs()
        self.finish()
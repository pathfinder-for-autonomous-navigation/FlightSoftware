# Empty test case. Gets cycle count purely for diagnostic purposes
from .base import SingleSatCase, PSimCase
from .utils import Enums, TestCaseFailure
from psim.sims import SingleAttitudeOrbitGnc

class EmptyCase(SingleSatCase):
    def __init__(self, *args, **kwargs):
        super(EmptyCase, self).__init__(*args, **kwargs)
        self.check_initial_state = False

    def run(self):
        self.cycle_no = self.flight_controller.read_state("pan.cycle_no")
        self.finish()
        
    def special(self):
        self.ws('telem.dump', True)
        self.cycle()

class FailingEmptyCase(SingleSatCase):
    def run(self):
        raise TestCaseFailure("Deliberate failure intended to test failure in CI.")

class EmptySimCase(SingleSatCase, PSimCase):
    def __init__(self, *args, **kwargs):
        super(EmptySimCase, self).__init__(*args, **kwargs)
        self.psim_configs += ['truth/deployment']

    def run(self):
        self.finish()

class FailingEmptySimCase(SingleSatCase, PSimCase):
    def __init__(self, *args, **kwargs):
        super(FailingEmptySimCase, self).__init__(*args, **kwargs)
        self.psim_configs += ['truth/deployment']

    def run(self):
        raise TestCaseFailure("Deliberate failure intended to test failure in CI.")

from ...data_consumers import Logger
import time
import math
import threading
import traceback
from ..utils import Enums, TestCaseFailure, suppress_faults
import psim # the actual python psim repo
import lin
import datetime
from .ptest_case import PTestCase

class AMCCase(PTestCase):
    """Base class for all HOOTL and HITL testcases involving two satellites.
    See the SingleSatCase for descriptions of the attributes. Many of the other
    functions used here mirror elements of SingleSatCase as well so be sure to
    checkout the documentation there too.
    """
    def populate_devices(self, devices, radios):
        self.radio_leader = radios["FlightControllerLeaderRadio"]
        self.radio_follower = radios["FlightControllerFollowerRadio"]
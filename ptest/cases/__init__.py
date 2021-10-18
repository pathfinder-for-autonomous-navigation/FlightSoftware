# REMEMBER to also add cases to:
# - tools/alltest.sh
# - .github/workflows/hootl.yml

# Testcases that require simulation
from .attitude_estimator import AttitudeFaultHandlerCase, AttitudeNoSunVectorsInitializationCase
from .empty_case import EmptySimCase, FailingEmptySimCase
from .dual_empty_case import DualEmptySimCase
from .piksi_fault_handler import PiksiFaultHandler
from .autonomous_mission_manager_pure_radio import AutonomousMissionController

# Testcases intended for use with mission rehearsals and general telemetry
# testing.
from .mission import (
    DualSatDetumbleCase,
    DualSatFarFieldCase,
    DualSatNearFieldCase,
    DualSatStandbyCase,
    DualSatStartupCase,
    SingleSatDetumbleCase,
    SingleSatStandbyCase,
    SingleSatStartupCase
)

from .empty_case import EmptyCase, FailingEmptyCase
from .dual_empty_case import DualEmptyCase
from .ditl import Ditl
from .ditl_case import DitlCase
from .gomspace_checkout_case import GomspaceCheckoutCase, CheckBatteryLevel
from .docking_checkout_case import DockingCheckoutCase
from .dcdc_checkout_case import DCDCCheckoutCase
from .prop_state_machine_case import PropStateMachineCase
from .spin_motors_case import SpinMotorsCase
from .adcs_checkout_case import ADCSCheckoutCase
from .piksi_checkout_case import PiksiCheckoutCase
from .deployment_to_init_hold_case import DeploymentToInitHold
from .gyro_heater_diag_case import GyroHeaterDiagCase
from .ci_case import CICase
from .simple_fault_handlers import ADCSWheelFaultHandler, LowBattFaultHandler
from .safehold_reboot import SafeholdReboot
from .prop_fault_handler import PropFaultHandler
from .gomspace_long_duration_case import GomspaceLongDurationCheckoutCase
from .hardware_stress_test_case import HardwareStressCheckoutCase
from .torquer_rotate_case import MTorquerCase
from .reset_for_flight import ResetforFlight
from .safehold_standby_transition_case import SafeholdStandbyTransitionCase
from .gomspacelogger import GomspaceLog
from .reboot_utility import Reboot
from .quake_power_cycling import QuakePowerCycling
from .piksi_near_field import PiksiFaultNearField
from .piksi_near_field_2 import PiksiFaultNearField2
from .flow_inspect import FlowInspect
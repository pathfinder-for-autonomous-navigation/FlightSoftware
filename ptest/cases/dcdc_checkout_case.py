from .base import SingleSatCase
from .utils import Enums, TestCaseFailure

class DCDCCheckoutCase(SingleSatCase):

    def __init__(self, *args, **kwargs):
        super(DCDCCheckoutCase, self).__init__(*args, **kwargs)

        self.__adcs_command = None
        self.__adcs = None
        self.__prop_command = None
        self.__prop = None
        self.__disable_command = None
        self.__reset_command = None

    @property
    def adcs_command(self) -> bool:
        assert self.__adcs_command is not None
        return self.__adcs_command

    @adcs_command.setter
    def adcs_command(self, value: bool):
        self.ws("dcdc.ADCSMotor_cmd", value)

    @property
    def adcs(self) -> bool:
        assert self.__adcs is not None
        return self.__adcs

    @property
    def prop_command(self) -> bool:
        assert self.__prop_command is not None
        return self.__prop_command

    @prop_command.setter
    def prop_command(self, value: bool):
        self.ws("dcdc.SpikeDock_cmd", value)

    @property
    def prop(self) -> bool:
        assert self.__prop is not None
        return self.__prop

    @property
    def disable_command(self) -> bool:
        assert self.__disable_command is not None
        return self.__disable_command

    @disable_command.setter
    def disable_command(self, value: bool):
        self.ws("dcdc.disable_cmd", value)

    @property
    def reset_command(self) -> bool:
        assert self.__reset_command is not None
        return self.__reset_command

    @reset_command.setter
    def reset_command(self, value: bool):
        self.ws("dcdc.reset_cmd", value)

    def abort(self, *args, **kwargs):
        """Logs a table of the current DCDC state fields to the terminal,
        attempts to shutdown the DCDCs, and raises a TestCaseFailure.
        """
        self.logger.put(f"Testcase failed:")
        self.logger.put(f"\tadcs_command    = {self.adcs_command}")
        self.logger.put(f"\tadcs            = {self.adcs}")
        self.logger.put(f"\tprop_command    = {self.prop_command}")
        self.logger.put(f"\tprop            = {self.prop}")
        self.logger.put(f"\tdisable_command = {self.disable_command}")
        self.logger.put(f"\treset_command   = {self.reset_command}")

        self.adcs_command = False
        self.prop_command = False
        self.cycle()

        raise TestCaseFailure(*args, **kwargs)

    def cycle(self):
        """Cycles the flight computer and reads in all DCDC state fields.

        All DCDC reads are performed here to avoid extra overhead from duplicate
        read state calls. In addition, it makes implementing the very useful
        abort function trivial.
        """
        super(DCDCCheckoutCase, self).cycle()

        self.__adcs_command = self.rs("dcdc.ADCSMotor_cmd")
        self.__adcs = self.rs("dcdc.ADCSMotor")
        self.__prop_command = self.rs("dcdc.SpikeDock_cmd")
        self.__prop = self.rs("dcdc.SpikeDock")
        self.__disable_command = self.rs("dcdc.disable_cmd")
        self.__reset_command = self.rs("dcdc.reset_cmd")
        self.rs("pan.cc_duration")

    def reset(self, has_enabled = True):
        """Runs and verifies the DCDC reset command executes as expected.
        """
        self.reset_command = True
        self.cycle()

        if has_enabled:
            if not self.reset_command:
                self.abort("Reset command should persist for one cycle is some elements are enabled.")

            self.cycle()

        if not self.adcs_command or not self.adcs or \
                not self.prop_command or not self.prop or self.reset_command:
            self.abort("Failed to reset the DCDCs.")

    def run(self):
        self.cycle()

        if self.adcs_command or self.adcs or self.prop or self.prop_command or \
                self.disable_command or self.reset_command:
            self.abort("All fields should be false on initialization; " +
                       "ensure the spacecraft was rebooted before running this case.")
        else:
            self.logger.put("Testcase initialized correctly.")
            self.logger.put("")

        self.logger.put("Step 1: Enable both DCDCs...")

        self.adcs_command = True
        self.prop_command = True
        self.cycle()

        if self.adcs_command and self.adcs and self.prop_command and self.prop:
            self.logger.put("Success!")
        else:
            self.abort("Failed to enable both DCDCs.")

        self.logger.put("")
        self.logger.put("Step 2: Set only ADCS to be on and then attempt a reset command...")

        self.adcs_command = True
        self.prop_command = False
        self.cycle()

        if not self.adcs_command or not self.adcs or self.prop_command or self.prop:
            self.abort("Failed to enable only the ADCS DCDC.")

        self.reset()
        self.logger.put("Success!")
        self.logger.put("")

        self.logger.put("Step 3: Enable only Prop to be on and then attempt a reset command...")

        self.adcs_command = False
        self.prop_command = True
        self.cycle()

        if self.adcs_command or self.adcs or not self.prop or not self.prop_command:
            self.abort("Failed to enable only the Prop DCDC.")

        self.reset()
        self.logger.put("Success!")
        self.logger.put("")

        self.logger.put("Step 4: Disable both DCDCs and attempt a reset command...")

        self.disable_command = True
        self.cycle()

        if self.adcs_command or self.adcs or self.prop_command or self.prop or self.disable_command:
            self.abort("Failed to disable the DCDCs.")

        self.reset(False)
        self.logger.put("Success!")
        self.logger.put("")

        self.adcs_command = False
        self.prop_command = False
        self.cycle()

        self.finish()

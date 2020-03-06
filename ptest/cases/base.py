# Base classes for writing testcases.


class Case(object):
    @property
    def run_sim(self):
        return False

    @property
    def single_sat_compatible(self):
        raise NotImplementedError

    def setup_case(self, simulation):
        raise NotImplementedError

    def run_case(self, simulation):
        raise NotImplementedError

    def cycle(self):
        self.simulation.flight_controller.write_state("cycle.start", "true")

# Base testcase for writing testcases that only work with a single-satellite mission.


class SingleSatOnlyCase(Case):
    @property
    def single_sat_compatible(self):
        return True

    def setup_case(self, simulation):
        if simulation.is_single_sat_sim:
            self.setup_case_singlesat(simulation)
        else:
            raise NotImplementedError

    def run_case(self, simulation):
        if not simulation.is_single_sat_sim:
            raise Exception(
                f"Testcase {__class__.__name__} only works for a single-satellite simulation.")
        self.run_case_singlesat(simulation)

    def run_case_singlesat(self, simulation):
        raise NotImplementedError

# Base testcase for writing testcases that only work with a full mission simulation
# with both satellites.


class MissionCase(Case):
    @property
    def single_sat_compatible(self):
        return False

    def run_case(self, simulation):
        if simulation.is_single_sat_sim:
            raise Exception(
                f"Testcase {__class__.__name__} only works for a full-mission simulation.")
        self.run_case_fullmission(simulation)

    def run_case_fullmission(self, simulation):
        raise NotImplementedError


class FlexibleCase(Case):
    @property
    def single_sat_compatible(self):
        return True

    def setup_case(self, simulation):
        if simulation.is_single_sat_sim:
            self.setup_case_singlesat(simulation)
        else:
            self.setup_case_fullmission(simulation)

    def setup_case_singlesat(self, simulation):
        raise NotImplementedError

    def setup_case_fullmission(self, simulation):
        raise NotImplementedError

    def run_case(self, simulation):
        if simulation.is_single_sat_sim:
            self.run_case_singlesat(simulation)
        else:
            self.run_case_fullmission(simulation)

    def run_case_singlesat(self, simulation):
        raise NotImplementedError

    def run_case_fullmission(self, simulation):
        raise NotImplementedError

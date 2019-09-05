TYPES = [
    "bool", "int", "state int", "float", "double", "float vector",
    "double vector", "quaternion", "gps time", "temperature"
]

GROUPS = {
    "master": "State::Master",
    "hardware": "State::Hardware",
    "gomspace": "State::Gomspace",
    "propulsion": "State::Propulsion",
    "adcs": "State::ADCS",
    "quake": "State::Quake",
    "gnc": "State::GNC",
    "piksi": "State::Piksi",
    "adcs_history": "StateHistory::ADCS",
    "propulsion_history": "StateHistory::Propulsion",
    "gnc_history": "StateHistory::GNC",
    "piksi_history": "StateHistory::Piksi",
    "adcs_faults": "FaultState::ADCS",
    "gomspace_faults": "FaultState::Gomspace",
    "propulsion_faults": "FaultState::Propulsion",
}
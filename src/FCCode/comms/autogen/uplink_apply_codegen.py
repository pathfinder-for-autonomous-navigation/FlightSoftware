import math
import csv

with open('uplink_fields.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    header = next(csv_reader)
    FIELDS = [dict(zip(header, map(str, row))) for row in csv_reader]

for field in FIELDS:
    field["size"] = int(field['size'])

helper_str = "#include \"master_helpers.hpp\"\n"
helper_str += "#include \"../../state/device_states.hpp\"\n"

num_constants = 0
for field in FIELDS:
    if "_id" in field["name"]:
        num_constants += 1
apply_constants_str =  "void Master::apply_constants(const Comms::Uplink& uplink) {\n"
apply_constants_str += "  std::array<unsigned int, {0}> constant_ids {{\n".format(num_constants)
for field in FIELDS:
    if "_id" in field["name"]:
        apply_constants_str += "     uplink.{0},\n".format(field["name"])
apply_constants_str += "  };\n"
apply_constants_str += "  std::array<unsigned int, {0}> constant_vals {{\n".format(num_constants)
for field in FIELDS:
    if "_val" in field["name"]:
        apply_constants_str += "     uplink.{0},\n".format(field["name"])
apply_constants_str += "  };"
apply_constants_str += """
  for(int i = 0; i < constant_ids.size(); i++) {
    unsigned int const_id = constant_ids[i];
    if (const_id > Constants::changeable_constants_map.size()) continue;
    rwMtxWLock(&Constants::changeable_constants_lock);
    *(Constants::changeable_constants_map[const_id]) = constant_vals[i];
    rwMtxWUnlock(&Constants::changeable_constants_lock);
  }
}
"""
helper_str += apply_constants_str + "\n"

tokens = ["adcs", "fc"]
for token in tokens:
    apply_hat_str = "void Master::apply_{0}_hat(const Comms::Uplink& uplink) {{\n".format(token)
    if token == "fc":
        apply_hat_str += "  rwMtxWLock(&State::Hardware::hardware_state_lock);\n"
    else:
        apply_hat_str += "  rwMtxWLock(&State::ADCS::adcs_state_lock);\n"
    for field in FIELDS:
        uplink_field = "uplink.{0}".format(field["name"])
        if token == "fc" and "fc_hat_" in field["name"]:
            error_ignored = "State::Hardware::hat.at(\"{0}\").error_ignored".format(
                field["name"][len("fc_hat_"):])
            apply_hat_str += "    {0} = {1};\n".format(error_ignored, uplink_field)
        elif token == "adcs" and "adcs_hat_" in field["name"]:
            error_ignored = "State::ADCS::adcs_hat.at(\"{0}\").is_functional".format(
                field["name"][len("adcs_hat_"):])
            apply_hat_str += "    {0} = {1};\n".format(error_ignored, uplink_field)

    if token == "fc":
        apply_hat_str += "  rwMtxWUnlock(&State::Hardware::hardware_state_lock);\n"
    else:
        apply_hat_str += "  rwMtxWUnlock(&State::ADCS::adcs_state_lock);\n"
    apply_hat_str += "}\n"
    helper_str += apply_hat_str + "\n"

f = open("../../controllers/master/apply_uplink_helpers.cpp", "w")
f.write(helper_str)
f.close()
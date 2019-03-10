import math
import csv

with open('downlink_fields.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    header = next(csv_reader)
    FIELDS = [dict(zip(header, map(str, row))) for row in csv_reader]

for field in FIELDS:
    field["size"] = int(field['size'])
    field["buf_size"] = int(field['buf_size'])
    field["total_size"] = field["size"] * field["buf_size"]

PACKET_SIZE_BYTES = 70
PACKET_SIZE_BITS = PACKET_SIZE_BYTES * 8
FRAME_SIZE_BITS = sum([field["total_size"] for field in FIELDS])
NUM_PACKETS = int(math.ceil((FRAME_SIZE_BITS + 0.0) / PACKET_SIZE_BITS))

#####################################
# Generates code for a history field of the form
#
# rwMtxRLock(&State::group::lock);
# while(!State::group::val.empty()) {
#   std::bitset<X> representation_i;
#   Comms::trim_float(State::group::val.get(), MIN, MAX, representation_i);
#   for(int i = 0; i < representation_i.size(); i++) {
#     packet.set(packet_ptr++, representation_i[i]);
#   }
# }
# rwMtxRUnlock(&State::group::lock);
#####################################
def generate_history_field_code(field, i, packet_no):
    serializer_code = ""

    lock_name = field["group"] + "::" + field["group"][14:].lower() + "_history_state_lock"
    serializer_code += "rwMtxRLock(&{0});\n".format(lock_name)
    serializer_code += "while(!{0}.empty()) {{\n".format(field["name"])

    bitset_name = "bitset_{0}".format(i)
    serializer_code += "  std::bitset<{0}> {1};\n".format(field["size"],bitset_name)

    # Generate field serializer based on field type
    if field["type"] == "bool":
        serializer_code += "  {0}.set(0, {1}.get());\n".format(bitset_name, field["name"])
    elif field["type"] == "int":
        serializer_code += "  Comms::trim_int({0}.get(), {1}, {2}, &{3});\n".format(field["name"], field['min'], field['max'], bitset_name)
    elif field["type"] == "temperature":
        serializer_code += "  Comms::trim_temperature({0}.get(), &{1});\n".format(field["name"], bitset_name)
    elif field["type"] == "float":
        serializer_code += "  Comms::trim_float({0}.get(), {1}, {2}, &{3});\n".format(field["name"], field['min'], field['max'], bitset_name)
    elif field["type"] == "double":
        serializer_code += "  Comms::trim_double({0}.get(), {1}, {2}, &{3});\n".format(field["name"], field['min'], field['max'], bitset_name)
    elif field["type"] == "float vector" or field["type"] == "double vector":
        serializer_code += "  Comms::trim_vector({0}.get(), {1}, {2}, &{3});\n".format(field["name"], field['min'], field['max'], bitset_name)
    elif field["type"] == "quaternion":
        serializer_code += "  Comms::trim_quaternion({0}.get(), &{1});\n".format(field["name"], bitset_name)
    elif field["type"] == "gps time":
        serializer_code += "  Comms::trim_gps_time({0}.get(), &{1});\n".format(field["name"], bitset_name)
    else:
        err = "Field {0} does not have a valid type".format(field["name"])
        print(err)

    serializer_code += "  for(int i = 0; i < {0}.size(); i++) packet.set(packet_ptr++,{0}[i]);}}\n".format(bitset_name)
    serializer_code += "rwMtxRUnlock(&{0});\n".format(lock_name)
    return serializer_code

#####################################
# Generates code for a non-historical field of the form

# std::bitset<X> representation_i;
# Comms::trim_float(State::read(State::group::val, State::group::lock), MIN, MAX, representation_i);
# for(int i = 0; i < representation_i.size(); i++) {
#   packet.set(packet_ptr++, representation_i[i]);
# }
#####################################
def generate_field_code(field, i, packet_no):
    bitset_name = "bitset_{0}".format(i)
    serializer_code = "std::bitset<{0}> {1};\n".format(field["size"], bitset_name)

    # Generate field serializer based on field type
    lock_name = field["group"] + "::" + field["group"][7:].lower() + "_state_lock"
    if field["type"] == "bool":
        serializer_code += "{0}.set(0, State::read({1}, {2}));\n".format(bitset_name, field["name"], lock_name)
    elif field["type"] == "int":
        serializer_code += "Comms::trim_int(State::read({0},{1}), {2}, {3}, &{4});\n".format(field["name"], lock_name, field['min'], field['max'], bitset_name)
    elif field["type"] == "temperature":
        serializer_code += "Comms::trim_temperature(State::read({0},{1}), &{2});\n".format(field["name"], lock_name, bitset_name)
    elif field["type"] == "float":
        serializer_code += "Comms::trim_float(State::read({0},{1}), {2}, {3}, &{4});\n".format(field["name"], lock_name, field['min'], field['max'], bitset_name)
    elif field["type"] == "double":
        serializer_code += "Comms::trim_double(State::read({0},{1}), {2}, {3}, &{4});\n".format(field["name"], lock_name, field['min'], field['max'], bitset_name)
    elif field["type"] == "float vector" or field["type"] == "double vector":
        serializer_code += "Comms::trim_vector(State::read({0},{1}), {2}, {3}, &{4});\n".format(field["name"], lock_name, field['min'], field['max'], bitset_name)
    elif field["type"] == "quaternion":
        serializer_code += "Comms::trim_quaternion(State::read({0},{1}), &{2});\n".format(field["name"], lock_name, bitset_name)
    elif field["type"] == "gps time":
        serializer_code += "Comms::trim_gps_time(State::read({0},{1}), &{2});\n".format(field["name"], lock_name, bitset_name)
    else:
        err = "Field {0} does not have a valid type".format(field["name"])
        print(err)

    # Adds field serialization to the packet data
    serializer_code += "for(int i = 0; i < {0}.size(); i++) packet.set(packet_ptr++,{0}[i]);".format(bitset_name)
    return serializer_code

#####################################
# Generates code for all packet generator functions
#####################################
def packet_fn_generator(fields):
    packet_generator_str = ""

    field_ptr = 0
    for packet_no in range(0, NUM_PACKETS):
        current_packet_size = 0

        # Generate preamble for packet definition
        packet_str  = "static void serialize_packet_{0}(std::bitset<Comms::PACKET_SIZE_BITS> &packet, unsigned int downlink_no) {{\n".format(packet_no)
        packet_str += "  unsigned int packet_ptr = 0;\n\n"
        packet_str += "  std::bitset<32> downlink_num_repr(downlink_no);\n"
        packet_str += "  for(int i = 0; i < 32; i++) packet.set(packet_ptr++, downlink_num_repr[i]);\n"
        current_packet_size += 32
        packet_str += "  std::bitset<8> packet_num_repr({0});".format(packet_no) + "\n"
        packet_str += "  for(int i = 0; i < 8; i++) packet.set(packet_ptr++, packet_num_repr[i]);\n"
        current_packet_size += 8

        # Generate data for each field that can fit in the packet
        start_field_ptr = field_ptr
        for x in range(field_ptr, len(fields)):
            field = fields[x]
            if current_packet_size + field["total_size"] > PACKET_SIZE_BITS:
                break

            if "history" in field["name"]:
                field_code = generate_history_field_code(field, field_ptr, packet_no)
            else:
                field_code = generate_field_code(field, field_ptr, packet_no)
            field_code_tabbed = "\n".join(["  " + line for line in field_code.splitlines()])
            packet_str += "\n" + field_code_tabbed + "\n"

            current_packet_size += field["total_size"]
            field_ptr += 1
        end_field_ptr = field_ptr

        # Generate packet documentation
        packet_fields = FIELDS[start_field_ptr:end_field_ptr]
        packet_fields.insert(0, {"name" : "packet_number", "type"   : "int", "size" : 8 ,  "buf_size" : 1, "total_size" : 8 })
        packet_fields.insert(0, {"name" : "downlink_number", "type" : "int", "size" : 32 , "buf_size" : 1, "total_size" : 32 })
        with open('packet_documentation/packet{0}.csv'.format(packet_no), 'w+') as output_file:
            dict_writer = csv.DictWriter(output_file, FIELDS[0].keys())
            dict_writer.writeheader()
            dict_writer.writerows(packet_fields)

        packet_str += "}\n\n"
        packet_generator_str += packet_str

    return packet_generator_str

frame_str = "/** THIS IS AN AUTOGENERATED FILE **/\n\n"
frame_str += "#include \"downlink_serializer.hpp\"\n"
frame_str += "#include \"../state/state_holder.hpp\"\n"
frame_str += "#include \"../state/state_history_holder.hpp\"\n"
frame_str += "#include <comms_utils.hpp>\n\n"
frame_str += packet_fn_generator(FIELDS)
frame_str += "void Comms::serialize_downlink(std::bitset<Comms::PACKET_SIZE_BITS> (&packets)[Comms::NUM_PACKETS], unsigned int downlink_no) {\n"
for x in range(0, NUM_PACKETS):
    frame_str += "  serialize_packet_{0}(packets[{0}], downlink_no);\n".format(x)
frame_str += "}\n"

f = open("../../downlink_serializer.cpp", "w")
f.write(frame_str)
f.close()

#####################################
# Generates code for the header file containing the packet downlink function definition.
#####################################
header_str = "/** THIS IS AN AUTOGENERATED FILE **/\n\n"
header_str += "#ifndef DOWNLINK_PACKET_GENERATOR_HPP_\n"
header_str += "#define DOWNLINK_PACKET_GENERATOR_HPP_\n\n"
header_str += "#include \"../state/state_holder.hpp\"\n"
header_str += "#include <bitset>" + "\n\n"
header_str += "namespace Comms {\n"
header_str += "  constexpr unsigned int PACKET_SIZE_BITS = {0};\n".format(PACKET_SIZE_BITS)
header_str += "  constexpr unsigned int PACKET_SIZE_BYTES = {0};\n".format(PACKET_SIZE_BYTES)
header_str += "  constexpr unsigned int FRAME_SIZE_BITS = {0};\n".format(FRAME_SIZE_BITS)
header_str += "  constexpr unsigned int NUM_PACKETS = (FRAME_SIZE_BITS / PACKET_SIZE_BITS) + (FRAME_SIZE_BITS % PACKET_SIZE_BITS != 0) ? 1 : 0;\n"
header_str += "  void serialize_downlink(std::bitset<PACKET_SIZE_BITS> (&packets)[NUM_PACKETS], unsigned int downlink_no);\n"
header_str += "}\n\n"
header_str += "#endif"

f = open("../../downlink_serializer.hpp", "w")
f.write(header_str)
f.close()
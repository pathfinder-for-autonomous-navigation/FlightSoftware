import math
import csv
import copy
import jinja2

templateLoader = jinja2.FileSystemLoader(searchpath="./templates")
templateEnv = jinja2.Environment(loader=templateLoader)

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
# Generates code for a non-historical field of the form

# std::bitset<X> representation_i;
# Comms::trim_float(State::read(State::group::val, State::group::lock), MIN, MAX, representation_i);
# for(int i = 0; i < representation_i.size(); i++) {
#   packet.set(packet_ptr++, representation_i[i]);
# }
#####################################
def generate_field_code(field):
    if field["type"] == "int":
        field["trim_fn_args"] = "{0}, {1},".format(field['min'], field['max'])
    elif field["type"] == "float":
        field["trim_fn_args"] = "{0}, {1},".format(field['min'], field['max'])
    elif field["type"] == "double":
        field["trim_fn_args"] = "{0}, {1},".format(field['min'], field['max'])
    elif field["type"] in ["float vector", "double vector"]:
        field["type"] = "vector"
        field["trim_fn_args"] = "{0}, {1},".format(field['min'], field['max'])
    elif field["type"] in ["temperature", "quaternion"]:
        field["trim_fn_args"] = ""
    elif field["type"] == "gps time":
        field["type"] = "gps_time"
    elif field["type"] != "bool":
        err = "Field {0} does not have a valid type".format(field["name"])
        print(err)
    return field

#####################################
# Generates code for all packet generator functions
#####################################
packets = []
field_ptr = 0
for packet_no in range(0, NUM_PACKETS):
    current_packet_size = 40 # We need to have the packet header at the top of each packet

    # Generate data for each field that can fit in the packet
    start_field_ptr = field_ptr
    for x in range(field_ptr, len(FIELDS)):
        field = FIELDS[x]
        if current_packet_size + field["total_size"] > PACKET_SIZE_BITS:
            break

        if "history" in field["name"]:
            field["name"] = "{0}.get()".format(field["name"])
        else:
            lock_name = field["group"] + "::" + field["group"][7:].lower() + "_state_lock"
            field["name"] = "State::read({0},{1})".format(field["name"], lock_name)
        field = generate_field_code(field)

        current_packet_size += field["total_size"]
        field_ptr += 1
    end_field_ptr = field_ptr

    packet_fields = FIELDS[start_field_ptr:end_field_ptr]
    packets.append(packet_fields)

    # Generate packet documentation
    packet_fields_copy = copy.deepcopy(packet_fields)
    packet_fields_copy.insert(0, {"name" : "packet_no", "type"   : "int", "size" : 8 ,  "buf_size" : 1, "total_size" : 8 })
    packet_fields_copy.insert(0, {"name" : "downlink_no", "type" : "int", "size" : 32 , "buf_size" : 1, "total_size" : 32 })
    with open('packet_documentation/packet{0}.csv'.format(packet_no), 'w+') as output_file:
        dict_writer = csv.DictWriter(output_file, FIELDS[0].keys())
        dict_writer.writeheader()
        dict_writer.writerows(packet_fields_copy)

impl_template = templateEnv.get_template('downlink_serializer_cpp.j2')
impl_str = impl_template.render({
    "packets" : packets
})
with open("../downlink_serializer.cpp", "w") as f:
    f.write(impl_str)
print "downlink_serializer.cpp generated."

#####################################
# Generates code for the header file containing the packet downlink function definition.
#####################################
header_template = templateEnv.get_template('downlink_serializer_hpp.j2')
header_str = header_template.render({
    "packet_size_bits": PACKET_SIZE_BITS,
    "packet_size_bytes": PACKET_SIZE_BYTES,
    "num_packets": NUM_PACKETS,
})
with open("../downlink_serializer.hpp", "w") as f:
    f.write(header_str)
print "downlink_serializer.hpp generated."
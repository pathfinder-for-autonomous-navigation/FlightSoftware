import math
import csv
import jinja2

templateLoader = jinja2.FileSystemLoader(searchpath="./templates")
templateEnv = jinja2.Environment(loader=templateLoader)

with open('uplink_fields.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    header = next(csv_reader)
    FIELDS = [dict(zip(header, map(str, row))) for row in csv_reader]

for field in FIELDS:
    field["size"] = int(field['size'])
total_size = sum([field["size"] for field in FIELDS])

TYPES = ["bool", "int", "state int", "float vector", "double vector", "quaternion","gps time"]

fields_data = []

for field in FIELDS:
    field_data = {}
    field_data["name"] = field["name"]

    if field["type"] == "int":
        field_data["type"] = "unsigned int"
    elif field["type"] == "float vector":
        field_data["type"] = "std::array<float, 3>"
    elif field["type"] == "double vector":
        field_data["type"] = "std::array<double, 3>"
    elif field["type"] == "gps time":
        field_data["type"] = "gps_time_t"
    elif field["type"] == "quaternion":
        field_data["type"] = "std::array<float, 4>"
    elif field["type"] not in TYPES:
        print "Undefined type for field {0}".format(field["name"])
    else:
        field_data["type"] = field["type"]

    fields_data.append(field_data)

struct_template = templateEnv.get_template('uplink_struct_hpp.j2')
struct_str = struct_template.render({
    "uplink_size_bits": total_size,
    "fields": fields_data
})

f = open("../uplink_struct.hpp", "w")
f.write(struct_str)
f.close()
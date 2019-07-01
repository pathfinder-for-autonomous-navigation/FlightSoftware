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

TYPES = [
    "bool", "int", "state int", "float vector", "double vector", "quaternion",
    "gps time"
]

# Create expansion metadata within field
for field in FIELDS:
    if field["type"] == "int" or field["type"] == "state int":
        field["type"] = "int"
        field["expand_fn_args"] = "{0}, {1}".format(field["min"], field["max"])
    elif field["type"] == "float vector" or field["type"] == "double vector":
        field["type"] = "vector"
        field["expand_fn_args"] = "{0}, {1}, &(uplink->{2})".format(field["min"], field["max"], field["name"])
    elif field["type"] == "gps time":
        field["type"] = "gps_time"
        field["expand_fn_args"] = "&(uplink->{0})".format(field["name"])
    elif field["type"] == "quaternion":
        field["expand_fn_args"] = "&(uplink->{0})".format(field["name"])
    elif field["type"] != "bool":
        print "Undefined type for field {0}".format(field["name"])
        continue

# Write to file
uplink_deserializer_template = templateEnv.get_template('uplink_deserializer_cpp.j2')
uplink_deserializer_str = uplink_deserializer_template.render({
    "fields" : FIELDS
})
with open("../uplink_deserializer.cpp", "w") as f:
    f.write(uplink_deserializer_str)
print "uplink_deserializer.cpp generated."
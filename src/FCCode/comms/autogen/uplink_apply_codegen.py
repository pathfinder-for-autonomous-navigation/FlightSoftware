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

num_constants = 0
for field in FIELDS:
    if "_id" in field["name"]:
        num_constants += 1

uplink_fields_adcs_hat = []
uplink_fields_fc_hat = []

for field in FIELDS:
    if "fc_hat_" in field["name"]:
        uplink_fields_fc_hat.append({
            "device": field["name"][len("fc_hat_"):],
            "uplink_field": field["name"]
        })
    elif "adcs_hat_" in field["name"]:
        uplink_fields_adcs_hat.append({
            "device": field["name"][len("adcs_hat_"):],
            "uplink_field": field["name"]
        })

helper_template = templateEnv.get_template('apply_uplink_helpers_cpp.j2')
helper_str = helper_template.render({
    "num_constants": num_constants,
    "uplink_fields_adcs_hat": uplink_fields_adcs_hat,
    "uplink_fields_fc_hat": uplink_fields_fc_hat
})
f = open("../../controllers/master/apply_uplink_helpers.cpp", "w")
f.write(helper_str)
f.close()
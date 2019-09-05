import csv
import jinja2

templateLoader = jinja2.FileSystemLoader(searchpath="./templates")
templateEnv = jinja2.Environment(loader=templateLoader)

with open('downlink_fields.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    header = next(csv_reader)
    FIELDS = [dict(zip(header, map(str, row))) for row in csv_reader]

# Parse data from CSV into memory structures
history_fields = []
for field in FIELDS:
    if "history" in field["name"]:
        history_fields.append(field)
history_group_data = [ ( field["group"][len("StateHistory::"):] , field["group_nickname"] )
                        for field in history_fields]
history_group_data = list(set(history_group_data))
history_groups = [{"name" : grp, "nickname" : nick} for (grp, nick) in history_group_data]

# Organize fields into each namespace
for group in history_groups:
    group_fields = []
    for field in history_fields:
        if not 'buf_size' in field:
            print "Field {0} has undefined size".format(field["name"])
        if field["group"] != "StateHistory::" + group["name"]:
            continue

        if field["type"] == "float vector":
            field["type"] = "std::array<float,3>"
        elif field["type"] == "double vector":
            field["type"] = "std::array<double,3>"
        elif field["type"] == "quaternion":
            field["type"] = "std::array<float,4>"
        elif field["type"] == "gps time":
            field["type"] = "gps_time_t"
        elif field["type"] == "temperature":
            field["type"] = "int"
        else:
            field["type"] = "unsigned int"

        group_field = {
            "name": field["name"][len(field["group"]) + 2:],
            "type": field["type"],
            "samples": field["buf_size"]
        }
        group_fields.append(group_field)
    group["fields"] = group_fields

######### Construct code for all files
## State history holder header
history_holder_header_template = templateEnv.get_template('state_history_holder.hpp.j2')
history_holder_header_str = history_holder_header_template.render({
    "namespaces" : history_groups
})
with open("../../state/state_history_holder.hpp", "w") as f:
    f.write(history_holder_header_str)
print "state_history_holder.hpp generated."

## State history holder implementation
history_holder_impl_template = templateEnv.get_template('state_history_holder.cpp.j2')
history_holder_impl_str = history_holder_impl_template.render({
    "namespaces" : history_groups
})
with open("../../state/state_history_holder.cpp", "w") as f:
    f.write(history_holder_impl_str)
print "state_history_holder.cpp generated."

## State history rates
state_history_rates_template = templateEnv.get_template('state_history_rates.hpp.j2')
state_history_rates_str = state_history_rates_template.render({
    "namespaces" : history_groups
})
with open("../../state/state_history_rates.hpp", "w") as f:
    f.write(state_history_rates_str)
print "state_history_rates.hpp generated."
import os,re

tracked_const_finder = r"TRACKED_CONSTANT\(\s*(?P<type>[\w\s]*\w)\s*,\s*(?P<id>[\w]+)\s*,\s*(?P<val>.*)\)"
consts = []

for root, dirs, files in os.walk("src"):
    for file in files:
        path = os.path.join(root, file)
        if path.endswith((".cpp", ".hpp", ".c", ".h")):
            with open(path, 'r') as f:
                found_consts = re.findall(tracked_const_finder, f.read())
                for const in found_consts:
                    consts.append({
                        "name" : const[1].strip(),
                        "type" : const[0].strip(),
                        "val" : const[2].strip(),
                        "file" : path
                    })

log = "Keep this file in version control so that changes to constants are well-known.\n\n"
for const in consts:
    log += f"{const['file']}: \"{const['name']}\" = \"{const['val']}\"\n"

with open("constants", 'w') as f:
    f.write(log)

import os,re

consts = []
log = "Keep this file in version control so that changes to constants are well-known.\n\n"

for root, dirs, files in os.walk("src"):
    for file in files:
        path = os.path.join(root, file)
        if path.endswith((".cpp", ".hpp", ".c", ".h", ".inl")):
            with open(path, 'r') as f:
                lines = f.readlines()

                for x in range(0, len(lines)):
                    found_decls = re.findall(r"TRACKED_CONSTANT", lines[x])
                    found_consts = re.findall(r"TRACKED_CONSTANT_?S?C?\(\s*(?P<type>[\w<>&:\s]*[\w<>&:])\s*,\s*(?P<id>[\w]+)\s*,\s*(?P<val>.*)\)", lines[x])

                    if len(found_consts) != len(found_decls):
                        log += f"{path}:{x+1}: error: regex could not process tracked constant declaration\n"
                        log += f"    {lines[x]}"
                        log += f"    ^\n"

                    for const in found_consts:
                        consts.append({
                            "name" : const[1].strip(),
                            "type" : const[0].strip(),
                            "val" : const[2].strip(),
                            "file" : path,
                            "line" : x + 1,
                        })

log += "\n"
log += "Found Constants\n"
log += "---------------\n"
for const in consts:
    log += f"{const['file']}:{const['line']}: \"{const['name']}\" = \"{const['val']}\"\n"

with open("constants", 'w') as f:
    f.write(log)

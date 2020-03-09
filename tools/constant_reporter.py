import os,re

consts = []
log = "Keep this file in version control so that changes to constants are well-known.\n\n"

for root, dirs, files in os.walk("src"):
    for file in files:
        path = os.path.join(root, file)
        if path.endswith((".cpp", ".hpp", ".c", ".h", ".inl")):
            with open(path, 'r') as f:
                text = f.read()
                found_decls = re.findall(r"TRACKED_CONSTANT", text)
                found_consts = re.findall(r"TRACKED_CONSTANT_?S?C?\(\s*(?P<type>[\w<>&:\s]*[\w<>&:])\s*,\s*(?P<id>[\w]+)\s*,\s*(?P<val>.*)\)", text)

                if len(found_consts) != len(found_decls):
                    log += f"{path}: regex error: only found {len(found_consts)} out of {len(found_decls)} tracked constant declarations.\n"

                for const in found_consts:
                    consts.append({
                        "name" : const[1].strip(),
                        "type" : const[0].strip(),
                        "val" : const[2].strip(),
                        "file" : path
                    })

log += "\n"
for const in consts:
    log += f"{const['file']}: \"{const['name']}\" = \"{const['val']}\"\n"

with open("constants", 'w') as f:
    f.write(log)

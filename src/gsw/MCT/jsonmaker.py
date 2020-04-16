import re
import io
import sys
file = open(sys.argv[1], "r")
text = file.read()
def printabove(objkey):

    above = """
            {
              "name": "<enter name>",
              "key": \"gomspace."""+objkey+""",
              "values": [
                {
                    "key": "value",
                    "name": "<enter name>",
    """
    return above
def printbelow():
    below = """
                    "hints": {
                    "range": 1
                    }
                },
                {
                    "key": "utc",
                    "source": "timestamp",
                    "name": "Timestamp",
                    "format": "utc",
                    "hints": {
                            "domain": 1
                        }
                }
            ]
      """
    return below

for line in text.splitlines():
    startobject = "},"
    keyvalue ="gomspace."
    endvalue = "writable"
    linestring = str(line)
    if startobject in linestring:
        #do nothing
        print("    "+linestring)
    elif keyvalue in linestring:
        objkey = linestring[linestring.index(".")+1:linestring.index(":")]
        print(printabove(objkey))
    elif endvalue in linestring:
        print("         "+linestring+"," + printbelow())
    else:
        print("         "+linestring)

#!/usr/bin/python3

import argparse
import os

parser = argparse.ArgumentParser(description='Bazel build helper file.')
parser.add_argument("action", choices=["build", "test"])
parser.add_argument("flavor", choices=["hitl32", "hitl35", "hitl36", "hootl", "gsw", "flight"])
parser.add_argument("satellite", choices=["leader", "follower"])
parser.add_argument("timing", choices=["realtime", "accelerated"])
parser.add_argument("target")
args = parser.parse_args()

# Throw out invalid configurations.
if args.flavor in ["gsw", "flight"] and args.timing == "accelerated":
    print("Cannot compile specified flavor with accelerated timing.")

cmd = "bazel " + args.action

flavor_map = {
    "hitl32" : ["flight=0","target=teensy32","gsw=0"],
    "hitl35" : ["flight=0","target=teensy35","gsw=0"],
    "hitl36" : ["flight=0","target=teensy36","gsw=0"],
    "hitl36" : ["flight=0","target=teensy36","gsw=0"],
    "hootl" :  ["flight=0","target=desktop","gsw=0"],
    "gsw" :    ["flight=1","target=desktop","gsw=1","realtime=1"],
    "flight" : ["flight=1","target=teensy36","gsw=0","realtime=1"],
}

cmd += " ".join([(" --define " + define) for define in flavor_map[args.flavor]])
cmd += " --define satellite=" + args.satellite + " "
if args.flavor not in ["gsw", "flight"]:
    cmd += " --define timing=" + args.timing + " "
if args.action == "test":
    cmd += " --define unit_test=1 "

cmd += args.target

print("Running Bazel with command:")
print(cmd)
os.system(cmd)

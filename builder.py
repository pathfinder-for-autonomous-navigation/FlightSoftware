#!/usr/bin/python3

import argparse
import os

parser = argparse.ArgumentParser(description='Bazel build helper file.')
parser.add_argument("action", choices=["build", "test"])

flavor_map = {
    "hitl32" : ["flight=0","target=teensy32","gsw=0"],
    "hitl35" : ["flight=0","target=teensy35","gsw=0"],
    "hitl36" : ["flight=0","target=teensy36","gsw=0"],
    "hitl36" : ["flight=0","target=teensy36","gsw=0"],
    "hootl"  : ["flight=0","target=desktop","gsw=0"],
    "gsw"    : ["flight=1","target=desktop","gsw=1","timing=realtime"],
    "flight" : ["flight=1","target=teensy36","gsw=0","timing=realtime"],
    "adcs"   : ["flight=1","target=teensy35","gsw=0","timing=realtime"],
}
parser.add_argument("flavor", choices=flavor_map.keys())
parser.add_argument("satellite", choices=["leader", "follower"])
parser.add_argument("timing", choices=["realtime", "accelerated"])
parser.add_argument("target")
args = parser.parse_args()

# Throw out invalid configurations.
if args.flavor in ["gsw", "flight"] and args.timing == "accelerated":
    print("Cannot compile specified flavor with accelerated timing.")

cmd = "bazel " + args.action

if args.flavor in ["hitl32", "hitl35", "hitl36", "flight", "adcs"]:
    cmd += " --platforms=@arm_none_eabi//platforms:arm_none_generic "

cmd += " " + " ".join([("--define " + define) for define in flavor_map[args.flavor]])
if args.flavor not in ["gsw", "flight"]:
    cmd += " --define timing=" + args.timing + " "

cmd += " --define satellite=" + args.satellite + " "

if args.action == "test":
    cmd += " --define unit_test=1 "

cmd += args.target

print("Running Bazel with command:")
print(cmd)
os.system(cmd)

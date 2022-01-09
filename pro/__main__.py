import sys
import json
from .pro import RendevousOrbitPredictor
from argparse import ArgumentParser

def main(args):
  parser = ArgumentParser(description="Predictor of Rendevous Orbit")
  parser.add_argument('-c', '--conf', action='store', help='Config File location', required=True)
  args = parser.parse_args(args)
  f = open(args.conf)
  config = json.load(f)
  pro = RendevousOrbitPredictor(config['times'], config['imei'],)
  pro.start()

if __name__ == "__main__":
    main(sys.argv[1:])
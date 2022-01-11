import sys
import json
from .pro import RendevousOrbitPredictor
from argparse import ArgumentParser

def main(args):
  parser = ArgumentParser(description="Predictor of Rendevous Orbit")
  parser.add_argument('-c', '--conf', action='store', help='Config File location', required=True)
  parser.add_argument('-n', '--now', action='store_true', help='Optionally specifiy to run now.', default=False)

  args = parser.parse_args(args)
  f = open(args.conf)
  config = json.load(f)
  pro = RendevousOrbitPredictor(config, args.now)
  pro.start()

if __name__ == "__main__":
    main(sys.argv[1:])
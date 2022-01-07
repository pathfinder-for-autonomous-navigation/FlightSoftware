import sys
from .pro import RendevousOrbitPredictor

def main(args):
  pro = RendevousOrbitPredictor(3, "412111111")
  pro.start()

if __name__ == "__main__":
    main(sys.argv[1:])
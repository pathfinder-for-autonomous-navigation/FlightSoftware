# GPS Time class for use in the simulation. It's a Python mirror of the flight software object of
# the same name.

class GPSTime(object):
    SECONDS_IN_WEEK = 7 * 24 * 60 * 60

    def __init__(self, wn, tow, ns):
        self.wn = wn
        self.tow = tow
        self.ns = ns

    def __init__(self, seconds):
        if isinstance(seconds, str):
            seconds = float(seconds)

        self.wn  = int(seconds / self.SECONDS_IN_WEEK)
        self.tow = int((seconds - self.wn * self.SECONDS_IN_WEEK) * 1000)
        self.ns  = int((seconds - self.wn * self.SECONDS_IN_WEEK - self.tow / 1000) * int(1e9))

    def __str__(self):
        return f"{self.wn},{self.tow},{self.ns}"

    def to_ns(self):
        return self.wn * SECONDS_IN_WEEK * 1e9 + self.tow * 1e6 + self.ns

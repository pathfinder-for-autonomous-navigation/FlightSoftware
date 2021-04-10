# GPS Time class for use in the simulation. It's a Python mirror of the flight software object of
# the same name.

class GPSTime(object):
    SECONDS_IN_WEEK = 7 * 24 * 60 * 60
    NS_IN_WEEK = SECONDS_IN_WEEK * 1000 * 1000 * 1000
    EPOCH_WN = 2045

    def __init__(self, wn, tow, ns):
        self.wn = wn
        self.tow = tow
        self.ns = ns

    def __init__(self, pan_ns_count_in):
        '''
        pan_ns_count_in should be a gps time *since* the PAN Epoch
        '''
        ns_count = self.EPOCH_WN*self.NS_IN_WEEK + pan_ns_count_in

        self.wn  = int(ns_count // self.NS_IN_WEEK)
        
        # tow is in ms
        self.tow = int((ns_count - self.wn * self.NS_IN_WEEK) // 1000000) # in ms
        self.ns  = int((ns_count - self.wn * self.NS_IN_WEEK) % 1000000)
        assert self.ns >= 0

    def __str__(self):
        return f"{self.wn},{self.tow},{self.ns}"

    def to_ns(self):
        return self.wn * SECONDS_IN_WEEK * 1e9 + self.tow * 1e6 + self.ns
    
    def to_seconds(self):
        return self.to_ns()*1e-9

    def to_list(self):
        return [self.wn, self.tow, self.ns]
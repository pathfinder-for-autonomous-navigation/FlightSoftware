import threading
import time

class UplinkTimer(object):
    def __init__(self, interval, function, args=[], kwargs={}):
        self._interval = interval
        self._function = function
        self._args = args
        self._kwargs = kwargs
        self.t = None
        self.timer_lock = threading.Lock()

    def start(self):
        '''
        Start the timer. Will run the given function after 
        the given time interval has passed
        '''
        with self.timer_lock:
            self.start_time = time.time()
            self.t = threading.Timer(self._interval, self._function, *self._args, **self._kwargs)
            self.t.start()

    def cancel(self):
        '''
        Stop the timer. Only works if the timer had 
        already been started.
        '''
        with self.timer_lock:
            self.t.cancel()
            self.t = None

    def is_alive(self):
        '''
        Check if the timer is running.
        '''
        with self.timer_lock:
            return self.t.is_alive()

    def run_time(self):
        '''
        Return how long the timer has been running.
        '''
        with self.timer_lock:
            return time.time()-self.start_time
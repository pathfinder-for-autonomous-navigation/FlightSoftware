import threading
import time

'''
This class builds off of the Timer class if the python threading library.
It is intended to make scheduling uplinks easier by adding pause/resume methods.
It also automatically restarts the thread whenever the timer is started.
'''
class UplinkTimer(object):
    def __init__(self, interval, function, args=[], kwargs={}):
        self.interval = interval
        self.function = function
        self._args = args
        self._kwargs = kwargs
        self.timer_lock = threading.Lock()

        # Holds the timer object from the python threading library
        self.t = None
        # The time at which the timer last started
        self.start_time = None
        # If the timer is paused, then this holds how much time was left on the timer
        self.time_remaining = None

    def start(self):
        '''
        Start the timer. Will run the given function after 
        the given time interval has passed. If the timer is 
        still running and/or hasn't been cancelled yet, 
        then this will do nothing.
        '''
        self.start_time = time.time()
        self.t = threading.Timer(self.interval, self.function, *self._args, **self._kwargs)
        self.t.start()
        self.time_remaining = None

    def cancel(self):
        '''
        Stop the timer. Only works if the timer had 
        already been started.
        '''
        self.timer_lock.acquire()
        if self.is_alive():
            self.t.cancel()
            self.t = None
            self.start_time = None
            self.time_remaining = None
        self.timer_lock.release()

    def is_alive(self):
        '''
        Check if the timer is running.
        '''
        self.timer_lock.acquire()
        if self.t is None:
            alive = False
        else:
            alive=self.t.is_alive()
        self.timer_lock.release()
        return alive

    # It looks like boolean values are passed by value into 
    # Python functions while lists are passed by reference. 
    # So I'm using a list as a parameter in this method for 
    # now, but I'll try to  think of a better way to do this 
    # in the near future.
    def is_alive2(self, alive):
        '''
        Check if the timer is running. Will set given 
        alive variable to whether or not the timer
        is running.
        '''
        self.timer_lock.acquire()
        if self.t is None:
            pass
        else:
            alive.append(self.t.is_alive())
        self.timer_lock.release()
        return alive

    def run_time(self):
        '''
        Return how long the timer has been running.
        '''
        self.timer_lock.acquire()
        if not self.is_alive():
            run_time = 0
        else:
            run_time = time.time()-self.start_time
        self.timer_lock.release()
        return run_time

    def time_left(self):
        '''
        Return the time remaining on the timer.
        '''
        self.timer_lock.acquire()
        if self.is_alive():
            time_left = self.interval-self.run_time()
        else:
            time_left = None
        self.timer_lock.release()
        return time_left

    def pause(self):
        '''
        Pause the timer. If the timer is not running, or if 
        we have already paused the timer and haven't resumed 
        since, then this does nothing.
        '''
        self.timer_lock.acquire()
        can_pause = self.is_alive()
        can_pause &= self.time_remaining is None
        if can_pause:
            # Get time remaining on timer
            self.time_remaining = self.interval - self.run_time()
            # Stop the timer
            self.cancel()
        self.timer_lock.release()

    def resume(self):
        '''
        Resume the timer. If the timer is already running, 
        or if we have not previously paused a timer, then 
        this does nothing.
        '''
        self.timer_lock.acquire()
        can_resume = not self.is_alive()
        can_resume &= self.time_remaining is not None
        if can_resume:
            # Get the original time interval set when constructing the timer
            original_interval = self.interval
            # Start the timer to run only for the remaining time on the paused timer
            self.interval = self.time_remaining
            self.start()
            # Reset the interval to the original value
            self.interval = original_interval
            self.time_remaining = None
        self.timer_lock.release()

import queue, os, json, threading, time, datetime
from tinydb import TinyDB
from tinydb.middlewares import CachingMiddleware
from tinydb.storages import JSONStorage

class DataConsumer(object):
    def __init__(self, device_name, data_dir):
        self.device_name = device_name
        self.data_dir = data_dir
        self.queue = queue.Queue()

    def consume_queue_item(self, item):
        """
        Takes an item put into the queue by a StateSession, and consumes it.
        """
        raise NotImplementedError

    def consume_queue(self):
        """ The data consumer thread. """
        while self.running:
            try:
                while True:
                    item = self.queue.get_nowait()
                    self.consume_queue_item(item)
            except queue.Empty:
                pass

            time.sleep(0.5) # Sleep 0.5 seconds

    def start(self):
        """ Start data consumer thread. """
        self.consumer_thread = threading.Thread(target=self.consume_queue)
        self.running = True
        self.consumer_thread.start()

    def save(self):
        """ Save consumed data to disk. """
        raise NotImplementedError

    def stop(self):
        """ Stop data consumer thread. """
        self.running = False
        try:
            self.consumer_thread.join()
        except AttributeError:
            pass
        self.save()

    def put(self, item):
        """
        External-facing method used by clients of this class to add data for consumption.

        Args:
         - item: Item to consume.
        """
        self.queue.put(item)


class Datastore(DataConsumer):
    def __init__(self, device_name, data_dir):
        super().__init__(device_name, data_dir)
        filename = f"{self.device_name}-telemetry.txt"
        self.db = TinyDB(f"/{data_dir}/{filename}", storage=CachingMiddleware(JSONStorage))

    def consume_queue_item(self, datapoint):
        """
        Adds a single data point to the telemetry log.
        """
        self.db.insert(datapoint)

    def save(self):
        """ Save telemetry log to a file. """
        self.db.storage.flush()
        self.db.close()

class Logger(DataConsumer):
    def __init__(self, device_name, data_dir, print=False):
        super().__init__(device_name, data_dir)
        self.log = ""
        filename = f"{self.device_name}-log.txt"
        filepath = os.path.join(self.data_dir, filename)
        self.logfile = open(filepath, "w")

        self.print = print

        # Used for determining whether or not to save data to file
        self.line_counter = 0

    def consume_queue_item(self, logline):
        """Add a new line to the log."""
        self.log += str(logline) + "\n"
        self.line_counter += 1
        if self.line_counter % 100 == 0:
            self.save()

    def save(self):
        """Save the log to a file."""
        self.logfile.write(self.log)
        self.log = ""
        self.logfile.flush()
        os.fsync(self.logfile.fileno())

    def put(self, logline, add_time = True):
        if self.print:
            print(logline)
        if add_time:
            logline = f"[{datetime.datetime.now()}] " + str(logline.rstrip())
        self.queue.put(logline)

    def stop(self):
        super().stop()
        self.logfile.close()

from ..data_consumers import Datastore
import os, tempfile, timeit
import pylab as plt

# Note, running this stress test requires exporting STRESS_TEST=1 as an
# environment variable.
def test_datastore_stressfully():
    if "STRESS_TEST" not in os.environ:
        return
    
    tempdir = tempfile.mkdtemp() + "/datastore_test_dir"
    os.makedirs(tempdir, exist_ok=True)
    datastore = Datastore("test_device", tempdir)
    datastore.start()

    elapsed_times = []

    for x in range(0, 10000):
        start_time = timeit.default_timer()
        for y in range(0, 100):
            datastore.put({"field": "test_field", "val" : "val", "time" : "2"})
        elapsed_time = int((timeit.default_timer() - start_time) * 1E6)
        elapsed_times.append(elapsed_time)

    plt.plot(elapsed_times)
    plt.ylabel("Duration to write 100 database values (microseconds)")
    plt.show()

    datastore.stop()

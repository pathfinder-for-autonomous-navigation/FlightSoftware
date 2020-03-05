from ptest.data_consumers import Datastore, Logger
import os, json, tempfile, time, tinydb

def test_datastore():
    tempdir = tempfile.mkdtemp() + "/datastore_test_dir"
    os.makedirs(tempdir, exist_ok=True)
    datastore = Datastore("test_device", tempdir)
    datastore.start()
    datastore.put({"field": "test_field", "val" : "val", "time" : "2"})
    time.sleep(1.5) # Allow for some time for the data to be processed by the queue processor
    datastore.stop()

    datastore = Datastore("test_device", tempdir)
    results = datastore.db.search(tinydb.Query().field == "test_field")
    assert len(results) == 1
    assert results[0]["time"] == "2"
    assert results[0]["val"] == "val"

def test_logger():
    tempdir = tempfile.mkdtemp() + "/logger_test_dir"
    os.makedirs(tempdir, exist_ok=True)
    logger = Logger("test_device", tempdir)
    logger.start()
    logger.put("Hello world")
    logger.put("Hello world2", add_time = False)
    time.sleep(1.5) # Allow for some time for the data to be processed by the queue processor
    logger.stop()

    with open(tempdir + "/test_device-log.txt", "r") as logfile:
        log = logfile.read()
        assert log.find("Hello world") != -1
        assert log.find("Hello world2") != -1

import sched
import time
import requests
from elasticsearch import Elasticsearch

searchURL = 'http://localhost:5000/search-es'
updateURL = 'http://localhost:5000/update-time'

class RendevousOrbitPredictor(object):
  # def __init__(self, amount_per_day, start, imei):
  def __init__(self, amount_per_day, imei):
    self.scheduler = sched.scheduler(time.time, time.sleep)
    # self.start_time = start
    self.seconds_apart = 60 * 60 * (24/int(amount_per_day))
    self.index = 'statefield_report_' + imei
    self.es = Elasticsearch()
    self.critical_telem = {
      "orbit.vel": -1,
      "orbit.pos": -1
    }

  # Starts the first run (currently time of starting the script 
  #  - can be changed to a fixed point in the future)
  def start(self):
    self.scheduler.enter(0, 1, self.periodic_send)
    self.scheduler.run()

  def periodic_send(self):
    #Scheduling next run
    self.scheduler.enter(self.seconds_apart, 1, self.periodic_send)

    #updating the current value of each of the telemetry frields from ES
    for key in self.critical_telem:
      self.critical_telem[key] = self.request_telem(key)
    
    # Call scripts for making calculations and generating files to upload
    self.call_scripts()

    # Upload file to space-track
    self.upload_file()

    # update the time field in elasticsearch
    self.update_time()

  
  def request_telem(self, name):
    val = requests.get(searchURL, params={'index':self.index, 'field':name})
    return val.text


  # TODO 2: Call python file that makes calculations/generates file
  def call_scripts(self):
    print("TODO: call_scripts")

  # TODO 3: Upload File to space-track using API or Email
  def upload_file(self):
    print("TODO: upload_file")
    
  # TODO 4: Update the "Last Uploaded" field
  def update_time(self):
    print("TODO: update_time")
  

    
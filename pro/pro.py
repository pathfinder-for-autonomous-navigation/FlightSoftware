import sched
import time
from datetime import datetime
import requests
import json
from elasticsearch import Elasticsearch

searchURL = 'http://localhost:5000/search-es'
updateURL = 'http://localhost:5000/update-time'
st_base = 'https://www.space-track.org'
st_login = '/ajaxauth/login'
st_upload = '/fileshare/query/class/upload/folder_id/'
#TODO: DETERMINE FOLDER ID
st_folder_id = 'ID'
st_logout = '/ajaxauth/logout'
class RendevousOrbitPredictor(object):
  # def __init__(self, amount_per_day, start, imei):
  def __init__(self, times, imei):
    self.scheduler = sched.scheduler(time.time, time.sleep)
    self.index = 'statefield_report_' + imei
    self.es = Elasticsearch()
    self.times = self.time_string_to_time(times)

  def time_string_to_time(self, l):
    time_objs = []
    for str in l:
      parsed_time = list(time.strptime(str, "%H:%M:%S"))
      current_time = list(time.localtime())
      full_time = time.struct_time(tuple(current_time[0:3] + parsed_time[3:6] + current_time[6:]))
      time_objs.append(time.mktime(full_time))
    return time_objs


  # Starts the first run (currently time of starting the script 
  #  - can be changed to a fixed point in the future)
  def start(self):
    
    # BEGIN HEADER CODE
    # import runpy
    # import os
    # # os.system("python -m ptest runsim -c ptest/configs/amc.json -t MonteCarlo --clean -ni")
    # # runpy.run_module("ptest", ['runsim', '-c', 'ptest/configs/amc.json', '-t', 'MonteCarlo', '--clean', '-ni'])
    
    # from subprocess import Popen, PIPE

    # process = Popen(['python', '-m', 'ptest', 'runsim', '-c', 'ptest/configs/amc.json', '-t', 'MonteCarlo', '--clean', '-ni'], stdout=PIPE, stderr=PIPE)
    # print("start")
    # stdout, stderr = process.communicate()
    # print("here")
    # return
    # END HEADER CODE
  
    self.next = self.get_next_time()
    if self.next>= len(self.times):
      self.next_day_of_times()
      self.next = 0
    self.scheduler.enterabs(self.times[self.next], 1, self.periodic_send)
    print("Waiting for " + self.epoch_time_to_string(self.times[self.next]) + "...")
    
    self.scheduler.run()
  

  def get_next_time(self):
    new_t =time.time()
    i = 0
    for t in self.times:
      if new_t < t:
        return i
      i = i+1
    return i

  def epoch_time_to_string(self, t):
    return time.strftime("%a, %d %b %Y %H:%M:%S %z", time.localtime(t))

  def periodic_send(self):
    #Scheduling next run
    self.next = self.next + 1
    if self.next>= len(self.times):
      self.next_day_of_times()
      self.next = 0
    self.scheduler.enterabs(self.times[self.next], 1, self.periodic_send)
    
    # Call scripts for making calculations and generating files to upload
    self.call_scripts()

    #Try Uploading the file to space-track 5 times
    # uploaded = false
    # tries = 0
    # while not uploaded and tries < 5:
    #   tries = tries + 1
    #   uploaded = self.upload_file()

    # update the time field in elasticsearch
    self.update_time()

    print("Waiting for " + self.epoch_time_to_string(self.times[self.next]) + "...")

  def next_day_of_times(self):
    i = 0
    for t in self.times:
      self.times[i] = t + (24*60*60)
      i = i + 1


  # TODO 1: Call python file that makes calculations/generates file
  def call_scripts(self):
    print("TODO: call_scripts")


  def upload_file(self):
    with requests.Session() as session:
      resp = session.post(st_base + st_login, data = {'identity': self.username, 'password': self.password})
      if resp.status_code != 200:
        return false
      resp = session.post(st_base + st_upload + st_folder_id, data = {'file': FILENAME_HERE})
      if resp.status_code != 200:
        return false
      return true

  def update_time(self):
    
    # Current Time
    val = time.strftime("%a, %d %b %Y %H:%M:%S %z", time.localtime())
    data=json.dumps({
        "pro.last": val,
        "time.downlink_received": str(datetime.utcnow().isoformat())[:-3]+'Z'
    })

    #index the time of file upload
    res = self.es.index(index=self.index, body=data)
    
    # Confirm result status
    print("pro.last " + res['result'] + ": " + val)

    
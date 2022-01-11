import sched
import time
from datetime import datetime
import requests
import json
from elasticsearch import Elasticsearch
import os
from subprocess import Popen, PIPE
import glob

searchURL = 'http://localhost:5000/search-es'
updateURL = 'http://localhost:5000/update-time'
st_base = 'https://www.space-track.org'
st_login = '/ajaxauth/login'
st_upload = '/fileshare/query/class/upload/folder_id/'
st_folder_id = '23214'
st_logout = '/ajaxauth/logout'
FOLDER_PATH = 'pro/mc_runs/'
DONE_FILE_PATH = FOLDER_PATH + 'done_file.done'

class RendevousOrbitPredictor(object):
  # def __init__(self, amount_per_day, start, imei):
  def __init__(self, config_dict, now):
    self.scheduler = sched.scheduler(time.time, time.sleep)
    self.index = 'statefield_report_' + config_dict['imei']
    self.es = Elasticsearch()
    self.times = self.time_string_to_time(config_dict['times'])
    self.run_now = now
    self.next = 0
    self.username = config_dict['username']
    self.password = config_dict['password']

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
    if self.run_now:
      self.periodic_send()
    else:
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

  def get_name_of_newest_file(self, folder_path):
    list_of_files = glob.glob(folder_path + '/MEME*') # * means all if need specific format then *.csv
    latest_file = max(list_of_files, key=os.path.getctime)
    file_name = latest_file.replace(folder_path, '') # remove folder from name
    return file_name

  def periodic_send(self):
    #Scheduling next run
    self.next = self.next + 1
    if self.next>= len(self.times):
      self.next_day_of_times()
      self.next = 0
    self.scheduler.enterabs(self.times[self.next], 1, self.periodic_send)
    
    # Call scripts for making calculations and generating files to upload
    self.call_scripts()

    newest_file_name = self.get_name_of_newest_file(FOLDER_PATH)
    print("Uploading " + newest_file_name + " to space-track")
    #Try Uploading the file to space-track 5 times
    uploaded = False
    tries = 0
    while not uploaded and tries < 5:
      tries = tries + 1
      uploaded = self.upload_file(newest_file_name)
    # update the time field in elasticsearch
    print(f'Upload worked: {uploaded}')
    if uploaded:
      self.update_time()
    print("Waiting for " + self.epoch_time_to_string(self.times[self.next]) + "...")

  def next_day_of_times(self):
    i = 0
    for t in self.times:
      self.times[i] = t + (24*60*60)
      i = i + 1


  # TODO 1: Call python file that makes calculations/generates file
  def call_scripts(self):
    # BEGIN HEADER CODE
    # os.system("python -m ptest runsim -c ptest/configs/amc.json -t MonteCarlo --clean -ni")
    # runpy.run_module("ptest", ['runsim', '-c', 'ptest/configs/amc.json', '-t', 'MonteCarlo', '--clean', '-ni'])
    print(f"Calling MonteCarlo Script at {datetime.now()}")
    
    proc_start_time = time.time()
    if os.path.exists(DONE_FILE_PATH):
      os.remove(DONE_FILE_PATH)
      print('Done file deleted')
    
    process = Popen(['python', '-m', 'ptest', 'runsim', '-c', 'ptest/configs/amc.json', '-t', 'MonteCarlo', '--clean', '-ni'], stdout=PIPE, stderr=PIPE)
    while not os.path.exists(DONE_FILE_PATH):
      time.sleep(1)
      # stdout, stderr = process.communicate()
      # print(stdout)
    process.kill()
    proc_end_time = time.time()
    print("Script took " + str(proc_end_time - proc_start_time) + " seconds to run")
    
    return

    # END HEADER CODE

  def upload_file(self, file_name):
    # print(f'Uploading {file_name}')
    with requests.Session() as session:
      resp = session.post(st_base + st_login, data = {'identity': self.username, 'password': self.password})
      if resp.status_code != 200:
        print("Login failed")
        return False
      with open(FOLDER_PATH + file_name, "rb") as ephemeris_data:
        file_dict = {file_name: ephemeris_data}
        resp = session.post(st_base + st_upload + st_folder_id, files = file_dict)
        if resp.status_code != 200:
          print("Upload failed")
          return False
      return True

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

    
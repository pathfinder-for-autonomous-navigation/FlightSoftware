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

class PredictorRendevousOrbit(object):
  def __init__(self, config_dict, run_now_option):
    '''Construct a PRO object.
    
    args:
      config_dict: A dictionary containing the configuration for PRO
      run_now_option: whether or not to run immeadiately, disregarding the schedule    
    '''
    self.scheduler = sched.scheduler(time.time, time.sleep)
    self.index = 'statefield_report_' + config_dict['imei']
    self.es = Elasticsearch()
    self.times = self.time_string_to_time(config_dict['times'])
    self.run_now = run_now_option
    self.next = 0
    self.username = config_dict['username']
    self.password = config_dict['password']

  def time_string_to_time(self, list_of_time_strs):
    '''Convert a list of time strings to time objects.
    
    args:
      local list of time strings
    returns:
      list of time objects
    '''
    time_objs = []
    for str in list_of_time_strs:
      parsed_time = list(time.strptime(str, "%H:%M:%S"))
      current_time = list(time.localtime())
      full_time = time.struct_time(tuple(current_time[0:3] + parsed_time[3:6] + current_time[6:]))
      time_objs.append(time.mktime(full_time))
    return time_objs

  def start(self):
    '''Start PRO.
    if self.run_now, run immediately, then follow the schedule
    otherwise, follow the schedule
    '''
    if self.run_now:
      self.periodic_send()
    else:
      self.next = self.get_initial_time_index()
      self.check_if_times_are_expired()
      self.scheduler.enterabs(self.times[self.next], 1, self.periodic_send)
      print("Waiting for " + self.epoch_time_to_string(self.times[self.next]) + "...")
      
      self.scheduler.run()

  def check_if_times_are_expired(self):
    '''If now is past all times, update self.times to tomorrow, set self.next to 0'''
    if self.next >= len(self.times):
      self.next_day_of_times()
      self.next = 0

  def get_initial_time_index(self):
    '''Get the initial time index to start from.
    
    returns:
      index of the initial time within self.times'''
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
    self.check_if_times_are_expired()
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
      self.update_pro_time_in_es()
    print("Waiting for " + self.epoch_time_to_string(self.times[self.next]) + "...")

  def next_day_of_times(self):
    '''Update self.times to the next day.'''
    self.times = [t + (24*60*60) for t in self.times]

  def call_scripts(self):
    '''Call the montecarlo script.
    '''
  
    print(f"Calling MonteCarlo Script at {datetime.now()}")
    
    proc_start_time = time.time()
    if os.path.exists(DONE_FILE_PATH):
      os.remove(DONE_FILE_PATH)
      print('Done file deleted')
    
    process = Popen(['python', '-m', 'ptest', 'runsim', '-c', 'ptest/configs/amc.json', '-t', 'MonteCarlo', '--clean', '-ni'], stdout=PIPE, stderr=PIPE)
    while not os.path.exists(DONE_FILE_PATH):
      time.sleep(1)

    process.kill()
    proc_end_time = time.time()
    print("Script took " + str(proc_end_time - proc_start_time) + " seconds to run")
    
    return

  def upload_file(self, file_name):
    '''Upload a file to space-track.
    
    args:
      file_name, the file to upload
    returns:
      if the upload succeeded
    '''
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

  def update_pro_time_in_es(self):
    """Update the pro time field in elasticsearch."""
    
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

    
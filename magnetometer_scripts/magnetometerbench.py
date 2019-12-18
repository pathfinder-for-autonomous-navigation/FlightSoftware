"""
13 DEC 2019
magnetometerbench.py
Nathan Zimmerberg (nhz2@cornell.edu)
uses examples from
http://docs.platformio.org/en/latest/projectconf/advanced_scripting.html#projectconf-advanced-scripting
"""

import fcserial
import time
import serial

Import("env", "projenv")

def after_upload(source, target, env):
    print("after_upload")
    print("Waitning for teensy to start up")
    testrates=([[0]*3,
                [50,0,0],
                [0,50,0],
                [0,0,50],
                [0]*3])
    time.sleep(5)
    portname=fcserial.findteensy()
    if (not portname):
        print('no teensy found')
        return

    print('\n\n Where should the data be saved? (CSV filename): \n Awaiting input')
    filename= raw_input();
    with serial.Serial(portname, 4000000, timeout=10) as ser:
        with open(filename,'w') as f:
            # write csv header line
            f.write("Test Start: "+time.asctime()+"\n")
            f.write("Filename: "+filename+"\n")
            print('\n\n External temperature (Celsius): \n Awaiting input')
            temp= raw_input();
            f.write("External temperature (Celsius): "+temp+"\n")
            print('\n\n External humidity (%): \n Awaiting input')
            temp= raw_input();
            f.write("External humidity (%): "+temp+"\n")
            print('\n\n Location: \n Awaiting input')
            temp= raw_input();
            f.write("Location: "+temp+"\n")
            print('\n\n People who did the test: \n Awaiting input')
            temp= raw_input();
            f.write("People who did the test: "+temp+"\n")
            print('\n\n Notes: \n Awaiting input')
            temp= raw_input();
            f.write("Notes: "+temp+"\n")
            headerline='reading time (s),magnetometer1 X (T),magnetometer1 Y (T),magnetometer1 Z (T),gyro X (rad/s),gyro Y (rad/s),gyro Z (rad/s),gyro temperature (C),wheel commanded rate X (rad/s),wheel commanded rate Y (rad/s),wheel commanded rate Z (rad/s), magrod commanded moment X (A*m^2),  magrod commanded moment Y (A*m^2),  magrod commanded moment Z (A*m^2)\n'
            f.write(headerline)
        for testrate in testrates:
            fcserial.writewheels(ser, testrate)
            #wait for teensy to acknowledge, ready for next message
            time.sleep(0.1)# this prevents python from busy waiting.
            ser.readline()
            for i in range(100):#get 100 readings
                print("starting %d of 100"%i)
                try:
                    line=fcserial.getreading(ser)
                    if (len(line)):
                        with open(filename,'a') as f:
                            fcserial.writecsvline(f,line)
                    #wait for teensy to acknowledge, ready for next message
                    time.sleep(0.1)# this prevents python from busy waiting.
                    ser.readline()
                except ValueError as e:
                    pass
    # do some actions

env.AddPostAction("upload", after_upload)

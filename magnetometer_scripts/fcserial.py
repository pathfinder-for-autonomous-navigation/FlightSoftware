"""
13 DEC 2019
fcserial.py
Nathan Zimmerberg (nhz2@cornell.edu)

Some utility functions to test the magnetometers.
"""
import time
import serial
from serial.tools import list_ports


def findteensy():
    """returns the port name to connect to a teensy over usb, returns None if no teensy is found."""
    port_list=serial.tools.list_ports.comports()
    for port in port_list:
        if port.manufacturer=='Teensyduino':
            return port.device

def getreading(ser):
    """returns a list of floats, of a reading, in the form of a csv, from teensy connected to serialport ser."""
    ser.reset_input_buffer()
    ser.write(b'r\n')
    line = ser.readline()   # read a '\n' terminated line
    return [float(s) for s in line.split(b',')[:-1]]

def writewheels(ser,rate):
    """write rate, list of 3 floats to wheel speeds, (rad/s), to teensy connected to open serialport ser."""
    ser.reset_input_buffer()
    ser.write(b'w %f %f %f\n'%tuple(rate))

def writecsvline(csvfile,line):
    """ writes a line to open file csvfile
    line is a list of floats"""
    csvfile.write(repr(line)[1:-1]+'\n')

def main(filename,testrates):
    portname=findteensy()
    if (not portname):
        print('no teensy found')
        return
    with serial.Serial(portname, 4000000, timeout=10) as ser:
        with open(filename,'w') as f:
            # write csv header line
            headerline='reading time (s),magnetometer1 X (T),magnetometer1 Y (T),magnetometer1 Z (T),gyro X (rad/s),gyro Y (rad/s),gyro Z (rad/s),gyro temperature (C),wheel commanded rate X (rad/s),wheel commanded rate Y (rad/s),wheel commanded rate Z (rad/s), magrod commanded moment X (A*m^2),  magrod commanded moment Y (A*m^2),  magrod commanded moment Z (A*m^2)\n'
            f.write(headerline)
        for testrate in testrates:
            writewheels(ser, testrate)
            #wait for teensy to acknowledge, ready for next message
            time.sleep(0.1)# this prevents python from busy waiting.
            ser.readline()
            for i in range(100):#get 100 readings
                print("starting %d of 100"%i)
                try:
                    line=getreading(ser)
                    if (len(line)):
                        with open(filename,'a') as f:
                            writecsvline(f,line)
                    #wait for teensy to acknowledge, ready for next message
                    time.sleep(0.1)# this prevents python from busy waiting.
                    ser.readline()
                except ValueError as e:
                    pass


if __name__ == '__main__':
    testrates=([[0]*3,
                [1]*3,
                [10]*3,
                [50]*3,
                [200]*3,
                [600]*3,
                [0]*3])
    main('test2.csv',testrates)

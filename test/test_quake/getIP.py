import serial
import traceback
import time
from email_helper import downloadFiles, sendFile
from argparse import ArgumentParser
from subprocess import check_output

if __name__ == '__main__':

    while True:
        with open("ip.sbd", "w") as f:
            f.write(check_output(['hostname', '-I']))

        sendFile("ip.sbd")
        time.sleep(60)

# sudo crontab -e
reboot python3 / home/pi/Desktop/exemple.py &

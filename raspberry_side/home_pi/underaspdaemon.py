#!/usr/bin/python -u
# -*- coding: utf-8 -*-

ARDUI2C="/home/pi/ardui2c"
HEARTBEAT_DAEMON="/home/pi/rpi_heartbeat.py"
BROADCAST_DAEMON="/home/pi/rpi_broadcast.py"
DELAY=5
LOCK_FILE="/tmp/rpi.lock"
LOCK_TIMEOUT=100
MIN_VOLTAGE=9.5
MIN_DISKSPACE=104857600

import time
import os
import sys
import syslog
from subprocess import Popen as popen
from subprocess import PIPE







def common_test():
   p = popen([ARDUI2C,"v"], shell=False, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
   voltage = float(p.stdout.read())
   if (voltage < MIN_VOLTAGE):
      syslog.syslog(syslog.LOG_ERR, "Voltage under limit")
      return False
   st = os.statvfs("/")
   disk_free = st.f_bavail * st.f_frsize
   if (disk_free < MIN_DISKSPACE):
      syslog.syslog(syslog.LOG_ERR, "No enought disk space")
      return False




#------------------------------
# Run sub_daemons
#------------------------------

syslog.syslog("Starting up")

p = popen(["nohup","python",HEARTBEAT_DAEMON], stdin=None, stdout=None, stderr=None, preexec_fn=os.setpgrp)
syslog.syslog("Launched heartbeat")


with open("/tmp/rpi_broadcast.msg","w") as file:
    file.write("test")


p = popen(["nohup","python",BROADCAST_DAEMON], stdin=None, stdout=None, stderr=None, preexec_fn=os.setpgrp)
syslog.syslog("Launched broadcast deaemon")






#------------------------------
# Global vars
#------------------------------

operations = {
   0: "developement",
   1: "streaming",
   2: "picture",
   3: "ctd",
}

debug=False




#------------------------------
# Getting running mode
#------------------------------
p = popen([ARDUI2C, "m"],shell=False, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
runstatus = p.stdout.read()
runstatus = int(runstatus[0:runstatus.find(".")])
print runstatus







opmode=runstatus&0x7f
if(runstatus&0x80>0): debug=True

print opmode, debug



def rpi_broadcast_message():
   return "hello world!"




if debug:
   # do some test

   p = popen([ARDUI2C,"y"], stdin=None, stdout=None, stderr=None)
   p.wait()
   time.sleep(10)
   p = popen([ARDUI2C,"Q"], stdin=None, stdout=None, stderr=None)
   p.wait()
   p = popen(["sudo","halt"], stdin=None, stdout=None, stderr=None)
   

else:
   # switch opmode
   pass
   # start core enviroment







#------------------------------
# Shutdown
#------------------------------

# Don't shutdown in case of lock (manual mode)
while (os.path.isfile(LOCK_FILE)):
   # Manual mode has a LOCK_TIMEOUT
   last_lock_time = os.stat(LOCK_FILE).ST_MTIME
   if (time.time()-last_lock_time>LOCK_TIMEOUT):
      break
   # Stop rpi in case of low voltage
   p = popen([ARDUI2C,"v"], shell=False, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
   voltage = float(p.stdout.read())
   if (voltage<MIN_VOLTAGE):
      break
   time.sleep(DELAY)

#p = popen([ARDUI2C,"Q"], shell=False, stdin=None, stdout=None, stderr=None, close_fds=True)


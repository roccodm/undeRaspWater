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
test_mode=False
WAIT_FOR_REMOTE=30


ERRCODE_INVALID_OPMODE="Y1"
ERRCODE_LOWBATT="Y11"
ERRCODE_DISKFULL="Y21"
ERRCODE_TEST_FAILED="Y101"
ERRCODE_RUN_FAILED="Y102"
ERRCODE_PROCESS_DIED="Y103"


import time
import os
import sys
import syslog
from subprocess import Popen, PIPE

from threading  import Thread
from Queue import Queue, Empty
def enqueue_output(out, queue):
    for line in iter(out.readline, b''):
        queue.put(line)
    out.close()





def ardIO(cmd):
   p = Popen([ARDUI2C,cmd], shell=False, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
   p.wait()
   output = float(p.stdout.read())
   return output



def common_tests():
   voltage = ardIO("v")
   if (voltage < MIN_VOLTAGE):
      syslog.syslog(syslog.LOG_ERR, "Voltage under limit")
      ardIO(ERRCODE_LOWBATT)
      return False
   st = os.statvfs("/")
   disk_free = st.f_bavail * st.f_frsize
   if (disk_free < MIN_DISKSPACE):
      syslog.syslog(syslog.LOG_ERR, "No enougth disk space")
      ardIO(ERRCODE_DISKFULL)
      return False
   return True

def quit(val):
   ardIO("Q")
   p = Popen(["sudo","halt"], stdin=None, stdout=None, stderr=None)
   sys.exit(val)




#------------------------------x
# Run sub_daemons
#------------------------------

syslog.syslog("Starting up")

heartbeat = Popen(["nohup","python",HEARTBEAT_DAEMON], stdin=None, stdout=None, stderr=None, preexec_fn=os.setpgrp)
syslog.syslog("Launched heartbeat")


broadcast = Popen(["nohup","python",BROADCAST_DAEMON], stdin=PIPE, stdout=None, stderr=None, preexec_fn=os.setpgrp)
syslog.syslog("Launched broadcast deaemon")






#------------------------------
# Global vars
#------------------------------

operations = {
   0: {"test":"/home/pi/test.manual.py","run":"/home/pi/run.manual.py"},
   1: {"test":"/home/pi/test.camera.py","run":"/home/pi/run.camera.py"},
}





#------------------------------
# Getting running mode
#------------------------------
runstatus = int(ardIO("m"))
opmode=runstatus&0x7f
if(runstatus&0x80>0):
   test_mode=True

# Check if operation is in the oplist
if not opmode in operations:
   # halt returnig error otherwise
   syslog.syslog("Unable to fine operation code %d" % opmode)
   ardIO(ERRCODE_INVALID_OPMODE)
   quit(1)

if not common_tests():
   quit(1)

if test_mode:
   main_process=Popen(["python",operations[opmode]["test"]], stdin=None, stdout=PIPE, stderr=None)
else:
   main_process=Popen(["python",operations[opmode]["run"]], stdin=None, stdout=PIPE, stderr=None)

q = Queue()
t = Thread(target=enqueue_output, args=(main_process.stdout, q))
t.daemon = True
t.start()

#------------------------------
# mainloop
#------------------------------

while True:
   try:  line = q.get_nowait()
   except Empty:
      # while waiting output from program
      if (ardIO("v")<MIN_VOLTAGE):
         ardIO(ERRCODE_LOWBATT)
         syslog.syslog("Low battery")
         quit(1)
         break
      if main_process.poll()!=None:
         ardIO(ERRCODE_PROCESS_DIED)
         syslog.syslog("Process died unexpectedly. Opmode: %d" % opmode)
         quit(1)
         break
   else:
      if (line.lower().find("ok")>=0):
         break
      else:
         if (test_mode):
            ardIO(ERRCODE_TEST_FAILED)
            syslog.syslog("Test failed. Opmode:%d" % opmode)
         else:
            ardIO(ERRCODE_RUN_FAILED)
            syslog.syslog("Run failed. Opmode:%d" % opmode)
         quit(1)
         break
   time.sleep(DELAY)



if not test_mode:
   time.sleep(WAIT_FOR_REMOTE)

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
   if (ardIO("v")<MIN_VOLTAGE):
      ardIO(ERRCODE_LOWBATT)
      syslog.syslog("Low battery" % opmode)
      quit(1)
   time.sleep(DELAY)

quit(0)

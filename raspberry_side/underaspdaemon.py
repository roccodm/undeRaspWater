#!/usr/bin/python -u
# -*- coding: utf-8 -*-

ARDUI2C="/home/pi/ardui2c"
BROADCAST_PORT=50000
DELAY=5

import time
import os
import sys
from socket import *
from subprocess import Popen as popen
from subprocess import PIPE


#------------------------------
# Getting running mode
#------------------------------
debug=False
cmd = ARDUI2C+" m"
p = popen(cmd, shell=True, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
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

   # switch opmode

else:
   # switch opmode


   # start core enviroment
   s = socket(AF_INET, SOCK_DGRAM)
   s.bind(('', 0))
   s.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
   while True:
      s.sendto("ciao\n\n", ('<broadcast>', BROADCAST_PORT))
      os.popen(ARDUI2C+" H")
      time.sleep(DELAY)


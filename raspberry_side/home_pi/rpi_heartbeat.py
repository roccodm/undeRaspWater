#!/usr/bin/python -u
# -*- coding: utf-8 -*-

ARDUI2C="/home/pi/ardui2c"


from subprocess import Popen as popen
from subprocess import PIPE
from time import sleep
import datetime



while True:
   cmd = ARDUI2C + " H"
   p = popen(cmd, shell=True, stdin=None, stdout=None, stderr=None, close_fds=True)
   cmd = ARDUI2C + " e"
   p = popen(cmd, shell=True, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
   if (p.stdout.read()==82 or p.stdout.read()==83):
      now = datetime.datetime.now()
      cmd = ARDUI2C + " T" + now.strftime("%y%m%d%H%M%S")
   sleep(10)




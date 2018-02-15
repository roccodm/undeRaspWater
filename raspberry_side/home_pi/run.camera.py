#!/usr/bin/python -u
# -*- coding: utf-8 -*-
##################################################
# run.camera.py
##################################################

ARDUI2C = "/home/pi/ardui2c"
PATH = "/var/www/html/data/pictures/"
TIMEOUT = 10

from subprocess import Popen,PIPE
import time
from datetime import datetime
import os
import sys


def ardIO(cmd):
   p = Popen([ARDUI2C,cmd], shell=False, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
   p.wait()
   output = float(p.stdout.read())
   return output


def error():
   ardIO("l")
   print("fail")
   sys.exit(1)


filename=PATH+datetime.now().strftime("%Y%m%d%H%M")+"capture.png"

ardIO("L")
time.sleep(1)
raspistill=Popen(["raspistill","-o",filename],stdin=None,stdout=None,stderr=None,shell=False)

timer=0
while (raspistill.poll() is None):
   if (timer >= TIMEOUT):
      raspistill.kill()
      error()
   timer += 1
   time.sleep(1)
ardIO("l")

if (os.path.isfile(filename) and os.stat(filename).st_size>0):
   print("ok")
else:
   error()



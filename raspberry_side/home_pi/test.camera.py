#!/usr/bin/python -u
# -*- coding: utf-8 -*-
##################################################
# test.camera.py
##################################################
# Simple rpi camera test routine
# Returns: ok when a picture can be aquired
#          fail otherwise
##################################################
from subprocess import Popen as popen
import time
import os

if (os.path.isfile("/tmp/test.png")):
   os.remove("/tmp/test.png")
x=popen("raspivid -o /tmp/test.png",stdin=None,stdout=None,stderr=None,shell=True)
time.sleep(10)
if x.poll() is None:
   print("fail")
   x.kill()
else:
   if (os.path.isfile("/tmp/test.png") and os.stat("/tmp/test.png").st_size>0):
      print("ok")
   else:
      print("fail");





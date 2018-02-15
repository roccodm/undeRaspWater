# Send UDP broadcast packets
MYPORT = 50000
DELAY = 5
ARDUI2C = "/home/pi/ardui2c"

import sys, time
from socket import *
from subprocess import Popen as popen
from subprocess import PIPE
import os



s = socket(AF_INET, SOCK_DGRAM)
s.bind(('', 0))
s.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)




str=""
json_string=""
while True:
   p = popen([ARDUI2C,"v"], shell=False, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
   p.wait()
   try:
      voltage = float(p.stdout.read())
   except:
      voltage = -1
   p = popen([ARDUI2C,"t"], shell=False, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
   p.wait()
   now = p.stdout.read()
   st = os.statvfs("/")
   disk_free = st.f_bavail * st.f_frsize
   str="Battery voltage: %f\nDisk free: %f\nTime: %s\n" % (voltage, disk_free,now)
   s.sendto(str+json_string, ('<broadcast>', MYPORT))
   time.sleep(DELAY)

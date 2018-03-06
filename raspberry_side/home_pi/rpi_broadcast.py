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
   now = p.stdout.read().strip().replace(" ","|")
   cpuserialcmd="cat /proc/cpuinfo | grep 'Serial\|Hardware' | cut -d ' ' -f 2"
   p = popen(cpuserialcmd, shell=True, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
   p.wait()
   cpuserial = p.stdout.read().strip().replace("\n","|")

   #data files checksum
   datacsmcmd="ls -lR /var/www/html/data |md5sum"
   p = popen(datacsmcmd, shell=True, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
   p.wait()
   datacsm = p.stdout.read().strip().replace("  -","")

   #no. data files
   ndatafilescmd="find /var/www/html/data -type f|wc -l"
   p = popen(ndatafilescmd, shell=True, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
   p.wait()
   ndatafiles = p.stdout.read().strip()



   st = os.statvfs("/")
   disk_free = st.f_bavail * st.f_frsize
   str="Battery voltage:%f\nDisk free:%f\nTime:%s\nSerial:%s\nData Csm:%s\nN. data files:%s\n" % (voltage, disk_free, now, cpuserial,datacsm,ndatafiles)
   s.sendto(str+json_string, ('<broadcast>', MYPORT))
   time.sleep(DELAY)

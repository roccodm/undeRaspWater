
# Send UDP broadcast packets
MYPORT = 50000

import sys, time
from socket import *
import os



# Return CPU temperature as a character string
def getCPUtemperature():
    res = os.popen('vcgencmd measure_temp').readline()
    return(res.replace("temp=","").replace("'C\n",""))

# Return % of CPU used by user as a character string
def getCPUuse():
    return(str(os.popen("top -n1 | awk '/Cpu\(s\):/ {print $2}'").readline().strip(\
)))

# Return information about disk space as a list (unit included)
# Index 0: total disk space
# Index 1: used disk space
# Index 2: remaining disk space
# Index 3: percentage of disk used
def getDiskSpace():
    p = os.popen("df -h /")
    i = 0
    while 1:
        i = i +1
        line = p.readline()
        if i==2:
            return(line.split()[1:5])



def getRTCdate():
    return os.popen('sudo hwclock -r').readline().replace("\n","")

def getBATTERY():
    return os.popen('/bin/ardui2c V|cut -b-6').readline().replace("\n","")

def getARDtemp():
    return os.popen('/bin/ardui2c C|cut -b-6').readline().replace("\n","")



s = socket(AF_INET, SOCK_DGRAM)
s.bind(('', 0))
s.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
while 1:
    data="CPU_TMP:"+getCPUtemperature()
    data+="\nARD_TMP:"+getARDtemp()
    data+="\nCPU_USG:"+getCPUuse()
    data+="\nDSK_FRE:"+str(getDiskSpace()[1])
    data+="\nBAT_VLT:"+getBATTERY()
    data+="\nRTC_DTE:"+getRTCdate()
    data+="\n\n"
    s.sendto(data, ('<broadcast>', MYPORT))
    time.sleep(5)



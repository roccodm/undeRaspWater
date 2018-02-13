#!/usr/bin/python
ARDUI2C="/home/pi/ardui2c"
HEARTBEAT_DAEMON="/home/pi/rpi_heartbeat.py"



import os
import cgi, cgitb
import json
from subprocess import Popen
from subprocess import PIPE
myform = cgi.FieldStorage()

out_data={}




print "Content-type: application/json\n\n"
cmd=myform.getvalue('cmd')




def get_voltage():
   cmd = ARDUI2C
   opt = "v"
   p = Popen([cmd, opt], shell=False, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
   out_data["volts"]=p.stdout.read()

def get_ampere():
   cmd = ARDUI2C
   opt = "a"
   p = Popen([cmd, opt], shell=False, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
   out_data["ampere"] = p.stdout.read()

def get_stats():
   out_data["cpuload"] = os.getloadavg()
   st = os.statvfs("/")
   out_data["disk_free"] = st.f_bavail * st.f_frsize
   out_data["disk_total"] = st.f_blocks * st.f_frsize
   out_data["disk_used"] = (st.f_blocks - st.f_bfree) * st.f_frsize

def set_manual_mode():
   out_data["manual_mode"]=True

def reset_manual_mode():
   out_data["manual_mode"]=False






commands = {
   "voltage": get_voltage,
   "ampere": get_ampere,
   "stats": get_stats,
   "set_manual": set_manual_mode,
   "reset_manual": reset_manual_mode,

}



if (cmd in commands):
   commands[cmd]()
   
else:
   out_data["error"]="Command not found"
   out_data["error_code"]=-1

print json.dumps(out_data, ensure_ascii=False)







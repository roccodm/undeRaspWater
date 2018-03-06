#!/usr/bin/python
# -------------------------------------------------------------------
#                       CGI User Interface
# -------------------------------------------------------------------
# Version: 100306_1


ARDUI2C="/home/pi/ardui2c"
HEARTBEAT_DAEMON="/home/pi/rpi_heartbeat.py"
LOCK_FILE="/tmp/rpi.lock"
HTML_PATH="/var/www/html/"
DATA_PATH=HTML_PATH+"data/"
STREAMER="/home/pi/rpi.streaming"


import os
import cgi, cgitb
import json
from subprocess import Popen
from subprocess import PIPE

myform = cgi.FieldStorage()

out_data={}


# Common utilities -----------------------------------------------------------

def touch(file):
    with open(file, 'a'):
        os.utime(file, None)

print "Content-type: application/json\n\n"
cmd=myform.getvalue('cmd')

def ardIO(cmd):
   p = Popen([ARDUI2C,cmd], shell=False, stdin=None, stdout=PIPE, stderr=None, close_fds=True)
   p.wait()
   try:
      output = float(p.stdout.read())
   except:
      output = p.stdout.read()
   return output

# User interface routines ---------------------------------------------------

def get_voltage():
   out_data["volts"]=ardIO("v")

def get_ampere():
   out_data["ampere"] = ardIO("a")

def get_stats():
   out_data["cpuload"] = os.getloadavg()
   st = os.statvfs("/")
   out_data["disk_free"] = st.f_bavail * st.f_frsize
   out_data["disk_total"] = st.f_blocks * st.f_frsize
   out_data["disk_used"] = (st.f_blocks - st.f_bfree) * st.f_frsize

def lock_on():
   touch(LOCK_FILE)
   out_data["locked_mode"]=True

def unlock():
   os.remove(LOCK_FILE)
   out_data["locked_mode"]=False

def list_files():
   filelist=[]
   for root, dirs, files in os.walk(DATA_PATH):
      for name in files:
         filelist.append(os.path.join(root.replace(HTML_PATH,""),name))
   out_data["data_files"]=filelist

def del_file():
   filename=myform.getvalue('file').replace("..","")
   if filename.startswith("/"):
      out_data["del_file"]="Error: forbidden path"
   else:
      filename=HTML_PATH+filename
      if os.path.isfile(filename):
         os.remove(filename)
         out_data["del_file"]=filename+" removed"
      else:
         out_data["del_file"]="Error: file not found"

def start_streaming():
   ardIO("L")
   cmd="nohup" + STREAMER + "&"
   os.system(cmd)
   out_data["Straming"]="Video streaming started"

def stop_streaming():
   ardIO("l")
   cmd="for PID in `ps ax |grep raspivid| cut -f1 -d' '`; do kill $PID; done"
   os.system(cmd)
   out_data["Straming"]="Video streaming stopped"

def ardui2c():
   par=myform.getvalue('par')
   out_data["ardui2c"]=ardIO(par)

def halt():
   ardIO("Q")
   p = Popen(["sudo","halt"], stdin=None, stdout=None, stderr=None)

#
# M A I N -----------------------------------------------------------
#
# All commands are available using cmd=cmd in the query string
# 
commands = {
   "voltage": get_voltage,
   "ampere": get_ampere,
   "stats": get_stats,
   "lock": lock_on,
   "unlock": unlock,
   "list_files": list_files,
   "del_file": del_file,	# requires file=file in qs
   "start_streaming": start_streaming,
   "stop_streaming": stop_streaming,
   "ardui2c": ardui2c,		# requires par=par in qs
   "halt": halt,
}



if (cmd in commands):
   commands[cmd]()
else:
   out_data["error"]="Command not found"
   out_data["error_code"]=-1

print json.dumps(out_data, ensure_ascii=False)







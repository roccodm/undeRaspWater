# Send UDP broadcast packets
MYPORT = 50000

import sys, time
from socket import *
import os



s = socket(AF_INET, SOCK_DGRAM)
s.bind(('', 0))
s.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
while 1:
    s.sendto("ciao\n\n", ('<broadcast>', MYPORT))
    time.sleep(5)

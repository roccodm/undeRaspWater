#!/bin/bash


./pinger.sh >/dev/null &

sudo mount -o umask=0022,gid=1000,uid=1000 /dev/sda /mnt




#ardui2c L
#raspivid -o /mnt/video.h264 -t 400000
#ardui2c l

#ardui2c Q
#sudo halt

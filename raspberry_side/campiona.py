import serial
import time
import RPi.GPIO as GPIO
import xml.etree.ElementTree as ET

co2_delay=500 # 5 minuti


try:
   ser_1=serial.Serial("/dev/ttyUSB0",9600,8,"N",1,timeout=1)
   ser_2=serial.Serial("/dev/ttyUSB1",9600,8,"N",1,timeout=1)
except:
   print "Problemi con connessione seriale"

#Identificazione porta seriale in funzione della natura dei dati
line=ser_1.readline()
if line[0:3]=="$GP":
   gps_ser=ser_1
   co2_ser=ser_2
   print "modo 1"
else:
   gps_ser=ser_2
   co2_ser=ser_1
   print "modo 2"




GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(18,GPIO.OUT)

GPIO.output(18,GPIO.HIGH)

# mainloop

gps_time=0
co2_time=0

while 1:
   line=gps_ser.readline()
#   print "gps:", line
   sentences=line.split(",")
   if sentences[0]=="$GPZDA":
      gps_time=int(float(sentences[1]))
      gps_date=sentences[2]+"/"+sentences[3]+"/"+sentences[4]
#      print "datetime:", gps_time, gps_date
   if sentences[0]=="$GPGGA":
      gps_lat=sentences[2] 
      gps_lon=sentences[4]
#      print "lat lon:", gps_lat, gps_lon 
   if 1:
#         print "*** accendo sensore"
         GPIO.output(18,GPIO.LOW)
#          attendo 20 secondi
         time.sleep(10)
#         leggo da seriale
         for i in range(11,100):
            co2data=co2_ser.readline()
            try:
               data=ET.fromstring(co2data)
               celltemp=float(data[0].find('celltemp').text)
               co2=float(data[0].find('co2').text)
               co2abs=float(data[0].find('co2abs').text)
            except:
               pass
            print ("%s\t%i\t%f\t%f\t%f") % (gps_time, i, co2, celltemp, co2abs)
#            print "Lon: %s\tLat: %s\tDate: %s\tTime: %s\tCelltemp: %f\t co2: %f\n" % \
#               (gps_lon, gps_lat, gps_date, gps_time, celltemp, co2)
            time.sleep(1)
         GPIO.output(18,GPIO.HIGH)
         co2_time=gps_time
         time.sleep(300)
gps_ser.close()
co2_ser.close()

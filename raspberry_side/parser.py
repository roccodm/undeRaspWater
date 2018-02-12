import xml.etree.ElementTree as ET

fo=open("log.co2","r")
lines=fo.readlines()

for line in lines:
   try:
            data=ET.fromstring(line)
            celltemp=float(data[0].find('celltemp').text)
            co2=float(data[0].find('co2').text)
            co2abs=float(data[0].find('co2abs').text)
            print "Celltemp: %f\t co2: %f\n" % \
               (celltemp, co2)
   except:
	    print "Error:", line

fo.close()

# undeRaspWater

Some useful links:
https://blog.retep.org/2014/02/15/connecting-an-arduino-to-a-raspberry-pi-using-i2c/

https://www.futurashop.it/Allegato_PDF_IT/7100-FT1073K.pdf

https://oscarliang.com/raspberry-pi-arduino-connected-i2c/


http://int03.co.uk/blog/2015/01/11/raspberry-pi-gpio-states-at-boot-time/
https://www.raspberrypi.org/documentation/configuration/pin-configuration.md


Per il voltmetro/amperometro:
http://www.adrirobot.it/arduino/voltmetro_amperometro/Arduino_voltmetro-amperometro.htm


Sulla raspberry vanno dissaldate le resistenza R1 e R2 per garantire il funzionamento del bus I2C quando è spenta. Il pullup è sulla scheda RTC, magari prevedere un pullup da 10K di backup

Per il driver dei led: http://electronics-diy.com/electronic_schematic.php?id=1012

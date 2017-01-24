#include <RTClib.h>
#include <EEPROM.h>

uint32_t wt=0; // waketime unix stamp

char buffer[20];

/*
 * Non serve che io scriva sempre sulla EEPROM.
 * Posso leggere la EEPROM e impostarci wt, variabile locale
 * poi variare wt senza salvare la eprom
 * Nel caso il sistema non venisse alimentato prima della data
 * prevista, partirebbe immediatamente con il timestep impostato.
 * 
 * TODO
 * 1) vedere come passare i parametri da raspberry ad arduino per settare
 * la EEPROM
 * 2) resettare wt dopo che è stata scritta/variata la eeprom
 * 
 * Note:
 * 1) non serve ricalcolare il tempo di riavvio allo spegnimento della rapsberry
 * in modo che i punti saranno intervallati in modo regolare
 * 
 * 2) si potrebbe fissare un tempo minimo di timestep, non inferiore a 10 min
 */

void setup() {
   Serial.begin(9600);
   // inizializzo eeprom se vuota
   if (EEPROM.read(0)!=17) {
       Serial.write("Inizio a scrivere la eeprom\n");
       EEPROM.write(0,17); //year, last two digit
       EEPROM.write(1,01); //month
       EEPROM.write(2,24); //day
       EEPROM.write(3,22); //Hour
       EEPROM.write(4,10); //minute
       EEPROM.write(5,15); //timestep in minutes
       Serial.write("Finito di scrivere la eeprom\n");
   }
   
   DateTime ora(EEPROM.read(0)+2000,
                EEPROM.read(1),
                EEPROM.read(2),
                EEPROM.read(3),
                EEPROM.read(4),
                00);
   showDate("ora",ora);
   wt=ora.unixtime()+(EEPROM.read(5)*60);


}

void loop() {
   DateTime sveglia(wt);
   showDate("sveglia",sveglia);
   delay(5000);

}

void showDate(const char* txt, const DateTime& dt) {
    Serial.print(txt);
    Serial.print(' ');
    Serial.print(dt.year(), DEC);
    Serial.print('/');
    Serial.print(dt.month(), DEC);
    Serial.print('/');
    Serial.print(dt.day(), DEC);
    Serial.print(' ');
    Serial.print(dt.hour(), DEC);
    Serial.print(':');
    Serial.print(dt.minute(), DEC);
    Serial.print(':');
    Serial.print(dt.second(), DEC);
    
    Serial.print(" = ");
    Serial.print(dt.unixtime());
    Serial.print("s / ");
    Serial.print(dt.unixtime() / 86400L);
    Serial.print("d since 1970");
    
    Serial.println();
}

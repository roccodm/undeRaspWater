/*************************************************************************************** 
 *  UnderRaspino
 ***************************************************************************************
 *  v. 1.0a
 *  by Rocco De Marco - rocco.demarco@an.ismar.cnr.it
 ***************************************************************************************/

// MESSAGES
#define START_MSG "\n\nUnderRaspino Ready."
#define I2C_INIT_FAIL "FATAL: i2c bus error"
#define RTC_INIT_FAIL "FATAL: RTC error"
#define LOWBAT "Low battery to start Raspberry"
#define RASP_FAIL "Unable to powerup Raspberry"
#define MENU1 "\nA\tpower used(A)\nC\tArduino temperature\nd/D\tread/write EEPROM datestam (YYMMDDHHMMXXX)\nE/e\tclear/read last error in eprom"
#define MENU2 "\nH/h\tset/read heartbeat\nK/k\tenable/disable raspberry serial out\nL\tBlink led\nP/p\tenable/disable arduino serial out\nQ\tQuit raspberry"
#define MENU3 "\nS/s\tstart/stop raspberry relay\nT/t\tSet/read RTC (YYMMDDHHMMSS)\nV\tread voltage\n?\tPrint his menu"

// ERR CODES
#define I2C_ERRCODE 1
#define RTC_ERRCODE 2
#define LOWBAT_ERRCODE 3
#define RASP_FAIL_ERRCODE 4

// CONFIG
#define VCC 3.3
#define SERIAL_RASPBERRY_PIN 13
#define SERIAL_ARDUINO_PIN 12
#define RELAY_SET_PIN 9
#define RELAY_RESET_PIN 8
#define OK_LED_PIN 7
#define FAIL_LED_PIN 6
#define RASPBERRY_STATUS_PIN 5
#define MOSFET_PIN 4
#define BAUD_RATE 9600
#define VOLTAGE_PIN A0
#define AMPERE_PIN A1
#define I2C_ADDRESS 0x04
#define I2C_SDA A4
#define I2C_SCL A5
#define RTC_MIN_DATE 1483228800 //1/1/2017 0:0:0
#define BUFFSIZE 21
#define R_ALPHA 0.152 // for V read purpose 
#define RASP_SHUTDOWN_TIME 30000
#define MAX_TRY 5        // max raspberry starting tentatives
#define TRY_DELAY 30000     // delay ( between tests
#define MIN_BATTERY_VOLTAGE 10  // minimum voltage to start raspberry in safe

// INCLUDES
#include <Wire.h>
#include "RTClib.h"
#include <EEPROM.h>

// GLOBAL VARS
RTC_DS1307 RTC;
DateTime now;
char buffer[BUFFSIZE];    // general purpose global buffer
bool error_status=false;  // 
bool rasp_running=false;  // raspberry running status flag
bool halt_request=false;  // raspberry asks for halt
bool heartbeat=false;     // Setted from raspberry via i2c
double i2c_val;           // return value for i2c operations
uint32_t wt=0;            // waketime unix stamp

/*
 * Function error_handler(errcode, msg)
 * 1) prints an error message on serial connection
 * 2) sets a errcode in eeprom 
 * 3) sets to true global variable error_status
 * 4) turns on a red led [TODO]
 */
void error_handler(uint8_t errcode, const char * msg){
   Serial.println(msg);
   error_status=true; 
   EEPROM.write(0,errcode);
   digitalWrite(OK_LED_PIN,0);
   digitalWrite(FAIL_LED_PIN,1);  // Turn on red led
}

/*
 * Function read_voltage()
 * reads 5 times voltage and return the median value
 */

double read_voltage(){
   double v[5];
   double temp;
   for (int i=0;i<5;i++) v[i]=(analogRead(VOLTAGE_PIN)*VCC/1024)/R_ALPHA;
   // sorting values
   for (int i=0;i<4;i++)
      for (int j=1;j<5;j++)
         if (v[j]>v[i]){
            temp=v[i];
            v[i]=v[j];
            v[j]=temp;
         }
    return v[2]; // return the median value
}


/*
 * function rasp_relay (set)
 * turn on/off raspberry
 * sets the global var "rasp_running"
 */

double rasp_relay(bool set){
   if (set) {
       digitalWrite(RELAY_SET_PIN,1);
       delay(100);
       digitalWrite(RELAY_SET_PIN,0);
       rasp_running=true;
       return 1;
   } else {
       digitalWrite(RELAY_RESET_PIN,1);
       delay(100);
       digitalWrite(RELAY_RESET_PIN,0);       
       rasp_running=false;
       return 0;
   }
}
/*
 * EPROM data structure
 * ----------------------------------------
 * byte | value
 * ----------------------------------------
 *   0  | Last error code
 *   1  | Year, last two digits  
 *   2  | Month
 *   3  | Day
 *   4  | Hour
 *   5  | Minute
 *   6  | Timestep for delay (minutes)
 * ----------------------------------------
 */

/*
 * Function read_eprom_date()
 * read the eeprom date,time and timestem and returns it in global var buffer
 */
double read_eprom_datetime(){ 
   sprintf(buffer,"%02d%02d%02d%02d%02d%03d",EEPROM.read(1),EEPROM.read(2),EEPROM.read(3),
           EEPROM.read(4),EEPROM.read(5),EEPROM.read(6));  
   // returns -2 that means the the return values are stored on global buffer[]
   return -2;
}




double set_eprom_datetime(char *data){
   if (strlen(data)!=13) return -1.2;    // Malformed query string
   char str[4];
   sprintf(str,"%c%c",data[0],data[1]);
   int year=atoi(str);
   sprintf(str,"%c%c",data[2],data[3]);
   int month=atoi(str);
   sprintf(str,"%c%c",data[4],data[5]);
   int day=atoi(str);
   sprintf(str,"%c%c",data[6],data[7]);
   int hour=atoi(str);
   sprintf(str,"%c%c",data[8],data[9]);
   int minute=atoi(str);
   sprintf(str,"%c%c%c",data[10],data[11],data[12]);
   int timestep=atoi(str);   
   if (year<17 || month<=0 || month>12 || day<=0 || day >31 || hour>=24 || minute>=60 || 
       timestep <=0 || timestep >250) return -1;  // Problem in conversion
   EEPROM.write(1,year);
   EEPROM.write(2,month);
   EEPROM.write(3,day);   
   EEPROM.write(4,hour);
   EEPROM.write(5,minute);
   EEPROM.write(6,timestep);  
   update_waketime(); 
   sprintf(buffer,"ok");
   return -2;
}

/*
 * Function set_rct_datetime(string)
 * sets the rtc datetime to given value
 * string format: YYMMDDHHMMSS
 */

double set_rtc_datetime(char *data){
   if (strlen(data)!=12) return -1;    // Malformed query string
   char str[4];
   sprintf(str,"%c%c",data[0],data[1]);
   int year=atoi(str);
   sprintf(str,"%c%c",data[2],data[3]);
   int month=atoi(str);
   sprintf(str,"%c%c",data[4],data[5]);
   int day=atoi(str);
   sprintf(str,"%c%c",data[6],data[7]);
   int hour=atoi(str);
   sprintf(str,"%c%c",data[8],data[9]);
   int minute=atoi(str);
   sprintf(str,"%c%c",data[10],data[11]);
   int second=atoi(str);
   sprintf(buffer,"%02d/%02d/%02d %02d:%02d:%02d",day,month,year,hour,minute,second); //debug
   Serial.println(buffer); //debug
   RTC.adjust(DateTime(year+2000,month,day,hour,minute,second));
   return 1;
}

/*
 * Function update_waketime()
 * 
 * Read the eprom and the next raspberry wake time
 * stores the result, in unix time format in the global wt
 */
void update_waketime(){
   DateTime ora(EEPROM.read(1)+2000,
                EEPROM.read(2),
                EEPROM.read(3),
                EEPROM.read(4),
                EEPROM.read(5),
                00);
   wt=ora.unixtime(); 
}

/*
 * Function next_waketime()
 * 
 * Increase the global wt (waketime) variable of minutes stored in flash
 */
void next_waketime(){
   now=RTC.now();
   wt=now.unixtime();
   wt+=EEPROM.read(6)*60;
}

/*
 * Function start_raspberry
 * start raspberry and check for heartbeat
 * MAX_TRY are performed, waiting for TRY_DELAY to check for heartbeat
 */

int start_raspberry(){
   if (user_interface("V")>MIN_BATTERY_VOLTAGE){     // if enought battery...
      Serial.println("Starting raspberry");
      next_waketime();    // calculate next waketime
      rasp_relay(true);   // turn on raspberry
      // check raspberry feedback
      if (!digitalRead(RASPBERRY_STATUS_PIN)) {
          rasp_relay(true); // try again activating relay
          delay(2000);      // wait for a couple of seconds
          if (!digitalRead(RASPBERRY_STATUS_PIN)) { // test again
             error_handler(RASP_FAIL_ERRCODE,RASP_FAIL);
             return -1;
          }
      }
      //waiting from raspberry heartbeat
      for (int i=0; i<MAX_TRY; i++){
         delay(TRY_DELAY);
         if (!heartbeat){
            Serial.println("Raspberry non partita"); //Debug            
         } 
         else break; // Partita
      }
      if (heartbeat) {
         Serial.write("Ricevuto heartbeat"); // debug
         blink_led();
         heartbeat=false;     // Reset heart
      } else {           //not heartbit
               // TODO: gestire errore               
      }
   } else {
      error_handler(LOWBAT_ERRCODE,LOWBAT);
   } //end cycle battery low
}

/* 
 * function quit_raspberry()
 * simple helper to wait for a delay and turn off raspberry
 */
double quit_raspberry(){
   halt_request=true;
   warning_led();
   return 1;
}


/*
 * function warning_led()
 * turns on the status led in yellow color for some second
 * a two pin red/green led in connected on OK_LED_PIN and FAIL_LED_PIN
 */
void warning_led() {
   int i=0;
   while (i++<1000){
      digitalWrite(OK_LED_PIN,0);
      digitalWrite(FAIL_LED_PIN,1);
      delay(2);
      digitalWrite(FAIL_LED_PIN,0);
      digitalWrite(OK_LED_PIN,1);
      delay(2);                         
   }
   digitalWrite(OK_LED_PIN,0);      
}

/*
 * function warning_led()
 * blink status led in green color for some second
 */
void blink_led() {
   digitalWrite(FAIL_LED_PIN,0);   // just to be sure...
   for (int i=0;i<5;i++){
      digitalWrite(OK_LED_PIN,1);  // turn on green led blinking
      delay(200);
      digitalWrite(OK_LED_PIN,0);
      delay(200);
   }
}


/*
 * Function GetTemp: return internal temperature
 * Credits: http://playground.arduino.cc/Main/InternalTemperatureSensor
 * by: Arduino LLC 
 * Creative Commons Attribution ShareAlike 3.0 
 */
double GetTemp(void)
{
   unsigned int wADC;
   double t;
   ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
   ADCSRA |= _BV(ADEN); // enable the ADC
   delay(50); // wait for voltages to become stable.
   ADCSRA |= _BV(ADSC); // Start the ADC
   while (bit_is_set(ADCSRA,ADSC));
   wADC = ADCW;
   t = (wADC - 324.31 ) / 1.22;
   return abs(t);
}


/*
 * Function user_interface (cmd_string)
 * It's a middleware between user-interaction (serial or i2c) and i/o functions
 * The first char in cmd_string contains the command selector, the other (optional)
 * chars are used as parameters.
 * Maximum lenght is BUFF_SIZE
 * 
 * Possible return values:
 *  1 = Function executed without problem
 * -1 = Function closed with error
 * -2 = Results stored in global buffer[]
 * any positive number = results from adc or similar
 */

double user_interface (char *cmd_string){
   char str[20];
   int i;
   char cmd=cmd_string[0];   
   double retval=-1;
   switch (cmd){
      case 'A':  // Ampere read /////////////////////////////////////////////////////////////////////////////////////////////
          retval=abs((analogRead(AMPERE_PIN)*VCC/1024)-2.5)/0.185;
          break;
      case 'C':  // internalTemperature read ////////////////////////////////////////////////////////////////////////////////
          retval=GetTemp();
          break;
      case 'd':  // read Eprom Date /////////////////////////////////////////////////////////////////////////////////////////
          retval=read_eprom_datetime();
          break;
      case 'D':  // set Eprom Date //////////////////////////////////////////////////////////////////////////////////////////
          if (strlen(cmd_string)!=14) return -1.1;
          for (i=0;i<13;i++) str[i]=cmd_string[i+1];
          str[i]='\0';
          retval=set_eprom_datetime(str);
          break;
      case 'E':  // clear last error in EPROM ///////////////////////////////////////////////////////////////////////////////
          EEPROM.write(0,0);
          retval=1;
          break;
      case 'e':  // read last error in EPROM ////////////////////////////////////////////////////////////////////////////////
          retval=EEPROM.read(0);
      case 'H':  // set earthbit ////////////////////////////////////////////////////////////////////////////////////////////
          heartbeat=true;
          retval=1;    
          break;
      case 'h':  // read heartbeat //////////////////////////////////////////////////////////////////////////////////////////
          retval=heartbeat;    
          break;     
      case 'K':  // enable raspberry serial output (and disable arduino output) /////////////////////////////////////////////
          digitalWrite(SERIAL_ARDUINO_PIN,0);       
          digitalWrite(SERIAL_RASPBERRY_PIN,1);
          break;     
      case 'k':  // disable raspberry serial output /////////////////////////////////////////////////////////////////////////
          digitalWrite(SERIAL_RASPBERRY_PIN,0);       
          break;     
      case 'L':  // Test led ////////////////////////////////////////////////////////////////////////////////////////////////
          blink_led();
          break;
      case 'P':  // enable arduino serial output (and disable raspberry output) /////////////////////////////////////////////
          digitalWrite(SERIAL_ARDUINO_PIN,1);       
          digitalWrite(SERIAL_RASPBERRY_PIN,0);
          Serial.println(START_MSG);
          user_interface("?");
          break;     
      case 'p':  // disable arduino serial output //////////////////////////////////////////////////////////////////////////
          digitalWrite(SERIAL_ARDUINO_PIN,0);
          break; 
      case 'Q':  // quit raspberry with delay ///////////////////////////////////////////////////////////////////////////////
          retval=quit_raspberry();
          break;
      case 'S':   // Start raspberry ////////////////////////////////////////////////////////////////////////////////////////        
          retval=rasp_relay(true);
          break;
      case 's':   // Stop raspberry immediately//////////////////////////////////////////////////////////////////////////////
          retval=rasp_relay(false);
          break;
      case 'T':  // set rtc datetime ////////////////////////////////////////////////////////////////////////////////////////
          if (strlen(cmd_string)<13) return -1;
          for (i=0;i<12;i++) str[i]=cmd_string[i+1];
          str[i]='\0';
          retval=set_rtc_datetime(str);       
          break;         
      case 't':  // return time /////////////////////////////////////////////////////////////////////////////////////////////
          if (!rasp_running) {
             now=RTC.now();
             sprintf(buffer,"%02d/%02d/%04d %02d:%02d:%02d",now.day(),now.month(),
                            now.year(),now.hour(),now.minute(),now.second());
             retval=-2;  // means print buffer
          }
          break;    
      case 'V':  // Voltage read ////////////////////////////////////////////////////////////////////////////////////////////
          retval=read_voltage();
          break;
      case '?':  // print menu //////////////////////////////////////////////////////////////////////////////////////////////
          Serial.print(MENU1);
          Serial.print(MENU2);
          Serial.println(MENU3);          
          break;          
          
   }
   return retval;
}


/*
 * Function serialEvent()
 * Is called in case of serialEvent
 * 1) assembles the command string
 * 2) calls the user_interface function
 * input is trunked at BUFF_SIZE max lenght
 */

void serialEvent(){
   char data=0;
   int i=0;
   double retval;
   while(Serial.available()) {
      data=Serial.read();    
      if (i<BUFFSIZE-1) buffer[i++]=data;
   }
   buffer[i]='\0';
   Serial.print("CMD ");
   Serial.print(buffer);
   Serial.print(":");
   retval=user_interface(buffer);
   if (retval==-2) Serial.println(buffer);
   else Serial.println(retval);
}


void i2c_receive(int count){
   char data=0;
   int i=0;
   while(Wire.available()) {    
      data=Wire.read();          
      if (i<BUFFSIZE-1) buffer[i++]=data;
   }
   buffer[i]='\0';
   i2c_val=user_interface(buffer);
   if (i2c_val!=-2) dtostrf(i2c_val,6,3, buffer);
}

void i2c_send(){
   Wire.write(buffer);  
}




void setup() {
  // Set ports
  pinMode(SERIAL_ARDUINO_PIN, OUTPUT);
  pinMode(SERIAL_RASPBERRY_PIN, OUTPUT);
  pinMode(RASPBERRY_STATUS_PIN,INPUT);
  pinMode(RELAY_SET_PIN, OUTPUT);
  pinMode(RELAY_RESET_PIN, OUTPUT);
  pinMode(MOSFET_PIN, OUTPUT);
  pinMode(OK_LED_PIN, OUTPUT);  
  pinMode(FAIL_LED_PIN, OUTPUT);
  // Inizialize I/O
  Serial.begin(BAUD_RATE); // Start Serial connection

  // Check if raspberry is running (due arduino reset a/o watchdog)
  if (digitalRead(RASPBERRY_STATUS_PIN)) rasp_running=true;

  // Inizialize I2C
  Wire.begin(I2C_ADDRESS); // Start I2C
  RTC.begin();             // Start the RTC clock
  if (!rasp_running){ // Cannot use i2c bus while raspberry is on
     if (analogRead(I2C_SDA)<900 || analogRead(I2C_SCL)<900) {
        error_handler(I2C_ERRCODE,I2C_INIT_FAIL);
     } else {
        // Check RTC is working
        if(! RTC.isrunning()) error_handler(RTC_ERRCODE,RTC_INIT_FAIL);
        now=RTC.now();
        if (now.unixtime()<RTC_MIN_DATE) error_handler(RTC_ERRCODE,RTC_INIT_FAIL);
        // Initialize wakeup time register
        update_waketime();
        // Chect wakeuptime, if is before now, set to now
        if (wt<now.unixtime()) {         // if the waketime is passed
           next_waketime();              // and increase of timestep
           warning_led();                // notify with warning light
        }
     } 
  } else blink_led();  // debug
  // Bind i2c callbacks
  Wire.onReceive(i2c_receive);
  Wire.onRequest(i2c_send);

  // Finally
  if (!error_status) {
     Serial.println(START_MSG);
     digitalWrite(FAIL_LED_PIN,0); 
     digitalWrite(OK_LED_PIN,1);  // Turn on green led
     delay (2000);
     digitalWrite(OK_LED_PIN,0);  // Turn off led
  }
}

/********************************************************************
 * MAIN LOOP 
 ********************************************************************/

void loop() {
  if(!error_status){
     if(!rasp_running){
        now=RTC.now();
        Serial.println(wt);
        Serial.println(now.unixtime());
        int32_t diff=wt-now.unixtime();
        Serial.println(diff);
                
        if(diff<0){
           start_raspberry();
        } //end cycle raspberry not running
     } else { //raspberry running
        Serial.println(heartbeat);
        heartbeat=false;
        if (halt_request) {
          warning_led();     //debug  
          delay(RASP_SHUTDOWN_TIME);
          rasp_relay(false);
          halt_request=false;
        }
     } //end cycle rasberry runnuing
  } else { // error status setted
    
  }
  delay(5000);

}



//TODO: watchdog

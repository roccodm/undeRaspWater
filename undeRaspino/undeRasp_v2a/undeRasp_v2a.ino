// INCLUDES
#include <Wire.h>
#include "RTClib.h"
#include <EEPROM.h>

// MESSAGES
#define START_MSG "\n\nUnderRaspino Ready."
#define I2C_INIT_FAIL "FATAL: i2c bus error"
#define RTC_INIT_FAIL "FATAL: RTC error"
#define LOWBAT "Low battery to start RB"
#define RASP_FAIL "Unable to powerup RB"
#define RASP_NO_HB "Unable to receive Heartbeat from RB"
#define RASP_STARTING "Starting RB"
#define RASP_STOP "RB stopped"
#define MENU1 "\nA\tpower used(A)\nC\tARD temperature\nd/D\tread/write EEPROM datestam (YYMMDDHHMMXXX)\nE/e\tclear/read last error in eprom"
#define MENU2 "\nH/h\tset/read heartbeat\nK/k\tenable/disable RB serial out\nL/l\tTurn on/off mosfet\nM\tSecs left to start RB\nO/o\tset/read keep on RB (use q to quit)\nP/p\tenable/disable ARD serial out\nQ\tQuit RB"
#define MENU3 "\nR\tRB running status\nS/s\tstart/stop RB relay\nT/t\tSet/read RTC (YYMMDDHHMMSS)\nV\tread voltage\nW\tread watt\n?\tPrint this menu"

// ERR CODES
#define I2C_ERRCODE 81
#define RTC_ERRCODE 82
#define LOWBAT_ERRCODE 83
#define RASP_FAIL_ERRCODE 84
#define RASP_NO_HB_ERRCODE 85

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
#define RTC_MIN_DATE 1483228800   //1/1/2017 0:0:0
#define BUFFSIZE 21
#define R_ALPHA 0.152             // for V read purpose, R partitor coeff.
#define MIN_BATTERY_VOLTAGE 9     // minimum voltage to start raspberry in safe

#define EEPROM_ERR_LOCATION 7

// GLOBAL VARS
RTC_DS1307 RTC;
DateTime now;
char buffer[BUFFSIZE];    // general purpose global buffer
bool error_status=false;  // flag setted up in case of fatal errors
double i2c_val;           // return value for i2c operations

void error_handler(char errcode, const char * msg) {
   Serial.println(msg);
   error_status=true; 
   EEPROM.write(EEPROM_ERR_LOCATION,errcode);
   digitalWrite(OK_LED_PIN,0);
   digitalWrite(FAIL_LED_PIN,1);  // Turn on red led
}

double user_interface (char *cmd_string){
   char str[20];
   int i;
   char cmd=cmd_string[0];   
   double retval=-1;
   switch (cmd){
    default:
      break; // unknown command
   }
   return retval;
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

void loop() {
  if (error_status) {
    return; // Disable loop if an error happend
  }
}

void setup(){

  // Pin setup
  pinMode(SERIAL_ARDUINO_PIN, OUTPUT);
  pinMode(SERIAL_RASPBERRY_PIN, OUTPUT);
  pinMode(RASPBERRY_STATUS_PIN,INPUT);
  pinMode(RELAY_SET_PIN, OUTPUT);
  pinMode(RELAY_RESET_PIN, OUTPUT);
  pinMode(MOSFET_PIN, OUTPUT);
  pinMode(OK_LED_PIN, OUTPUT);  
  pinMode(FAIL_LED_PIN, OUTPUT);
  // Reset pins
  digitalWrite(RELAY_SET_PIN,0);
  digitalWrite(RELAY_RESET_PIN,0);  
  digitalWrite(MOSFET_PIN,0); 
  digitalWrite(SERIAL_ARDUINO_PIN,1);

  // Inizialize I/O
  Serial.begin(BAUD_RATE); // Start Serial connection

  // Inizialize I2C
  Wire.begin(I2C_ADDRESS); // Start I2C
  RTC.begin();             // Start the RTC clock
  if (analogRead(I2C_SDA)<900 || analogRead(I2C_SCL)<900) {
      error_handler(I2C_ERRCODE,I2C_INIT_FAIL);
  } else {
      // Check RTC is working
      if(! RTC.isrunning()){
        error_handler(RTC_ERRCODE,RTC_INIT_FAIL);
      }
  }

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

  cli();//stop interrupts

  // TIMER 1 for interrupt frequency 1 Hz:
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 1 Hz increments
  OCR1A = 62499; // = 16000000 / (256 * 1) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 256 prescaler
  TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts

  sei();//allow interrupts

}

int dbg_counter = 0;
void dbg_timer() {
   char buf[16];
   itoa(dbg_counter, buf, 10);
   Serial.println(buf);
   dbg_counter++;
}

ISR(TIMER1_COMPA_vect){
   // Timer1 interrupt
   dbg_timer(); // used for serial debug, disable when going live

}

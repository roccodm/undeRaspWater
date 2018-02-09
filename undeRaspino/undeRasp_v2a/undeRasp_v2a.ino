// INCLUDES
#include <EEPROM.h>
#include <Wire.h>
#include "RTClib.h"
#include "defines.h"
#include "rpi.h"

// GLOBAL VARS
RTC_DS1307 RTC;
DateTime now;
char buffer[BUFFSIZE];      // general purpose global buffer
bool error_status = false;  // flag setted up in case of fatal errors
double i2c_val;		    // return value for i2c operations

void error_handler(char errcode, const char *msg) {
	Serial.println(msg);
	error_status = true;
	EEPROM.write(EEPROM_ERR_LOCATION, errcode);
	digitalWrite(OK_LED_PIN, 0);
	digitalWrite(FAIL_LED_PIN, 1);  // Turn on red led
}

double user_interface(char *cmd_string) {
	char str[20];
	int i;
	char cmd = cmd_string[0];
	double retval = -1;
	switch (cmd) {
		default:
			break;  // unknown command
	}
	return retval;
}

void i2c_receive(int count) {
	char data = 0;
	int i = 0;
	while (Wire.available()) {
		data = Wire.read();
		if (i < BUFFSIZE - 1) buffer[i++] = data;
	}
	buffer[i] = '\0';
	i2c_val = user_interface(buffer);
	if (i2c_val != -2) dtostrf(i2c_val, 6, 3, buffer);
}

void i2c_send() { Wire.write(buffer); }

void loop() {
	if (error_status) {
		return;  // Disable loop if an error happend
	}
	if (first_start) {
		// do first run, raspberry will do self test and shutdown
		if (!rpi_started) {
			start_rpi();
		} else if (!is_rpi_running() &&
			   rpi_cooldown > RPI_START_COOLDOWN) {
			// raspberry has shutdown?! reset first start, entering
			// normal operation mode
			first_start = false;
			stop_rpi();
		}
	} else {
	}
}

void setup() {
	// Pin setup
	pinMode(SERIAL_ARDUINO_PIN, OUTPUT);
	pinMode(SERIAL_RASPBERRY_PIN, OUTPUT);
	pinMode(RASPBERRY_STATUS_PIN, INPUT);
	pinMode(RELAY_SET_PIN, OUTPUT);
	pinMode(RELAY_RESET_PIN, OUTPUT);
	pinMode(MOSFET_PIN, OUTPUT);
	pinMode(OK_LED_PIN, OUTPUT);
	pinMode(FAIL_LED_PIN, OUTPUT);
	// Reset pins
	digitalWrite(RELAY_SET_PIN, 0);
	digitalWrite(RELAY_RESET_PIN, 0);
	digitalWrite(MOSFET_PIN, 0);
	digitalWrite(SERIAL_ARDUINO_PIN, 1);

	// Inizialize I/O
	Serial.begin(BAUD_RATE);  // Start Serial connection

	// Inizialize I2C
	Wire.begin(I2C_ADDRESS);  // Start I2C
	RTC.begin();		  // Start the RTC clock
	if (analogRead(I2C_SDA) < 900 || analogRead(I2C_SCL) < 900) {
		error_handler(I2C_ERRCODE, I2C_INIT_FAIL);
	} else {
		// Check RTC is working
		if (!RTC.isrunning()) {
			error_handler(RTC_ERRCODE, RTC_INIT_FAIL);
		}
	}

	// Bind i2c callbacks
	Wire.onReceive(i2c_receive);
	Wire.onRequest(i2c_send);

	// Finally
	if (!error_status) {
		Serial.println(START_MSG);
		digitalWrite(FAIL_LED_PIN, 0);
		digitalWrite(OK_LED_PIN, 1);  // Turn on green led
		delay(2000);
		digitalWrite(OK_LED_PIN, 0);  // Turn off led
	}

	cli();  // stop interrupts

	// TIMER 1 for interrupt frequency 1 Hz:
	TCCR1A = 0;  // set entire TCCR1A register to 0
	TCCR1B = 0;  // same for TCCR1B
	TCNT1 = 0;   // initialize counter value to 0
	// set compare match register for 1 Hz increments
	OCR1A = 62499;  // = 16000000 / (256 * 1) - 1 (must be <65536)
	// turn on CTC mode
	TCCR1B |= (1 << WGM12);
	// Set CS12, CS11 and CS10 bits for 256 prescaler
	TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10);
	// enable timer compare interrupt
	TIMSK1 |= (1 << OCIE1A);

	sei();  // allow interrupts
}

#if DEBUG
int dbg_counter = 0;
void dbg_timer() {
	char buf[16];
	itoa(dbg_counter, buf, 10);
	Serial.println(buf);
	dbg_counter++;
}
#endif

// Timer1 interrupt
ISR(TIMER1_COMPA_vect) {
#if DEBUG
	dbg_timer();  // used for serial debug, disable when going live
#endif

	rpi_cooldown_update();
}

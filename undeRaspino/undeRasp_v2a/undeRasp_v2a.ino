// INCLUDES
#include <EEPROM.h>
#include <Wire.h>
#include "RTClib.h"
#include "defines.h"
#include "rpi.h"
#include "utils.h"

// GLOBAL VARS
char buffer[BUFFSIZE];  // general purpose global buffer
double i2c_val;		// return value for i2c operations

double user_interface(char *cmd_string) {
	int i;
	int dpow;
	char cmd = cmd_string[0];
	double retval = -1;
	switch (cmd) {
		case '?':  // print menu
			print_menu();
			retval = 1;
			break;
		case 'a':  // get ampere
			retval = get_ampere();
			break;
		case 'c':  // get temperature
			retval = get_temperature();
			break;
		case 'D':  // set eeprom date
			retval = set_eeprom_datetime(cmd_string, buffer);
			rpi_update_waketime();
			break;
		case 'd':  // read eeprom date and wake delay
			retval = get_eeprom_datetime(buffer);
			break;
		case 'E':  // clear last error
			reset_error();
			retval = 1;
			break;
		case 'e':  // read last error
			retval = get_last_error();
			break;
		case 'H':  // set heartbeat
			rpi_set_heartbeat(true);
			retval = 1;
			break;
		case 'h':  // get RPI heartbeat
			retval = rpi_get_heartbeat();
			break;
		case 'K':  // enable RPI/disable arduino serial output
			rpi_set_serial_enabled(true);
			retval = 1;
			break;
		case 'k':  // disable RPI/enable arduino serial output
			rpi_set_serial_enabled(false);
			retval = 1;
			break;
		case 'L':  // turn on mosfet
			set_mosfet_enabled(true);
			retval = 1;
			break;
		case 'l':  // turn off mosfet
			set_mosfet_enabled(false);
			retval = 0;
			break;
		case 'M':  // set operation mode
			if (strlen(cmd_string) < 2) break;
			retval = rpi_set_run_mode_s(&cmd_string[1], buffer);
			break;
		case 'm':  // get operation mode
			retval = rpi_get_run_mode();
			break;
		case 'O':  // set RPI keepalive
			rpi_set_keepalive(true);
			retval = 1;
			break;
		case 'o':  // unset RPI keepalive
			rpi_set_keepalive(false);
			retval = 0;
			break;
		case 'r':  // get RPI running status
			retval = rpi_is_running();
			break;
		case 'S':  // start RPI
			rpi_start();
			retval = 1;
			break;
		case 's':  // stop RPI
			rpi_stop();
			retval = 0;
			break;
		case 'T':  // set RTC datetime
			if (!rpi_is_running()) {
				retval = set_rtc_time_s(cmd_string, buffer);
			} else {
				sprintf(buffer, "RPI is running");
				retval = -2;
			}
			break;
		case 't':  // get RTC datime
			if (!rpi_is_running()) {
				retval = get_rtc_time_s(buffer);
			} else {
				sprintf(buffer, "RPI is running");
				retval = -2;
			}
			break;
		case 'v':  // get voltage
			retval = get_voltage();
			break;
		case 'w':  // get watts
			retval = get_watts();
			break;
		case 'z':  // get seconds left before starting raspberry
			retval = rpi_get_restart_time_left();
			break;
#if BB_DEBUG
		case '>':
			digitalWrite(DBG_PIN, 1);
			break;
		case '<':
			digitalWrite(DBG_PIN, 0);
			break;
#endif
		default:
			break;  // unknown command
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

void serialEvent() {
	char data = 0;
	int i = 0;
	double retval;
	delay(50);  // FIXME ugly hack
	while (Serial.available()) {
		data = Serial.read();
		if (i < BUFFSIZE - 1) buffer[i++] = data;
	}
	buffer[i] = '\0';
	Serial.print("CMD ");
	Serial.print(buffer);
	Serial.print(":");
	retval = user_interface(buffer);
	if (retval == -2)
		Serial.println(buffer);
	else
		Serial.println(retval);
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
	if (rpi_is_first_start()) {
		rpi_handle_checks();
	} else {
		rpi_handle_ops();
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
		set_error(I2C_ERRCODE, I2C_INIT_FAIL);
	} else {
		// Check RTC is working
		if (!RTC.isrunning()) {
			set_error(RTC_ERRCODE, RTC_INIT_FAIL);
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

	// Print help menu
	print_menu();

#if BB_DEBUG
	pinMode(DBG_PIN, OUTPUT);
	digitalWrite(DBG_PIN, 0);
	delay(5000); // wait first serial command?
#endif

	// Check initial RPI status
	rpi_started = rpi_has_power();

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

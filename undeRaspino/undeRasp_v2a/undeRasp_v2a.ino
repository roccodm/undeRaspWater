// INCLUDES
#include <EEPROM.h>
#include <Wire.h>
#include "RTClib.h"
#include "defines.h"
#include "rpi.h"
#include "utils.h"

// GLOBAL VARS
char serial_buf[BUFFSIZE];  // serial buffer
short serial_pos = -1;      // serial position (incremented at the beginning)

char i2c_buf[BUFFSIZE];  // i2c buffer
char i2c_out_buf[7];     // i2c output buffer

char out_buf[BUFFSIZE];  // output buffer

double user_interface(char *cmd_s) {
	int i;
	int dpow;
	char cmd = cmd_s[0];
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
			retval = set_eeprom_datetime(&cmd_s[1], out_buf);
			rpi_update_waketime();
			break;
		case 'd':  // read eeprom date and wake delay
			retval = get_eeprom_datetime(out_buf);
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
			rpi_set_serial(true);
			retval = 1;
			break;
		case 'k':  // disable RPI/enable arduino serial output
			rpi_set_serial(false);
			retval = 1;
			break;
		case 'L':  // turn on mosfet
			set_mosfet(true);
			retval = 1;
			break;
		case 'l':  // turn off mosfet
			set_mosfet(false);
			retval = 0;
			break;
		case 'M':  // set operation mode
			if (strlen(cmd_s) < 2) break;
			retval = rpi_set_run_mode_s(&cmd_s[1], out_buf);
			break;
		case 'm':  // get operation mode
			retval = rpi_get_run_mode();
			break;
		case 'O':  // set RPI manual mode
			rpi_set_manual(true);
			retval = 1;
			break;
		case 'o':  // unset RPI manual mode
			rpi_set_manual(false);
			retval = 0;
			break;
		case 'Q':
			rpi_set_halting(true);
			retval = 1;
			break;
		case 'q':
			rpi_set_halting(false);
			retval = 0;
			break;
		case 'R':
			if (!rpi_is_manual()) {
				sprintf(out_buf, "Not in manual mode");
				retval = -2;
			} else {
				rpi_setup();  // Reset RPI status
				retval = 1;
			}
			break;
		case 'r':  // get RPI running status
			retval = rpi_get_status();
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
			if (!rpi_has_power()) {
				retval = set_rtc_datetime_s(&cmd_s[1], out_buf);
			} else {
				sprintf(out_buf, "RPI is running");
				retval = -2;
			}
			break;
		case 't':  // get RTC datime
			if (!rpi_has_power()) {
				retval = get_rtc_datetime_s(out_buf);
			} else {
				sprintf(out_buf, "RPI is running");
				retval = -2;
			}
			break;
		case 'v':  // get voltage
			retval = get_voltage();
			break;
		case 'w':  // get watts
			retval = get_watts();
			break;
		case 'x':  // get cooldown timer value
			retval = rpi_get_cooldown();
			break;
		case 'z':  // get seconds left before starting raspberry
			retval = rpi_get_restart_time_left();
			break;
#if BB_DEBUG
		case '>':
			digitalWrite(DBG_PIN, 1);
			retval = 1;
			break;
		case '<':
			digitalWrite(DBG_PIN, 0);
			retval = 0;
			break;
#endif
		default:
			break;  // unknown command
	}
	return retval;
}

/*
 * Function serialEvent()
 * Called when new data is available on the serial
 * 1) assembles the command string
 * 2) calls the user_interface function
 * input is truncated at BUFF_SIZE length
 */

void serialEvent() {
	char data = 0;
	double retval;
	while (Serial.available()) {
		data = Serial.read();
		serial_pos += 1;
		if (serial_pos >= BUFFSIZE) {
			// message too long, wrapping!
			serial_pos = 0;
		}
		serial_buf[serial_pos] = data;
		if (serial_buf[serial_pos] == '\n') {
			serial_buf[serial_pos] = '\0';
			break;
		}
	}
	if (serial_buf[serial_pos] != '\0') {
		return;  // message is not over yet
	}
	serial_pos = -1;
	while (Serial.available()) {
		Serial.read();  // trash what's left
	}
	Serial.print("CMD ");
	Serial.print(serial_buf);
	Serial.print(":");
	retval = user_interface(serial_buf);
	if (retval == -2)
		Serial.println(out_buf);
	else
		Serial.println(retval);
}

void i2c_receive(int count) {
	char data = 0;
	int i = 0;
	double retval;
	while (Wire.available()) {
		data = Wire.read();
		if (i < BUFFSIZE - 1) i2c_buf[i++] = data;
	}
	i2c_buf[i] = '\0';
	retval = user_interface(i2c_buf);
	if (retval != -2) dtostrf(retval, 6, 3, i2c_out_buf);
}

void i2c_send() { Wire.write(i2c_out_buf); }

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
	rpi_set_manual(true);
#endif

	rpi_setup();

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
	dbg_timer();
#endif
	if (error_status) {
		return;  // Disable timers if an error happend
	}

	if (rpi_is_manual()) {
		return;  // Disable timers if in manual mode
	}
	rpi_timers_update();
}

// Main loop
void loop() {
	if (error_status) {
		return;  // Disable loop if an error happend
	}

	if (rpi_is_manual()) {
		return;  // Disable loop when in manual mode
	}

	rpi_handle_ops();
}

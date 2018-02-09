#include "utils.h"

void print_menu() {
	Serial.print(MENU1);
	Serial.print(MENU2);
	Serial.println(MENU3);
}

bool datoi(char *in, int offset, int size, char *data, char *err) {
	int i;
	if (strlen(in) != size) {
		if (err) sprintf(err, "too short");
		return false;
	}
	for (i = offset; i < size; i++) {
		if (in[i] < 48 || in[i] > 57) {
			if (err) sprintf(err, "invalid char");
			return false;
		}
		data[i - offset] = in[i] - 48;
	}
	return true;
}

void set_error(char errcode, const char *msg) {
	Serial.println(msg);
	error_status = true;
	EEPROM.write(EEPROM_ERR_LOCATION, errcode);
	digitalWrite(OK_LED_PIN, 0);
	digitalWrite(FAIL_LED_PIN, 1);  // Turn on red led
}

uint8_t get_last_error() { return EEPROM.read(EEPROM_ERR_LOCATION); }

void reset_error() {
	error_status = false;
	EEPROM.write(EEPROM_ERR_LOCATION, 0);
}

DateTime get_rtc_time() { return RTC.now(); }

/*
 * Function set_rct_time_s(string, err)
 * sets the rtc datetime to given value
 * string format: YYMMDDHHMMSS
 */

double set_rtc_time_s(char *in, char* err) {
	char data[20];
	if (!datoi(in, 1, 13, data, err)) return -2;  // error
	int year = *data * 10 + *(data + 1);
	int month = *(data + 2) * 10 + *(data + 3);
	int day = *(data + 4) * 10 + *(data + 5);
	int hour = *(data + 6) * 10 + *(data + 7);
	int minute = *(data + 8) * 10 + *(data + 9);
	int second = *(data + 10) * 10 + *(data + 11);
	RTC.adjust(DateTime(year + 2000, month, day, hour, minute, second));
	return 1;
}

double get_rtc_time_s(char *out) {
	DateTime now = RTC.now();
	sprintf(buffer, "%02d/%02d/%04d %02d.%02d.%02d",
		now.day(), now.month(), now.year(),
		now.hour(), now.minute(), now.second());
	return -2;
}

void set_mosfet_enabled(bool enabled) { digitalWrite(MOSFET_PIN, enabled); }

/*
 * Function get_eeprom_datetime()
 * read the eeprom datetime and timestep and print it to buf
 * out format: YYMMDDHHMMTTT where TTT is the timestep (max 250)
 */
double get_eeprom_datetime(char *out) {
	sprintf(out, "%02d%02d%02d%02d%02d%03d", EEPROM.read(1), EEPROM.read(2),
		EEPROM.read(3), EEPROM.read(4), EEPROM.read(5), EEPROM.read(6));
	// returns -2 that means the the return values are stored in buf
	return -2;
}

/*
 * Function set_eeprom_datetime(char *)
 * sets eeprom datetime and timestep to the given value
 * string format: YYMMDDHHMMTTT where TTT is the timestep (max 250)
 */
double set_eeprom_datetime(char *in, char *out) {
	char data[20];
	if (!datoi(in, 1, 14, data, out)) return -2;  // error

	int year = *data * 10 + *(data + 1);
	int month = *(data + 2) * 10 + *(data + 3);
	int day = *(data + 4) * 10 + *(data + 5);
	int hour = *(data + 6) * 10 + *(data + 7);
	int minute = *(data + 8) * 10 + *(data + 9);
	int timestep = *(data + 10) * 100 + *(data + 11) * 10 + *(data + 12);
	if (year < 17 || month <= 0 || month > 12 || day <= 0 || day > 31 ||
	    hour >= 24 || minute >= 60 || timestep <= 0 || timestep > 250) {
		sprintf(out, "invalid int");
		return -2;  // Problem in conversion
	}
	EEPROM.write(1, year);
	EEPROM.write(2, month);
	EEPROM.write(3, day);
	EEPROM.write(4, hour);
	EEPROM.write(5, minute);
	EEPROM.write(6, timestep);
	sprintf(out, "ok");
	return -2;
}

/*
 * Function get_pin_median(pinNumber, readDelay)
 *
 * Sometimes the adc could read dirty values. To avoid wrong values in critical
 * operations
 * more values are read and stored in an ordered array of odd number of
 * elements.
 * Sorting the array, the bad value will go in a boundary and, returning the
 * median value
 * (the central element in the array), it's highly probable to get a good value.
 *
 * The InsertionSort algorithm has been used to sort the values, that is pretty
 * fast (such
 * as the quicksort algorithm!!!) for small array (under 10 element, optimus is
 * 7).
 *
 * Input:
 *   PinNumber: analog pin to be read
 *   Delay: optional delay between read
 * Output:
 *   The median value of the array of seven reads (integer)
 */

int get_pin_median(int pinNumber, int readDelay) {
	int value, v[7];
	for (int n = 0; n < 7; n++) {
		int i = 0;
		value = analogRead(pinNumber);
		while (i < n and value <= v[i]) i++;
		for (int j = n - 1; j >= i; j--) v[j + 1] = v[j];
		v[i] = value;
		delay(readDelay);
	}
	return v[3];
}

double get_voltage() {
	return (get_pin_median(VOLTAGE_PIN) * VCC / 1024) / R_ALPHA;
}

double get_ampere() {
	return ((get_pin_median(AMPERE_PIN) * VCC / 1024) - 2.5) / 0.185;
}

double get_watts() { return get_voltage() * get_ampere(); }

double get_temperature() {
	unsigned int wADC;
	double t;
	ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
	ADCSRA |= _BV(ADEN);  // enable the ADC
	delay(50);	    // wait for voltages to become stable.
	ADCSRA |= _BV(ADSC);  // Start the ADC
	while (bit_is_set(ADCSRA, ADSC))
		;
	wADC = ADCW;
	t = (wADC - 324.31) / 1.22;
	return abs(t);
}

#include "utils.h"

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

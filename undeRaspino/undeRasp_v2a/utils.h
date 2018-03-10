#ifndef UTILS_H
#define UTILS_H

#include "defines.h"

// simple atoi function
bool atoi(char *in, int *out, char *err);
// simple atoi function for dates (operate on each byte)
bool atod(char *in, char *data, char *err);

void print_menu();
bool has_error();
void set_error(uint8_t error, const char *msg);
uint8_t get_last_error();
void reset_error();

void set_mosfet(bool enabled);
int get_pin_median(int pin, int delay = 0);
double get_voltage();
double get_ampere();
double get_watts();
double get_temperature();
time_t get_eeprom_timestamp();
double get_eeprom_datetime(char *out);
double set_eeprom_datetime(char *in, char *out);
DateTime get_rtc_datetime();
bool sync_time();
time_t get_internal_time();

double set_rtc_datetime_s(char *in, char *err);
double get_rtc_datetime_s(char *err);
double get_internal_datetime_s(char *err);
double set_internal_datetime_s(char *in, char *err);
void update_internal_clock();

void set_led_status(unsigned short int mode);
void update_samples();
void update_led_timer();

void adc_setup();

extern RTC_DS1307 RTC;
#endif

#ifndef UTILS_H
#define UTILS_H

#include "defines.h"

// simple atoi function
bool atoi(char *in, int *out, char *err);
// simple atoi function for dates (operate on each byte)
bool atod(char *in, char *data, char *err);

void print_menu();
void set_error(uint8_t error);
uint8_t get_last_error();
void reset_error();

void set_mosfet_enabled(bool enabled);
int get_pin_median(int pin, int delay = 0);
double get_voltage();
double get_ampere();
double get_watts();
double get_temperature();
double get_eeprom_date();
double set_eeprom_date(char *buf);
DateTime get_rtc_datetime();
double set_rtc_datetime_s(char *in, char *err);
double get_rtc_datetime_s(char *err);

bool error_status = false; // flag is set in case of fatal errors
RTC_DS1307 RTC;
#endif

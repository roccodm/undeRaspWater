#include "utils.h"

#define SAMPLES_SIZE 21

char prog_buf[100]; // initialize program buffer (from defines.h)
RTC_DS1307 RTC;     // initialize RTC

bool error_status = false; // flag is set in case of fatal errors
time_t curr_time = RTC_MIN_DATE;
unsigned short int led_mode = 0;
unsigned int led_timer = 0;
int voltage_samples[SAMPLES_SIZE];
int ampere_samples[SAMPLES_SIZE];
int samples_pos = 0;
bool is_reading_temp = false;

bool atoi(char *in, int *out, char *err) {
   int i;
   int dpow = 1;
   *out = 0;
   for (i = strlen(in) - 1; i >= 0; i--) {
      if (in[i] < 48 or in[i] > 57)
         return false;
      (*out) += (in[i] - 48) * dpow;
      dpow *= 10;
   }
   return true;
}

bool atod(char *in, char *data, char *err) {
   int i;
   for (i = 0; i < strlen(in); i++) {
      if (in[i] < 48 || in[i] > 57) {
         if (err)
            sprintf(err, MSG_INVALID_INT);
         return false;
      }
      data[i] = in[i] - 48;
   }
   return true;
}

void set_error(uint8_t errcode, const char *msg) {
#if DEBUG
   Serial.println(msg);
#endif
   error_status = true;
   EEPROM.write(EEPROM_ERROR, errcode);
   set_led_status(LED_ERROR);
}

bool has_error() { return error_status; }

uint8_t get_last_error() { return EEPROM.read(EEPROM_ERROR); }

void reset_error() {
   error_status = false;
   EEPROM.write(EEPROM_ERROR, 0);
   set_led_status(LED_OFF);
}

void print_menu() {
   Serial.println(PS("?\tprint this menu"));
   Serial.println(PS("a\tpower used(A)"));
   Serial.println(PS("c\ttemperature(C)"));
   Serial.println(
       PS("D/d\twrite/read EEPROM datetime and delay (YYMMDDHHMMTTT)"));
   Serial.println(PS("E/e\tclear/read last error in EEPROM"));
   Serial.println(PS("H/h\tset/read heartbeat"));
   Serial.println(PS("P/p\tenable/disable RB serial out"));
   Serial.println(PS("L/l\tenable/disable mosfet"));
   Serial.println(PS("M/m\tset/read current RPI working mode"));
   Serial.println(PS("O/o\tset/unset manual operations mode"));
   Serial.println(PS("Q/q\tset/unset RPI shutdown request"));
   Serial.println(PS("r\tget RPI running status"));
   Serial.println(PS("S/s\tstart/stop RPI"));
   Serial.println(PS("T/t\tset/read RTC datetime (YYMMDDHHMMSS)"));
   Serial.println(PS("v\tread current voltage"));
   Serial.println(PS("w\tread current watts"));
   Serial.println(PS("z\tSeconds left before starting RPI"));
#if BB_DEBUG
   Serial.println(PS("</>\tSend low/high on breadbord debug pin (D2)"));
#endif
}

DateTime get_rtc_time() { return RTC.now(); }

bool sync_time() {
   uint32_t rtc_time = RTC.now().unixtime();
   if (rtc_time < RTC_MIN_DATE)
      return false;

   curr_time = rtc_time;
   return true;
}

time_t get_internal_time() { return curr_time; }

/*
 * Function set_rct_datetime_s(string, err)
 * sets the rtc datetime to given value
 * string format: YYMMDDHHMMSS
 */

double set_rtc_datetime_s(char *in, char *err) {
   char data[20];
   if (strlen(in) != 12) {
      if (err)
         sprintf(err, MSG_TOO_SHORT);
      return -2;
   }
   if (!atod(in, data, err))
      return -2; // error
   int year = *data * 10 + *(data + 1);
   int month = *(data + 2) * 10 + *(data + 3);
   int day = *(data + 4) * 10 + *(data + 5);
   int hour = *(data + 6) * 10 + *(data + 7);
   int minute = *(data + 8) * 10 + *(data + 9);
   int second = *(data + 10) * 10 + *(data + 11);
   RTC.adjust(DateTime(year + 2000, month, day, hour, minute, second));
   sync_time();
   return 1;
}

double set_internal_datetime_s(char *in, char *err) {
   char data[20];
   if (strlen(in) != 12) {
      if (err)
         sprintf(err, MSG_TOO_SHORT);
      return -2;
   }
   if (!atod(in, data, err))
      return -2; // error
   int year = *data * 10 + *(data + 1);
   int month = *(data + 2) * 10 + *(data + 3);
   int day = *(data + 4) * 10 + *(data + 5);
   int hour = *(data + 6) * 10 + *(data + 7);
   int minute = *(data + 8) * 10 + *(data + 9);
   int second = *(data + 10) * 10 + *(data + 11);
   curr_time =
       DateTime(year + 2000, month, day, hour, minute, second).unixtime();
   return 1;
}

double get_rtc_datetime_s(char *out) {
   DateTime now = RTC.now();
   sprintf(out, "%02d/%02d/%04d %02d.%02d.%02d", now.day(), now.month(),
           now.year(), now.hour(), now.minute(), now.second());
   return -2;
}

double get_internal_datetime_s(char *out) {
   DateTime now = DateTime(curr_time);
   sprintf(out, "%02d/%02d/%04d %02d.%02d.%02d", now.day(), now.month(),
           now.year(), now.hour(), now.minute(), now.second());
   return -2;
}

void set_mosfet(bool enabled) { digitalWrite(MOSFET_PIN, enabled); }

time_t get_eeprom_timestamp() {
   return DateTime(EEPROM.read(EEPROM_YEAR) + 2000, EEPROM.read(EEPROM_MONTH),
                   EEPROM.read(EEPROM_DAY), EEPROM.read(EEPROM_HOUR),
                   EEPROM.read(EEPROM_MINUTE))
       .unixtime();
}
/*
 * Function get_eeprom_datetime()
 * read the eeprom datetime and timestep and print it to buf
 * out format: YYMMDDHHMMTTT where TTT is the timestep (max 250)
 */
double get_eeprom_datetime(char *out) {
   sprintf(out, "%02d%02d%02d%02d%02d%03d", EEPROM.read(EEPROM_YEAR),
           EEPROM.read(EEPROM_MONTH), EEPROM.read(EEPROM_DAY),
           EEPROM.read(EEPROM_HOUR), EEPROM.read(EEPROM_MINUTE),
           EEPROM.read(EEPROM_STEP));
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

   if (strlen(in) != 13) {
      if (out)
         sprintf(out, MSG_TOO_SHORT);
      return -2;
   }
   if (!atod(in, data, out))
      return -2; // error

   int year = *data * 10 + *(data + 1);
   int month = *(data + 2) * 10 + *(data + 3);
   int day = *(data + 4) * 10 + *(data + 5);
   int hour = *(data + 6) * 10 + *(data + 7);
   int minute = *(data + 8) * 10 + *(data + 9);
   int timestep = *(data + 10) * 100 + *(data + 11) * 10 + *(data + 12);
   if (year < 17 || month <= 0 || month > 12 || day <= 0 || day > 31 ||
       hour >= 24 || minute >= 60 || timestep <= 0 || timestep > 250) {
      sprintf(out, MSG_INVALID_INT);
      return -2; // Problem in conversion
   }
   EEPROM.write(EEPROM_YEAR, year);
   EEPROM.write(EEPROM_MONTH, month);
   EEPROM.write(EEPROM_DAY, day);
   EEPROM.write(EEPROM_HOUR, hour);
   EEPROM.write(EEPROM_MINUTE, minute);
   EEPROM.write(EEPROM_STEP, timestep);
   sprintf(out, MSG_OK);
   return -2;
}

/*
 * Function get_pin_median(pinNumber, readDelay)
 *
 * Sometimes the adc could read dirty values.
 * To avoid wrong values in critical operations, samples are taken at fixed
 * steps and stored in an array of odd number of elements.
 *
 * Sorting the array, the bad value will go in a boundary and, returning the
 * median value (the central element in the array), it's highly probable to
 * get a good value.
 *
 * The InsertionSort algorithm has been used to sort the values, that is pretty
 * fast for small array (under 10 element, optimus is 7).
 *
 * Input:
 *   samples: the samples array
 * Output:
 *   The median value of the samples array (integer)
 */
int get_pin_median(int *samples) {
   int value, v[SAMPLES_SIZE];
   for (int n = 0; n < SAMPLES_SIZE; n++) {
      int i = 0;
      value = samples[n];
      while (i < n and value <= v[i])
         i++;
      for (int j = n - 1; j >= i; j--)
         v[j + 1] = v[j];
      v[i] = value;
   }
   return v[(SAMPLES_SIZE - 1) / 2];
}

double get_voltage() {
   return (get_pin_median(voltage_samples) * VCC / 1024) / R_ALPHA;
}

double get_ampere() {
   return ((get_pin_median(ampere_samples) * VCC / 1024) - 2.5) / 0.185;
}

double get_watts() { return get_voltage() * get_ampere(); }

double get_temperature() {
   is_reading_temp = true;
   unsigned int wADC;
   double t;
   ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
   ADCSRA |= _BV(ADEN); // enable the ADC
   delay(50);           // wait for voltages to become stable.
   ADCSRA |= _BV(ADSC); // Start the ADC
   while (bit_is_set(ADCSRA, ADSC))
      ;
   wADC = ADCW;
   t = (wADC - 324.31) / 1.22;
   is_reading_temp = false;
   return abs(t);
}

void update_samples() {
   if (is_reading_temp)
      return; // we cannot perform analogReads when reading temperature
   voltage_samples[samples_pos] = analogRead(VOLTAGE_PIN);
   ampere_samples[samples_pos] = analogRead(AMPERE_PIN);
   samples_pos += 1;
   if (samples_pos >= SAMPLES_SIZE) {
      samples_pos = 0;
   }
}

void update_internal_clock() { curr_time += 1; }

void set_led_status(unsigned short int mode) { led_mode = mode; }

void update_led_timer() {
   led_timer += 1;
   if (led_timer > 999)
      led_timer = 0;

   switch (led_mode) {
   case LED_OFF:
      digitalWrite(OK_LED_PIN, 0);
      digitalWrite(FAIL_LED_PIN, 0);
      break;
   case LED_CHECKING:
      digitalWrite(FAIL_LED_PIN, 0);
      if (led_timer < 500) {
         digitalWrite(OK_LED_PIN, 1);

      } else {
         digitalWrite(OK_LED_PIN, 0);
      }
      break;
   case LED_OK:
      digitalWrite(OK_LED_PIN, 1);
      digitalWrite(FAIL_LED_PIN, 0);
      break;
   case LED_WARNING:
      if (led_timer % 2 == 0) {
         digitalWrite(OK_LED_PIN, 1);
         digitalWrite(FAIL_LED_PIN, 0);
      } else {
         digitalWrite(OK_LED_PIN, 0);
         digitalWrite(FAIL_LED_PIN, 1);
      }
      break;
   case LED_ERROR:
      digitalWrite(OK_LED_PIN, 0);
      digitalWrite(FAIL_LED_PIN, 1);
      break;
   default:
      break;
   }
}

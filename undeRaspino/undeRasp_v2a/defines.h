#ifndef DEFINES_H
#define DEFINES_H

#include "Arduino.h"
#include "RTClib.h"
#include <EEPROM.h>
#include <Wire.h>

typedef uint32_t time_t;

#define DEBUG 1

#if DEBUG
#define BB_DEBUG 1

#if BB_DEBUG
#define DBG_PIN 2
#endif

#endif

// Function to use program memory for strings
#define PS(str) (strcpy_P(prog_buf, PSTR(str)), prog_buf)
extern char prog_buf[100]; // initialized in utils.h

// LED States
#define LED_OFF 0
#define LED_CHECKING 1
#define LED_OK 2
#define LED_WARNING 3
#define LED_ERROR 4

// EEPROM locations
#define EEPROM_YEAR 1
#define EEPROM_MONTH 2
#define EEPROM_DAY 3
#define EEPROM_HOUR 4
#define EEPROM_MINUTE 5
#define EEPROM_STEP 6
#define EEPROM_ERROR 7
#define EEPROM_MODE 8
#define EEPROM_APP_ERROR 9

// Messages
#define MSG_START PS("\n\nUnderRaspino Ready.")
#define MSG_I2C_FAIL PS("FATAL: i2c bus error")
#define MSG_I2C_BUSY PS("FATAL: i2c bus is busy (rpi is running)")
#define MSG_RTC_FAIL PS("FATAL: RTC error")
#define MSG_RTC_INVALID_DATE PS("FATAL: RTC initial date is invalid")
#define MSG_VOLTAGE_LOW PS("Battery too low to start RPI")
#define MSG_VOLTAGE_CRITICAL PS("Battery level critical. Entering safe mode")

#define MSG_TOO_SHORT PS("too short")
#define MSG_INVALID_INT PS("invalid int")
#define MSG_OK PS("ok")

#define MSG_RPI_START PS("Starting Raspberry")
#define MSG_RPI_STOP PS("Stopping Raspberry")

#define MSG_RPI_NO_POWER PS("RPI is unpowered")
#define MSG_RPI_NO_RESPONSE PS("RPI is unresponsive")
#define MSG_RPI_NO_BOOT PS("RPI failed to boot")

// ERR CODES
#define ERR_I2C_FAIL 11
#define ERR_I2C_BUSY 12

#define ERR_RTC_FAIL 21
#define ERR_RTC_INVALID_DATE 22

#define ERR_VOLTAGE_LOW 31
#define ERR_VOLTAGE_CRITICAL 32

#define RPI_ERR_UNPOWERED 41    // RPI was powered on, but looks unpowered
#define RPI_ERR_BOOT_FAILED 42  // RPI has power, but looks unable to boot
#define RPI_ERR_UNRESPONSIVE 43 // RPI booted, but seems unresponsive now

// CONFIG
#define VCC 3.3
#define MOSFET_PIN 4
#define RASPBERRY_STATUS_PIN 5
#define FAIL_LED_PIN 6
#define OK_LED_PIN 7
#define RELAY_RESET_PIN 8
#define RELAY_SET_PIN 9
#define SERIAL_ARDUINO_PIN 12
#define SERIAL_RASPBERRY_PIN 13
#define BAUD_RATE 9600
#define VOLTAGE_PIN A0
#define AMPERE_PIN A1
#define I2C_ADDRESS 0x04
#define I2C_SDA A4
#define I2C_SCL A5
#define RTC_MIN_DATE 1483228800 // 1/1/2017 0:0:0
#define BUFFSIZE 21
#define R_ALPHA 0.152      // for V read purpose, R partitor coeff.
#define VOLTAGE_LOW 9.5    // minimum operation voltage
#define VOLTAGE_CRITICAL 9 // critical voltage

#endif

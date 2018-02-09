#define DEBUG 1

// Messages
#define START_MSG "\n\nUnderRaspino Ready."
#define I2C_INIT_FAIL "FATAL: i2c bus error"
#define RTC_INIT_FAIL "FATAL: RTC error"
#define LOWBAT "Low battery to start RB"
#define RASP_FAIL "Unable to powerup RB"
#define RASP_NO_HB "Unable to receive Heartbeat from RB"
#define RASP_STARTING "Starting RB"
#define RASP_STOP "RB stopped"
#define MENU1                                                             \
	"\nA\tpower used(A)\nC\tARD temperature\nd/D\tread/write EEPROM " \
	"datestam (YYMMDDHHMMXXX)\nE/e\tclear/read last error in eprom"
#define MENU2                                                                \
	"\nH/h\tset/read heartbeat\nK/k\tenable/disable RB serial "          \
	"out\nL/l\tTurn on/off mosfet\nM\tSecs left to start "               \
	"RB\nO/o\tset/read keep on RB (use q to quit)\nP/p\tenable/disable " \
	"ARD serial out\nQ\tQuit RB"
#define MENU3                                                                  \
	"\nR\tRB running status\nS/s\tstart/stop RB relay\nT/t\tSet/read RTC " \
	"(YYMMDDHHMMSS)\nV\tread voltage\nW\tread watt\n?\tPrint this menu"

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
#define RTC_MIN_DATE 1483228800  // 1/1/2017 0:0:0
#define BUFFSIZE 21
#define R_ALPHA 0.152	  // for V read purpose, R partitor coeff.
#define MIN_BATTERY_VOLTAGE 9  // minimum voltage to start raspberry in safe

#define EEPROM_ERR_LOCATION 7

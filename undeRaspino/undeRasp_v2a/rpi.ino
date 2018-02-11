#include <EEPROM.h>
#include "defines.h"
#include "rpi.h"
#include "utils.h"

/*
 * Function rpi_update_waketime()
 *
 * Sets the restart timeout after which we want to turn RPI back on.
 */
void rpi_update_waketime() {
	rpi_restart_timer = 0;
	rpi_restart_delay = EEPROM.read(6) * 60;
}

int rpi_get_restart_time_left() {
	if(rpi_started) return -1;
	return rpi_restart_delay - rpi_restart_timer;
}

void rpi_set_serial_enabled(bool enabled) {
	if (enabled) {
		digitalWrite(SERIAL_ARDUINO_PIN, 0);
		digitalWrite(SERIAL_RASPBERRY_PIN, 1);
	} else {
		digitalWrite(SERIAL_ARDUINO_PIN, 1);
		digitalWrite(SERIAL_RASPBERRY_PIN, 0);
	}
}

uint8_t rpi_set_run_mode_s(char *in, char *out) {
	int dpow = 1;
	int val;
	if (!atoi(in, &val, out) || val > 127)
		return -1;
	EEPROM.write(EEPROM_MODE_LOCATION, val);
	return val;
};

uint8_t rpi_get_run_mode() {
	uint8_t ret = EEPROM.read(EEPROM_MODE_LOCATION);
	if (rpi_is_first_start()) ret |= 0x80;
	return ret;
}

void rpi_set_heartbeat(bool on) {
	rpi_heartbeat = on;
	rpi_heartbeat_cooldown = 0;
	rpi_cooldown = RPI_START_COOLDOWN;
}

bool rpi_get_heartbeat() { return rpi_heartbeat; }

void rpi_set_keepalive(bool on) { rpi_keepalive = on; }

bool rpi_has_power() { return digitalRead(RASPBERRY_STATUS_PIN) == 1; }

bool rpi_is_running() {
	return rpi_has_power() &&
	       (rpi_cooldown < RPI_START_COOLDOWN || rpi_get_heartbeat());
}

bool rpi_is_first_start() { return first_start; }

void rpi_handle_checks() {
	// first run, rpi will do self test and shutdown
	if (!rpi_started) {
		rpi_start();
	} else if (!rpi_is_running()) {
		if (rpi_keepalive) return;
		// rpi shutdown?! enter normal operation mode
		first_start = false;
		rpi_stop();
	}
}

void rpi_handle_ops() {
	// start RPI if rpi_restart_timer has passed
	if (!rpi_is_running() && rpi_restart_delay > 0 &&
	    rpi_restart_timer >= rpi_restart_delay) {
		rpi_restart_timer = 0;
		rpi_start();
	}
}

void rpi_start() {
#if DEBUG
	Serial.println("Starting raspberry");
#if BB_DEBUG
	digitalWrite(DBG_PIN, 1);
#endif
#endif
	digitalWrite(RELAY_SET_PIN, 1);
	delay(100);
	digitalWrite(RELAY_SET_PIN, 0);
	rpi_started = true;
	rpi_cooldown = 0;
}

void rpi_stop() {
	if (rpi_keepalive) return;
#if DEBUG
	Serial.println("Stopping raspberry");
#if BB_DEBUG
	digitalWrite(DBG_PIN, 0);
#endif
#endif
	digitalWrite(RELAY_RESET_PIN, 1);
	delay(100);
	digitalWrite(RELAY_RESET_PIN, 0);
	rpi_started = false;
	rpi_cooldown = 0;
}

void rpi_cooldown_update() {
	// rpi start/stop cooldown
	if (rpi_started && rpi_cooldown < RPI_START_COOLDOWN) {
		rpi_cooldown += 1;
	} else if (!rpi_started && rpi_cooldown < RPI_STOP_COOLDOWN) {
		rpi_cooldown += 1;
	}

	// heartbeat reset function
	if (rpi_get_heartbeat()) {
		rpi_heartbeat_cooldown += 1;
		if (rpi_heartbeat_cooldown >= RPI_HEARTBEAT_RESET_TIME) {
			rpi_heartbeat_cooldown -= RPI_HEARTBEAT_RESET_TIME;
			rpi_heartbeat = false;
		}
	}

	// restart timer.
	// after a shutdown will restart RPI after rpi_restart_delay
	if (!rpi_started && rpi_restart_delay > 0) {
		rpi_restart_timer += 1;
	}
}

#include "defines.h"
#include "rpi.h"
#include <EEPROM.h>

void rpi_set_run_mode(uint8_t mode) {
	EEPROM.write(EEPROM_MODE_LOCATION, mode);
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
	return rpi_has_power() && (rpi_cooldown < RPI_START_COOLDOWN || rpi_get_heartbeat());
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

void rpi_handle_ops() {}

void rpi_start() {
#if DEBUG
	Serial.println("Starting raspberry");
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
	if (rpi_get_heartbeat()) {
		rpi_heartbeat_cooldown += 1;
		if (rpi_heartbeat_cooldown >= RPI_HEARTBEAT_RESET_TIME) {
			rpi_heartbeat_cooldown -= RPI_HEARTBEAT_RESET_TIME;
			rpi_heartbeat = false;
		}
	}
}

#include "defines.h"
#include "rpi.h"

void rpi_set_keepalive(bool on) { rpi_keepalive = on; }

bool rpi_is_running() { return digitalRead(RASPBERRY_STATUS_PIN) == 1; }

bool rpi_is_first_start() { return first_start; }

void rpi_handle_checks() {
	// first run, rpi will do self test and shutdown
	if (!rpi_started) {
		start_rpi();
	} else if (!rpi_is_running() && rpi_cooldown > RPI_START_COOLDOWN) {
		if (rpi_keepalive) return;
		// rpi shutdown?! enter normal operation mode
		first_start = false;
		stop_rpi();
	}
}

void rpi_handle_ops() {}

void start_rpi() {
#if DEBUG
	Serial.println("Starting raspberry");
#endif
	digitalWrite(RELAY_SET_PIN, 1);
	delay(100);
	digitalWrite(RELAY_SET_PIN, 0);
	rpi_started = true;
	rpi_cooldown = 0;
}

void stop_rpi() {
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
}

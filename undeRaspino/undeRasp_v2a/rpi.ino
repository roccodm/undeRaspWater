#include "defines.h"
#include "rpi.h"

bool is_rpi_running() {
	return digitalRead(RASPBERRY_STATUS_PIN) == 1;
}

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

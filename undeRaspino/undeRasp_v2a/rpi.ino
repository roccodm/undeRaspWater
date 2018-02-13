#include "defines.h"
#include "rpi.h"
#include "utils.h"
#include <EEPROM.h>

//
// RPI global vars, should not get exposed.
//
bool rpi_first = true;   // true when first start, used to run self-checks
bool rpi_manual = false; // set true to prevent RPI shutdown

// RPI status (could use bitfield)
bool rpi_started = false;   // If RPI was started
bool rpi_heartbeat = false; // If RPI is alive
bool rpi_halting = false;   // If RPI requested shutdown
bool rpi_booted = false;    // If RPI sent the first heartbeat
int rpi_cooldown = 0;       // Cooldown used for start/stop operations
int rpi_checks_result = 0;  // Result from RPI checks on first start

// Restarting
int rpi_restart_delay = 0; // The delay after which restart the RPI
int rpi_restart_timer = 0; // The timer to accumulate restart delay

void rpi_setup() {
   // Reset all variables (except manual mode)
   rpi_first = true;
   rpi_started = false;
   rpi_heartbeat = false;
   rpi_halting = false;
   rpi_booted = false;
   rpi_cooldown = 0;
   rpi_restart_delay = 0;
   rpi_restart_timer = 0;

   // Check initial RPI status
   rpi_started = rpi_has_power();
   rpi_update_waketime();
}

/*
 * Function rpi_update_waketime()
 *
 * Sets the restart timeout after which we want to turn RPI back on.
 */
void rpi_update_waketime() {
   rpi_restart_timer = 0;
   rpi_restart_delay = EEPROM.read(6) * 60;
}

int rpi_get_cooldown() { return rpi_cooldown; }

int rpi_get_restart_time_left() {
   if (rpi_started)
      return -1;
   return rpi_restart_delay - rpi_restart_timer;
}

void rpi_set_serial(bool enabled) {
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
   if (rpi_first)
      ret |= 0x80;
   return ret;
}

void rpi_set_heartbeat(bool on) {
   rpi_heartbeat = on;
   rpi_cooldown = 0;
   rpi_booted = true;
}

void rpi_set_halting(bool enabled) {
   if (enabled)
      rpi_cooldown = 0;
   rpi_halting = enabled;
}

bool rpi_get_heartbeat() { return rpi_heartbeat; }

void rpi_set_manual(bool on) { rpi_manual = on; }
bool rpi_is_manual() { return rpi_manual; }

bool rpi_has_power() { return digitalRead(RASPBERRY_STATUS_PIN) == 1; }

bool rpi_get_status() {
   int status = 0;
   if (rpi_has_power())
      status += 1;
   if (rpi_booted)
      status += 2;
   if (rpi_heartbeat)
      status += 4;
   if (rpi_halting)
      status += 8;
   return status;
}

int rpi_set_checks_result(char *in, char *err) {
   rpi_checks_result = 0;
   if (strlen(in) < 1)
      return 0;
   if (!atoi(in, &rpi_checks_result, err)) {
      rpi_checks_result = 1;
      return -2;
   }
   return rpi_checks_result;
}

int rpi_get_checks_result() { return rpi_checks_result; }

int rpi_get_checks_result();

void rpi_handle_ops() {
   // Check if the RPI was started
   if (!rpi_started) {
      if (rpi_first) {
         // On first run, immediately start
         rpi_start();
      } else if (rpi_restart_timer >= rpi_restart_delay) {
         // It's time to start the RPI.
         rpi_restart_timer = 0;
         rpi_start();
      }
      // All good.
      return;
   }

   if (rpi_halting) {
      // RPI asked to shutdown
      if (rpi_cooldown >= RPI_STOP_COOLDOWN) {
         // It's time to shut it down as requested. All good.
         rpi_stop();
      }
      // All good.
      return;
   }

   if (rpi_get_heartbeat()) {
      // RPI was started, is not halting, and looks alive. All good.
      return;
   }

   if (!rpi_has_power() && rpi_cooldown > 0) {
      // RPI was started, did not request a shutdown, but looks dead,
      // and is unpowered. Setting error.
      set_error(RPI_ERR_UNPOWERED, MSG_RPI_NO_POWER);
      return;
   }

   if (rpi_booted) {
      // RPI was started, did not request shutdown, is powered, and
      // booted properly but looks dead. Setting error.
      set_error(RPI_ERR_UNRESPONSIVE, MSG_RPI_NO_RESPONSE);
      return;
   }

   if (rpi_cooldown >= RPI_START_COOLDOWN) {
      // RPI was started, did not request shutdown, is powered, looks
      // dead, and the boot timer has expired. Setting error.
      set_error(RPI_ERR_BOOT_FAILED, MSG_RPI_NO_BOOT);
      return;
   }

   // If we made it this far, it means it's all going according to plan.
}

void rpi_start() {
#if BB_DEBUG
#else
   if (get_voltage() <= VOLTAGE_LOW) {
      set_error(ERR_VOLTAGE_LOW, MSG_VOLTAGE_LOW);
      return;
   }
#endif
#if DEBUG
   Serial.println(MSG_RPI_START);
#if BB_DEBUG
   digitalWrite(DBG_PIN, 1);
#endif
#endif
   digitalWrite(RELAY_SET_PIN, 1);
   delay(100);
   digitalWrite(RELAY_SET_PIN, 0);
   rpi_started = true;  // If the RPI was started
   rpi_halting = false; // If RPI requested shutdown
   rpi_booted = false;  // If RPI sent the first heartbeat
   rpi_cooldown = 0;    // The cooldown used for start/stop operations
}

void rpi_stop() {
#if DEBUG
   Serial.println(MSG_RPI_STOP);
#if BB_DEBUG
   digitalWrite(DBG_PIN, 0);
#endif
#endif
   digitalWrite(RELAY_RESET_PIN, 1);
   delay(100);
   digitalWrite(RELAY_RESET_PIN, 0);
   rpi_first = false;
   rpi_started = false;
   rpi_cooldown = 0;
}

void rpi_timers_update() {
   if (rpi_started) {
      if (!rpi_halting && rpi_booted) {
         // Heartbeat reset
         rpi_cooldown += 1;
         if (rpi_cooldown >= RPI_HEARTBEAT_RESET_TIME) {
            rpi_cooldown = 0;
            rpi_heartbeat = false;
         }
      } else if (rpi_halting && rpi_cooldown < RPI_STOP_COOLDOWN) {
         // RPI halting
         rpi_cooldown += 1;
      } else if (!rpi_booted && rpi_cooldown < RPI_START_COOLDOWN) {
         // RPI starting
         rpi_cooldown += 1;
      }
   } else {
      // will restart RPI after rpi_restart_delay
      if (rpi_restart_timer < rpi_restart_delay) {
         rpi_restart_timer += 1;
      }
   }
}

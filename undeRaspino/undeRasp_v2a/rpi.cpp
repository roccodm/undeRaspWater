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
bool rpi_halting = false;   // If RPI requested shutdown
bool rpi_booted = false;    // If RPI sent the first heartbeat
int rpi_checks_result = 0;  // Result from RPI checks on first start

uint32_t rpi_boot_cd = 0;
uint32_t rpi_hb_cd = 0;
uint32_t rpi_halt_cd = 0;

// Restarting
time_t rpi_restart_time = 0; // The time at which to start RPI

void rpi_reset_cds() {
   rpi_hb_cd = 0;
   rpi_halt_cd = 0;
   rpi_boot_cd = 0;
}

void rpi_setup() {
   // Reset all variables (except manual mode)
   rpi_first = true;
   rpi_started = false;
   rpi_halting = false;
   rpi_booted = false;
   rpi_boot_cd = 0;
   rpi_reset_cds();

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
   time_t ets = get_eeprom_timestamp();
   time_t curr = get_internal_time();
   time_t step = EEPROM.read(EEPROM_STEP) * 60;
   if (step <= 0)
      step = 60;
   if (curr > ets) {
      rpi_restart_time = curr + (step - ((curr - ets) % step));
   } else {
      rpi_restart_time = ets;
   }
}

int32_t rpi_get_restart_time_left() {
   return rpi_restart_time - get_internal_time();
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
   EEPROM.write(EEPROM_MODE, val);
   return val;
};

uint8_t rpi_get_run_mode() {
   uint8_t ret = EEPROM.read(EEPROM_MODE);
   if (rpi_first)
      ret |= 0x80;
   return ret;
}

void rpi_set_booted() {
   rpi_reset_cds();
   rpi_booted = true;
}

void rpi_set_halting(bool enabled) {
   rpi_reset_cds();
   rpi_halting = enabled;
#if DEBUG
   Serial.println("RPI wants to quit!");
#endif
}

void rpi_set_manual(bool on) { rpi_manual = on; }
bool rpi_is_manual() { return rpi_manual; }

bool rpi_has_power() { return digitalRead(RASPBERRY_STATUS_PIN) == 1; }

bool rpi_get_status() {
   int status = 0;
   if (rpi_has_power())
      status += 1;
   if (rpi_booted)
      status += 2;
   if (rpi_halting)
      status += 4;
   return status;
}

int rpi_set_checks_result(char *in, char *err) {
   int out = 0;
   rpi_checks_result = 0;
   if (strlen(in) > 0 && !atoi(in, &rpi_checks_result, err)) {
      rpi_checks_result = 1;
      out = -2; // return atoi error
   } else {
      out = rpi_checks_result; // return check result
   }
   if (!has_error()) { // set led
      if (rpi_checks_result == 0)
         set_led_status(LED_OFF);
      else
         set_led_status(LED_WARNING);
   }
   return out;
}

int rpi_get_checks_result() { return rpi_checks_result; }

int rpi_get_checks_result();

void rpi_handle_ops() {
   // Check if the RPI was started
   if (!rpi_started) {
      if (rpi_first) {
#if DEBUG
         Serial.println("First start");
#endif
         // On first run, immediately start
         rpi_start();
      } else if (get_internal_time() >= rpi_restart_time) {
#if DEBUG
         Serial.println("Timed start");
#endif
         // It's time to start the RPI.
         rpi_start();
      }
      // All good.
      return;
   } else {
      // We should start the RPI but it's still running
      if (get_internal_time() >= rpi_restart_time) {
         // Skipping a start, we should log this
         rpi_update_waketime();
      }
   }

   if (rpi_halting) {
      // RPI asked to shutdown
      if (rpi_halt_cd >= RPI_STOP_COOLDOWN) {
         // It's time to shut it down as requested. All good.
         rpi_stop();
      }
      // All good.
      return;
   }

   if (!rpi_has_power() && rpi_boot_cd > 0) {
      // RPI was started, did not request a shutdown, but looks dead,
      // and is unpowered. Setting error.
      set_error(RPI_ERR_UNPOWERED, MSG_RPI_NO_POWER);
      return;
   }

   if (!rpi_booted && !rpi_halting && rpi_boot_cd >= RPI_START_COOLDOWN) {
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
   rpi_started = true;    // If the RPI was started
   rpi_halting = false;   // If RPI requested shutdown
   rpi_booted = false;    // If RPI sent the first heartbeat
   rpi_reset_cds();

   rpi_update_waketime();
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
   rpi_halting = false;
   rpi_booted = false;
   rpi_reset_cds();
   sync_time();
}

void rpi_timers_update() {
   if (rpi_started) {
      if (rpi_halting && rpi_halt_cd < RPI_STOP_COOLDOWN) {
         // RPI halting
         rpi_halt_cd += 1;
      } else if (!rpi_booted && rpi_boot_cd < RPI_START_COOLDOWN) {
         // RPI starting
         rpi_boot_cd += 1;
      }
   }
}

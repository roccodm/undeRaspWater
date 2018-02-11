#ifndef RPI_H
#define RPI_H

// TIMERS
#define RPI_START_COOLDOWN 150
#define RPI_STOP_COOLDOWN 60
#define RPI_HEARTBEAT_RESET_TIME 20

// Setup
void rpi_setup();

// Run mode
uint8_t rpi_set_run_mode(uint8_t mode);
uint8_t rpi_get_run_mode();

// Status
bool rpi_get_status();
bool rpi_has_power();

// Operations
void rpi_handle_ops();
void rpi_timers_update();
void rpi_start();
void rpi_stop();

// Start timer
void rpi_update_waketime();
int rpi_get_restart_time_left();

// Manual mode
void rpi_set_manual_mode(bool on);
bool rpi_is_manual();

// Heartbeat
void rpi_set_heartbeat(bool on);
bool rpi_get_heartbeat();

// Serial status
void rpi_set_serial_enabled(bool enabled);

#endif

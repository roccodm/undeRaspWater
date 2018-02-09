#ifndef RPI_H
#define RPI_H

// TIMERS
#define RPI_START_COOLDOWN 150
#define RPI_STOP_COOLDOWN 60
#define RPI_HEARTBEAT_RESET_TIME 20

bool first_start = true;  // true when first start, used to run self-checks
int rpi_cooldown = 0;  // the rpi start/stop cooldown
bool rpi_started = false;  // set to true when rpi has been started
bool rpi_keepalive = false; // set true to prevent RPI shutdown
bool rpi_heartbeat = false; // set true by raspberry to signal it's running status
int rpi_heartbeat_cooldown = 0; // the heartbeat cooldown status

void rpi_cooldown_update();
void rpi_start();
void rpi_stop();
bool rpi_is_running();
bool rpi_is_first_start();
void rpi_handle_checks();
void rpi_handle_ops();
void rpi_set_keepalive(bool on);
void rpi_set_heartbeat(bool on);
bool rpi_get_heartbeat();

#endif
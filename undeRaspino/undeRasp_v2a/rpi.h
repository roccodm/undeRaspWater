#ifndef RPI_H
#define RPI_H

bool first_start = true;  // true when first start, used to run self-checks
int rpi_cooldown = 0;  // the rpi start/stop cooldown
bool rpi_started = false;  // set to true when rpi has been started

void rpi_cooldown_update();
void start_rpi();
void stop_rpi();
bool rpi_is_running();
bool rpi_is_first_start();
void rpi_handle_checks();

#endif

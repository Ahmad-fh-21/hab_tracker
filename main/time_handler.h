#ifndef TIME_HANDLER_H
#define TIME_HANDLER_H

#include "all_depend.h"

#include "esp_sntp.h"


int time_handler_get_year();
int time_handler_get_month();
int time_handler_get_day();
int time_handler_get_hour();
int time_handler_get_minute();

void time_handler_get_time_from_server();
void time_handler_init();

#endif // ALL_DEPEND_H
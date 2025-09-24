#ifndef TIME_HANDLER_H
#define TIME_HANDLER_H

#include "all_depend.h"

#include "esp_sntp.h"


// struct to store the time 
typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
}time_storage_t;

int time_handler_get_year();
int time_handler_get_month();
int time_handler_get_day();
int time_handler_get_hour();
int time_handler_get_minute();
int time_handler_get_sec();

void time_handler_get_time_from_server();
void time_handler_init();

// void time_handler_resync();
//void time_handler_start_resync_timer(); 

#endif // ALL_DEPEND_H
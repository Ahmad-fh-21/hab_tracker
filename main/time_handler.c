#include "time_handler.h"


// Wait until time is set
time_t now = 0;
struct tm timeinfo = { 0 };
int retry = 0;
const int retry_count = 10;

static void initialize_sntp(void) {
    printf("Initializing SNTP...\n");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "bevtime1.metrologie.at");  // You can set your own server
    esp_sntp_init();
}

void time_handler_init()
{
    initialize_sntp();

}

void time_handler_get_time_from_server()
{
    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) 
    {
   // printf("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    time(&now);
    // Set Vienna timezone (CET/CEST)
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    }
}

   // // Now we have the time
    // char strftime_buf[64];
    // strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    // printf("Current date/time: %s\n", strftime_buf);

    // // Access individual components
    // printf("Year: %d\n", timeinfo.tm_year + 1900);
    // printf("Month: %d\n", timeinfo.tm_mon + 1); 
    // printf("Day: %d\n", timeinfo.tm_mday);
    // printf("Hour: %d\n", timeinfo.tm_hour);
    // printf("Minute: %d\n", timeinfo.tm_min);
    // printf("Second: %d\n", timeinfo.tm_sec);
int time_handler_get_year()
{
    return  timeinfo.tm_year + 1900;
}

int time_handler_get_month()
{
    return  timeinfo.tm_mon + 1; // tm_mon: 0 = Jan
}

int time_handler_get_day()
{
    return  timeinfo.tm_mday;
}

int time_handler_get_hour()
{
    return  timeinfo.tm_hour;
}

int time_handler_get_minute()
{
    return  timeinfo.tm_min;
}

#include "time_handler.h"


// Wait until time is set
time_t now = 0;
struct tm timeinfo = { 0 };
int retry = 0;
const int retry_count = 10;

// #define RESYNC_INTERVAL_HOURS 6   // how often to resync
// static TimerHandle_t ntp_timer;


// static void ntp_timer_callback(TimerHandle_t xTimer)
// {
//     time_handler_resync();
// }

// // this function configure a timer to call a callback to resync the time after fixed time
// void time_handler_start_resync_timer()
// {
//     if (ntp_timer == NULL)
//     {
//         ntp_timer = xTimerCreate("ntp_timer",
//                                  pdMS_TO_TICKS(RESYNC_INTERVAL_HOURS * 3600 * 1000),
//                                  pdTRUE,   // auto reload
//                                  NULL,
//                                  ntp_timer_callback);
//     }

//     if (ntp_timer != NULL)
//     {
//         xTimerStart(ntp_timer, 0);
//         printf("NTP resync timer started (every %d hours)\n", RESYNC_INTERVAL_HOURS);
//     }
// }



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
    time(&now);
    localtime_r(&now, &timeinfo);
    return  timeinfo.tm_year + 1900;
}

int time_handler_get_month()
{
    time(&now);
    localtime_r(&now, &timeinfo);
    return  timeinfo.tm_mon + 1; // tm_mon: 0 = Jan
}

int time_handler_get_day()
{
    time(&now);
    localtime_r(&now, &timeinfo);
    return  timeinfo.tm_mday;
}

int time_handler_get_hour()
{
    time(&now);
    localtime_r(&now, &timeinfo);
    return  timeinfo.tm_hour;
}

int time_handler_get_minute()
{
    time(&now);
    localtime_r(&now, &timeinfo);    
    return  timeinfo.tm_min;
}

int time_handler_get_sec()
{
    time(&now);
    localtime_r(&now, &timeinfo);    
    return  timeinfo.tm_sec;
}



// void time_handler_resync()
// {
//     printf("Forcing SNTP resync...\n");
//     esp_sntp_restart();   // trigger SNTP update

//     while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) 
//     {
//         vTaskDelay(2000 / portTICK_PERIOD_MS);
//         time(&now);
//         setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
//         tzset();
//         localtime_r(&now, &timeinfo);
//     }

//     if (retry < retry_count) {
//         printf("Time successfully resynced\n");
//     } else {
//         printf("Failed to resync time\n");
//     }
// }
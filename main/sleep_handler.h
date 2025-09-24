#ifndef SLEEP_HANDLER_H
#define SLEEP_HANDLER_H

#include "all_depend.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"

void sleep_go_to_deep_sleep(uint64_t time_in_sec);
void sleep_handler_print_wake_reason(esp_sleep_wakeup_cause_t wakeup_reason);

#endif
#include "sleep_handler.h"


static const char *TAG = "SLEEP_HANDLER";


void sleep_go_to_deep_sleep(uint64_t time_in_sec)
{
  //  ESP_LOGI(TAG, "Preparing to sleep");
    
    // Flush any pending serial output
    fflush(stdout);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    // Configure wakeup sources
    esp_sleep_enable_timer_wakeup(time_in_sec * 1000000);
    
    // 2. External wakeup (GPIO1) - wake up when GPIO1 goes LOW (button press)
    // Using EXT1 wakeup for GPIO1
    const uint64_t ext_wakeup_pin_1_mask = 1ULL << GPIO_NUM_1;
   // ESP_LOGI(TAG, "Enabling EXT1 wakeup on GPIO1");
    
    // Configure GPIO1 as RTC GPIO with pull-up for deep sleep
    rtc_gpio_init(GPIO_NUM_1);
    rtc_gpio_set_direction(GPIO_NUM_1, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_en(GPIO_NUM_1);
    rtc_gpio_pulldown_dis(GPIO_NUM_1);
    
    // Wait a bit to ensure pin is stable before sleep
    vTaskDelay(pdMS_TO_TICKS(100));
    
    esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_1_mask, ESP_EXT1_WAKEUP_ANY_LOW );

    // Power Impact:
    //Without isolation: Deep sleep current ~15-20µA
    //With isolation: Deep sleep current ~10µA
    #ifdef CONFIG_IDF_TARGET_ESP32
    rtc_gpio_isolate(GPIO_NUM_12);
    #endif

    // Enter deep sleep
    esp_deep_sleep_start();
    //esp_deep_sleep(time_in_us);
}

void sleep_handler_print_wake_reason(esp_sleep_wakeup_cause_t wakeup_reason)
{
    switch(wakeup_reason) 
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        ESP_LOGI(TAG, "Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        ESP_LOGI(TAG, "Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        ESP_LOGI(TAG, "Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        ESP_LOGI(TAG, "Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        ESP_LOGI(TAG, "Wakeup caused by ULP program");
        break;
    default:
        ESP_LOGI(TAG, "Not a deep sleep reset %d",wakeup_reason);
        break;
    }
}
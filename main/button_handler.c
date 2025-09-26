#include "button_handler.h"

#include "driver/gpio.h"
static const char *TAG = "Button";



static int64_t press_start_time = 0;
static int64_t last_press_time = 0;
volatile bool button_pressed_flag = false;
volatile bool button_released_flag = false;


volatile uint8_t press_type = IDLE;

#define LONG_PRESS_THRESHOLD_MS 1000  // e.g. 1.5s
#define VERY_LONG_PRESS_MS     3000




static void IRAM_ATTR button_isr_handler(void *arg)
{
    int level = gpio_get_level(BUTTON_GPIO);
    int64_t now = esp_timer_get_time() / 1000; // ms

    if ((now - last_press_time) < DEBOUNCE_DELAY_MS)
        return; // debounce

    last_press_time = now;

    if (level == 0) {  
        // Falling edge = pressed
        press_start_time = now;
        button_pressed_flag = true;
    } else { 
        // Rising edge = released
        button_released_flag = true;
    }
}




// Configure button with interrupt
void button_init(/*button_handler_t *button*/ void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,  // falling edge (button press if pulled-up)
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1ULL << BUTTON_GPIO,
        .pull_up_en = GPIO_PULLUP_ENABLE,  // enable internal pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE
    };
    gpio_config(&io_conf);

    // Install GPIO ISR service
    gpio_install_isr_service(0);  // 0 = default flags
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, (void *)BUTTON_GPIO);

    //button->button_pressed = false; // init for button flag 
    ESP_LOGI(TAG, "Button configured on GPIO %d", BUTTON_GPIO);
}


uint8_t button_get_press_type(void)
{
    return press_type;
}

void button_set_press_type(uint8_t state)
{
    press_type = state;
}


void button_task(void *pvParameter)
{
    while (1) {
        if (button_released_flag) 
        {
            button_released_flag = false;

            button_pressed_flag = false;
            int64_t now = esp_timer_get_time() / 1000;
            int64_t press_duration = now - press_start_time;

            // if (press_duration >= VERY_LONG_PRESS_MS) {
            //     ESP_LOGI(TAG, "Very long press: %lld ms", press_duration);
            //     press_type = VERY_LONG;
            //     //do_very_long_press_action();
            // } 
            // else 
            if (press_duration >= LONG_PRESS_THRESHOLD_MS) {
                ESP_LOGI(TAG, "Long press: %lld ms", press_duration);
                press_type = LONG;
                //do_long_press_action();
            } 
            else {
                ESP_LOGI(TAG, "Short press: %lld ms", press_duration);
                press_type = SHORT;
                //do_short_press_action();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));  // check periodically
    }
}
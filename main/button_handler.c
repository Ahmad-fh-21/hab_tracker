#include "button_handler.h"

#include "driver/gpio.h"




// Last valid press timestamp
static int64_t last_press_time = 0;
volatile bool button_status  = false;

static const char *TAG = "Button";


static void IRAM_ATTR button_isr_handler(void *arg)
{
    int64_t now = esp_timer_get_time() / 1000; // current time in ms
    if ((now - last_press_time) < DEBOUNCE_DELAY_MS)
        return; // ignore bounces

    last_press_time = now;
    button_status = true;

}


bool button_get_status(void)
{
    return button_status;
}

void button_set_status(bool status)
{
    button_status = status;
}

// Configure button with interrupt
void button_init(/*button_handler_t *button*/ void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,  // falling edge (button press if pulled-up)
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



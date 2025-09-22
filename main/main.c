
#include "all_depend.h"

#include "wifi_handler.h"
#include <esp_idf_version.h>
#include "driver/spi_master.h"
#include <max7219.h>
#include "esp_timer.h"

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 0, 0)
#define HOST    HSPI_HOST
#else
#define HOST    SPI2_HOST
#endif

#define CASCADE_SIZE 4
#define MOSI_PIN 18
#define CS_PIN 17 
#define CLK_PIN 19


#define ALL_DIGITS 8






max7219_t     dev = {
    .cascade_size = CASCADE_SIZE,
    .digits = 0,
    .mirrored = true
    };





esp_err_t max7219_set_dot(max7219_t *dev, uint8_t chip, uint8_t row, uint8_t col)
{
    // CHECK_ARG(dev);
    // CHECK_ARG(chip < dev->cascade_size);
    // CHECK_ARG(row < ALL_DIGITS);
    // CHECK_ARG(col < 8);

    // Compute absolute digit index across all cascades
    uint8_t digit = chip * ALL_DIGITS + row;

    // Prepare bit pattern for that column
    uint8_t val = 1 << col;

    return max7219_set_digit(dev, digit, val);
}

// uint8_t framebuffers[8]; // 8 rows

// esp_err_t max7219_set_dot_fb(max7219_t *dev, uint8_t row, uint8_t col, bool on)
// {
//     // CHECK_ARG(dev);
//     // CHECK_ARG(row < ALL_DIGITS);
//     // CHECK_ARG(col < 8);

//     if (on)
//         framebuffers[row] |= (1 << col);   // set bit
//     else
//         framebuffers[row] &= ~(1 << col);  // clear bit

//     return max7219_set_digit(dev, row, framebuffers[row]);
// }

uint8_t framebuffer[32]; // 32 rows (4 chips × 8 rows each)

esp_err_t max7219_update_display(max7219_t *dev)
{
    for (uint8_t row = 0; row < dev->digits; row++)
        max7219_set_digit(dev, row, framebuffer[row]);
    return ESP_OK;
}

void set_pixel(uint8_t row, uint8_t col, bool on)
{
    if (on)
        framebuffer[row] |= (1 << col);
    else
        framebuffer[row] &= ~(1 << col);
}

esp_err_t max7219_clear_all(max7219_t *dev)
{
    for (uint8_t i = 0; i < 32; i++)
        framebuffer[i] = 0;


    max7219_update_display(dev);    
    return ESP_OK;
}


static const char *TAG = "main";


// static const uint64_t symbols[] = {

//     0x3c66666e76663c00, // digits 0-9
//     0x7e1818181c181800, 
//     0x7e060c3060663c00,
//     0x3c66603860663c00,
//     0x30307e3234383000,
//     0x3c6660603e067e00,
//     0x3c66663e06663c00,
//     0x1818183030667e00,
//     0x3c66663c66663c00,
//     0x3c66607c66663c00 ,

//     0x383838fe7c381000, // arrow up
//     0x10387cfe38383800, //arrow down
//     0x10307efe7e301000, //arrow right
//     0x1018fcfefc181000 //arrow left
    
// };
//static const size_t symbols_size = sizeof(symbols) - sizeof(uint64_t) * CASCADE_SIZE;



uint8_t chip,row,col;

uint8_t presses = 0;
bool state = false;

bool toogle_state(bool *state)
{
    return *state = !*state;
}

void task(void *pvParameter)
{

//     max7219_t dev = {
//     .cascade_size = CASCADE_SIZE,
//     .digits = 0,
//     .mirrored = true
//     };
//     ESP_ERROR_CHECK(max7219_init_desc(&dev, HOST, MAX7219_MAX_CLOCK_SPEED_HZ, CS_PIN));
//     ESP_ERROR_CHECK(max7219_init(&dev));

//    // size_t offs = 0;

     ESP_ERROR_CHECK(max7219_set_brightness(&dev, 2));

   while (1)
    {
        // for (uint8_t i=0; i<CASCADE_SIZE; i++)
        // max7219_draw_image_8x8(&dev,i*8,(uint8_t *)symbols + i*8 + offs);
        // vTaskDelay(pdMS_TO_TICKS(500));


        // if (++offs == symbols_size)
        //     offs = 0;

        // for (uint8_t chip = 0; chip < CASCADE_SIZE; chip++)
        // {
        //     for (uint8_t row = 0; row < 8; row++)
        //     {
        //         for (uint8_t col = 0; col < 8; col++)
        //         {
        //             //ESP_ERROR_CHECK(max7219_set_dot(&dev,chip ,row, col));
        //             set_pixel(chip * 8 + row, col, true);
        //             max7219_update_display(&dev);

        //             vTaskDelay(pdMS_TO_TICKS(200));
        //         }
        //     }
            
        // }

        switch (presses)
        {
        case 1:
            set_pixel(chip * 8 + row, col, toogle_state(&state));
            max7219_update_display(&dev);
            break;
        case 2:
            set_pixel(chip * 8 + row, col, true);
            max7219_update_display(&dev);
            break;
        // case 3:
        //     set_pixel(chip * 8 + row, col, toogle_state(&state));
        //     max7219_update_display(&dev);
        //     break;
        
        default:
            break;
        }


        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// Minimum interval between valid presses (ms)
#define DEBOUNCE_DELAY_MS 100

// Last valid press timestamp
static int64_t last_press_time = 0;
volatile bool button_pressed = false;




static void IRAM_ATTR button_isr_handler(void *arg)
{
    int64_t now = esp_timer_get_time() / 1000; // current time in ms
    if ((now - last_press_time) < DEBOUNCE_DELAY_MS)
        return; // ignore bounces

    last_press_time = now;
    button_pressed = true;
    // Safe to handle the button press here (very short!)
   // ets_printf("Button pressed!\n");
}
#define BUTTON_GPIO GPIO_NUM_1 


// Configure button with interrupt
void button_init(void)
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

    ESP_LOGI(TAG, "Button configured on GPIO %d", BUTTON_GPIO);
}

void button_task(void *arg)
{   
    while (1) {
        if (button_pressed) {

            button_pressed = false;
            presses++;
            if (presses > 2) presses = 1;
            // Now it's safe to log or do any processing
            printf("Button pressed!\n");
            //max7219_clear_all(&dev);

        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void time_task(void *arg)
{   
    while (1) 
    {
        col++;
        if (col >= 8)
        {
            col = 0;
            row++;
        }
        if (row >= 8)
        {
            row = 0;
            chip++;
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}



void app_main(void)
{
    //wifi_handler_init_NVM();

   // ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
  //  wifi_handler_init_sta();



    


    button_init();

      spi_bus_config_t cfg = {
       .mosi_io_num = MOSI_PIN,
       .miso_io_num = -1,
       .sclk_io_num = CLK_PIN,
       .quadwp_io_num = -1,
       .quadhd_io_num = -1,
       .max_transfer_sz = 0,
       .flags = 0
    };
    ESP_ERROR_CHECK(spi_bus_initialize(HOST, &cfg, SPI_DMA_CH_AUTO));


    // dev = {
    // .cascade_size = CASCADE_SIZE,
    // .digits = 0,
    // .mirrored = true
    // };
    ESP_ERROR_CHECK(max7219_init_desc(&dev, HOST, MAX7219_MAX_CLOCK_SPEED_HZ, CS_PIN));
    ESP_ERROR_CHECK(max7219_init(&dev));

    chip=0; row=0; col=0;


    xTaskCreatePinnedToCore(task, "task", configMINIMAL_STACK_SIZE * 3, NULL, 4, NULL, APP_CPU_NUM);
    
    xTaskCreate(button_task, "button_task", 4096, NULL, 4, NULL);

    xTaskCreate(time_task, "time_task", 4096, NULL, 5, NULL);
    
}








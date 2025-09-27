
#include "all_depend.h"

#include "wifi_handler.h"
#include <esp_idf_version.h>
#include "driver/spi_master.h"
#include <max7219.h>

#include "button_handler.h"
#include "time_handler.h"
#include "sleep_handler.h"
#include "nvm_handler.h"

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 0, 0)
#define HOST    HSPI_HOST
#else
#define HOST    SPI2_HOST
#endif

// display chip number and SPI pis
#define CASCADE_SIZE 4
#define MOSI_PIN 18
#define CS_PIN 17 
#define CLK_PIN 19


#define ALL_DIGITS 8
#define STANDBY_TIME_IN_SEC   30
#define QUEUE_LENGTH 7



enum USER
{
    USER_1 = 0,
    USER_2,
};

uint8_t user_panel = USER_1;           // var to switch between User panels
bool    set_value = false;             // to store the last selected LED status by user
bool    set_value_2 = false;           // to store the last selected LED status by user 2 
bool    user_switch = false;           // to indicate if a user switch happens


// global Var
QueueHandle_t sleep_queue;
bool ready_to_sleep = false;           // for the Queue
QueueHandle_t wifi_queue;
bool init_wifi_before_sleep = false;

bool isWakeup_Timer = false;
bool wifi_initiliazed = false;

uint8_t   secCounter = 0;              // to count the standby sec - when is equal to STANDBY_TIME_IN_SEC device goes to sleep
uint16_t  calculated_sleep_time = 0;   // to store the time in sec for the sleep wake up timer


uint8_t presses = 0;                   // to store the button presses number 
bool state = false;                    // to store the last LED state - used for blinking and chossing the set during Operation
bool state2 = false;                   // to store the last LED  2 state - used for blinking and chossing the set during Operation
uint8_t framebuffer[FRAMEBUFFER_SIZE];   // Panel 1 values
uint8_t framebuffer2[FRAMEBUFFER_SIZE];  // to be used to add a sec Panel 
bool firstboot = true;                 // to indicate if this is the First boot - this bool will be used only once and then the NVM is initialized

//uint8_t chip,row,col;                  // to control the LED position - calculated throw the month - day 

// Store variables in RTC slow memory
RTC_DATA_ATTR uint8_t chip = 0;
RTC_DATA_ATTR uint8_t row = 0;
RTC_DATA_ATTR uint8_t col = 0;




time_storage_t mytime;                 // var to stroe the time after getting it from the time_handler

// for the display init
max7219_t     dev = {
    .cascade_size = CASCADE_SIZE,
    .digits = 0,
    .mirrored = true
    };


//           LOCAL FUNCTIONS 
/*************************************** */
static void main_update_matrix();
static void main_init_display();
static void main_init_NVM();
bool toogle_state(bool *state);

static void main_short_long_press_handler();
static void main_led_status_slecter_handler();




// for Serial Monitor 
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











void sleep_task(void *pvParameters) 
{
    while (1) 
    {
        // Wait indefinitely for data to arrive in the queue
        if (xQueueReceive(sleep_queue, &ready_to_sleep, portMAX_DELAY) == pdPASS) 
        {
            
            // set_pixel(chip * 8 + row, col, set_value,framebuffer);
            // max7219_update_display(&dev,framebuffer);

            // set_pixel(chip * 8 + row, col, set_value_2,framebuffer2);
            // max7219_update_display(&dev,framebuffer2);

            // ESP_ERROR_CHECK(max7219_set_shutdown_mode(&dev,true));




           // ESP_LOGI(TAG, "Saving data before deep sleep...");
            nvm_handler_save_data_to_nvs(framebuffer, firstboot, framebuffer2);
            sleep_go_to_deep_sleep(calculated_sleep_time);
        }
    }
}

void wifi_task(void *pvParameters) 
{
    while (1) 
    {
        // Wait indefinitely for data to arrive in the queue
        if (xQueueReceive(wifi_queue, &init_wifi_before_sleep, portMAX_DELAY) == pdPASS) 
        {
           // ESP_LOGI(TAG, "wifi Task -----------------------");
            set_pixel(chip * 8 + row, col, set_value,framebuffer);
            max7219_update_display(&dev,framebuffer);

            set_pixel(chip * 8 + row, col, set_value_2,framebuffer2);
            max7219_update_display(&dev,framebuffer2);

            ESP_ERROR_CHECK(max7219_set_shutdown_mode(&dev,true));


            if (isWakeup_Timer == false && wifi_initiliazed == false)
            {
               // wifi_handler_init_NVM();

              //  ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
                wifi_handler_init_sta();
                time_handler_init();

                wifi_initiliazed = true;
            
            }

            time_handler_get_time_from_server();

            

            mytime.month = time_handler_get_month();
            mytime.day   = time_handler_get_day();
            mytime.hour  = time_handler_get_hour();
            mytime.min   = time_handler_get_minute();
            mytime.sec   = time_handler_get_sec();

            calculated_sleep_time = mytime.sec + ((60 - mytime.min) * 60 );
           // ESP_LOGI(TAG, "Reamining sleep time %d", calculated_sleep_time);
            main_update_matrix();
        

            if (xQueueSend(sleep_queue, &ready_to_sleep, pdMS_TO_TICKS(100)) == pdPASS) 
            {
                vTaskSuspend(NULL);  // Suspend self after sending to sleep queue
            } 


  
        }
    }
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

         main_short_long_press_handler();
         main_led_status_slecter_handler();
       

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}


void time_task(void *arg)
{   
    while (1) 
    {
        ESP_LOGI(TAG, "secConter %d", secCounter);
        if (secCounter == STANDBY_TIME_IN_SEC)
        {
            
                // if (xQueueSend(sleep_queue, &ready_to_sleep, pdMS_TO_TICKS(100)) == pdPASS) 
                // {
                //     vTaskSuspend(NULL);  // Suspend self after sending to sleep queue
                // } 
            if (xQueueSend(wifi_queue, &init_wifi_before_sleep, pdMS_TO_TICKS(100)) == pdPASS) 
            {
                vTaskSuspend(NULL);  // Suspend self after sending to sleep queue
            } 
        }

        secCounter++;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}




void update_led_position_task(void *arg)
{   
    while (1) 
    {
        if (isWakeup_Timer)
        {
            time_handler_get_time_from_server();

            mytime.month = time_handler_get_month();
            mytime.day   = time_handler_get_day();
            mytime.hour  = time_handler_get_hour();
            mytime.min   = time_handler_get_minute();
            mytime.sec   = time_handler_get_sec();

            calculated_sleep_time = mytime.sec + ((60 - mytime.min) * 60 );

            main_update_matrix();
            
            // printf("Year: %d\n", mytime.year );
            // printf("Month: %d\n", mytime.month );  // tm_mon: 0 = Jan
            // printf("Day: %d\n", mytime.day);
            // printf("Hour: %d\n", mytime.hour);
            // printf("Minute: %d\n", mytime.min);
            // printf("sec: %d\n", mytime.sec);
            // printf("time to sleep: %d\n", calculated_sleep_time);
            // vTaskDelay(pdMS_TO_TICKS(10000));
        }
       
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

static void main_update_matrix()
{
    if ((mytime.day -1) % 7 == 0 && mytime.day != 1 )  // get correct col
    {
        col = 0;
    }
    else
    {
        col = (mytime.day -1) % 7;
    }

    chip = mytime.month % 4;  // get correct chip 
    row =(int) ((mytime.day - 1) / 7);   // get correct row

    // if ((mytime.min -1) % 7 == 0 && mytime.min != 1 )  // get correct col
    // {
    //     col = 0;
    // }
    // else
    // {
    //     col = (mytime.min -1) % 7;
    // }

    // chip = mytime.month % 4;  // get correct chip 
    // row =(int) ((mytime.min - 1) / 7);   // get correct row




    if (col == 0 && row == 0) // if new month / chip clear old saving
    {
        max7219_clear_chip(&dev,chip,framebuffer);
        max7219_clear_chip(&dev,chip,framebuffer2);
    }

    
    // send the new data
    // set_pixel(chip * 8 + row, col, true,framebuffer);
    // max7219_update_display(&dev,framebuffer);
}



void app_main(void)
{


    uint64_t start = esp_timer_get_time();
    /************************************* */
    button_init();

    main_init_display();
   // ESP_LOGI(TAG, "in main");
    // wifi_handler_init_NVM();

    // ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    // wifi_handler_init_sta();
    // time_handler_init();
    
    // time_handler_get_time_from_server();

    /************************************* */

    wifi_handler_init_NVM();

    if (firstboot == true )
    {
        
     //   ESP_LOGI(TAG, "wifi init for first boot");
        if (wifi_initiliazed == false )
        {
            
            wifi_handler_init_sta();
            time_handler_init();
           
            wifi_initiliazed = true;
        
        }

    }
   // nvm_handler_clear_nvs_data();  //<--------------- comment when SW is finished
    main_init_NVM();




    // Print wake up reason
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    sleep_handler_print_wake_reason( wakeup_reason);
  //  ESP_LOGI(TAG, "in main ----------> wake up timer - first boot %d   %d ", wakeup_reason,firstboot);
    if (wakeup_reason != ESP_SLEEP_WAKEUP_TIMER  && wakeup_reason != 0 )
    {   
   //     sleep_handler_print_wake_reason( wakeup_reason);
        ESP_ERROR_CHECK(max7219_set_shutdown_mode(&dev,false));
        ESP_ERROR_CHECK(max7219_set_brightness(&dev, 2));
        max7219_update_display(&dev,framebuffer);
    //    ESP_LOGI(TAG, "in main ----------> interrupt");
    }
    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER && firstboot == false ) // only if the timer wake up - get time without turn on the display
    {
      //  ESP_LOGI(TAG, "in main ----------> wake up timer - first boot %d   %d ", wakeup_reason,firstboot);
        isWakeup_Timer = true;
       // firstboot = false;
        // wifi_handler_init_NVM();

        if (wifi_initiliazed == false)
        {
            wifi_handler_init_sta();
            time_handler_init();

            wifi_initiliazed = true;
        }
        time_handler_get_time_from_server();
    }





    // Create the queue
    sleep_queue = xQueueCreate(QUEUE_LENGTH, sizeof(bool));
    if (sleep_queue == NULL) 
    {
        //ESP_LOGE(TAG, "Sleep Queue creation failed!");
        return;
    }

    // Create the queue
    wifi_queue = xQueueCreate(QUEUE_LENGTH, sizeof(bool));
    if (wifi_queue == NULL) 
    {
       // ESP_LOGE(TAG, "Sleep Queue creation failed!");
        return;
    }

    /************************************* */
    xTaskCreatePinnedToCore(task, "task", configMINIMAL_STACK_SIZE * 3, NULL, 4, NULL, APP_CPU_NUM); 
    xTaskCreate(button_task, "button_task", 4096, NULL, 4, NULL); // task function in Button_handler.c
    xTaskCreate(time_task, "time_task", 4096, NULL, 4, NULL);
    xTaskCreate(update_led_position_task, "update_led_position_task", 4096, NULL, 5, NULL);
    // Start sensor reading task
    xTaskCreate(sleep_task, "sleep_task", 4096, NULL, 5, NULL);
    xTaskCreate(wifi_task, "wifi_task", 4096, NULL, 5, NULL);
    /************************************* */
    

    uint64_t end = esp_timer_get_time(); 
    
  //  ESP_LOGI(TAG, " took %llu microseconds", (end - start));
}


static void main_init_display()
{
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

    ESP_ERROR_CHECK(max7219_init_desc(&dev, HOST, MAX7219_MAX_CLOCK_SPEED_HZ, CS_PIN));
    ESP_ERROR_CHECK(max7219_init(&dev));

    ESP_ERROR_CHECK(max7219_set_brightness(&dev, 0));
    ESP_ERROR_CHECK(max7219_set_shutdown_mode(&dev, true));
}

static void main_init_NVM()
{
    // Load data from NVS on startup
    nvm_handler_load_data_from_nvs(framebuffer, &firstboot, framebuffer2);
  //  ESP_LOGI(TAG, "firstboot state %d",firstboot);
    // Check if this is first boot
    if (firstboot) 
    {
        nvm_handler_clear_nvs_data();
      //  ESP_LOGI(TAG, "This is the first boot!");
        // For "VIT" in last 3 rows of chip 0 
        framebuffer[5] = 0b01100110;   // V I T (top row)
        framebuffer[6] = 0b00111100;   // V I T (middle row)  
        framebuffer[7] = 0b00011000;   // V I T (bottom row)

        // For "VIT" in last 3 rows of chip 1 
        framebuffer[13] = 0b00011000;  
        framebuffer[14] = 0b00011000;  
        framebuffer[15] = 0b00011000;  

        // For "VIT" in last 3 rows of chip 2 
        framebuffer[21] = 0b01111110;  
        framebuffer[22] = 0b00011000;  
        framebuffer[23] = 0b00011000;  

        // For "GYM" in last 3 rows of chip 0 
        framebuffer2[5] = 0b00011111;   
        framebuffer2[6] = 0b01100011;   
        framebuffer2[7] = 0b01011111;   

        // For "GYM" in last 3 rows of chip 1 
        framebuffer2[13] = 0b01100110;  
        framebuffer2[14] = 0b00011000;  
        framebuffer2[15] = 0b00011000;  

        // For "GYM"" in last 3 rows of chip 2 
        framebuffer2[21] = 0b11111111;  
        framebuffer2[22] = 0b11011011;  
        framebuffer2[23] = 0b11011011;  

        firstboot = false; // Set to false after first boot   ----> comment because it will be set to false in later stage in main 
    } 
    else 
    {
        ESP_LOGI(TAG, "This is a subsequent boot, framebuffers restored");
    }

}

// toggle last stored value for the LED - used in blinking 
bool toogle_state(bool *state)
{
    return *state = !*state;
}



static void main_short_long_press_handler()
{
    
        if(button_get_press_type() == SHORT)
        {
            button_set_press_type(IDLE);
            ESP_ERROR_CHECK(max7219_set_shutdown_mode(&dev,false));
            ESP_ERROR_CHECK(max7219_set_brightness(&dev, 2));
           user_switch = false;
            presses++;
            if (presses > 2) presses = 1;
            
         //   printf("Button pressed!\n");
            //max7219_clear_all(&dev);
            secCounter = 0; // reset the conter to wait agian from 0
        }
        else if (button_get_press_type() == LONG)
        {
            button_set_press_type(IDLE);
            if (user_panel == USER_1) 
            {
                user_panel = USER_2;
                max7219_update_display(&dev,framebuffer2);
            }
            else
            {
                user_panel = USER_1;
                max7219_update_display(&dev,framebuffer);
            } 
           // printf("Button pressed LONG!\n");
            
            user_switch = true;
        }

}


static void main_led_status_slecter_handler()
{
    switch (presses)
    {
    case 1:
        if (user_panel == USER_1 && user_switch == false)
        {
            set_value = false;
            set_pixel(chip * 8 + row, col, toogle_state(&state),framebuffer);
            max7219_update_display(&dev,framebuffer);
        }
        else if (user_panel == USER_2 && user_switch == false)
        {
            set_value_2 = false;
            set_pixel(chip * 8 + row, col, toogle_state(&state2),framebuffer2);
            max7219_update_display(&dev,framebuffer2);
        }

        break;
    case 2:
    if (user_panel == USER_1 && user_switch == false)
        {
            set_value = true;
            set_pixel(chip * 8 + row, col, true,framebuffer);
            max7219_update_display(&dev,framebuffer);
        }
        else if (user_panel == USER_2 && user_switch == false)
        {
            set_value_2 = true;
            set_pixel(chip * 8 + row, col, true,framebuffer2);
            max7219_update_display(&dev,framebuffer2);
        }
        break;

    // case 3:
    //     set_pixel(chip * 8 + row, col, toogle_state(&state));
    //     max7219_update_display(&dev);
    //     break;
    
    default:
            break;
        }

} 
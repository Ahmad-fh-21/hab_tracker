#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "all_depend.h"
 

#define BUTTON_GPIO GPIO_NUM_1 

#define DEBOUNCE_DELAY_MS 70   // Minimum interval between valid presses (ms)

enum PRESS_TYPE
{
    IDLE = 0,
    SHORT ,
    LONG ,
    VERY_LONG
};


// typedef struct 
// {
//     volatile bool button_pressed ;
// }button_handler_t;



//void button_init(button_handler_t *button);
void button_init(void);
// bool button_get_status(void);
// void button_set_status(bool status);



void button_set_press_type(uint8_t state);
uint8_t button_get_press_type(void);

void button_task(void *pvParameter);
#endif


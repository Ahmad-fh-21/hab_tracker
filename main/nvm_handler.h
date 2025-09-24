#ifndef NVM_HANDLER_H
#define NVM_HANDLER_H

#include "all_depend.h"
#include "nvs_flash.h"
#include "nvs.h"

// Define the framebuffer size as a constant
#define FRAMEBUFFER_SIZE 32



// Function declarations
esp_err_t nvm_handler_nvs_storage_init(void);
esp_err_t nvm_handler_save_data_to_nvs(uint8_t framebuffer[], bool firstboot, uint8_t framebuffer2[]);
esp_err_t nvm_handler_load_data_from_nvs(uint8_t framebuffer[], bool *firstboot, uint8_t framebuffer2[]);
esp_err_t nvm_handler_clear_nvs_data(void);

#endif
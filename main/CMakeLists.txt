idf_component_register(SRCS "nvm_handler.c" "wifi_handler.c" "main.c" "max7219.c" "button_handler.c" "time_handler.c" "sleep_handler.c" "nvm_handler.c"
                    PRIV_REQUIRES esp_wifi nvs_flash driver log esp_timer
                    INCLUDE_DIRS ".")

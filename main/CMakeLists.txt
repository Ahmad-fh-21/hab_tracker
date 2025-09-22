idf_component_register(SRCS "wifi_handler.c" "main.c" "max7219.c" "button_handler.c"
                    PRIV_REQUIRES esp_wifi nvs_flash driver log esp_timer
                    INCLUDE_DIRS ".")

#include "nvm_handler.h"

static const char *TAG = "NVS_STORAGE";
static const char* NVS_NAMESPACE = "storage";



// Initialize NVS - don't duplicate with wifi function 
esp_err_t nvm_handler_nvs_storage_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
  //  ESP_LOGI(TAG, "NVS initialized");
    return ret;
}

// Save data and firstboot to NVS
esp_err_t nvm_handler_save_data_to_nvs(uint8_t framebuffer[], bool firstboot, uint8_t framebuffer2[])
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }

    // Save framebuffer (use defined size instead of sizeof)
    err = nvs_set_blob(nvs_handle, "framebuffer", framebuffer, FRAMEBUFFER_SIZE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving framebuffer: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    // Save framebuffer2 (use defined size instead of sizeof)
    err = nvs_set_blob(nvs_handle, "framebuffer2", framebuffer2, FRAMEBUFFER_SIZE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving framebuffer2: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    // Save firstboot flag
    err = nvs_set_u8(nvs_handle, "firstboot", (uint8_t)firstboot);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving firstboot: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    // Commit changes to flash
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing to NVS: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Data saved to NVS successfully");
    }

    // Close NVS handle
    nvs_close(nvs_handle);
    return err;
}

// Load framebuffer and firstboot from NVS
esp_err_t nvm_handler_load_data_from_nvs(uint8_t framebuffer[], bool *firstboot, uint8_t framebuffer2[])
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }

    // Load framebuffer
    size_t required_size = FRAMEBUFFER_SIZE;
    err = nvs_get_blob(nvs_handle, "framebuffer", framebuffer, &required_size);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Framebuffer loaded from NVS (%d bytes)", required_size);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Framebuffer not found in NVS, using defaults");
        // Initialize with default values if needed
        memset(framebuffer, 0, FRAMEBUFFER_SIZE);
    } else {
        ESP_LOGE(TAG, "Error loading framebuffer: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    // Load framebuffer2
    size_t required_size2 = FRAMEBUFFER_SIZE;
    err = nvs_get_blob(nvs_handle, "framebuffer2", framebuffer2, &required_size2);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Framebuffer2 loaded from NVS (%d bytes)", required_size2);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Framebuffer2 not found in NVS, using defaults");
        // Initialize with default values if needed
        memset(framebuffer2, 0, FRAMEBUFFER_SIZE);
    } else {
        ESP_LOGE(TAG, "Error loading framebuffer2: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    // Load firstboot flag
    uint8_t firstboot_val;
    err = nvs_get_u8(nvs_handle, "firstboot", &firstboot_val);
    if (err == ESP_OK) {
        *firstboot = (bool)firstboot_val;
        ESP_LOGI(TAG, "Firstboot flag loaded from NVS: %s", *firstboot ? "true" : "false");
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Firstboot flag not found in NVS, using default: true");
        *firstboot = true;
    } else {
        ESP_LOGE(TAG, "Error loading firstboot: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    // Close NVS handle
    nvs_close(nvs_handle);
    return ESP_OK;
}

// Clear all data from NVS (optional - for testing/reset)
esp_err_t nvm_handler_clear_nvs_data(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_erase_all(nvs_handle);
    if (err == ESP_OK) {
        err = nvs_commit(nvs_handle);
        ESP_LOGI(TAG, "NVS data cleared");
    }

    nvs_close(nvs_handle);
    return err;
}

// Example usage in app_main
/*
void app_main(void)
{
    // Declare your variables
    uint8_t framebuffer[FRAMEBUFFER_SIZE];
    uint8_t framebuffer2[FRAMEBUFFER_SIZE];
    bool firstboot = true;
    
    // Initialize NVS
    nvs_storage_init();
    
    // Load data from NVS on startup
    load_data_from_nvs(framebuffer, &firstboot, framebuffer2);
    
    // Check if this is first boot
    if (firstboot) {
        ESP_LOGI(TAG, "This is the first boot!");
        // Initialize framebuffer with default pattern or clear it
        memset(framebuffer, 0x55, FRAMEBUFFER_SIZE); // Example pattern
        memset(framebuffer2, 0xAA, FRAMEBUFFER_SIZE); // Example pattern
        firstboot = false; // Set to false after first boot
    } else {
        ESP_LOGI(TAG, "This is a subsequent boot, framebuffers restored");
    }
    
    // Your application logic here
    // Modify framebuffers as needed...
    
    // Before going to deep sleep, save data
    ESP_LOGI(TAG, "Saving data before deep sleep...");
    save_data_to_nvs(framebuffer, firstboot, framebuffer2);
    
    // Configure wake-up sources and go to sleep
    // ... your deep sleep code here ...
}
*/
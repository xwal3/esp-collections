#include <cli.h>
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "main"; 

static esp_err_t cmd_help(int argc, char ** argv){
    (void)argc;
    (void)argv;
    ESP_LOGI(TAG, "help: no commands registered yet besides this one");
    return ESP_OK;
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting kv-cli");

    esp_err_t ret = cli_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CLI init failed: %s", esp_err_to_name(ret));
        return;
    }

      ret = cli_register_command("help", "Show available commands", cmd_help);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register 'help': %s", esp_err_to_name(ret));
        return;
    }

    ret = cli_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CLI start failed: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "kv-cli running");
}

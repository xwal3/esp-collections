#include "cli.h"
#include "kv_store.h"
#include "cmd_kv.h"
#include "esp_log.h"

static const char *TAG = "main";

esp_err_t cmd_kv_register_all(void);

void app_main(void)
{
    ESP_LOGI(TAG, "Starting kv-cli");

    ESP_ERROR_CHECK(cli_init());
    ESP_ERROR_CHECK(kv_store_init());
    ESP_ERROR_CHECK(kv_store_load());

    esp_err_t ret = cmd_kv_register_all();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register commands: %s", esp_err_to_name(ret));
        return;
    }

    ESP_ERROR_CHECK(cli_start());
    ESP_LOGI(TAG, "kv-cli running");
}
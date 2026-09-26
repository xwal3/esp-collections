#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register all kv-cli commands with the CLI subsystem.
 *
 * Must be called after cli_init and before cli_start.
 *
 * @return ESP_OK on success, error otherwise.
 */
esp_err_t cmd_kv_register_all(void);

#ifdef __cplusplus
}
#endif
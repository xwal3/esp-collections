#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Signature for a CLI command handler.
 *
 * @param argc  Number of arguments (including the command name).
 * @param argv  NULL-terminated argument vector.
 * @return ESP_OK on success, error otherwise.
 */
typedef esp_err_t (*cli_cmd_fn_t)(int argc, char **argv);

/**
 * @brief Register a command with the CLI.
 *
 * @param name   Command name (must be unique, stable pointer).
 * @param help   Short help string (must be a stable pointer).
 * @param fn     Handler function.
 * @return ESP_OK on success, ESP_ERR_NO_MEM if the command table is full,
 *         ESP_ERR_INVALID_STATE if the name is already registered.
 */
esp_err_t cli_register_command(const char *name, const char *help, cli_cmd_fn_t fn);

/**
 * @brief Initialize the CLI subsystem.
 *
 * @return ESP_OK on success.
 */
esp_err_t cli_init(void);

/**
 * @brief Start the CLI task.
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if already running.
 */
esp_err_t cli_start(void);

/* Exposed for unit tests only. Not part of the public API. */
int cli_tokenize(char *line, char **argv, int max_args);
esp_err_t cli_dispatch(int argc, char **argv);

#ifdef __cplusplus
}
#endif
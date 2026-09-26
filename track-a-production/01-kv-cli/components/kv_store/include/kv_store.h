#pragma once

#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback for iterating over store entries.
 *
 * @param key    Entry key.
 * @param value  Entry value.
 * @param user   User context from kv_store_iterate.
 */
typedef void (*kv_iter_fn_t)(const char *key, const char *value, void *user);

/**
 * @brief Initialize the store.
 *
 * @return ESP_OK on success, ESP_ERR_NO_MEM if mutex creation fails,
 *         ESP_ERR_INVALID_STATE if already initialized.
 */
esp_err_t kv_store_init(void);

/**
 * @brief Insert or update a key-value pair.
 *
 * @param key    Key (copied into internal buffer).
 * @param value  Value (copied into internal buffer).
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG on NULL or oversized
 *         input, ESP_ERR_NO_MEM if the store is full.
 */
esp_err_t kv_store_set(const char *key, const char *value);

/**
 * @brief Copy the value for a key into the caller's buffer.
 *
 * @param key        Key to look up.
 * @param out_value  Caller-provided buffer.
 * @param out_size   Size of out_value in bytes.
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG on NULL or zero-size,
 *         ESP_ERR_NOT_FOUND if key does not exist, ESP_ERR_INVALID_SIZE
 *         if out_size is too small.
 */
esp_err_t kv_store_get(const char *key, char *out_value, size_t out_size);

/**
 * @brief Remove a key from the store.
 *
 * @param key  Key to remove.
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if key does not exist.
 */
esp_err_t kv_store_delete(const char *key);

/**
 * @brief Remove all entries.
 *
 * @return ESP_OK on success.
 */
esp_err_t kv_store_clear(void);

/**
 * @brief Call fn for each entry in the store.
 *
 * @param fn    Callback invoked per entry.
 * @param user  User context passed through to fn.
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if fn is NULL.
 */
esp_err_t kv_store_iterate(kv_iter_fn_t fn, void *user);

/**
 * @brief Return the number of entries currently stored.
 */
size_t kv_store_count(void);

#ifdef __cplusplus
}
#endif
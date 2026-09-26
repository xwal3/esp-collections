#include "kv_store.h"

#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "sdkconfig.h"

static const char *TAG = "kv_store";

typedef struct{
    char key[CONFIG_KV_MAX_KEY_LEN];
    char value[CONFIG_KV_MAX_VALUE_LEN];
    bool in_use;
}kv_entry_t;

static kv_entry_t      s_entries[CONFIG_KV_MAX_KEYS];
static size_t          s_count = 0;
static SemaphoreHandle_t s_mutex = NULL;


static kv_entry_t *find_entry(const char *key){
    for(size_t i = 0; i < CONFIG_KV_MAX_KEYS; i++){
        if(s_entries[i].in_use && strcmp(s_entries[i].key, key) == 0){
            return &s_entries[i];
        }
    }
    return NULL;
}

static kv_entry_t *find_free(void)
{
    for (size_t i = 0; i < CONFIG_KV_MAX_KEYS; i++) {
        if (!s_entries[i].in_use) {
            return &s_entries[i];
        }
    }
    return NULL;
}
esp_err_t kv_store_init(void){
    if (s_mutex != NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    s_mutex = xSemaphoreCreateMutex();

    if (s_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return ESP_ERR_NO_MEM;
    }

    memset(s_entries, 0, sizeof(s_entries));
    s_count = 0;

    ESP_LOGI(TAG, "Store initialized (%d slots, key<=%d, value<=%d)",
             CONFIG_KV_MAX_KEYS, CONFIG_KV_MAX_KEY_LEN, CONFIG_KV_MAX_VALUE_LEN);
    return ESP_OK;

}

esp_err_t kv_store_set(const char *key, const char *value)
{
    if (key == NULL || value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (strlen(key) >= CONFIG_KV_MAX_KEY_LEN || strlen(value) >= CONFIG_KV_MAX_VALUE_LEN) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    kv_entry_t *e = find_entry(key);
    if (e == NULL) {
        e = find_free();
        if (e == NULL) {
            xSemaphoreGive(s_mutex);
            ESP_LOGW(TAG, "Store full, cannot add '%s'", key);
            return ESP_ERR_NO_MEM;
        }
        strncpy(e->key, key, sizeof(e->key) - 1);
        e->key[sizeof(e->key) - 1] = '\0';
        e->in_use = true;
        s_count++;
    }
    strncpy(e->value, value, sizeof(e->value) - 1);
    e->value[sizeof(e->value) - 1] = '\0';

    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t kv_store_get(const char *key, char *out_value, size_t out_size){
    
    if (key == NULL || out_value == NULL || out_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    kv_entry_t *e = find_entry(key);

    if (e == NULL) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NOT_FOUND;
    }

        if (strlen(e->value) >= out_size) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_INVALID_SIZE;
    }
    strcpy(out_value, e->value);

    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t kv_store_delete(const char *key)
{
    if (key == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    kv_entry_t *e = find_entry(key);
    if (e == NULL) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NOT_FOUND;
    }
    memset(e, 0, sizeof(*e));
    s_count--;

    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t kv_store_clear(void)
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    memset(s_entries, 0, sizeof(s_entries));
    s_count = 0;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}
esp_err_t kv_store_iterate(kv_iter_fn_t fn, void *user)
{
    if (fn == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    for (size_t i = 0; i < CONFIG_KV_MAX_KEYS; i++) {
        if (s_entries[i].in_use) {
            fn(s_entries[i].key, s_entries[i].value, user);
        }
    }

    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

size_t kv_store_count(void)
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    size_t n = s_count;
    xSemaphoreGive(s_mutex);
    return n;
}
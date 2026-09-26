#include "cli.h"
#include "kv_store.h"
#include "cmd_kv.h"
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "cmd_kv";

static esp_err_t cmd_set(int argc, char **argv)
{
    if (argc != 3) {
        printf("usage: set <key> <value>\n");
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t ret = kv_store_set(argv[1], argv[2]);
    if (ret != ESP_OK) {
        printf("error: %s\n", esp_err_to_name(ret));
    } else {
        printf("OK\n");
    }
    return ret;
}

static esp_err_t cmd_get(int argc, char **argv)
{
    if (argc != 2) {
        printf("usage: get <key>\n");
        return ESP_ERR_INVALID_ARG;
    }
    char value[CONFIG_KV_MAX_VALUE_LEN];
    esp_err_t ret = kv_store_get(argv[1], value, sizeof(value));
    if (ret != ESP_OK) {
        printf("error: %s\n", esp_err_to_name(ret));
    } else {
        printf("%s\n", value);
    }
    return ret;
}

static esp_err_t cmd_del(int argc, char **argv)
{
    if (argc != 2) {
        printf("usage: del <key>\n");
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t ret = kv_store_delete(argv[1]);
    if (ret != ESP_OK) {
        printf("error: %s\n", esp_err_to_name(ret));
    } else {
        printf("OK\n");
    }
    return ret;
}

static void print_pair(const char *key, const char *value, void *user)
{
    int *count = (int *)user;
    printf("%-20s = %s\n", key, value);
    (*count)++;
}

static esp_err_t cmd_list(int argc, char **argv)
{
    (void)argc; (void)argv;
    int count = 0;
    esp_err_t ret = kv_store_iterate(print_pair, &count);
    if (ret != ESP_OK) {
        printf("error: %s\n", esp_err_to_name(ret));
        return ret;
    }
    printf("(%d entries)\n", count);
    return ESP_OK;
}

static esp_err_t cmd_clear(int argc, char **argv)
{
    (void)argc; (void)argv;
    kv_store_clear();
    printf("OK\n");
    return ESP_OK;
}

static esp_err_t cmd_help(int argc, char **argv)
{
    (void)argc; (void)argv;
    printf("Available commands:\n");
    printf("  set <key> <value>   store a value\n");
    printf("  get <key>           print a value\n");
    printf("  del <key>           delete a key\n");
    printf("  list                print all pairs\n");
    printf("  clear               delete all pairs\n");
    printf("  help                this message\n");
    return ESP_OK;
}

esp_err_t cmd_kv_register_all(void)
{
    const struct { const char *name; const char *help; cli_cmd_fn_t fn; } cmds[] = {
        { "set",   "Store a value",    cmd_set   },
        { "get",   "Print a value",    cmd_get   },
        { "del",   "Delete a key",     cmd_del   },
        { "list",  "Print all pairs",  cmd_list  },
        { "clear", "Delete all pairs", cmd_clear },
        { "help",  "Show commands",    cmd_help  },
    };

    for (size_t i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++) {
        esp_err_t ret = cli_register_command(cmds[i].name, cmds[i].help, cmds[i].fn);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register '%s': %s",
                     cmds[i].name, esp_err_to_name(ret));
            return ret;
        }
    }
    return ESP_OK;
}
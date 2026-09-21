#include "cli.h"

#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "cli";

#define CLI_MAX_COMMANDS     16
#define CLI_TASK_STACK_SIZE  4096
#define CLI_TASK_PRIORITY    5
#define CLI_MAX_LINE_LEN  128
#define CLI_MAX_ARGS         8

typedef struct {
    const char  *name;
    const char  *help;
    cli_cmd_fn_t fn;
} cli_command_t;


static cli_command_t s_commands[CLI_MAX_COMMANDS];
static size_t        s_command_count = 0;
static TaskHandle_t  s_cli_task = NULL;

esp_err_t cli_register_command(const char *name, const char *help, cli_cmd_fn_t fn)
{
    if (name == NULL || fn == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (s_command_count >= CLI_MAX_COMMANDS) {
        ESP_LOGE(TAG, "Command table full, cannot register '%s'", name);
        return ESP_ERR_NO_MEM;
    }

    for (size_t i = 0; i < s_command_count; i++) {
        if ( strcmp(s_commands[i].name, name) == 0 ) {
            ESP_LOGW(TAG, "Command '%s' already registered", name);
            return ESP_ERR_INVALID_STATE;
        }
    }

    s_commands[s_command_count].name = name;
    s_commands[s_command_count].help = help;
    s_commands[s_command_count].fn   = fn;
    s_command_count++;

    ESP_LOGD(TAG, "Registered command '%s'", name);
    return ESP_OK;
}

/**
 * @brief Tokenize a command line in place.
 *
 * Splits `line` on spaces and tabs by replacing separators with NUL
 * terminators and storing pointers to each token in `argv`.
 *
 * @param line      Mutable input line. Modified in place.
 * @param argv      Output array of pointers to tokens.
 * @param max_args  Maximum number of tokens to store.
 * @return Number of tokens found (argc).
 */

static int cli_tokenize(char *line, char **argv, int max_args){
    int argc = 0;
    char *p = line;

    while(*p != '\0' && argc < max_args){

        while(*p == ' ' || *p == '\t'){
            p++;
        }

        if(*p == '\0'){
            break;
        }

        argv[argc++] = p;

        while(*p != '\0' && *p != ' ' && *p != '\t' ){
            p++;
        }

        /* Terminate the current token and advance past the separator.
        * The outer loop exits when *p is '\0' or argv is full. */
        if( *p != '\0' ){
            *p++ = '\0';
        }
    }

    return argc;
}

/**
 * @brief Look up and invoke a command.
 *
 * @param argc  Number of arguments.
 * @param argv  Argument vector (argv[0] is the command name).
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if unknown command.
 */

 static esp_err_t cli_dispatch(int argc, char **argv){

    if (argc == 0) {
        return ESP_OK;
    }

    for (size_t i = 0; i < s_command_count; i++) {
        if(strcmp(s_commands[i].name, argv[0]) == 0 ){
            return s_commands[i].fn(argc, argv);
        }
    }   

    ESP_LOGW(TAG, "Unknown command: '%s'", argv[0]);
    return ESP_ERR_NOT_FOUND;
 }

static void cli_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "CLI task started");

    static const char *test_lines[] = {
        "help",
        "set wifi.ssid MyNetwork",
        "unknown_command foo bar",
        "   set   spaced    out   ",
    };

    for (size_t i = 0; i < sizeof(test_lines) / sizeof(test_lines[0]); i++) {
        char buf[CLI_MAX_LINE_LEN];
        char *argv[CLI_MAX_ARGS];

        strncpy(buf, test_lines[i], sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        int argc = cli_tokenize(buf, argv, CLI_MAX_ARGS);
        ESP_LOGI(TAG, "Line: \"%s\" -> argc=%d", test_lines[i], argc);

        for (int j = 0; j < argc; j++) {
            ESP_LOGI(TAG, "  argv[%d] = \"%s\"", j, argv[j]);
        }

        cli_dispatch(argc, argv);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    ESP_LOGI(TAG, "Parser test complete, idling");

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

esp_err_t cli_init(void)
{
    ESP_LOGI(TAG, "Initializing CLI");
    s_command_count = 0;
    return ESP_OK;
}

esp_err_t cli_start(void)
{
    if (s_cli_task != NULL) {
        ESP_LOGW(TAG, "CLI task already running");
        return ESP_ERR_INVALID_STATE;
    }

    BaseType_t ret = xTaskCreate(
        cli_task,
        TAG,
        CLI_TASK_STACK_SIZE,
        NULL,
        CLI_TASK_PRIORITY,
        &s_cli_task
    );

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create CLI task");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}
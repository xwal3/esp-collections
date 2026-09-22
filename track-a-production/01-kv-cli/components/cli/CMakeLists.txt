idf_component_register(
    SRCS "cli.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_system
    PRIV_REQUIRES freertos esp_driver_uart
)
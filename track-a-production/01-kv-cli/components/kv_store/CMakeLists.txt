idf_component_register(
    SRCS "kv_store.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_system
    PRIV_REQUIRES freertos log
)

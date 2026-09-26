idf_component_register(
    SRCS "cmd_kv.c" "01-kv-cli.c"
    INCLUDE_DIRS "."
    REQUIRES cli kv_store
)

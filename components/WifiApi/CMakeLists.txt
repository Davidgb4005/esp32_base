idf_component_register(
    SRCS "WifiApi.cpp"
    INCLUDE_DIRS "."
    PRIV_REQUIRES 
        esp_wifi
        esp_event
        log
        nvs_flash
        lwip
        esp_system
)
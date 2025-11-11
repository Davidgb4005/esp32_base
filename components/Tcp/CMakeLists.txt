idf_component_register(
    SRCS "TcpApi.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        RingBuffer
    PRIV_REQUIRES 
        esp_wifi
        esp_event
        log
        nvs_flash
        lwip
        esp_system
        Debug


)
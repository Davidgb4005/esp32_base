idf_component_register(
    SRCS "TcpApi.cpp"
    INCLUDE_DIRS "."
    PRIV_REQUIRES 
        RingBuffer
        esp_wifi
        esp_event
        log
        nvs_flash
        lwip
        esp_system
        ErrorHandler
        Config


)
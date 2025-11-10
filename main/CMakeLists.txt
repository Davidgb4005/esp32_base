idf_component_register(SRCS "station_example_main.cpp"
                    PRIV_REQUIRES esp_wifi nvs_flash
                    PRIV_REQUIRES RingBuffer
                    PRIV_REQUIRES Tcp
                    INCLUDE_DIRS ".")

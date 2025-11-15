idf_component_register(SRCS "station_example_main.cpp"
                    PRIV_REQUIRES esp_wifi nvs_flash
                    PRIV_REQUIRES Tcp
                    PRIV_REQUIRES ErrorHandler
                    PRIV_REQUIRES Config       
                    PRIV_REQUIRES WifiApi                        
                    INCLUDE_DIRS ".")

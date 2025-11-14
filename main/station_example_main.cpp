
#include <iostream>
#include <string.h>
#include "RingBuffer.hpp"
#include "Debug.hpp"
#include "TcpApi.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_task_wdt.h"


// test
extern "C" void app_main(void)
{
    char test[] = "Hello";
    RingBuffer::Telegram message1{
        5,
        test,
        0
    };
    char buffer[255];
    int len = 0;
    TcpApi socket(
        "192.168.8.116",
        8090,          // TCP Port
        5000,          // rx_ring_buffer
        5000,          // tx_ring_buffer
        TcpApi::CLIENT // non_blocking
    );
    TcpApi::WifiConfigCheck();
    TcpApi::WifiInit("ESP32", "Pa55w0rd");

        while (1)
        {
            socket.tx_ring->WriteData(message1);
            socket.tx_ring->PrintMsg(message1);
            socket.TcpTask();
            vTaskDelay(300 / portTICK_PERIOD_MS);
            vTaskDelay(30);
        }
    
}
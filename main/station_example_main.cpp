/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <iostream>
#include <string.h>
#include "RingBuffer.hpp"
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

//test
extern "C" void app_main(void)
{
    TcpTaskParams parameters = {
        new RingBuffer(5000), // rx_ring_buffer
        new RingBuffer(5000), // tx_ring_buffer
        true                  // non_blocking
    };

    char message1[] = {5, 'H', 'E', 'L', 'L', 'O'};
    char message2[] = {4, 'T', 'H', 'I', 'S'};
    char message3[] = {2, 'I', 'S'};
    char message4[] = {5, 'W', 'O', 'R', 'D', 'S'};
    char message5[] = {12, 'L', 'O', 'N', 'G', 'M', 'E', 'S', 'S', 'A', 'G', 'E', '1'}; // longer message
    char message6[] = {7, 'N', 'E', 'W', ' ', 'M', 'S', 'G'};
    char message7[] = {3, 'B', 'Y', 'E'};

    // Fragmented messages
    char message8_part1[] = {10, 'F', 'R', 'A', 'G', 'M', 'E'}; // first part with length = 10
    char message8_part2[] = {'N', 'T', 'E', 'D'};               // continuation, no length byte

    char message9_part1[] = {16, 'V', 'E', 'R', 'Y', 'L', 'O', 'N', 'G', 'M', 'E', 'S'}; // first part
    char message9_part2[] = {'S', 'A', 'G', 'E', '2'};                                   // second part

    char message10[] = {4, 'T', 'E', 'S', 'T'};

    TcpApi::WifiConfigCheck();
    TcpApi::WifiInit("ESP32", "Pa55w0rd");
    xTaskCreate(TcpApi::TcpServerTask, "tcp_server_task", 4096, &parameters, 5, NULL);
    char buffer[255];
    int len;
    while (1)
    {
        if (parameters.tx_ring->DataAvailible() == 0 && parameters.rx_ring->DataAvailible() != 0)
        {
            // Write all messages and fragments to ring buffer
            parameters.tx_ring->WriteData(message1, sizeof(message1));
            parameters.tx_ring->WriteData(message2, sizeof(message2));
            parameters.tx_ring->WriteData(message3, sizeof(message3));
            parameters.tx_ring->WriteData(message4, sizeof(message4));
            parameters.tx_ring->WriteData(message5, sizeof(message5));
            parameters.tx_ring->WriteData(message6, sizeof(message6));
            parameters.tx_ring->WriteData(message7, sizeof(message7));

            parameters.tx_ring->WriteData(message8_part1, sizeof(message8_part1));
            parameters.tx_ring->WriteData(message8_part2, sizeof(message8_part2));
            parameters.tx_ring->WriteData(message9_part1, sizeof(message9_part1));
            parameters.tx_ring->WriteData(message9_part2, sizeof(message9_part2));

            parameters.tx_ring->WriteData(message10, sizeof(message10));
        }
        len = parameters.rx_ring->ReadData(buffer);
        if (len > 0)
        {
            RingBuffer::PrintMsg(buffer, len);
        }
        vTaskDelay(1);
    }
}
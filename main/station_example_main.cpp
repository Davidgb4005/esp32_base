
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

static void task(void *PvParameters)
{
    char buffer[255];
    int len = 0;
    TcpApi *socket = new TcpApi(
        "192.168.8.116",
        8090,          // TCP Port
        5000,          // rx_ring_buffer
        5000,          // tx_ring_buffer
        TcpApi::CLIENT // non_blocking
    );
    *((TcpApi **)PvParameters) = socket;
    while (1)
    {
        socket->TcpTask();
        vTaskDelay(5);
    }
}

// test
extern "C" void app_main(void)
{
    TcpApi *socket = nullptr;

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
    xTaskCreate(task, "tcp_server_task", 4096, &socket, 5, NULL);
    char buffer[255];
    int len = 0;
    bool flip_bit = true;
    if (1)
    {
        while (socket == nullptr)
        {
            vTaskDelay(1);
        }
        while (1)
        {
            if ((socket->tx_ring->DataAvailible() == 0 && socket->rx_ring->DataAvailible() != 0) && 0)
            {
                // Write all messages and fragments to ring buffer
                socket->tx_ring->WriteData(message1, sizeof(message1));
                socket->tx_ring->WriteData(message2, sizeof(message2));
                socket->tx_ring->WriteData(message3, sizeof(message3));
                socket->tx_ring->WriteData(message4, sizeof(message4));
                socket->tx_ring->WriteData(message5, sizeof(message5));
                socket->tx_ring->WriteData(message6, sizeof(message6));
                socket->tx_ring->WriteData(message7, sizeof(message7));

                socket->tx_ring->WriteData(message8_part1, sizeof(message8_part1));
                socket->tx_ring->WriteData(message8_part2, sizeof(message8_part2));
                socket->tx_ring->WriteData(message9_part1, sizeof(message9_part1));
                socket->tx_ring->WriteData(message9_part2, sizeof(message9_part2));

                socket->tx_ring->WriteData(message10, sizeof(message10));
                flip_bit = false;
            }
            //len = socket->rx_ring->ReadData(buffer);
            if (len > 0)
            {
                if (buffer[2] == 49 && 0)
                {
                    // socket->ResetSocket();
                }
                socket->tx_ring->WriteData(message1, sizeof(message1));
                RingBuffer::PrintMsg(buffer, len);
            }
            socket->tx_ring->WriteData(message1, sizeof(message1));
            vTaskDelay(300 / portTICK_PERIOD_MS);
            vTaskDelay(30);
        }
    }
}
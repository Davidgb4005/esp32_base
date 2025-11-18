/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <iostream>
#include "WifiApi.hpp"
#include "RingBuffer.hpp"
#include "TcpApi.hpp"
// test
static void TcpThread(void *PvParameters)
{
#if 1
    TcpApi *tcp_task = static_cast<TcpApi *>(PvParameters);
    WifiConfigCheck();
    WifiInit("ESP32", "Pa55w0rd");
    while (1)
    {
        if (WifiConnected() == false)
        {
            WifiConnect();
        }
        else if (!tcp_task->SocketActive())
        {
            std::cout << "Trying Cocket" << std::endl;
            tcp_task->ClientInit();
        }

        else
        {
            tcp_task->EnableBlocking(false);
            while (tcp_task->SocketActive())
            {
                if (tcp_task->tx_data->data_ready)
                {
                    std::cout << "Sending" << std::endl;
                    tcp_task->TcpTaskSend(tcp_task->tx_data);
                }
                if (!(tcp_task->rx_data->data_ready))
                {
                    std::cout << "Reading" << std::endl;
                    tcp_task->TcpTaskRecv(tcp_task->rx_data);
                }
                vTaskDelay(50);
            }
        }
    }
#endif
}

extern "C" void app_main(void)
{

    TcpBuffer *rx_data = new RingBuffer;
    TcpBuffer *tx_data = new RingBuffer;
    TcpApi *tcp_task = new TcpApi(rx_data, tx_data, "192.168.8.116", 8090, CLIENT);
    xTaskCreate(TcpThread, "Tcp Task", 4096, tcp_task, 5, NULL);
    int i = 0;
    while (1)
    {
        if (!(tcp_task->tx_data->data_ready))
        {
            tx_data->data[0] = i & 0xff;
            tx_data->data[1] = i >> 8;
            tx_data->data_ready = true;
            tx_data->msg_len = 2;
            i++;
        }
        if (tcp_task->rx_data->data_ready)
        {
            std::cout << "Printing Data Out" << std::endl;
            for (int k = 0; k < rx_data->msg_len; k++)
            {
                std::cout << rx_data->data[k];
            }
            std::cout << std::endl;
            rx_data->data_ready = false;
        }
        vTaskDelay(50);
    }
}
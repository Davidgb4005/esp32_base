/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <iostream>
#include "WifiApi.hpp"
#include "TcpApi.hpp"
// test
static void TcpThread(void *PvParameters)
{
#if 1
    TcpApi *tcp_task = static_cast<TcpApi *>(PvParameters);
    WifiConfigCheck();
    WifiInit("ESP32", "Pa55w0rd");
    int do_once = 0;
    while (1)
    {
        std::cout << "THIS IS THE TEST" << std::endl;
        if (WifiConnected() == false)
        {
            WifiConnect();
        }
        else if (!tcp_task->SocketActive())
        {
            std::cout << "Trying Cocket" << std::endl;
            tcp_task->ClientInit();
            do_once = 1;
        }

        else
        {
            char testbuffer[256];
            int testlen = 0;
            tcp_task->EnableBlocking(false);
            tcp_task->tx_buffer->ResetBuffer();
            tcp_task->rx_buffer->ResetBuffer();
            recv(tcp_task->sock,testbuffer, sizeof(testbuffer) - 1, 0);
             while (tcp_task->socket_active)
            {
                // vTaskDelay(pdMS_TO_TICKS(10));
                tcp_task->TcpTask();
                if (tcp_task->rx_buffer->BufferFull() || true)
                {

                    testlen = tcp_task->rx_buffer->ReadData(testbuffer);
                    if (testlen < 0)
                    {
                        // std::cout << testlen << std::endl;
                    }
                    else
                    {
                        // std::cout << testlen << std::endl;
                        //  tcp_task->rx_buffer->PrintData();
                        //  tcp_task->tx_buffer->WriteStruct(testbuffer);
                        for (int k = 1; k < testlen; k++)
                        {
                            std::cout << testbuffer[k];
                        }
                        std::cout << std::endl;
                        testlen = -1;
                    }
                }
            }
        }
    }
#endif
}

extern "C" void app_main(void)
{

    TcpApi *tcp_task = new TcpApi(5000, 5000, "192.168.8.116", 8090, CLIENT);
    xTaskCreate(TcpThread, "Tcp Task", 4096, tcp_task, 5, NULL);
}
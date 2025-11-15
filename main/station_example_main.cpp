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
extern "C" void app_main(void)
{

    WifiConfigCheck();
    WifiInit("ESP32", "Pa55w0rd");
    TcpApi tcp_task(30, 30, "192.168.8.116", 8090, CLIENT);
    while (true)
    {
        if (WifiConnected() == false)
        {
            WifiConnect();
        }
        else if (!tcp_task.SocketActive())
        {
            std::cout << "Trying Cocket" << std::endl;
            tcp_task.ClientInit();
        }
        else{
            tcp_task.EnableBlocking(false);
            tcp_task.TcpTask();
            
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
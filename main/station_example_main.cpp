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

enum MessageType
{
    STEPPER_MOTOR = 45,
    ENCODER = 34,
    CLAMP = 78
};

struct StepperMotorTelegram : Telegram
{
    int speed;
    int direction;
    bool active;
    char mode;
    int time;
    StepperMotorTelegram()
    {
        type = STEPPER_MOTOR;
        address = 88;                                                        // Remove Null Termination Char
        len_msb = static_cast<uint8_t>(sizeof(StepperMotorTelegram)) >> 8;   // Add Length for Base Telegram info
        len_lsb = static_cast<uint8_t>(sizeof(StepperMotorTelegram)) & 0xff; // Add Length for Base Telegram info
    }
};

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
            uint16_t offset = 0;
            while (tcp_task->SocketActive())
            {
                if (tcp_task->tx_data->MessageAvailible())
                {
                    std::cout << "Sending" << std::endl;
                    tcp_task->tx_data->PrintData();
                    tcp_task->TcpTaskSend();
                }
                if (tcp_task->rx_data->MessageAvailible())
                {
                    std::cout << "Reading" << std::endl;
                    offset = tcp_task->TcpTaskRecv(offset);
                }
                vTaskDelay(50);
            }
        }
    }
#endif
}
    StepperMotorTelegram stp_in;
extern "C" void app_main(void)
{


    stp_in.speed = 71;
    stp_in.direction = 72;
    stp_in.active = true;
    stp_in.time = 73;
    stp_in.mode = 'c';
    if (0)
    {
        StepperMotorTelegram stp_out;
        stp_in.speed = 1231;
        stp_in.direction = 423;
        stp_in.active = false;
        stp_in.time = 432;
        stp_in.mode = 'c';
    }
    RingBuffer *rx_data = new RingBuffer(512);
    RingBuffer *tx_data = new RingBuffer(512);
    TcpApi *tcp_task = new TcpApi(rx_data, tx_data, "192.168.8.116", 8090, CLIENT);
    xTaskCreate(TcpThread, "Tcp Task", 4096, tcp_task, 5, NULL);
    int result = 0;
    while (1)
    {
        if (tcp_task->tx_data->BytesRemaining() > sizeof(stp_in))
        {
            result = tcp_task->tx_data->Write(&stp_in);
            tx_data->PrintData();
        }
        if (tcp_task->rx_data->MessageAvailible())
        {
            // result = tcp_task->rx_data->Read(&stp_out);
        }
        vTaskDelay(50);
    }
}
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
#include <iostream>

TcpApi::TcpApi(int tx_buffer_size, int rx_buffer_size,
               const char *ip_addr, int port,
               ConnectionType socket_type)
{
    this->rx_buffer = new RingBuffer(rx_buffer_size);
    this->tx_buffer = new RingBuffer(tx_buffer_size);
    this->ip_addr = ip_addr;
    this->port = port;
    this->connection_type = connection_type;
}
TcpApi::~TcpApi()
{
    delete[] rx_buffer;
    delete[] tx_buffer;
}
void TcpApi::ClientInit()
{
    if (sock < 0)
    {
        struct sockaddr_in server_addr{};
        socklen_t addr_len = sizeof(server_addr);

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(ip_addr);
        server_addr.sin_port = htons(port);

        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0)
        {
            std::cout << "Client Socket Error" << errno << std::endl;
        }

        if (connect(sock, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0)
        {
            std::cout << "Client Connection Error" << errno << std::endl;
            CloseSocket();
        }
        else
            socket_active = true;
        std::cout << "Client Connected " << sock << std::endl;
    }
    else
    {
        std::cout << "TCP Object Already Has Socket" << errno << std::endl;
    }
}
void TcpApi::CloseSocket()
{
    if (sock < 0)
    {
        // std::cout << "No Active Socket to close. sock=" << sock << std::endl;
        return;
    }
    else
    {
        int status = close(sock);
        if (status < 0)
        {
            std::cout << "Failed to close socket: errno=" << errno << std::endl;
            socket_active = false;
        }
        else
        {
            std::cout << "Socket closed: sock=" << sock << std::endl;
            socket_active = false;
            sock = -1;
        }
    }
}

void TcpApi::TcpTask()
{
    char temp_rx_buffer[256];
    char temp_tx_buffer[256];
    int rx_len = 0;
    int tx_len = 0;
    while (socket_active)
    {
        rx_len = 0;
        tx_len = 0;

        if (!rx_buffer->BufferFull())
        {
            rx_len = recv(sock, temp_rx_buffer, sizeof(temp_rx_buffer) - 1, 0);
            if (rx_len < 0 && !(errno == EAGAIN || errno == EWOULDBLOCK))
            {
                CloseSocket();
            }
            else
            {
                rx_len = rx_buffer->WriteData(temp_rx_buffer, rx_len);
                if (rx_len < 0)
                {
                    rx_buffer->ResetBuffer();
                }
            }
        }
        if (tx_buffer->DataAvailible() > 0)
        {
            tx_len = tx_buffer->ReadData(temp_tx_buffer);

            if (tx_len < 0)
            {
                tx_buffer->ResetBuffer();
            }
            else if (send(sock, temp_tx_buffer, tx_len, 0) < 0)
            {
                CloseSocket();
            }
        }

        char testbuffer[256];
        int testlen;
        rx_buffer->PrintData();
        testlen = Read(testbuffer);
        if (testlen < 0)
        {
            //std::cout << testlen << std::endl;
        }
        else
        {
            
            tx_buffer->WriteData(testbuffer, testlen);
            RingBuffer::PrintMsg(testbuffer, testlen);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void TcpApi::Send(char *buffer, int len)
{
    char temp_buffer[256];
    for (int i = 0; i < len; i++)
    {
        temp_buffer[i + 1] = buffer[i];
    }
    temp_buffer[0] = len;
    tx_buffer->WriteData(temp_buffer, len + 1);
}
int TcpApi::Read(char *buffer)
{
    return rx_buffer->ReadData(buffer);
}
bool TcpApi::SocketActive()
{
    return socket_active;
}

void TcpApi::EnableBlocking(bool blocking)
{
    if (blocking)
    {
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
        blocking = true;
    }
    else
    {
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
        blocking = false;
    }
}
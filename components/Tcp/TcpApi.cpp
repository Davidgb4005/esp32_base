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

TcpApi::TcpApi(TcpBuffer * rx_data,TcpBuffer * tx_data,
               const char *ip_addr, int port,
               ConnectionType socket_type)
{
    this->rx_data = rx_data;
    this->tx_data = tx_data;
    this->ip_addr = ip_addr;
    this->port = port;
    this->connection_type = connection_type;
}
TcpApi::~TcpApi()
{

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

int TcpApi::TcpTaskSend(TcpBuffer *buffer)
{
    buffer->msg_len = send(sock, buffer->data, buffer->msg_len, 0);
    if (buffer->msg_len < 0 && 0)
    {
        return -1;
    }
    else
    {
        buffer->data_ready = false;
        return 1;
    }
}
int TcpApi::TcpTaskRecv(TcpBuffer *buffer)
{
    buffer->msg_len = recv(sock, buffer->data, sizeof(buffer->data) - 1, 0);
    if (buffer->msg_len < 0 && 0)
    {
        return -1;
    }
    else
    {
        buffer->data_ready = true;
        return 1;
    }
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


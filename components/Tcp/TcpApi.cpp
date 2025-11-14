/**
 * @file TcpApi.cpp
 * @brief Implementation of the TcpApi class for Wi-Fi initialization, TCP server, and TCP client handling on ESP32.
 * @details
 * This module provides functionality to:
 *  - Initialize Wi-Fi in station mode.
 *  - Handle Wi-Fi events (connect, disconnect, got IP).
 *  - Create and manage TCP server and client sockets.
 *  - Transmit and receive data via FreeRTOS tasks.
 */

#include "TcpApi.hpp"
#include "Debug.hpp"
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
#include <fcntl.h>
#include <cstring>

/// Event group to signal Wi-Fi connection status
static EventGroupHandle_t s_wifi_event_group;

/// Event bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

/// Socket blocking behavior
#define SOCKET_BLOCKING false

/// Wi-Fi log tag
static const char *TAG = "wifi_station";

/// Retry counter for Wi-Fi connection attempts
static int s_retry_num = 0;

// ============================================================================
//                              Constructors / Destructors
// ============================================================================

TcpApi::TcpApi(const char *ip_addr, int port, int tx_buffer_size, int rx_buffer_size, int socket_type)
{

    this->ip_addr = ip_addr;
    this->port = port;
    rx_ring = new RingBuffer(rx_buffer_size);
    tx_ring = new RingBuffer(tx_buffer_size);
    this->socket_type = socket_type;
}

TcpApi::~TcpApi()
{
}

void TcpApi::WifiConfigCheck(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

static void EventHandler(void *arg, esp_event_base_t event_base,
                         int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < 5)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "Failed to connect to the AP");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = static_cast<ip_event_got_ip_t *>(event_data);
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * @brief Initializes Wi-Fi in Station mode and connects to an access point.
 * @param[in] ssid The SSID of the target Wi-Fi network.
 * @param[in] password The password of the Wi-Fi network.
 */
void TcpApi::WifiInit(const char *ssid, const char *password)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &EventHandler,
                                                        nullptr,
                                                        &instance_any_id));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &EventHandler,
                                                        nullptr,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {};
    strncpy(reinterpret_cast<char *>(wifi_config.sta.ssid), ssid, sizeof(wifi_config.sta.ssid));
    strncpy(reinterpret_cast<char *>(wifi_config.sta.password), password, sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi Station Mode Initialized.");

    // Wait for Wi-Fi connection
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "Connected to SSID:%s with password:%s", ssid, password);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s with password:%s", ssid, password);
    }
    else
    {
        ESP_LOGE(TAG, "Unexpected event");
    }
}

void TcpApi::ServerSocket()
{
    struct sockaddr_in server_addr{}, client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = (ip_addr && strlen(ip_addr) > 0)
                                      ? inet_addr(ip_addr)
                                      : INADDR_ANY;
    server_addr.sin_port = htons(port);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0)
    {
        PrintError("Creating", errno);
    }

    if (bind(sock, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0)
    {
        PrintError("Binding", errno);
        close(sock);
    }

    if (listen(sock, 10) < 0)
    {
        PrintError("Listening", errno);
        close(sock);
    }

    sock = accept(sock, reinterpret_cast<struct sockaddr *>(&client_addr), &addr_len);

    if (sock < 0)
    {
        PrintError("Accepting", errno);
        close(sock);
    }
}

void TcpApi::ClientSocket()
{
    struct sockaddr_in server_addr{};
    socklen_t addr_len = sizeof(server_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    server_addr.sin_port = htons(port);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 1)
    {
        PrintError("Socket Creation", errno);
    }

    if (connect(sock, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 1)
    {
        PrintError("Socket Connection", errno);
        close(sock);
    }
    PrintReport("Socket Connected: ", sock);
}

void TcpApi::BeginSocket()
{
    reset_socket = false;
    vTaskDelay(300 / portTICK_PERIOD_MS);
    if (sock > 0)
    {
        PrintReport("Already Active Socket: ", sock);
    }
    else
    {
        switch (socket_type)
        {
        case SERVER:
            PrintReport("Starting Server - IP: ", ip_addr, " Port: ", port);
            ServerSocket();
            if (sock < 1)
                PrintReport("Server Not Ready");
            else
                PrintReport("Server Ready");
            break;
        case CLIENT:
            PrintReport("Client Connecting To - IP: ", ip_addr, " Port: ", port);
            ClientSocket();
            if (sock < 1)
                PrintReport("Client Not Ready");
            else
                PrintReport("Client Ready");
            break;
        default:
            sock = -1;
            PrintReport("Invalid Socket Type Check SOCKET_TYPE Enum For Details");
            break;
        }
    }
}
void TcpApi::CloseSocket()
{
    if (sock <= 1)
    {
        PrintReport("No Active Socket");
    }
    else
    {
        if (1)
        {
            close(sock);
            rx_ring->ResetBuffer();
            tx_ring->ResetBuffer();
            sock = -1;
            switch (socket_type)
            {
            case SERVER:
                PrintReport("Closing Server Socket: ", sock);
                break;
            case CLIENT:
                PrintReport("Closing Client Socket: ", sock);
                break;
            default:
                PrintReport("Invalid Socket Type Check SOCKET_TYPE Enum For Details");
                break;
            }
        }
        else
        {
            PrintReport("Failed To Close Socket: ", sock);
        }
    }
}
void TcpApi::SetBlocking(bool blocking)
{
    if (blocking)
    {
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
    }
    else
    {
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    }
}

int TcpApi::Read(int status)
{
    RingBuffer::Telegram data{
        0,
        new char[255],
        0
    };
    int error_code = 0;
    if (status != RingBuffer::BUFFER_FULL)
    {
        data.message_length = recv(sock, data.message, sizeof(data.message) - 1, 0);
        if (data.message_length < 1)
        {
            PrintError("Socket Revc Error: ", errno);
            PrintReport("Socket Revc Error:", error_code);
            CloseSocket();
        }
        else
            status = rx_ring->WriteData(data);
    }
    return status;
}
int TcpApi::Send(int status)
{
    RingBuffer::Telegram data{
        0,
        new char[255],
        0
    };
    if (tx_ring->DataAvailible() > 0)
    {
        tx_ring->ReadData(data);
    }
        send(sock, data.message, data.message_length, 0);
    return status;
}

void TcpApi::TcpTask()
{
    int rx_status = 0, tx_status = 0;
    if (sock <= 0)
    {
        BeginSocket();
    }
    else
    {
        Send(tx_status);
    }
}

void TcpApi::ResetSocket()
{
    reset_socket = true;
}

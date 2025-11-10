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
#include "ErrorHandler.hpp"
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
#include <iostream>
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

TcpApi::TcpApi(const char *ip_addr, int port, RingBuffer *tx_ring, RingBuffer *rx_ring, bool non_blocking)
{

    parameters.ip_addr = ip_addr;
    parameters.port = port;
    parameters.tx_ring = tx_ring;
    parameters.rx_ring = rx_ring;
    parameters.non_blocking = non_blocking;
}

TcpApi::~TcpApi() = default;

// ============================================================================
//                              Wi-Fi Configuration
// ============================================================================

/**
 * @brief Ensures NVS flash is initialized before using Wi-Fi.
 */
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

/**
 * @brief Event handler for Wi-Fi and IP events.
 */
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

// ============================================================================
//                              Socket Handling
// ============================================================================

/**
 * @brief Creates a TCP server socket and waits for a client connection.
 * @param[in] ip_addr The IP address to bind the server to (use nullptr for INADDR_ANY).
 * @param[in,out] port Reference to the port to listen on.
 * @return The accepted socket descriptor, or -1 on failure.
 */
int TcpApi::AttachSocket()
{
    int listen_sock, accept_sock;
    struct sockaddr_in server_addr{}, client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = (parameters.ip_addr && strlen(parameters.ip_addr) > 0)
                                      ? inet_addr(parameters.ip_addr)
                                      : INADDR_ANY;
    server_addr.sin_port = htons(parameters.port);

    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0)
    {
        PrintError("Creating", errno);
        return -1;
    }

    if (bind(listen_sock, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0)
    {
        PrintError("Binding", errno);
        close(listen_sock);
        return -1;
    }

    if (listen(listen_sock, 10) < 0)
    {
        PrintError("Listening", errno);
        close(listen_sock);
        return -1;
    }

    accept_sock = accept(listen_sock, reinterpret_cast<struct sockaddr *>(&client_addr), &addr_len);
    close(listen_sock);

    if (!SOCKET_BLOCKING)
    {
        int flags = fcntl(accept_sock, F_GETFL, 0);
        fcntl(accept_sock, F_SETFL, flags | O_NONBLOCK);
    }

    if (accept_sock < 0)
    {
        PrintError("Accepting", errno);
        return -1;
    }

    return accept_sock;
}

/**
 * @brief Connects to a TCP server socket.
 * @param[in] ip_addr The server's IP address.
 * @param[in,out] port Reference to the server port.
 * @return The connected socket descriptor, or -1 on failure.
 */
int TcpApi::ConnectSocket()
{
    int client_socket;
    struct sockaddr_in server_addr{};
    socklen_t addr_len = sizeof(server_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(parameters.ip_addr);
    server_addr.sin_port = htons(parameters.port);

    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (client_socket < 0)
    {
        PrintError("Socket Creation", errno);
        return -1;
    }

    if (connect(client_socket, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0)
    {
        PrintError("Socket Connection", errno);
        close(client_socket);
        return -1;
    }

    if (!SOCKET_BLOCKING)
    {
        int flags = fcntl(client_socket, F_GETFL, 0);
        fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);
    }

    return client_socket;
}

// ============================================================================
//                              Server Task
// ============================================================================

/**
 * @brief FreeRTOS task that manages TCP server communication.
 * @param[in] PvParameters Pointer to a TcpTaskParams struct.
 */
void TcpApi::TcpServerTask(void *PvParameters)
{
    TcpApi *instance = static_cast<TcpApi *>(PvParameters);

    char rx_buffer[256], tx_buffer[256];
    int rx_len = 0, tx_len = 0, bytes_sent = 0;
    int rx_buffer_status = 0, tx_buffer_status = 0;

    int accept_sock = instance->AttachSocket();

    // Lambda for closing the socket safely
    auto CloseSocket = [=, &accept_sock]()
    {
        ESP_LOGI(TAG, "Client disconnected");
        close(accept_sock);
        instance->parameters.tx_ring->ResetBuffer();
        instance->parameters.rx_ring->ResetBuffer();
        accept_sock = 0;
    };

    if (instance->parameters.non_blocking)
    {
        int flags = fcntl(accept_sock, F_GETFL, 0);
        fcntl(accept_sock, F_SETFL, flags | O_NONBLOCK);
    }

    while (true)
    {
        if (accept_sock > 0)
        {
            if (rx_buffer_status != RingBuffer::BUFFER_FULL)
                rx_len = recv(accept_sock, rx_buffer, sizeof(rx_buffer) - 1, 0);

            if (rx_len > 0)
                rx_buffer_status = instance->parameters.rx_ring->WriteData(rx_buffer, rx_len);
            else if (rx_len == 0 || rx_buffer_status <= RingBuffer::UNEXPECTED_ERROR)
                CloseSocket();

            if (bytes_sent > 0 || 1) // TODO: Replace condition
                tx_len = instance->parameters.tx_ring->ReadData(tx_buffer);

            if (tx_buffer_status <= RingBuffer::UNEXPECTED_ERROR)
                CloseSocket();

            if (tx_len > 0)
            {
                bytes_sent = send(accept_sock, tx_buffer, tx_len, 0);
                if (bytes_sent <= 1)
                    CloseSocket();
            }
            if (instance->SetSocket())
            {
                CloseSocket();
            }
        }
        else
        {

            ESP_LOGI(TAG, "No active connection. Re-establishing socket...");
            accept_sock = instance->AttachSocket();
            if (accept_sock < 0)
            {
                ESP_LOGE(TAG, "Failed to re-establish socket");
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
            else
            {
                ESP_LOGI(TAG, "New connection established");
            }
        }
        vTaskDelay(1);
    }
}

// ============================================================================
//                              Client Task
// ============================================================================

/**
 * @brief FreeRTOS task that manages TCP client communication.
 * @param[in] PvParameters Pointer to a TcpTaskParams struct.
 */
void TcpApi::TcpClientTask(void *PvParameters)
{
    TcpApi *instance = static_cast<TcpApi *>(PvParameters);
    char empty_buffer[1] = {0};
    char rx_buffer[256], tx_buffer[256];
    int rx_len = 0, tx_len = 0, bytes_sent = 0;
    int rx_buffer_status = 0, tx_buffer_status = 0;

    int client_socket = instance->ConnectSocket();

    auto CloseSocket = [=, &client_socket]()
    {
        if (client_socket > 0)
        {
            ESP_LOGI(TAG, "Server disconnected");
            close(client_socket);
            instance->parameters.tx_ring->ResetBuffer();
            instance->parameters.rx_ring->ResetBuffer();
            client_socket = -1;
        }
    };
    auto ShutDownSocket = [=, &client_socket]()
    {
        if (client_socket > 0)
        {
            shutdown(client_socket, SHUT_RDWR);
            ESP_LOGI(TAG, "Server disconnected");
            close(client_socket);
            instance->parameters.tx_ring->ResetBuffer();
            instance->parameters.rx_ring->ResetBuffer();
            client_socket = -1;
        }
    };

    if (instance->parameters.non_blocking)
    {
        int flags = fcntl(client_socket, F_GETFL, 0);
        fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);
    }

    while (true)
    {
        if (client_socket > 0)
        {
            if (rx_buffer_status != RingBuffer::BUFFER_FULL and client_socket > 0)
            {
                rx_len = recv(client_socket, rx_buffer, sizeof(rx_buffer) - 1, 0);
            }

            if (rx_len > 0)
            {
                rx_buffer_status = instance->parameters.rx_ring->WriteData(rx_buffer, rx_len);
            }
            else if (rx_len == 0 || rx_buffer_status <= RingBuffer::UNEXPECTED_ERROR)
            {
                CloseSocket();
            }

            if (bytes_sent > 0 || 1) // TODO: Replace condition
            {
                tx_len = instance->parameters.tx_ring->ReadData(tx_buffer);
            }

            if (tx_buffer_status <= RingBuffer::UNEXPECTED_ERROR)
            {
                CloseSocket();
            }

            if (tx_len > 0 and client_socket > 0)
            {
                bytes_sent = send(client_socket, tx_buffer, tx_len, 0);
                if (bytes_sent <= 1)
                    CloseSocket();
            }
            if (instance->SetSocket())
            {
                ShutDownSocket();
            }
        }
        else
        {
            ESP_LOGI(TAG, "No active connection. Re-establishing socket...");
            if (client_socket < 0)
            {
                ESP_LOGE(TAG, "Failed to re-establish socket");
                vTaskDelay(300 / portTICK_PERIOD_MS);
                client_socket = instance->ConnectSocket();
            }
            else
            {
                ESP_LOGI(TAG, "New connection established");
            }
        }
        vTaskDelay(1);
    }
}

void TcpApi::ResetSocket()
{
    reset_socket = true;
}
bool TcpApi::SetSocket()
{
    if (reset_socket)
    {
        std::cout<<"RESETING"<<std::endl;
        reset_socket = false;
        return true;
    }
    return reset_socket;
}

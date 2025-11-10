#include "TcpApi.hpp"
#include "ErrorHandler.hpp"
#include <fcntl.h>
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

static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define SOCKET_BLOCKING false
static const char *TAG = "wifi station";
static int s_retry_num = 0;

TcpApi::TcpApi(/* args */)
{
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
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}
// Wi-Fi initialization for station mode

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
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &EventHandler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi Station Mode Initialized.");
    // Wait until either the connection is established or fails
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

int TcpApi::AttachSocket(const char *ip_addr, int &port)
{
    int listen_sock, accept_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (ip_addr && strlen(ip_addr) > 0)
    {
        server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    }
    else
    {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    }
    server_addr.sin_port = htons(port); // Define your server port here

    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0)
    {
        PrintError("Creating", errno);
        return -1; // Error creating socket
    }

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        PrintError("Binding", errno);
        close(listen_sock);
        return -1; // Error binding
    }

    if (listen(listen_sock, 10) < 0)
    {
        PrintError("Listening", errno);
        close(listen_sock);
        return -1; // Error listening
    }

    // Accept the connection
    accept_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
    close(listen_sock); // Close the listening socket after accepting a connection
    // If socket blocking is active if true, non blocking if flase
    if (!SOCKET_BLOCKING)
    {
        int flags = fcntl(accept_sock, F_GETFL, 0);
        fcntl(accept_sock, F_SETFL, flags | O_NONBLOCK);
    }
    if (accept_sock < 0)
    {
        PrintError("Accepting", errno);
        return -1; // Error accepting
    }

    return accept_sock; // Return the accepted socket
}

int TcpApi::ConnectSocket(const char *ip_addr, int &port)
{
    int client_socket, connection_error;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    server_addr.sin_port = htons(port); // Define your server port here

    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (client_socket < 0)
    {
        PrintError("Socket Creation", errno);
        return -1; // Error creating socket
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        PrintError("Socket Binding", errno);
        close(client_socket);
        return -1; // Error binding
    }
    // If socket blocking is active if true, non blocking if flase
    if (!SOCKET_BLOCKING)
    {
        int flags = fcntl(client_socket, F_GETFL, 0);
        fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);
    }
    if (client_socket < 0)
    {
        PrintError("Accepting", errno);
        return -1; // Error accepting
    }

    return client_socket; // Return the accepted socket
}

void TcpApi::TcpServerTask(void *PvParameters)
{

    TcpTaskParams *parameters = (TcpTaskParams *)PvParameters;
    const char *ip_addr = parameters->ip_addr;
    int port = parameters->port;
    RingBuffer *rx_ring_buffer = parameters->rx_ring;
    RingBuffer *tx_ring_buffer = parameters->tx_ring;
    bool non_blocking = parameters->non_blocking;

    char rx_buffer[256];
    int rx_len = 0;
    int rx_buffer_status = 0;
    char tx_buffer[256];
    int tx_len = 0;
    int tx_buffer_status = 0;
    int accept_sock = AttachSocket(ip_addr, port);
    int bytes_sent = 0; // to do
    // Closure for closing socket
    auto CloseSocket = [tx_ring_buffer, rx_ring_buffer, &accept_sock]()
    {
        ESP_LOGI(TAG, "Client disconnected");
        close(accept_sock);
        tx_ring_buffer->ResetBuffer();
        rx_ring_buffer->ResetBuffer();
        accept_sock = 0;
    };

    if (non_blocking)
    {
        int flags = fcntl(accept_sock, F_GETFL, 0);
        fcntl(accept_sock, F_SETFL, flags | O_NONBLOCK);
    }
    while (1)
    {
        if (accept_sock > 0)
        {
            if (rx_buffer_status != RingBuffer::BUFFER_FULL)
            {
                rx_len = recv(accept_sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            }
            if (rx_len > 0)
            {
                rx_buffer_status = rx_ring_buffer->WriteData(rx_buffer, rx_len);
            }
            else if (rx_len == 0 or rx_buffer_status <= RingBuffer::UNEXPECTED_ERROR)
            {
                CloseSocket();
            }
            if (bytes_sent > 0 || 1) // TO DO
            {
                tx_len = tx_ring_buffer->ReadData(tx_buffer);
            }
            if (tx_buffer_status <= RingBuffer::UNEXPECTED_ERROR)
            {
                CloseSocket();
            }
            if (tx_len > 0)
            {
                bytes_sent = send(accept_sock, tx_buffer, tx_len, 0);
                if (bytes_sent <= 1)
                {
                    CloseSocket();
                }
            }
        }
        else
        {
            ESP_LOGI(TAG, "No active connection. Re-establishing socket...");

            accept_sock = AttachSocket(ip_addr, port); // Try to accept a new connection
            if (accept_sock < 0)
            {
                ESP_LOGE(TAG, "Failed to re-establish socket");
                vTaskDelay(100);
            }
            else
            {
                ESP_LOGI(TAG, "New connection established");
            }
        }
        vTaskDelay(1);
    }
}

void TcpApi::TcpClientTask(void *PvParameters)
{
    TcpTaskParams *parameters = (TcpTaskParams *)PvParameters;
    const char *ip_addr = parameters->ip_addr;
    int port = parameters->port;
    RingBuffer *rx_ring_buffer = parameters->rx_ring;
    RingBuffer *tx_ring_buffer = parameters->tx_ring;
    bool non_blocking = parameters->non_blocking;

    char rx_buffer[256];
    int rx_len = 0;
    int rx_buffer_status = 0;
    char tx_buffer[256];
    int tx_len = 0;
    int tx_buffer_status = 0;
    int client_socket = ConnectSocket(ip_addr, port);
    int bytes_sent =0 ;// TO DO
    auto CloseSocket = [tx_ring_buffer, rx_ring_buffer, &client_socket]()
    {
        ESP_LOGI(TAG, "Server disconnected");
        close(client_socket);
        tx_ring_buffer->ResetBuffer();
        rx_ring_buffer->ResetBuffer();
        client_socket = 0;
    };

    if (non_blocking)
    {
        int flags = fcntl(client_socket, F_GETFL, 0);
        fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);
    }
    while (1)
    {
        if (client_socket > 0)
        {
            if (rx_buffer_status != RingBuffer::BUFFER_FULL)
            {
                rx_len = recv(client_socket, rx_buffer, sizeof(rx_buffer) - 1, 0);
            }
            if (rx_len > 0)
            {
                rx_buffer_status = rx_ring_buffer->WriteData(rx_buffer, rx_len);
            }
            else if (rx_len == 0 or rx_buffer_status <= RingBuffer::UNEXPECTED_ERROR)
            {
                CloseSocket();
            }
            if (bytes_sent > 0 || 1) // TO DO
            {
                tx_len = tx_ring_buffer->ReadData(tx_buffer);
            }
            if (tx_buffer_status <= RingBuffer::UNEXPECTED_ERROR)
            {
                CloseSocket();
            }
            if (tx_len > 0)
            {
                bytes_sent = send(client_socket, tx_buffer, tx_len, 0);
                if (bytes_sent <= 1)
                {
                    CloseSocket();
                }
            }
        }
        else
        {
            ESP_LOGI(TAG, "No active connection. Re-establishing socket...");

            client_socket = ConnectSocket(ip_addr, port); // Try to accept a new connection
            if (client_socket < 0)
            {
                ESP_LOGE(TAG, "Failed to re-establish socket");
                vTaskDelay(100);
            }
            else
            {
                ESP_LOGI(TAG, "New connection established");
            }
        }
        vTaskDelay(1);
    }
}

#pragma once
#include "RingBuffer.hpp"
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


struct TcpTaskParams
{
    RingBuffer *rx_ring;
    RingBuffer *tx_ring;
    bool non_blocking;
};
class TcpApi
{
private:
    static int AttachSocket();

public:
    TcpApi(/* args */);
    ~TcpApi();
    static void WifiInit(const char *SSID, const char *Password);
    static void WifiConfigCheck(void);
    static void TcpServerTask(void *PvParameters);
};

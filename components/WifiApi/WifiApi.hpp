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

void WifiConfigCheck(void);
static void EventHandler(void *arg, esp_event_base_t event_base,
                         int32_t event_id, void *event_data);
void WifiInit(const char *ssid, const char *password);
bool WifiConnected();
void WifiConnect();
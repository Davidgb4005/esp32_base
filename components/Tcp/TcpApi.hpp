#pragma once
#include "RingBuffer.hpp"
#include "Config.hpp"

struct TcpTaskParams
{
    const char * ip_addr;
    int port;
    RingBuffer *rx_ring;
    RingBuffer *tx_ring;
    bool non_blocking;
};
class TcpApi
{
private:
    static int AttachSocket(const char * ip_addr, int & port);
    static int ConnectSocket(const char * ip_addr, int & port);
public:
    TcpApi(/* args */);
    ~TcpApi();
    static void WifiInit(const char *SSID, const char *Password);
    static void WifiConfigCheck(void);
    static void TcpServerTask(void *PvParameters);
    static void TcpClientTask(void *PvParameters);
};

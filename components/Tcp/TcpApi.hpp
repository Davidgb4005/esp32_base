/**
 * @file TcpApi.hpp
 * @brief Declaration of the TcpApi class for managing Wi-Fi setup and TCP communication on the ESP32.
 * 
 * @details
 * This header defines:
 *  - The `TcpApi` class, which provides functions to configure Wi-Fi in station mode,
 *    handle TCP server/client socket creation, and manage FreeRTOS-based communication tasks.
 *  - The `TcpTaskParams` structure, used for passing parameters to TCP FreeRTOS tasks.
 */

#pragma once

#include "RingBuffer.hpp"
#include "Debug.hpp"



class TcpApi
{
private:


    //Functions
    int Send(int tx_status);
    int Read(int rx_status);
    void ServerSocket();
    void ClientSocket();
    void BeginSocket();
    void CloseSocket();
    //Variables
    bool reset_socket = false;
    const char *ip_addr="0.0.0.0";
    int port=0;
    bool non_blocking=0;
    int sock =0;
    int socket_type=0;
public:
    RingBuffer * tx_ring;
    RingBuffer * rx_ring;
    //
    enum SocketType{
        CLIENT = 100,
        SERVER = 101,
    };
    TcpApi(const char *ip_addr, int port, int tx_buffer_size, int rx_buffer_size, int socket_type);

    ~TcpApi();


    static void WifiInit(const char *SSID, const char *Password);
    static void WifiConfigCheck(void);


    void TcpTask();

    void ResetSocket();
    void SetBlocking(bool blocking);
};
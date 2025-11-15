#pragma once
#include "RingBuffer.hpp"

enum ConnectionType
{
    CLIENT = 0,
    SERVER = 1
};

class TcpApi
{
private:
    const char *ip_addr = "0.0.0.0";
    int port = 0;
    ConnectionType connection_type = CLIENT;
    RingBuffer *rx_buffer;
    RingBuffer *tx_buffer;
    int sock = -1;
    bool socket_active = false;
    bool blocking = true;

public:
    TcpApi(int tx_buffer_size = 256, int rx_buffer_size = 256,
           const char *ip_addr = "0.0.0.0", int port = 0,
           ConnectionType socket_type = CLIENT);
    ~TcpApi();
    // TEMP FUNCS
    bool SocketActive();
    // TODO
    void ServerInit();
    // IN PROGRESS
    void TcpTask();
    void Send(char * buffer, int len);
    int Read(char * buffer);
    // TESTING  
    void CloseSocket();
    void ClientInit();
    void EnableBlocking(bool blocking);
    // DONE
};

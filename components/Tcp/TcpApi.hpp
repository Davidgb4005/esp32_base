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
#if 0
    const char *ip_addr = "0.0.0.0";
    int port = 0;
    ConnectionType connection_type = CLIENT;
    RingBuffer *rx_buffer;
    RingBuffer *tx_buffer;
    int sock = -1;
    bool socket_active = false;
    bool blocking = true;
#endif

public:
    TcpApi(RingBuffer * rx_buffer, RingBuffer tx_buffer,
           const char *ip_addr = "0.0.0.0", int port = 0,
           ConnectionType socket_type = CLIENT);
    ~TcpApi();
    // TEMP FUNCS
    bool SocketActive();
    // TODO
    void ServerInit();
    // IN PROGRESS
    int TcpTaskRecv();
    int TcpTaskSend();
    int WriteString(char *buffer, int len);
    int WriteChars(char *buffer, int len);
    int WriteStruct(void *data);
    int ReadData(void *buffer); // Read 1 Complete Message From (This->buffer) and copy it into (c)
    // TESTING
    void CloseSocket();
    void ClientInit();
    void EnableBlocking(bool blocking);
    // DONE

#if 1
    const char *ip_addr = "0.0.0.0";
    int port = 0;
    ConnectionType connection_type = CLIENT;
    TcpBuffer *rx_data;
    TcpBuffer *tx_data;
    int sock = -1;
    bool socket_active = false;
    bool blocking = true;
#endif
};

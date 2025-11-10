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
#include "Config.hpp"

/**
 * @struct TcpTaskParams
 * @brief Parameter structure used for passing socket configuration to TCP tasks.
 * 
 * @details
 * This structure is typically passed as the argument to the `TcpApi::TcpServerTask`
 * or `TcpApi::TcpClientTask` functions (FreeRTOS task entry points).
 */
struct TcpTaskParams
{
    const char *ip_addr;   ///< IP address of the remote or local endpoint.
    int port;              ///< Port number for socket connection or listening.
    RingBuffer *rx_ring;   ///< Pointer to the receive ring buffer.
    RingBuffer *tx_ring;   ///< Pointer to the transmit ring buffer.
    bool non_blocking;     ///< Enables non-blocking socket mode if true.
};

/**
 * @class TcpApi
 * @brief Provides a high-level interface for Wi-Fi setup and TCP communication on the ESP32.
 * 
 * @details
 * The `TcpApi` class wraps lower-level ESP-IDF networking and socket APIs, providing:
 *  - Wi-Fi configuration and connection handling.
 *  - TCP server and client setup with optional non-blocking I/O.
 *  - FreeRTOS task entry points for continuous send/receive socket management.
 */
class TcpApi
{
private:
    /**
     * @brief Creates a TCP server socket and waits for an incoming connection.
     * 
     * @param[in] ip_addr  IP address to bind the server to (use `nullptr` for INADDR_ANY).
     * @param[in,out] port Reference to the port number to listen on.
     * @return Socket descriptor for the accepted connection, or `-1` on failure.
     */
    int AttachSocket();

    /**
     * @brief Establishes a TCP client connection to a remote server.
     * 
     * @param[in] ip_addr  Remote server IP address.
     * @param[in,out] port Reference to the server port number.
     * @return Socket descriptor for the connected socket, or `-1` on failure.
     */
    int ConnectSocket();
    bool reset_socket = false;
public:
    TcpTaskParams parameters;

    /**
     * @brief Default constructor.
     */
    TcpApi(const char *ip_addr, int port, RingBuffer *tx_ring, RingBuffer *rx_ring, bool non_blocking);

    /**
     * @brief Destructor.
     */
    ~TcpApi();

    /**
     * @brief Initializes the ESP32 Wi-Fi in station mode and connects to a network.
     * 
     * @param[in] SSID     The target Wi-Fi network SSID.
     * @param[in] Password The Wi-Fi network password.
     */
    static void WifiInit(const char *SSID, const char *Password);

    /**
     * @brief Initializes or restores NVS flash storage for Wi-Fi configuration.
     * 
     * @details
     * This function ensures NVS is ready for use by Wi-Fi components.
     */
    static void WifiConfigCheck(void);

    /**
     * @brief FreeRTOS task that manages TCP server operation.
     * 
     * @param[in] PvParameters Pointer to a `TcpTaskParams` structure.
     */
    static void TcpServerTask(void *PvParameters);

    /**
     * @brief FreeRTOS task that manages TCP client operation.
     * 
     * @param[in] PvParameters Pointer to a `TcpTaskParams` structure.
     */
    static void TcpClientTask(void *PvParameters);

    void ResetSocket();
    bool SetSocket();

};

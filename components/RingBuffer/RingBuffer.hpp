/**
 * @file RingBuffer.hpp
 * @brief Header file for the RingBuffer circular buffer class.
 *
 * @details
 * This class implements a fixed-size circular buffer for storing and retrieving
 * data messages. Supports read/write operations, internal error handling, and
 * optional debugging output via `#define DEBUG 1`.
 */

#pragma once

#include "freertos/FreeRTOS.h"
#include "Config.hpp"

/**
 * @class RingBuffer
 * @brief Implements a fixed-length circular buffer with error handling.
 */
class RingBuffer
{
public:
    /**
     * @brief Constructs a RingBuffer of specified length.
     * @param len Length of the buffer in bytes.
     */
    RingBuffer(int len);

    /**
     * @brief Destructor. Frees the allocated buffer memory.
     */
    ~RingBuffer();

    /**
     * @brief Resets the buffer to an empty state.
     *
     * Resets the read/write pointers, clears errors, and sets the available data count to zero.
     */
    void ResetBuffer();

    /**
     * @brief Reads one complete message from the buffer.
     * @param[out] c Pointer to a destination buffer where the data will be copied.
     * @return Number of bytes read on success, or a negative error code.
     */
    int ReadData(char *c);

    /**
     * @brief Writes data into the buffer.
     * @param[in] c Pointer to the source data.
     * @param[in] len Number of bytes to write.
     * @return Number of bytes written on success, or a negative error code.
     */
    int WriteData(char *c, int len);

    /**
     * @brief Returns the number of bytes currently available in the buffer.
     * @return Number of bytes available to read.
     */
    int DataAvailible();

    /* ------------------------ Debugging Functions ------------------------ */

    /**
     * @brief Prints all current buffer contents without consuming them.
     * @note Only works if `#define DEBUG 1` is enabled.
     */
    void PrintData();

    /**
     * @brief Prints a message of given length.
     * @param[in] c Pointer to the message.
     * @param[in] len Number of bytes to print.
     * @note Only works if `#define DEBUG 1` is enabled.
     */
    static void PrintMsg(char *c, int len);

    /**
     * @brief Prints a debug message.
     * @param[in] c The message string to print.
     * @note Only works if `#define DEBUG 1` is enabled.
     */
    void PrintDebug(const char *c);

    /**
     * @brief Prints the number of available bytes in the buffer.
     * @param[in] i Reference to the available byte count.
     * @note Only works if `#define DEBUG 1` is enabled.
     */
    void PrintDataAvailibleDebug(int &i);

    /* ------------------------ Public Variables ------------------------ */
    int data_availible; ///< Number of valid bytes currently in the buffer
    char *buffer;       ///< The underlying buffer storage

    /**
     * @brief Error codes returned by ReadData and WriteData.
     */
    enum Error
    {
        NO_DATA = 0,           ///< No data available for reading
        INCOMPLETE_DATA = -1,  ///< Partial message in buffer
        INVALID_DATA = -3,     ///< Invalid message length
        BUFFER_FULL = -4,      ///< Buffer is full
        UNEXPECTED_ERROR = -100, ///< Unexpected error
        FATEL_ERROR = -101,      ///< Fatal error
        BUFFER_OVERREAD = -102,  ///< Attempted to read more than available
        BUFFER_OVERFLOW = -103   ///< Attempted to write more than buffer size
    };

private:
    bool debug;             ///< Internal debug flag
    char *read_ptr;         ///< Pointer to the current read position
    char *write_ptr;        ///< Pointer to the current write position
    char *end_ptr;          ///< Pointer to the end of the buffer
    char *start_ptr;        ///< Pointer to the start of the buffer
    int buffer_len;         ///< Total length of the buffer
    bool fatel_error;       ///< Fatal error flag
    bool disable_on_buffer_overflow = false; ///< Disable flag for buffer overflow
};

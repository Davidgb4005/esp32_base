/**
 * @file RingBuffer.cpp
 * @brief Implementation of the RingBuffer class for circular buffer operations.
 *
 * @details
 * This class provides a fixed-length circular buffer for storing and retrieving
 * data streams (e.g., TCP or UART data). It includes internal error handling and
 * optional debugging output via `#define DEBUG 1`.
 */

#include "RingBuffer.hpp"
#include <memory>

#if DEBUG
#include <iostream>
#endif

/**
 * @brief Constructs a RingBuffer of a specified length.
 * 
 * @param len Length of the buffer in bytes.
 */
RingBuffer::RingBuffer(int len)
{
    buffer = new char[len];
    start_ptr = buffer;
    end_ptr = buffer + len;
    read_ptr = buffer;
    write_ptr = buffer;
    fatel_error = false;
    buffer_len = len;
    data_availible = 0;
}

/**
 * @brief Destructor. Frees allocated buffer memory.
 */
RingBuffer::~RingBuffer()
{
    delete[] buffer;
}

/**
 * @brief Resets the buffer to an empty state.
 */
void RingBuffer::ResetBuffer()
{
    read_ptr = buffer;
    write_ptr = buffer;
    fatel_error = false;
    data_availible = 0;
    PrintDebug("Buffer Reset");
}

/**
 * @brief Returns the number of bytes currently available in the buffer.
 * 
 * @return int Number of bytes available for reading.
 */
int RingBuffer::DataAvailible()
{
    return data_availible;
}

/**
 * @brief Reads one message from the buffer.
 * 
 * @param[out] c Pointer to a destination buffer for the data.
 * @return 
 *  - >0: Number of bytes read  
 *  - <=0: Error code (e.g., `NO_DATA`, `BUFFER_OVERREAD`, etc.)
 */
int RingBuffer::ReadData(char *c)
{
    if (fatel_error)
    {
        PrintDebug("Fatal Error");
        return FATEL_ERROR;
    }
    if (read_ptr == write_ptr)
    {
        return NO_DATA;
    }

    int msg_len = *read_ptr;
    if (msg_len < 1)
    {
        read_ptr++;
        data_availible--;
        PrintDebug("Invalid Data");
        return INVALID_DATA;
    }

    if (data_availible < msg_len)
    {
        PrintDebug("Incomplete Data");
        return INCOMPLETE_DATA;
    }

    read_ptr++;
    data_availible--;
    if (read_ptr == end_ptr)
        read_ptr = start_ptr;

    int i;
    for (i = 0; i < msg_len; i++)
    {
        *c++ = *read_ptr++;
        data_availible--;

        if (read_ptr == end_ptr)
            read_ptr = start_ptr;

        if (data_availible < 0)
        {
            fatel_error = true;
            PrintDebug("Buffer Overread");
            return BUFFER_OVERREAD;
        }
    }

    PrintDataAvailibleDebug(data_availible);
    return i;
}

/**
 * @brief Writes data to the buffer.
 * 
 * @param[in] c   Pointer to the source data.
 * @param[in] len Length of data in bytes.
 * 
 * @return 
 *  - >0: Number of bytes written  
 *  - <=0: Error code (e.g., `BUFFER_FULL`, `BUFFER_OVERFLOW`, etc.)
 */
int RingBuffer::WriteData(char *c, int len)
{
    if (fatel_error)
    {
        PrintDebug("Fatal Error");
        return FATEL_ERROR;
    }
    else if (data_availible > buffer_len)
    {
        PrintDebug("Buffer Overflow");
        fatel_error = true;
        return BUFFER_OVERFLOW;
    }
    else if (data_availible + len > buffer_len - 1)
    {
        vTaskDelay(1);
        PrintDebug("Buffer Full");
        return BUFFER_FULL;
    }
    int i;
    for (i = 0; i < len; i++)
    {
        *write_ptr++ = *c++;
        data_availible++;

        if (write_ptr == end_ptr)
            write_ptr = start_ptr;
    }

    PrintDataAvailibleDebug(data_availible);
    return i;
}

/* -------------------------------------------------------------------------- */
/*                              Debug Functions                               */
/* -------------------------------------------------------------------------- */

/**
 * @brief Prints a debug message if DEBUG is enabled.
 * 
 * @param[in] c The message to print.
 */
void RingBuffer::PrintDebug(const char *c)
{
#if DEBUG
    std::cout << c << std::endl;
#endif
}

/**
 * @brief Prints the number of bytes currently available in the buffer.
 * 
 * @param[in] i Reference to the available byte count.
 */
void RingBuffer::PrintDataAvailibleDebug(int &i)
{
#if DEBUG
    std::cout << "Data Available: " << i << std::endl;
#endif
}

/**
 * @brief Prints all current buffer contents (for debugging only).
 */
void RingBuffer::PrintData()
{
#if DEBUG
    int len = 0;
    char *temp_read_ptr = read_ptr;
    int temp_data_available = data_availible;

    if (temp_data_available < 1)
        return;

    char c;
    while (temp_data_available > 0)
    {
        len = *temp_read_ptr++;
        temp_data_available--;

        if (len > temp_data_available)
        {
            std::cout << "Data Fragment (" 
                      << temp_data_available << "/" << static_cast<int>(len) 
                      << "): ";
        }
        else
        {
            std::cout << "Data Length (" << static_cast<int>(len) << "): ";
        }

        for (int k = 0; k < len; ++k)
        {
            c = *temp_read_ptr++;
            std::cout << c;
            temp_data_available--;

            if (temp_read_ptr > end_ptr)
                temp_read_ptr = start_ptr;
        }
        std::cout << std::endl;
    }
#endif
}

/**
 * @brief Prints a single message for debugging.
 * 
 * @param[in] c   Pointer to the message data.
 * @param[in] len Length of the message.
 */
void RingBuffer::PrintMsg(char *c, int len)
{
#if DEBUG
    for (int k = 0; k < len; k++)
    {
        std::cout << c[k];
    }
    std::cout << std::endl;
#endif
}

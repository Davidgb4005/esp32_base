#include "RingBuffer.hpp"
#include <memory>
#define DEBUG 1
#if DEBUG
#include <iostream>
#endif
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

RingBuffer::~RingBuffer()
{
    delete[] buffer;
}
void RingBuffer::ResetBuffer()
{
    read_ptr = buffer;
    write_ptr = buffer;
    fatel_error = false;
    data_availible = 0;
    PrintDebug("Buffer Reset");
}
   int RingBuffer::DataAvailible(){
    return data_availible;
   }

int RingBuffer::ReadData(char *c)
{
    if (fatel_error)
    {
        PrintDebug("Fatel Error");
        return FATEL_ERROR;
    }
    if (read_ptr == write_ptr)
    {
        // PrintDebug("No Data");
        return NO_DATA;
    }
    int msg_len = *read_ptr;
    if (msg_len < 1)
    {
        read_ptr++;
        PrintDebug("Invalid Data");
        return INVALID_DATA;
    }

    if (data_availible < msg_len)
    {
        PrintDebug("Incomplete Data");
        return INCOMPLETE_DATA;
    }
    else
    {
        read_ptr++;
        data_availible--;
        if (read_ptr == end_ptr)
            read_ptr = start_ptr;
        int i;
        for (i = 0; i < msg_len; i++)
        {
            *c = *(read_ptr);
            c++;
            read_ptr++;
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
    PrintDebug("Unexpected Error");
    return UNEXPECTED_ERROR;
}

int RingBuffer::WriteData(char *c, int len)
{
    if (fatel_error)
    {
        PrintDebug("Fatel Error");
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
    else
    {
        int i;
        for (i = 0; i < len; i++)
        {
            *write_ptr = *c;
            c++;
            write_ptr++;
            data_availible++;
            if (write_ptr == end_ptr)
            {
                write_ptr = start_ptr;
            }
        }
        PrintDataAvailibleDebug(data_availible);
        return i;
    }
    PrintDebug("Unexpected Error");
    return UNEXPECTED_ERROR;
}


// These are all debugging functions only enabled if #define DEBUG 1
void RingBuffer::PrintDebug(const char *c)
{
#if DEBUG
    std::cout << c << std::endl;
#endif
}

void RingBuffer::PrintDataAvailibleDebug(int &i)
{
#if DEBUG
    std::cout << "Data Availible : " << i << std::endl;
#endif
}

void RingBuffer::PrintData()
{
#if DEBUG // Enables Console Output For Debugging
    int len = 0;
    char *temp_read_ptr = read_ptr;
    int temp_data_available = data_availible;
    if (temp_data_available < 1)
    {
        // std::cout << "No Data" << std::endl;
        return;
    }
    char c;
    while (temp_data_available > 0)
    {
        len = *temp_read_ptr;
        temp_read_ptr++; // skip length prefix
        temp_data_available--;
        if (len > temp_data_available)
        {
            std::cout << "Data Fragment(" << (temp_data_available) << "/" << static_cast<int>(len) << ") : ";
        }
        else
        {
            std::cout << "Data Length(" << static_cast<int>(len) << ") : ";
        }
        for (int k = 0; k < len; ++k)
        {
            c = *temp_read_ptr;
            std::cout << c;
            temp_read_ptr++;
            temp_data_available--;
            if (temp_read_ptr > end_ptr)
            {
                temp_read_ptr = start_ptr;
            }
        }
        std::cout << std::endl;
    }
#endif
}
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

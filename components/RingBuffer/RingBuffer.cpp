#include "RingBuffer.hpp"
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
    buffer_len = len;
    data_availible = 0;
}

RingBuffer::~RingBuffer()

{
    delete[] buffer;
}
void RingBuffer::ResetBuffer()
{
    std::cout<<"Reset Buffer"<<data_availible<<std::endl;
    read_ptr = buffer;
    write_ptr = buffer;
    data_availible = 0;
    message_complete = true;
}

int RingBuffer::ReadData(char *buffer)
{
    int len = (*read_ptr);
    if (data_availible < len)
    {
        return INCOMPLETE_DATA;
    }
    else
    {
        // Read Message Length As First Byte
        uint16_t read_check_sum = 0;
        int i;
        for (i = 0; i < len; i++)
        {
            *(buffer + i) = *(read_ptr);
            read_check_sum += *(read_ptr);
            AdvanceReadPointer();
        }
        if (!ValidateCheckSum(&read_check_sum))
        {
            return INVALID_CHECKSUM;
        }
        return i;
    }
}

int RingBuffer::WriteData(char *buffer, int len)
{

    if (len + data_availible > buffer_len - 1)
    {
        buffer_full = true;
        return BUFFER_FULL;
    }
    else
    {
        buffer_full = false;
    }
    if (len < 0)
    {
        return INVALID_DATA;
    }
    else
    {
        int i;
        for (i = 0; i < len; i++)
        {
            if (bytes_remaining <= 0)
            {
                bytes_remaining = (*(buffer + i));
            }
            *write_ptr = (*(buffer + i));
            check_sum += *write_ptr;
            AdvanceWritePointer();
            if (bytes_remaining == 0)
            {
                InsertCheckSum();
            }
        }
        return i;
    }
    return UNEXPECTED_ERROR;
}
int RingBuffer::DataAvailible()
{
    return data_availible;
}
bool RingBuffer::BufferFull()
{
    return buffer_full;
}
int RingBuffer::AdvanceReadPointer()
{
    if (data_availible < 1)
    {
        return BUFFER_OVERREAD;
    }
    else if (read_ptr == end_ptr)
    {
        data_availible--;
        read_ptr = start_ptr;
    }
    else
    {
        data_availible--;
        read_ptr++;
    }
    return (0);
}

int RingBuffer::AdvanceWritePointer()
{

    if (write_ptr == read_ptr - 1 || (write_ptr == end_ptr && read_ptr == start_ptr))
    {
        return BUFFER_OVERFLOW;
    }
    else if (write_ptr == end_ptr)
    {
        bytes_remaining--;
        data_availible++;
        write_ptr = start_ptr;
    }
    else
    {
        bytes_remaining--;
        data_availible++;
        write_ptr++;
    }
    return (0);
}
void RingBuffer::InsertCheckSum()
{
    check_sum = ~check_sum + 1;
    char msb = check_sum >> 8;
    char lsb = check_sum & 0xff;
    *write_ptr = msb;
    AdvanceWritePointer();
    *write_ptr = lsb;
    AdvanceWritePointer();
    message_complete = true;
    check_sum = 0;
}

bool RingBuffer::ValidateCheckSum(uint16_t *check_sum_ptr)
{
    uint16_t msb = (*read_ptr) << 8;
    AdvanceReadPointer();
    uint16_t lsb = (*read_ptr) & 0x00ff;
    AdvanceReadPointer();
    uint16_t read_check_sum = msb + lsb + *check_sum_ptr;
    // if ((msb + lsb + *check_sum_ptr) != 0) Figure Out Why This Fucks it
    if ((read_check_sum) != 0)
    {
        return false;
    }
    return true;
}

void RingBuffer::PrintData()
{
#if DEBUG // Enables Console Output For Debugging
    int len = 0;
    char *temp_read_ptr = read_ptr;
    int temp_data_available = data_availible;
    if (temp_data_available < 1)
    {
        std::cout << "No Data" << std::endl;
        return;
    }
    char c;
    while (temp_data_available > 0)
    {
        len = (*temp_read_ptr);

        if (len - 1 > temp_data_available)
        {
            std::cout << "Data Fragment(" << (temp_data_available) << "/" << static_cast<int>(len) << ") : ";
        }
        else
        {
            std::cout << "Data Length(" << static_cast<int>(*temp_read_ptr) << ") : ";
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
        std::cout << " - Checksum MSB:" << static_cast<int>(*temp_read_ptr);
        temp_read_ptr++;
        temp_data_available--;
        std::cout << " LSB:" << static_cast<int>(*temp_read_ptr);
        temp_read_ptr++;
        temp_data_available--;
        std::cout << std::endl;
    }
#endif
}

void RingBuffer::PrintMsg(char *buffer, int len)
{
#if DEBUG
    for (int k = 0; k < len; k++)
    {
        std::cout << buffer[k];
    }
    std::cout << std::endl;
#endif
}

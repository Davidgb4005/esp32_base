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
    read_ptr = buffer;
    write_ptr = buffer;
    buffer_overflow = false;
    data_availible = 0;
}

int RingBuffer::ReadData(Telegram & data)
{

    data.message_length = *read_ptr;
    if (data_availible < data.message_length)
    {
        return INCOMPLETE_DATA;
    }
    else
    {
        // Read Message Length As First Byte
        data.check_sum = 0;
        data.check_sum += *(read_ptr);
        AdvanceReadPointer();
        int i;
        for (i = 0; i < data.message_length; i++)
        {
            *(data.message + i) = *(read_ptr);
            data.check_sum += *(read_ptr);
            AdvanceReadPointer();
        }
        if (!ValidateCheckSum(&data.check_sum))
        {
            return INVALID_CHECKSUM;
        }

        return i;
    }
    return UNEXPECTED_ERROR;
}

int RingBuffer::WriteData(Telegram data)
{
    if (data.message_length < 2)
    {
        return INVALID_DATA;
    }
    else if (data.message_length+data_availible > buffer_len) {
        return BUFFER_FULL;
    }
    else if (data.message_length > 253)
    {
        return MESSAGE_OVERLENGTH;
    }
    else
    {
        uint16_t check_sum = 0;
        *write_ptr = data.message_length;
        check_sum += *write_ptr;
        AdvanceWritePointer();
        int i;
        for (i = 0; i < data.message_length; i++)
        {
            *write_ptr = *(data.message + i);
            check_sum += *write_ptr;
            AdvanceWritePointer();
        }
        InsertCheckSum(&check_sum);
        return i;
    }
    return UNEXPECTED_ERROR;
}
int RingBuffer::DataAvailible()
{
    return data_availible;
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
        data_availible++;
        write_ptr = start_ptr;
    }
    else
    {
        data_availible++;
        write_ptr++;
    }
    return (0);
}
void RingBuffer::InsertCheckSum(uint16_t *check_sum_ptr)
{
    uint16_t check_sum = *check_sum_ptr;
    check_sum = ~check_sum + 1;
    char msb = check_sum >> 8;
    char lsb = check_sum & 0xff;
    *write_ptr = msb;
    AdvanceWritePointer();
    *write_ptr = lsb;
    AdvanceWritePointer();
}

bool RingBuffer::ValidateCheckSum(uint16_t *check_sum_ptr)
{
    uint16_t msb = (*read_ptr) << 8;
    AdvanceReadPointer();
    uint16_t lsb = (*read_ptr) & 0x00ff;
    AdvanceReadPointer();
    uint16_t check_sum = msb + lsb + *check_sum_ptr;
    // if ((msb + lsb + *check_sum_ptr) != 0) Figure Out Why This Fucks it
    if ((check_sum) != 0)
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

        temp_read_ptr++;
        temp_data_available--;
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

void RingBuffer::PrintMsg(Telegram data)
{
#if DEBUG
    for (int k = 0; k < data.message_length; k++)
    {
        std::cout << data.message[k];
    }
    std::cout << std::endl;
#endif
}

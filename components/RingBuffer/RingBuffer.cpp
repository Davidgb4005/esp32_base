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
    PrintDebug("Buffer Reset");
    read_ptr = buffer;
    write_ptr = buffer;
    data_availible = 0;
    message_complete = true;
}
int RingBuffer::GetMessageType()
{
    return *(read_ptr + 1);
}
int RingBuffer::ReadData(void *buffer)
{

    int len = (*read_ptr);
    if (data_availible < len)
    {
        //PrintDebug("Incomplete Data");
        return INCOMPLETE_DATA;
    }
    else
    {
        // Read Message Length As First Byte
        uint16_t read_check_sum = 0;
        int i;
        for (i = 0; i < len; i++)
        {
            *(static_cast<char *>(buffer) + i) = *(read_ptr);
            read_check_sum += *(read_ptr);
            AdvanceReadPointer();
        }
        if (!ValidateCheckSum(&read_check_sum))
        {
            PrintDebug("Invalid Data");
            return INVALID_CHECKSUM;
        }
        return i;
    }
}

int RingBuffer::WriteData(char *buffer)
{
    int len = buffer[0];
    if (len + data_availible + 2 > buffer_len - 1)
    {
        PrintDebug("Buffer Full");
        buffer_full = true;
        return BUFFER_FULL;
    }
    else
    {
        buffer_full = false;
    }
    if (len < 0)
    {
        PrintDebug("Invalid Data");
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
    PrintDebug("Unexpect Error");
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
    if (data_availible < 0)
    {
        PrintDebug("Buffer Overread");
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

    if (write_ptr == read_ptr - 1)
    {
        PrintDebug("Buffer Overflow 2");
        return BUFFER_OVERFLOW;
    }
    else if (write_ptr == end_ptr && read_ptr == start_ptr){
                PrintDebug("Buffer Overflow 1");
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
        PrintDebug("Checksum Fail");
        return false;
    }
    return true;
}

int RingBuffer::WriteString(char *buffer, int len)
{
    char temp_buffer[256];
    for (int i = 0; i < len - 1; i++)
    {
        temp_buffer[i + 1] = buffer[i];
    }
    temp_buffer[0] = len;
    return WriteData(temp_buffer);
}
int RingBuffer::WriteChars(char *buffer, int len)
{
    char temp_buffer[256];
    for (int i = 0; i < len; i++)
    {
        temp_buffer[i + 1] = buffer[i];
    }
    temp_buffer[0] = len + 1;
    return WriteData(temp_buffer);
}
int RingBuffer::WriteStruct(void *data)
{
    return WriteData(static_cast<char *>(data));
}

void RingBuffer::PrintDebug(const char * c){
    std::cout<<c<<std::endl;
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
        std::cout << "Struct Type(" << static_cast<int>(*(temp_read_ptr + 1)) << ") : ";
        for (int k = 0; k < len; ++k)
        {
            c = *temp_read_ptr;
            if (false) // Debugging Causes output to be in Int instead of char
                std::cout << static_cast<int>(c);
            else
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

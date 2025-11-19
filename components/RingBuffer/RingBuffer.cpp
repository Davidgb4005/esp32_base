#include "RingBuffer.hpp"

RingBuffer::RingBuffer(uint16_t buffer_size)
{
    this->buffer_size = buffer_size;

    start_ptr = new uint8_t[buffer_size];
    write_ptr = start_ptr;
    read_ptr = start_ptr;
    end_ptr = start_ptr + buffer_size - 1;
}
RingBuffer::~RingBuffer()
{
    delete[] start_ptr;
}
uint16_t RingBuffer::MessageAvailible()
{
    return messages_availible;
}
uint16_t RingBuffer::BytesAvailible()
{
    return bytes_availible;
}
uint16_t RingBuffer::BytesRemaining()
{

    return buffer_size - bytes_availible;
}

uint16_t RingBuffer::Read(Telegram *data)
{
    if (messages_availible > 0)
    {
        size_t i = 0;
        int16_t err = 0;
        uint8_t *data_ptr = reinterpret_cast<uint8_t *>(data);
        *data_ptr = *read_ptr;
        uint16_t length = 0;

        length = *read_ptr << 8;
        err = AdvanceReadPtr();
        if (err < 0)
        {
            PrintDebug("Advance Read Pointer Error", err, __LINE__);
            return (err);
        }
        *(data_ptr + 1) = *read_ptr;
        length += *read_ptr & 0xff;
        err = AdvanceReadPtr();
        if (err < 0)
        {
            PrintDebug("Advance Read Pointer Error", err, __LINE__);
            return (err);
        }
        for (i = 2; i < length; i++)
        {
            *(data_ptr + i) = *read_ptr;
            err = AdvanceReadPtr();
            if (err < 0)
            {
                PrintDebug("Advance Read Pointer Error", err, __LINE__);
                return (err);
            }
        }
        messages_availible--;
        return i;
    }
    else
    {
        PrintDebug("No Messages", NO_MESSAGES, __LINE__);
        return NO_MESSAGES;
    }
}
uint16_t RingBuffer::Read(StringTelegram *data, char *buffer)
{
    if (messages_availible > 0)
    {
        size_t i = 0;
        int16_t err = 0;
        uint8_t *data_ptr = reinterpret_cast<uint8_t *>(data);
        *data_ptr = *read_ptr;
        uint16_t length = 0;

        length = *read_ptr << 8;
        err = AdvanceReadPtr();
        if (err < 0)
        {
            PrintDebug("Advance Read Pointer Error", err, __LINE__);
            return (err);
        }
        *(data_ptr + 1) = *read_ptr;
        length += *read_ptr & 0xff;
        err = AdvanceReadPtr();
        if (err < 0)
        {
            PrintDebug("Advance Read Pointer Error", err, __LINE__);
            return (err);
        }
        int val = 0;
        for (i = 2; i < length + 1; i++)
        {

            if (i < sizeof(StringTelegram) - 8)
            {
                *(data_ptr + i) = *read_ptr;
                err = AdvanceReadPtr();
                if (err < 0)
                {
                    PrintDebug("Advance Read Pointer Error", err, __LINE__);
                    return (err);
                }
            }
            else if (i == sizeof(StringTelegram) - 8)
            {
                data->data = (uint8_t *)buffer;
                val = i + 1;
            }
            else
            {
                *(buffer + i - val) = *read_ptr;
                err = AdvanceReadPtr();
                if (err < 0)
                {
                    PrintDebug("Advance Read Pointer Error", err, __LINE__);
                    return (err);
                }
            }
        }
        messages_availible--;
        return i - val;
    }
    else
    {
        PrintDebug("No Messages", NO_MESSAGES, __LINE__);
        return NO_MESSAGES;
    }
}
uint16_t RingBuffer::Read(CharArrayTelegram *data, char *buffer)
{
    if (messages_availible > 0)
    {
        size_t i = 0;
        int16_t err = 0;
        uint8_t *data_ptr = reinterpret_cast<uint8_t *>(data);
        *data_ptr = *read_ptr;
        uint16_t length = 0;

        length = *read_ptr << 8;
        err = AdvanceReadPtr();
        if (err < 0)
        {
            PrintDebug("Advance Read Pointer Error", err, __LINE__);
            return (err);
        }
        *(data_ptr + 1) = *read_ptr;
        length += *read_ptr & 0xff;
        err = AdvanceReadPtr();
        if (err < 0)
        {
            PrintDebug("Advance Read Pointer Error", err, __LINE__);
            return (err);
        }
        int val = 0;
        for (i = 2; i < length + 1; i++)
        {

            if (i < sizeof(StringTelegram) - 8)
            {
                *(data_ptr + i) = *read_ptr;
                err = AdvanceReadPtr();
                if (err < 0)
                {
                    PrintDebug("Advance Read Pointer Error", err, __LINE__);
                    return (err);
                }
            }
            else if (i == sizeof(StringTelegram) - 8)
            {
                data->data = (uint8_t *)buffer;
                val = i + 1;
            }
            else
            {
                *(buffer + i - val) = *read_ptr;
                err = AdvanceReadPtr();
                if (err < 0)
                {
                    PrintDebug("Advance Read Pointer Error", err, __LINE__);
                    return (err);
                }
            }
        }
        messages_availible--;
        return i - val;
    }
    else
    {
        PrintDebug("No Messages", NO_MESSAGES, __LINE__);
        return NO_MESSAGES;
    }
}
uint16_t RingBuffer::Write(Telegram *data)
{
    int16_t size = (data->len_msb << 8) + (data->len_lsb & 0xff);
    if (size + bytes_availible > buffer_size - 1)
    {
        PrintDebug("Data Larger Than Remaing Bytes", STRUCT_LARGER_THAN_BUFFER, __LINE__);
        return STRUCT_LARGER_THAN_BUFFER;
    }
    bytes_remaining = size;

    uint8_t *data_ptr = reinterpret_cast<uint8_t *>(data);
    size_t i = 0;

    for (i = 0; i < size; i++)
    {
        *write_ptr = *(data_ptr + i);
        std::cout << (uint16_t)(*write_ptr) << "  ";
        int16_t err = AdvanceWritePtr();

        if (err < 0)
        {
            PrintDebug("Advance Write Pointer Error", err, __LINE__);
            return (err);
        }
    }
    messages_availible++;
    return i - size;
}
uint16_t RingBuffer::Write(StringTelegram *data)
{
    int16_t size = (data->len_msb << 8) + (data->len_lsb & 0xff);
    if (size > (buffer_size - bytes_availible))
    {
        PrintDebug("Data Larger Than Remaing Bytes", STRING_LARGER_THAN_BUFFER, __LINE__);
        return STRING_LARGER_THAN_BUFFER;
    }
    bytes_remaining = size;
    uint8_t offset = reinterpret_cast<uint8_t *>(&data->data) - reinterpret_cast<uint8_t *>(data);
    uint8_t *data_ptr = reinterpret_cast<uint8_t *>(data);
    size_t i = 0;
    for (i = 0; i < size; i++)
    {
        if (i < offset)
        {
            *write_ptr = *(data_ptr + i);
            int16_t err = AdvanceWritePtr();
            if (err < 0)
            {
                PrintDebug("Advance Read Pointer Error", err, __LINE__);
                return (err);
            }
        }
        else
        {
            *write_ptr = data->data[i - offset];
            int16_t err = AdvanceWritePtr();
            if (err < 0)
            {
                PrintDebug("Advance Read Pointer Error", err, __LINE__);
                return (err);
            }
        }
    }
    messages_availible++;
    return i - size;
}
uint16_t RingBuffer::Write(CharArrayTelegram *data)

{
    int16_t size = (data->len_msb << 8) + (data->len_lsb & 0xff);
    if (size > (buffer_size - bytes_availible))
    {
        PrintDebug("Data Larger Than Remaing Bytes", CHAR_ARRAY_LARGER_THAN_BUFFER, __LINE__);
        return CHAR_ARRAY_LARGER_THAN_BUFFER;
    }

    bytes_remaining = size;
    uint8_t offset = reinterpret_cast<uint8_t *>(&data->data) - reinterpret_cast<uint8_t *>(data);
    uint8_t *data_ptr = reinterpret_cast<uint8_t *>(data);
    size_t i = 0;
    for (i = 0; i < size; i++)
    {
        if (i < offset)
        {
            *write_ptr = *(data_ptr + i);
            int16_t err = AdvanceWritePtr();
            if (err < 0)
            {
                PrintDebug("Advance Write Pointer Error", err, __LINE__);
                return (err);
            }
        }
        else
        {
            *write_ptr = data->data[i - offset];
            int16_t err = AdvanceWritePtr();
            if (err < 0)
            {
                PrintDebug("Advance Write Pointer Error", err, __LINE__);
                return (err);
            }
        }
    }
    messages_availible++;
    return i - size;
}
uint16_t RingBuffer::WriteRaw(uint8_t *data, uint16_t len, uint16_t offset)

{
    if (bytes_availible + len > buffer_size - 1)
    {
        PrintDebug("Data Larger Than Remaing Bytes", RAW_DATA_LARGER_THAN_BUFFER, __LINE__);
        return RAW_DATA_LARGER_THAN_BUFFER;
    }
    if (len < 2)
    {
        PrintDebug("Write Raw Data To Short", DATA_TO_SHORT, __LINE__);
        return DATA_TO_SHORT;
    }

    uint8_t *data_ptr = data;
    size_t i = offset;
    for (i = offset; i < len; i++)
    {
        if (buffer_full)
        {
            PrintDebug("Write Raw Data Buffer Full", BUFFER_FULL, __LINE__);
            return i - len;
        }
        if (bytes_remaining == 0 && message_complete)
        {
            bytes_remaining_msb = data[i] << 8;
            message_complete = false;
        }
        else if (bytes_remaining == -1)
        {
            bytes_remaining_lsb = data[i] & 0xff;
            bytes_remaining += bytes_remaining_lsb + bytes_remaining_msb;
            bytes_remaining_lsb = 0;
            bytes_remaining_msb = 0;
        }
        *write_ptr = *(data_ptr + i);
        int16_t err = AdvanceWritePtr();

        // If You Enable The Return Line It Will Return The ErrorCode For Debugging Not The Remaining Bytes
        // It Will Break The Ability For the Function To Process Partial Messages BEWARE
#if 0
            if (err < 0)
            {
                PrintDebug("Advance Write Pointer Error", err, __LINE__);
                return (err);
            }
#endif

        if (bytes_remaining == 0)
        {
            messages_availible++;
            message_complete = true;
        }
    }
    return i - len;
}
uint16_t RingBuffer::ReadRaw(uint8_t *data)
{
    if (messages_availible > 0)
    {
        size_t i = 0;
        int16_t err = 0;
        uint8_t *data_ptr = reinterpret_cast<uint8_t *>(data);
        *data_ptr = *read_ptr;
        uint16_t length = 0;
        length = *read_ptr << 8;
        err = AdvanceReadPtr();
        if (err < 0)
        {
            PrintDebug("Advance Read Pointer Error", err, __LINE__);
            return (err);
        }
        *(data_ptr + 1) = *read_ptr;
        length += *read_ptr & 0xff;
        err = AdvanceReadPtr();
        if (err < 0)
        {
            PrintDebug("Advance Read Pointer Error", err, __LINE__);
            return (err);
        }
        for (i = 2; i < length; i++)
        {
            *(data_ptr + i) = *read_ptr;
            err = AdvanceReadPtr();
            if (err < 0)
            {
                PrintDebug("Advance Read Pointer Error", err, __LINE__);
                return (err);
            }
        }
        messages_availible--;
        return i;
    }
    else
    {
        PrintDebug("No Messages", NO_MESSAGES, __LINE__);
        return NO_MESSAGES;
    }
}
int16_t RingBuffer::GetAdr()
{
    if (messages_availible < 1)
    {
        PrintDebug("No Messages", NO_MESSAGES, __LINE__);
        return NO_MESSAGES;
    }
    else
    {
        uint8_t *temp_ptr = read_ptr;
        for (int i = 0; i < 3; i++)
        {
            int16_t err = AdvanceTempReadPtr(temp_ptr);
            if (err < 0)
            {
                PrintDebug("Advance Write Pointer Error", err, __LINE__);
                return (err);
            }
        }
        return *temp_ptr;
    }
}
int16_t RingBuffer::GetType()
{
    if (messages_availible < 1)
    {
        PrintDebug("No Messages", NO_MESSAGES, __LINE__);
        return NO_MESSAGES;
    }
    else
    {
        uint8_t *temp_ptr = read_ptr;
        for (int i = 0; i < 2; i++)
        {
            int16_t err = AdvanceTempReadPtr(temp_ptr);
            if (err < 0)
            {
                PrintDebug("Advance Write Pointer Error", err, __LINE__);
                return (err);
            }
        }
        return *temp_ptr;
    }
    PrintDebug("Unexpect Error", UNEXPECTED_ERROR_GetType, __LINE__);
    return UNEXPECTED_ERROR_GetType;
}
int16_t RingBuffer::AdvanceWritePtr()
{
    if (write_ptr + 1 == read_ptr || (write_ptr == end_ptr && read_ptr == start_ptr))
    {
        PrintDebug("Cant Advance Pointer Buffer Is Full", BUFFER_FULL, __LINE__);
        buffer_full = true;
        return BUFFER_FULL;
    }
    else if (write_ptr == end_ptr)
    {
        buffer_empty = false;
        write_ptr = start_ptr;
        bytes_availible++;
        bytes_remaining--;
        return 0;
    }
    else
    {
        buffer_empty = false;
        write_ptr++;
        bytes_availible++;
        bytes_remaining--;
        return 0;
    }
    PrintDebug("Unexpect Error", UNEXPECTED_ERROR_AdvanceWritePtr, __LINE__);
    return UNEXPECTED_ERROR_AdvanceWritePtr;
}

int16_t RingBuffer::AdvanceReadPtr()
{
    if (read_ptr == write_ptr)
    {
        buffer_empty = true;
        return BUFFER_EMPTY;
    }
    else if (read_ptr == end_ptr)
    {
        buffer_full = false;
        read_ptr = start_ptr;
        bytes_availible--;
        return 0;
    }
    else
    {
        buffer_full = false;
        read_ptr++;
        bytes_availible--;
        return 0;
    }
    PrintDebug("Unexpect Error", UNEXPECTED_ERROR_AdvanceReadPtr, __LINE__);
    return UNEXPECTED_ERROR_AdvanceReadPtr;
}
int16_t RingBuffer::AdvanceTempReadPtr(uint8_t *&temp_ptr)
{

    if (temp_ptr == end_ptr)
    {
        temp_ptr = start_ptr;
        return 0;
    }
    else
    {
        temp_ptr++;
        return 0;
    }
    PrintDebug("Unexpect Error", UNEXPECTED_ERROR_AdvanceTempReadPtr, __LINE__);
    return UNEXPECTED_ERROR_AdvanceTempReadPtr;
}

#if (_DEBUG == 1)
#include <iostream>

#endif

void RingBuffer::PrintData()
{
#if (_DEBUG == 1)
    uint8_t *temp_read_ptr = read_ptr;
    if (bytes_availible < 1)
    {
        std::cout << "No Data" << std::endl;
        return;
    }
    uint8_t c;
    uint16_t temp_bytes_availibe = bytes_availible;
    while (temp_bytes_availibe > 0)
    {
        int16_t len = (*(temp_read_ptr) << 8) + (*(temp_read_ptr + 1) & 0xff);

        if (len > temp_bytes_availibe)
        {
            std::cout << "Data Fragment(" << (temp_bytes_availibe) << "/" << static_cast<int>(len) << ") : ";
        }
        else
        {
            std::cout << "Data Length(" << len << ") : ";
        }
        std::cout << "Struct Type(" << static_cast<int>(*(temp_read_ptr + 2)) << ") : ";
        std::cout << "Priority(" << static_cast<int>(*(temp_read_ptr + 3)) << ") : ";

        for (int k = 0; k < len; ++k)
        {
            c = *temp_read_ptr;
            if (false) // Debugging Causes output to be in Int instead of char
                std::cout << static_cast<int>(c);
            else
                std::cout << c;
            if (temp_read_ptr == end_ptr)
            {
                temp_read_ptr = start_ptr;
                temp_bytes_availibe--;
            }
            else
            {
                temp_read_ptr++;
                temp_bytes_availibe--;
            }
            if (temp_read_ptr > end_ptr)
            {
                temp_read_ptr = start_ptr;
            }
        }
        std::cout << std::endl;
    }
#endif
}

void RingBuffer::PrintDebug(const char *str, int16_t error_code, int16_t line)
{
#if (_DEBUG == 1)
    std::cout << "File: " << __FILE__ << " - Line: " << line << " - Err Msg: " << str << " - Err Code: " << error_code << std::endl;
#endif
}
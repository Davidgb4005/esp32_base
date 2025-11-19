#pragma once
#include <memory>
#include <iostream>

#define _DEBUG 1


enum ErrorCodes
{
    RESERVED = -1,
    NO_MESSAGES = -2,
    BUFFER_FULL = -3,
    BUFFER_EMPTY = -4,
    CHAR_ARRAY_LARGER_THAN_BUFFER = -5,
    STRING_LARGER_THAN_BUFFER = -6,
    STRUCT_LARGER_THAN_BUFFER = -7,
    RAW_DATA_LARGER_THAN_BUFFER = -8,
    DATA_TO_SHORT = -9,

    UNEXPECTED_ERROR_GetType = -900,
    UNEXPECTED_ERROR_AdvanceWritePtr = -901,
    UNEXPECTED_ERROR_AdvanceReadPtr = -902,
    UNEXPECTED_ERROR_AdvanceTempReadPtr = -903,

};

struct Telegram
{
    uint8_t len_msb = 0;
    uint8_t len_lsb = 0;
    uint8_t type = 0;
    uint8_t address = 0;
};

struct __attribute__((packed)) StringTelegram : Telegram
{
    uint8_t *data = nullptr;
    void Set(const char *buffer, int length, uint8_t adr)
    {
        type = 1;
        address = adr;
        length--;                                                                     // Remove Null Termination Char
        len_msb = static_cast<uint8_t>((length + sizeof(StringTelegram) - 8) >> 8);   // Add Length for Base Telegram info
        len_lsb = static_cast<uint8_t>((length + sizeof(StringTelegram) - 8) & 0xff); // Add Length for Base Telegram info
        data = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(buffer));
    }
};
struct __attribute__((packed)) CharArrayTelegram : Telegram
{
    uint8_t *data = nullptr;
    void Set(uint8_t *buffer, int length, uint8_t adr)
    {
        type = 2;
        address = adr;                                                                                   // Remove Null Termination Char
        len_msb = static_cast<uint8_t>((length + sizeof(CharArrayTelegram) - sizeof(uintptr_t)) >> 8);   // Add Length for Base Telegram info
        len_lsb = static_cast<uint8_t>((length + sizeof(CharArrayTelegram) - sizeof(uintptr_t)) & 0xff); // Add Length for Base Telegram info
        data = buffer;
    }
};

class RingBuffer
{

private:
    bool message_complete = true;
    int16_t bytes_remaining = 0;
    int16_t bytes_remaining_msb = 0;
    int16_t bytes_remaining_lsb = 0;
    uint16_t bytes_availible = 0;
    uint16_t messages_availible = 0;
    uint16_t buffer_size = 0;
    bool buffer_full = false;
    bool buffer_empty = false;
    // Debugging Features
    void PrintDebug(const char *str, int16_t error_code, int16_t line);

public:
    RingBuffer(uint16_t buffer_size);
    ~RingBuffer();

    uint16_t Write(Telegram *data);
    uint16_t Write(StringTelegram *data);
    uint16_t Write(CharArrayTelegram *data);

    uint16_t WriteRaw(uint8_t *data, uint16_t len, uint16_t offset);
    uint16_t ReadRaw(uint8_t *data);
    uint16_t Read(Telegram *data);
    uint16_t Read(StringTelegram *data, char *buffer);
    uint16_t Read(CharArrayTelegram *data, char *buffer);

    int16_t AdvanceTempReadPtr(uint8_t *&temp_ptr);
    int16_t AdvanceWritePtr();
    int16_t AdvanceReadPtr();
    void PrintData();
    int16_t GetType();
    int16_t GetAdr();
    uint16_t BytesRemaining();
    uint16_t MessageAvailible();
    uint16_t BytesAvailible();
    uint8_t *write_ptr = nullptr;
    uint8_t *read_ptr = nullptr;
    uint8_t *start_ptr = nullptr;
    uint8_t *end_ptr = nullptr;
};

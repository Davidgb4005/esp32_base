#pragma once
#include <memory>
class RingBuffer
{
public:
    // Constructors
    RingBuffer(int len);
    ~RingBuffer();
    // Resets
    void ResetBuffer(); // Resets All Errors And Resets (read_ptr) And (write_ptr) To (start_ptr)

    // Memeber Functions
    int WriteString(char *buffer, int len);
    int WriteChars(char *buffer, int len);
    int WriteStruct(void *data);
    int ReadData(void *buffer); // Read 1 Complete Message From (This->buffer) and copy it into (c)
    int WriteData(char *buffer); // Write (len) Bytes From (c) Into (This->buffer)
    int DataAvailible();
    bool BufferFull();
    int GetMessageType();

    // Debugging Memeber Functions MUST USE (#DEFINE DEBUG 1)
    void PrintData();           // Prints All Bytes In Telegram DOES NOT CONSUME!
    static void PrintMsg(char *buffer, int len); // Prints (len) Bytes From Pointer (c)
    void PrintDebug(const char *c);
    // Variables
    char *buffer; // Ring Telegram

    // Enums
    enum Error
    {
        INCOMPLETE_DATA = -1,
        BUFFER_FULL = -2,
        INVALID_DATA = -3,
        UNEXPECTED_ERROR = -100,
        MESSAGE_OVERLENGTH = -101,
        BUFFER_OVERREAD = -102,
        BUFFER_OVERFLOW = -103,
        INVALID_CHECKSUM = -104,
    };
    // Any Inherited Structs Must Have the First 2 Memeber Be
    //(char len = sizeof(*YourStruct*);) And (char struct_type = *Custom User Value*;)
    struct TelegramStruct
    {
    };
    struct DefaultBuffer : TelegramStruct
    {
        char len;
        char struct_type;
        char *data;
    };

private:
    bool debug; // Debug enabled Flag
    char *read_ptr;
    char *write_ptr;
    char *end_ptr;
    char *start_ptr;
    int buffer_len;
    int data_availible; // Amount Of Valid Bytes In Telegram
    bool buffer_full = false;
    uint16_t check_sum;
    int bytes_remaining = 0;
    bool message_complete = false;

    bool ValidateCheckSum(uint16_t *check_sum_ptr);
    void InsertCheckSum();
    int AdvanceReadPointer();
    int AdvanceWritePointer();
};

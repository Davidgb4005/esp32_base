#pragma once
#include <memory>
class RingBuffer
{
public:
    // Structs
    struct Telegram{
        uint8_t message_length;
        char * message;
        uint16_t check_sum;
    };
    // Constructors
    RingBuffer(int len);
    ~RingBuffer();
    // Resets
    void ResetBuffer();// Resets All Errors And Resets (read_ptr) And (write_ptr) To (start_ptr)

    // Memeber Functions
    int ReadData(Telegram & data);// Read 1 Complete Message From (This->buffer) and copy it into (c)
    int WriteData(Telegram data);// Write (len) Bytes From (c) Into (This->buffer)
    int DataAvailible();


    // Debugging Memeber Functions MUST USE (#DEFINE DEBUG 1)
    void PrintData(); // Prints All Bytes In Telegram DOES NOT CONSUME!
    static void PrintMsg(Telegram data); // Prints (len) Bytes From Pointer (c)
    void PrintDebug(char * c);
    // Variables
    char *buffer;       // Ring Telegram

    // Enums
    enum Error
    {
        NO_DATA = 0,
        INCOMPLETE_DATA = -1,
        BUFFER_FULL = -2,
        INVALID_DATA = -3,
        UNEXPECTED_ERROR = -100,
        MESSAGE_OVERLENGTH = -101,
        BUFFER_OVERREAD = -102,
        BUFFER_OVERFLOW = -103,
        INVALID_CHECKSUM = -104
    };

private:
    bool debug;         // Debug enabled Flag
    char *read_ptr;
    char *write_ptr;
    char *end_ptr;
    char *start_ptr;
    int buffer_len;
    bool buffer_overflow = false;
    int data_availible; // Amount Of Valid Bytes In Telegram


    bool ValidateCheckSum(uint16_t * check_sum_ptr);
    void InsertCheckSum(uint16_t * check_sum_ptr);
    int AdvanceReadPointer();
    int AdvanceWritePointer();
};

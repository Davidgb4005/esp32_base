#pragma once
#include "freertos/FreeRTOS.h"
#include "Config.hpp"
class RingBuffer
{
public:
    // Constructors
    RingBuffer(int len);
    ~RingBuffer();
    // Resets
    void ResetBuffer();// Resets All Errors And Resets (read_ptr) And (write_ptr) To (start_ptr)

    // Memeber Functions
    int ReadData(char *c);// Read 1 Complete Message From (This->buffer) and copy it into (c)
    int WriteData(char *c, int len);// Write (len) Bytes From (c) Into (This->buffer)
    int DataAvailible();


    // Debugging Memeber Functions MUST USE (#DEFINE DEBUG 1)
    void PrintData(); // Prints All Bytes In Buffer DOES NOT CONSUME!
    static void PrintMsg(char *c, int len); // Prints (len) Bytes From Pointer (c)
    void PrintDebug(const char* c);
    void PrintDataAvailibleDebug(int & i);
    // Variables
    int data_availible; // Amount Of Valid Bytes In Buffer
    char *buffer;       // Ring Buffer
    // Enums
    enum Error
    {
        NO_DATA = 0,
        INCOMPLETE_DATA = -1,
        INVALID_DATA = -3,
        BUFFER_FULL = -4,
        UNEXPECTED_ERROR = -100,
        FATEL_ERROR = -101,
        BUFFER_OVERREAD = -102,
        BUFFER_OVERFLOW = -103
    };

private:
    bool debug;         // Debug enabled Flag
    char *read_ptr;
    char *write_ptr;
    char *end_ptr;
    char *start_ptr;
    int buffer_len;
    bool fatel_error;
    bool disable_on_buffer_overflow = false;
};

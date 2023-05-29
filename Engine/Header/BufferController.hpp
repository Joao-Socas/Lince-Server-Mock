#pragma once

// STD INCLUDES
#include<map>
#include<mutex>
#include<vector>

// Classe para controlar os buffers a serem enviados
class BufferController
{
public:
    BufferController(unsigned int width, unsigned int height);

    std::vector<std::vector<uint8_t>> drawing_buffer;
    bool new_on_wait = false;

    void SwapDrawingBuffer();
    char* GetBufferToSend();
    unsigned int GetBufferToSendSize();

    unsigned int width;
    unsigned int height;

private:
    std::vector<std::vector<uint8_t>> waiting_buffer;
    std::vector<std::vector<uint8_t>> sendding_buffer;

    std::mutex swap_mutex;
};
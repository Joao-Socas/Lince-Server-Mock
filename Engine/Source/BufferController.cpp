#include "BufferController.hpp"

BufferController::BufferController(unsigned int width, unsigned int height) : width(width), height(height)
{
    drawing_buffer.reserve(width * height * 3);
    waiting_buffer.reserve(width * height * 3);
    sendding_buffer.reserve(width * height * 3);
}

void BufferController::SwapDrawingBuffer()
{
    swap_mutex.lock();
    std::swap(drawing_buffer, waiting_buffer);
    new_on_wait = true;
    swap_mutex.unlock();
}

char* BufferController::GetBufferToSend()
{
    swap_mutex.lock();
    std::swap(sendding_buffer, waiting_buffer);
    new_on_wait = false;
    swap_mutex.unlock();
    return (char*)sendding_buffer.data()->data();
}

unsigned int BufferController::GetBufferToSendSize()
{
    return sendding_buffer.size();
}
#include "FifoBuffer.h"
#include <string.h>

//#define FIFO_BUFFER_DEBUG_TRACE

#ifdef FIFO_BUFFER_DEBUG_TRACE
#include <QDebug>
#endif

FIFOBuffer::FIFOBuffer(uint32_t depth, uint32_t size) :
    bufferPushIndex(0),
    bufferPopIndex(0)
{
    // Init Buffer
    init(depth, size);
}

FIFOBuffer::~FIFOBuffer()
{
    for(uint32_t i = 0; i < bufferDepth; i++)
    {
        delete []fifoBufferP[i];
    }
    delete []fifoBufferP;

    delete []sizeIndex;
}

bool FIFOBuffer::pushData(const char *dataP, uint32_t len)
{
    bool ret = false;

    if(NULL == dataP || 0 == len)
    {
        ret = false;
        return ret;
    }

    if(len >= bufferSize)
    {
        len = bufferSize;
    }

    // Write data to buffer
    memcpy(fifoBufferP[bufferPushIndex], dataP, len);

    // Store write length
    sizeIndex[bufferPushIndex] = len;

    // Move forward index
    bufferPushIndex = (bufferPushIndex + 1) % bufferDepth;

    ret = true;

#ifdef FIFO_BUFFER_DEBUG_TRACE
    qDebug() << "pushData() bufferPushIndex = " << bufferPushIndex << "len = " << len;
#endif

    return ret;
}

bool FIFOBuffer::popData(char *dataP, uint32_t &len)
{
    bool ret = false;

    if(NULL == dataP)
    {
        len = 0;
        ret = false;
        return ret;
    }

    if(0 == sizeIndex[bufferPopIndex])
    {
        len = 0;
        ret = false;

#ifdef FIFO_BUFFER_DEBUG_TRACE
        qDebug() << "popData() no data, bufferPopIndex = " << bufferPopIndex;
#endif

        return ret;
    }

    // Copy buffer data to dataP
    memcpy(dataP, fifoBufferP[bufferPopIndex], sizeIndex[bufferPopIndex]);
    len = sizeIndex[bufferPopIndex];

    // Pop succeed, clear this bufferPopIndex area
    memset(fifoBufferP[bufferPopIndex], 0, bufferSize);

    // Clear index size, it means there's no data and writeable
    sizeIndex[bufferPopIndex] = 0;

    // Move forward index
    bufferPopIndex = (bufferPopIndex + 1) % bufferDepth;

    ret = true;

#ifdef FIFO_BUFFER_DEBUG_TRACE
    qDebug() << "popData() bufferPopIndex = " << bufferPopIndex << "len = " << len;
#endif

    return ret;
}


void FIFOBuffer::init(uint32_t depth, uint32_t size)
{
    if(0 == depth || 0 == size)
    {
        return;
    }

    // Reset push&pop index
    bufferPushIndex = 0;
    bufferPopIndex = 0;

    // Set buffer depth & size
    bufferDepth = depth;
    bufferSize = size;

    // Malloc buffer in memory
    sizeIndex = new uint32_t [bufferDepth];

    fifoBufferP = new char * [bufferDepth];
    for(uint32_t i = 0; i < bufferDepth; i++)
    {
        fifoBufferP[i] = new char [bufferSize];
    }

    // Clear buffer content
    clear();

}

void FIFOBuffer::clear()
{
    if(NULL == sizeIndex || NULL == fifoBufferP)
    {
        return;
    }

    // Clear buffer content
    memset(sizeIndex, 0, sizeof(uint32_t) * bufferDepth);

    for(uint32_t i = 0; i < bufferDepth; i++)
    {
        memset(fifoBufferP[i], 0, sizeof(char) * bufferSize);
    }

}

uint32_t FIFOBuffer::getDepth() const
{
    return bufferDepth;
}

uint32_t FIFOBuffer::getSize() const
{
    return bufferSize;
}

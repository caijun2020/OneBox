#ifndef FIFOBUFFER_H
#define FIFOBUFFER_H
#include <stdint.h>

/*
 The total buffer size = depth * bufferSize
 The struct of FIFOBuffer is shown as below,

 fifoBufferP [depth][bufferSize] =
 {
    buf0[bufferSize],
    buf1[bufferSize],
    ...
    buf(n-1)[bufferSize]
 }

 which is a 2-matrix buffer

 call pushDdata() will write data to buf0[], then bufferIndex++
 next time call pushDdata() will write data to buf1[], then bufferIndex++
    ...
*/

class FIFOBuffer
{
public:
    FIFOBuffer(uint32_t depth = FIFO_BUFFER_DEPTH, uint32_t size = FIFO_BUFFER_SIZE);
    virtual ~FIFOBuffer();

    // Write data to buffer
    bool pushData(const char * dataP, uint32_t len);

    // Read data from buffer
    bool popData(char *dataP, uint32_t &len);

    // Clear FIFO buffer
    void clear();

    // Return FIFO depth
    uint32_t getDepth() const;

    // Return FIFO buffer size
    uint32_t getSize() const;

private:

    enum FIFO_BUFFER_TYPE
    {
        FIFO_BUFFER_DEPTH = 100,
        FIFO_BUFFER_SIZE = 4096
    };

    uint32_t bufferPushIndex;
    uint32_t bufferPopIndex;

    uint32_t bufferDepth;
    uint32_t bufferSize;

    // Pointer of FIFO buffer
    char **fifoBufferP;

    // Used to store length of fifoBufferP[index][]
    uint32_t *sizeIndex;


    // Init buffer depth & size
    void init(uint32_t depth = FIFO_BUFFER_DEPTH, uint32_t size = FIFO_BUFFER_SIZE);

};

#endif // FIFOBUFFER_H

#ifndef LOOPBUFFER_H
#define LOOPBUFFER_H

#include <stdint.h>
#include <QByteArray>

class LoopBuffer
{
public:
    LoopBuffer(int bufferSize = LOOP_BUFFER_SIZE);
    virtual ~LoopBuffer();

    // Ture: if there is undeal data in buffer
    bool isUndealDataExist();

    // Set Minimum size of undeal data
    void setUndealDataMinSize(unsigned int minSize);

    // Clear Loop Buffer
    void clear();

    // Buffer Size or Length
    int size() const;
    int length() const;

    // Write Data to buffer
    void writeData(QByteArray temp);

    // Read Data from buffer
    QByteArray readData(uint32_t len);
    // Read All left data from buffer
    QByteArray readAll();

    // Get the size of undeal data
    int getUndealDataSize();

    // Set the end char of a message
    // If this function is called, then isEndFlagExist() will be active
    // otherwise isEndFlagExist() will always return true
    void setMsgEndChar(char flag);

private:
    enum
    {
        LOOP_BUFFER_SIZE = 10240
    };

    uint32_t readPointerPos;     // The read pointer postion in the LoopBuffer
    uint32_t writePointerPos;    // The write pointer postion in the LoopBuffer

    uint32_t undealDataMinSize;      // The minimum size of undeal data
    uint32_t undealDataSize;         // The size of undeal data

    uint32_t loopBufferSize;
    QByteArray *dataBuffer;     // Loop Buffer

    // This flag is used to indicate if it's needed to check received msg is complete
    // For example, '\n' is received which means a complete msg packet is received
    // then isUndealDataExist() return true
    bool checkEndFlag;

    // This char is used to check the rx msg is complete
    // For example, '\n' is the last char of a msg
    char endFlagChar;

    void init(int bufferSize = LOOP_BUFFER_SIZE);

    // Check if there's endFlag('\n') @(writePointerPos - 1)
    bool isEndFlagExist();
};

#endif // LOOPBUFFER_H

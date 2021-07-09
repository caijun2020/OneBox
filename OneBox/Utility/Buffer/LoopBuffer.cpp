#include "LoopBuffer.h"
#include <QDebug>

LoopBuffer::LoopBuffer(int bufferSize) :
    readPointerPos(0),
    writePointerPos(0),
    undealDataMinSize(1),
    undealDataSize(0),
    checkEndFlag(false)
{
    init(bufferSize);
}


LoopBuffer::~LoopBuffer()
{
    delete dataBuffer;
}

bool LoopBuffer::isUndealDataExist()
{
    bool isUndealDataFlag  = false;

    if(writePointerPos >= readPointerPos)
    {
        undealDataSize = writePointerPos - readPointerPos;
        if((undealDataSize >= undealDataMinSize) && isEndFlagExist())
        {
            isUndealDataFlag = true;
        }
        else
        {
            isUndealDataFlag = false;
        }
    }
    else
    {
        undealDataSize = writePointerPos + loopBufferSize - readPointerPos;
        if((undealDataSize >= undealDataMinSize) && isEndFlagExist())
        {
            isUndealDataFlag = true;
        }
        else
        {
            isUndealDataFlag = false;
        }
    }

    return isUndealDataFlag;
}

void LoopBuffer::setUndealDataMinSize(unsigned int minSize)
{
    undealDataMinSize = minSize;
}

void LoopBuffer::init(int bufferSize)
{
    dataBuffer = new QByteArray;
    dataBuffer->reserve(bufferSize);
    dataBuffer->clear();     //it's very necesssary to clear the buffer
    dataBuffer->resize(bufferSize);

    loopBufferSize = dataBuffer->size();
}


void LoopBuffer::clear()
{
    // Clear Buffer
    dataBuffer->clear();
    dataBuffer->resize(loopBufferSize);

    // Reset read/write position
    readPointerPos = 0;
    writePointerPos = 0;
}

int LoopBuffer::size() const
{
    return loopBufferSize;
}

int LoopBuffer::length() const
{
    return loopBufferSize;
}

void LoopBuffer::writeData(QByteArray temp)
{
    if( !temp.isEmpty() )
    {
        if( (writePointerPos + temp.count()) >= loopBufferSize )
        {
            dataBuffer->insert(writePointerPos, temp.left(loopBufferSize - writePointerPos));
            dataBuffer->remove(0, temp.count() - (loopBufferSize - writePointerPos));

            //qDebug() << "Remove length = " << temp.count() - (loopBufferSize - writePointerPos);
            //qDebug() << "size of dataBuffer = " << dataBuffer->size();
            //qDebug() << "temp.left(loopBufferSize - writePointerPos) = " << temp.left(loopBufferSize - writePointerPos);
            dataBuffer->prepend( temp.right(temp.count() - (loopBufferSize - writePointerPos)) );
            //qDebug() << "temp.right(temp.count() - (loopBufferSize - writePointerPos)) =" << temp.right(temp.count() - (loopBufferSize - writePointerPos));
            //qDebug() << "size of dataBuffer = " << dataBuffer->size();
            writePointerPos += temp.count();
            writePointerPos -= loopBufferSize;
        }
        else
        {
            dataBuffer->insert(writePointerPos, temp);
            writePointerPos += temp.count();
        }
        //qDebug() << "writePointerPos = " << writePointerPos;
    }
}


QByteArray LoopBuffer::readData(uint32_t len)
{
    QByteArray temp;
    temp.clear();

    if(true == isUndealDataExist())
    {
        if(len >= undealDataSize)
        {
            len = undealDataSize;
        }

        if((readPointerPos + len) >= loopBufferSize)
        {
            temp.append(dataBuffer->data() + readPointerPos, loopBufferSize - readPointerPos);
            temp.append(dataBuffer->data(), readPointerPos + len - loopBufferSize);

            readPointerPos = readPointerPos + len - loopBufferSize;
        }
        else
        {
            temp.append(dataBuffer->data() + readPointerPos, len);

            readPointerPos += len;
        }

        //qDebug() << "readPointerPos = " << readPointerPos;
    }

    return temp;
}

QByteArray LoopBuffer::readAll()
{
    int len = 0;
    QByteArray temp;
    temp.clear();

    if(true == isUndealDataExist())
    {
        len = undealDataSize;

        if((readPointerPos + len) >= loopBufferSize)
        {
            temp.append(dataBuffer->data() + readPointerPos, loopBufferSize - readPointerPos);
            temp.append(dataBuffer->data(), readPointerPos + len - loopBufferSize);

            readPointerPos = readPointerPos + len - loopBufferSize;
        }
        else
        {
            temp.append(dataBuffer->data() + readPointerPos, len);

            readPointerPos += len;
        }

        clear();
        //qDebug() << "readPointerPos = " << readPointerPos;
    }

    return temp;
}

int LoopBuffer::getUndealDataSize()
{
    int ret = 0;

    if(true == isUndealDataExist())
    {
        ret = undealDataSize;
    }

    return ret;
}

void LoopBuffer::setMsgEndChar(char flag)
{
    // Set checkEndFlag
    checkEndFlag = true;

    endFlagChar = flag;
}


bool LoopBuffer::isEndFlagExist()
{
    bool ret = true;

    if(true == checkEndFlag)
    {
        // if writePointerPos = 0, check the end of dataBuffer
        // else check (writePointerPos - 1)
        if(0 == writePointerPos)
        {
            if(endFlagChar == dataBuffer->at(loopBufferSize - 1) ||
                endFlagChar == dataBuffer->at(loopBufferSize - 2))
            {
                ret = true;
            }
            else
            {
                ret = false;
            }
        }
        else
        {
            if(endFlagChar == dataBuffer->at(writePointerPos - 1) ||
                endFlagChar == dataBuffer->at(writePointerPos - 2))
            {
                ret = true;
            }
            else
            {
                ret = false;
            }
        }
    }
    else
    {
        ret = true;
    }

    return ret;
}

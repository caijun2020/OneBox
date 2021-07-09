/**********************************************************************
PACKAGE:        Communication
FILE:           UdpClient.cpp
COPYRIGHT (C):  All rights reserved.

PURPOSE:        UDP Client interface
**********************************************************************/

#include "UdpClient.h"
#include <QMutexLocker>
#include <QDebug>

#define UDP_CLIENT_DEBUG_TRACE

UDPClient::UDPClient(QObject *parent) :
    QThread(parent),
    udpSocket(new QUdpSocket),
    fifoBuf(new FIFOBuffer)
{
    resetTxRxCnt();
}

UDPClient::~UDPClient()
{
  delete udpSocket;
  delete fifoBuf;
}

void UDPClient::run()
{
  while(1)
  {
      usleep( 1 );
  }

  exec();
}

void UDPClient::initSocket(const QHostAddress &address, uint16_t port)
{
    // If socket is already bind, need to close socket, then bind again
    closeSocket();

    udpSocket->bind(address, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
}

void UDPClient::closeSocket()
{
    udpSocket->close();
}

void UDPClient::sendData(QHostAddress &address, uint16_t port, const char *data, uint32_t len)
{
    if(NULL == data || 0 == len)
    {
        return;
    }

    hostAddr = address;
    serverPort = port;

    txPacketCnt++;
    txTotalBytesSize += len;

    udpSocket->writeDatagram(data, len, address, port);

    // Emit signal
    emit newDataTx(address, port, QByteArray(data, len));
}

void UDPClient::sendData(QHostAddress &address, uint16_t port, QByteArray &data)
{
    if(data.isEmpty())
    {
        return;
    }

    sendData(address, port, data.constData(), data.size());
}

void UDPClient::readPendingDatagrams()
{
    qDebug() << "readPendingDatagrams()";
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray temp;
        temp.resize(udpSocket->pendingDatagramSize());

        udpSocket->readDatagram(temp.data(), temp.size(), &clientAddr, &clientPort);

        if(!temp.isEmpty())
        {
            {
                QMutexLocker locker(&mutex);
                fifoBuf->pushData(temp.data(), temp.size());
            }

            rxPacketCnt++;
            rxTotalBytesSize += temp.size();

            // Emit signal
            emit newDataReady();

#ifdef UDP_CLIENT_DEBUG_TRACE
            QString ipPortStr;
            ipPortStr = clientAddr.toString();
            ipPortStr.append(":");
            ipPortStr.append(QString::number(clientPort));

            QString tmpStr;
            tmpStr.clear();

            for(int i = 0; i < temp.size(); i++)
            {
                tmpStr.append(QString::number((uint8_t)temp.at(i), 16).rightJustified(2, '0').toUpper());
                tmpStr.append(" ");
            }

            qDebug() << "Received data from : " << ipPortStr;
            qDebug() << tmpStr;
#endif
        }
    }
}

uint32_t UDPClient::getServerPort() const
{
    return serverPort;
}

QHostAddress UDPClient::getHostAddress() const
{
    return hostAddr;
}

bool UDPClient::getUndealData(char *dataP, uint32_t &len)
{
    bool ret = false;
    QMutexLocker locker(&mutex);

    // Pop data from FIFO
    ret = fifoBuf->popData(dataP, len);

    return ret;
}

bool UDPClient::getUndealData(QByteArray &data)
{
    bool ret = false;
    uint32_t len = 0;
    QByteArray tempData;
    tempData.reserve(fifoBuf->getSize());

    // Pop data from FIFO
    ret = getUndealData(tempData.data(), len);

    // Resize data to the real size
    tempData.resize(len);

    data = tempData;

    return ret;
}

uint32_t UDPClient::getTxDiagramCnt() const
{
    return txPacketCnt;
}

uint32_t UDPClient::getRxDiagramCnt() const
{
    return rxPacketCnt;
}

uint32_t UDPClient::getTotalTxBytes() const
{
    return txTotalBytesSize;
}

uint32_t UDPClient::getTotalRxBytes() const
{
    return rxTotalBytesSize;
}

void UDPClient::resetTxRxCnt()
{
    txPacketCnt = 0;
    rxPacketCnt = 0;

    txTotalBytesSize = 0;
    rxTotalBytesSize = 0;
}

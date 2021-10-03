/**********************************************************************
PACKAGE:        Communication
FILE:           UdpServer.cpp
COPYRIGHT (C):  All rights reserved.

PURPOSE:        UDP Server interface
**********************************************************************/

#include "UdpServer.h"
#include <QMutexLocker>
#include <QDebug>

#undef UDP_SERVER_DEBUG_TRACE

UDPServer::UDPServer(QObject *parent) :
    QThread(parent),
    udpSocket(new QUdpSocket),
    fifoBuf(new FIFOBuffer),
    txPacketCnt(0),
    rxPacketCnt(0),
    txTotalBytesSize(0),
    rxTotalBytesSize(0),
    isRunning(false)
{
}

UDPServer::~UDPServer()
{
    delete udpSocket;
    delete fifoBuf;
}

void UDPServer::run()
{
    while(1)
    {
        usleep( 1 );
    }

    exec();
}

void UDPServer::initSocket(const QHostAddress &address, uint16_t port)
{
    hostAddr = address;
    serverPort = port;

	
    // If socket is already bind, need to close socket, then bind again
    closeSocket();

    udpSocket->bind(hostAddr, serverPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
	
	isRunning = true;
	
	// Emit signal
    emit serverChanged(hostAddr, serverPort);
	emit connectionChanged(isRunning);	
}

void UDPServer::closeSocket()
{
    udpSocket->close();
	
	isRunning = false;

    // Emit signal
    emit connectionChanged(isRunning);
}

void UDPServer::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray temp;
        temp.resize(udpSocket->pendingDatagramSize());

        udpSocket->readDatagram(temp.data(), temp.size(), &clientAddr, &clientPort);

        // If client is new, add it to list
        addClientToList(clientAddr, clientPort);

        if(!temp.isEmpty())
        {
            {
                QMutexLocker locker(&mutex);
                fifoBuf->pushData(temp.data(), temp.size());
            }

            rxPacketCnt++;
            rxTotalBytesSize += temp.size();

            int index = getClientIndex(clientAddr, clientPort);
            if(index != -1)
            {
                // Emit signal
                emit newDataReady(index);
                emit newDataReady(index, temp);
            }

#ifdef UDP_SERVER_DEBUG_TRACE
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

uint32_t UDPServer::getServerPort() const
{
    return serverPort;
}

QHostAddress UDPServer::getHostAddress() const
{
    return hostAddr;
}

uint32_t UDPServer::getConnectionCount() const
{
    return clientList.size();
}

QString UDPServer::getClientInfo(uint32_t clientIndex)
{
    QString infoStr = "";
    if(clientIndex < (uint32_t)clientList.size())
    {
        infoStr = clientList[clientIndex].address.toString();
        infoStr.append(":");
        infoStr.append(QString::number(clientList[clientIndex].port));
    }

    return infoStr;
}

bool UDPServer::getUndealData(char *dataP, uint32_t &len)
{
    bool ret = false;
    QMutexLocker locker(&mutex);

    // Pop data from FIFO
    ret = fifoBuf->popData(dataP, len);

    return ret;
}

bool UDPServer::getUndealData(QByteArray &data)
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

void UDPServer::sendData(uint32_t clientIndex, const char *data, uint32_t len)
{
    if(clientIndex >= (uint32_t)clientList.size())
    {
        qDebug("clientIndex %d is out of connections", clientIndex);
        return;
    }

    sendData(clientList[clientIndex].address, clientList[clientIndex].port, data, len);
}

void UDPServer::sendData(uint32_t clientIndex, QByteArray &data)
{
    sendData(clientIndex, data.constData(), data.size());
}

void UDPServer::sendData(QHostAddress &address, uint16_t port, const char *data, uint32_t len)
{
    if(NULL == data || 0 == len)
    {
        return;
    }

    txPacketCnt++;
    txTotalBytesSize += len;

    udpSocket->writeDatagram(data, len, address, port);

    // Emit signal
    emit newDataTx(address, port, QByteArray(data, len));
}

void UDPServer::sendData(QHostAddress &address, uint16_t port, QByteArray &data)
{
    sendData(address, port, data.constData(), data.size());
}

void UDPServer::addClientToList(QHostAddress address, uint16_t port)
{
    int index = -1;

    struct INCOMING_CLIENT_INFO temp;
    temp.address = address;
    temp.port = port;

    for(index = 0; index < clientList.size(); index++)
    {
        if(clientList[index].address == address
                && clientList[index].port == port)
        {
            break;
        }
    }

    // If not exist in list, add it to list
    if(index >= clientList.size())
    {
        clientList.append(temp);

        // Emit signals to notice connection changed
        emit connectionIn(getClientInfo(clientList.size() - 1));
    }
}

void UDPServer::removeClientFromList(QHostAddress address, uint16_t port)
{
    int index = -1;
    struct INCOMING_CLIENT_INFO temp;
    temp.address = address;
    temp.port = port;

    for(index = 0; index < clientList.size(); index++)
    {
        if(clientList[index].address == address
                && clientList[index].port == port)
        {
            break;
        }
    }

    // If exist in list, remove it from list
    if(index < clientList.size())
    {
        // Emit signals to notice connection changed
        emit connectionOut(getClientInfo(index));

        clientList.removeAt(index);
    }
}

int UDPServer::getClientIndex(QHostAddress address, uint16_t port)
{
    int index = -1;
    struct INCOMING_CLIENT_INFO temp;
    temp.address = address;
    temp.port = port;

    for(index = 0; index < clientList.size(); index++)
    {
        if(clientList[index].address == address
                && clientList[index].port == port)
        {
            break;
        }
    }

    // If not exist in list, return -1
    if(index >= clientList.size())
    {
        index = -1;
    }

    return index;
}

uint32_t UDPServer::getTxDiagramCnt() const
{
    return txPacketCnt;
}

uint32_t UDPServer::getRxDiagramCnt() const
{
    return rxPacketCnt;
}

uint32_t UDPServer::getTotalTxBytes() const
{
    return txTotalBytesSize;
}

uint32_t UDPServer::getTotalRxBytes() const
{
    return rxTotalBytesSize;
}

void UDPServer::resetTxRxCnt()
{
    txPacketCnt = 0;
    rxPacketCnt = 0;

    txTotalBytesSize = 0;
    rxTotalBytesSize = 0;
}

bool UDPServer::getRunningStatus() const
{
    return isRunning;
}

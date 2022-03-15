/**********************************************************************
PACKAGE:        Communication
FILE:           UdpServer.cpp
COPYRIGHT (C):  All rights reserved.

PURPOSE:        UDP Server interface
**********************************************************************/

#include "UdpServer.h"
#include <QMutexLocker>
#include <QDebug>

//#define UDP_SERVER_DEBUG_TRACE

UDPServer::UDPServer(QObject *parent) :
    QThread(parent),
    udpSocket(NULL),
    fifoBuf(new FIFOBuffer(8000, 1500)),
    txPacketCnt(0),
    rxPacketCnt(0),
    txTotalBytesSize(0),
    rxTotalBytesSize(0),
    isRunning(false),
    timerForCheck(NULL),
    lostCheckImMs(500),
    lostCheckEnabled(false)
{
    // Register data type to remove warning while running
    qRegisterMetaType<QAbstractSocket::SocketError>("SocketError");

    connect(this, SIGNAL(startConnectionCheck()), this, SLOT(startCheckTimer()));
    connect(this, SIGNAL(stopConnectionCheck()), this, SLOT(stopCheckTimer()));
    connect(this, SIGNAL(startListen()), this, SLOT(startSocket()));
    connect(this, SIGNAL(stopListen()), this, SLOT(stopSocket()));
}

UDPServer::~UDPServer()
{
    if(NULL != udpSocket)
    {
        delete udpSocket;
        udpSocket = NULL;
    }

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

    // Emit signal
    emit serverChanged(hostAddr, serverPort);
    emit startListen();

}

void UDPServer::closeSocket()
{
    // Emit signal
    emit stopListen();
}

void UDPServer::startSocket()
{
    // If socket is already bind, need to close socket, then bind again
    stopSocket();

    // As there is the same mutex lock in stopSocket(), can not place locker before stopSocket()
    // Need to wait until lock released
    QMutexLocker locker(&mutex);

    udpSocket = new QUdpSocket;
    udpSocket->bind(hostAddr, serverPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
    connect(udpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(handleError(QAbstractSocket::SocketError)));

    isRunning = true;

    // Emit signal
    emit connectionChanged(isRunning);
    emit startConnectionCheck();
}

void UDPServer::stopSocket()
{
    QMutexLocker locker(&mutex);

    if(NULL != udpSocket)
    {
        udpSocket->close();

        disconnect(udpSocket, 0, this, 0);
        delete udpSocket;
        udpSocket = NULL;
    }

    isRunning = false;

    // Emit signal
    emit connectionChanged(isRunning);
    emit stopConnectionCheck();
}

void UDPServer::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray temp;
        temp.resize(udpSocket->pendingDatagramSize());

        int rxLen = udpSocket->readDatagram(temp.data(), temp.size(), &clientAddr, &clientPort);
        if(rxLen != temp.size())
        {
            qDebug() << "readDatagram length != pendingDatagramSize() rxLen=" << rxLen;
        }

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

void UDPServer::handleError(QAbstractSocket::SocketError errNo)
{
    QString logStr;

    logStr = QString("UDPServer::Error Code = %1").arg(errNo);
    qDebug() << logStr;

    logStr = QString("UDPServer::Error string = %1").arg(udpSocket->errorString());
    qDebug() << logStr;
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
        //qDebug("clientIndex %d is out of connections", clientIndex);
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
    QMutexLocker locker(&mutex);

    if(NULL == data || 0 == len)
    {
        return;
    }

    // When socket is close, do not send out data
    if(NULL == udpSocket)
    {
        return;
    }

    if(udpSocket->writeDatagram(data, len, address, port) >= 0)
    {
        txPacketCnt++;
        txTotalBytesSize += len;

        // Emit signal
        emit newDataTx(address, port, QByteArray(data, len));
    }
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

void UDPServer::startCheckTimer()
{
    stopCheckTimer();

    if(lostCheckEnabled)
    {
        timerForCheck = new QTimer;
        connect(timerForCheck, SIGNAL(timeout()), this, SLOT(lostConnectionCheck()));
        timerForCheck->start(lostCheckImMs);  //timeOut = 1s
    }
}

void UDPServer::stopCheckTimer()
{
    if(NULL != timerForCheck)
    {
        timerForCheck->stop();
        disconnect(timerForCheck, 0, this, 0);
        delete timerForCheck;
        timerForCheck = NULL;
    }
}

void UDPServer::lostConnectionCheck()
{
    static uint32_t oldRxCnt = 0;
    static uint32_t errCnt = 0;
    uint32_t newRxCnt = 0;

    QString logStr = "UDPServer::lostConnectionCheck() initSocket again";

    if(isRunning)
    {
        newRxCnt = getRxDiagramCnt();

        if(newRxCnt <= oldRxCnt)
        {
            if(++errCnt >= MAX_ERROR_CNT)
            {
                errCnt = 0;

                // Update log
                qDebug() << logStr;

                // Emit signal to initSocket again
                emit startListen();

                // Mask initSocket, call readPendingDatagrams() to read data from buffer
                // Then signal readyRead() may received normally!
                //readPendingDatagrams();
            }
        }
        else
        {
            errCnt = 0;
        }

        oldRxCnt = newRxCnt;
    }
}

bool UDPServer::getRunningStatus() const
{
    return isRunning;
}

void UDPServer::setLostCheckEnabled(bool flag)
{
    lostCheckEnabled = flag;
}

bool UDPServer::getLostCheckFlag() const
{
    return lostCheckEnabled;
}

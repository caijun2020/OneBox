/**********************************************************************
PACKAGE:        Communication
FILE:           TcpClient.cpp
COPYRIGHT (C):  All rights reserved.

PURPOSE:        TCP Client interface
**********************************************************************/

#include "TcpClient.h"

#undef TCP_CLIENT_DEBUG_TRACE

TCPClient::TCPClient(QObject *parent) :
    tcpClient(new QTcpSocket(this)),
    fifoBuf(new FIFOBuffer),
    hostAddr(QHostAddress::Any),
    listenPort(0)
{
    resetTxRxCnt();

    // Disconnect all connections
    tcpClient->abort();

    connect(tcpClient, SIGNAL(readyRead()), this, SLOT(readPendingData()));
    connect(tcpClient, SIGNAL(disconnected()), this, SLOT(removeConnection()));
    connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(readError(QAbstractSocket::SocketError)));
}

TCPClient::~TCPClient()
{
    delete tcpClient;
    delete fifoBuf;
}

void TCPClient::run()
{
    while(1)
    {
        usleep( 1 );
    }

    exec();
}

void TCPClient::readPendingData()
{
    QByteArray temp = tcpClient->readAll();

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

#ifdef TCP_CLIENT_DEBUG_TRACE
        QString ipPortStr;
        ipPortStr = tcpClient->peerAddress().toString();
        ipPortStr.append(":");
        ipPortStr.append(QString::number(tcpClient->peerPort()));

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


void TCPClient::readError(QAbstractSocket::SocketError)
{
    tcpClient->disconnectFromHost();

    qDebug() << "failed to connect server : " << tcpClient->errorString();
}

void TCPClient::removeConnection()
{
    // If the client in UnconnectedState
    if(tcpClient->state() == QAbstractSocket::UnconnectedState)
    {
        // Emit signals to notice connection changed
        emit connectionOut();
    }
}

bool TCPClient::connectToServer(const QHostAddress &ip, uint16_t port)
{
    bool ret = false;

    tcpClient->connectToHost(ip, port);
    if (tcpClient->waitForConnected(1000))
    {
        ret = true;
        hostAddr = ip;
        listenPort = port;
    }

    return ret;
}

bool TCPClient::connectToServer(QString ip, uint16_t port)
{
    bool ret = false;

    tcpClient->connectToHost(ip, port);
    if (tcpClient->waitForConnected(1000))
    {
        ret = true;
        hostAddr = QHostAddress(ip);
        listenPort = port;
    }

    return ret;
}

void TCPClient::disconnectFromServer()
{
    tcpClient->disconnectFromHost();
    if (tcpClient->state() == QAbstractSocket::UnconnectedState ||
             tcpClient->waitForDisconnected(1000))
    {
        qDebug() << "disconnected successfully";
    }
    else
    {
        qDebug() << tcpClient->errorString();
    }
}

uint32_t TCPClient::getListenPort() const
{
    return listenPort;
}

QHostAddress TCPClient::getHostAddress() const
{
    return hostAddr;
}

bool TCPClient::getUndealData(char *dataP, uint32_t &len)
{
    bool ret = false;
    QMutexLocker locker(&mutex);

    // Pop data from FIFO
    ret = fifoBuf->popData(dataP, len);

    return ret;
}

bool TCPClient::getUndealData(QByteArray &data)
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

void TCPClient::sendData(const char *data, uint32_t len)
{
    // If not in connected state then return
    if(tcpClient->state() != QAbstractSocket::ConnectedState)
    {
        return;
    }

    if(NULL == data || 0 == len)
    {
        return;
    }

    txPacketCnt++;
    txTotalBytesSize += len;

    tcpClient->write((char *)data, len);

    // Emit signal
    emit newDataTx(hostAddr, listenPort, QByteArray(data, len));
}

void TCPClient::sendData(QByteArray &data)
{
    sendData(data.constData(), data.size());
}

uint32_t TCPClient::getTxDiagramCnt() const
{
    return txPacketCnt;
}

uint32_t TCPClient::getRxDiagramCnt() const
{
    return rxPacketCnt;
}

uint32_t TCPClient::getTotalTxBytes() const
{
    return txTotalBytesSize;
}

uint32_t TCPClient::getTotalRxBytes() const
{
    return rxTotalBytesSize;
}

void TCPClient::resetTxRxCnt()
{
    txPacketCnt = 0;
    rxPacketCnt = 0;

    txTotalBytesSize = 0;
    rxTotalBytesSize = 0;
}

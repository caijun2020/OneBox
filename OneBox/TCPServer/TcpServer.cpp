/**********************************************************************
PACKAGE:        Communication
FILE:           TcpServer.cpp
COPYRIGHT (C):  All rights reserved.

PURPOSE:        TCP Server interface
**********************************************************************/

#include "TcpServer.h"
#include <QMutexLocker>

#undef TCP_SERVER_DEBUG_TRACE

TCPServer::TCPServer(QObject *parent) :
    QThread(parent),
    tcpServer(new QTcpServer(this)),
    fifoBuf(new FIFOBuffer),
    hostAddr(QHostAddress::Any),
    listenPort(0)
{
    tcpClientList.clear();
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));

    resetTxRxCnt();
}

TCPServer::~TCPServer()
{
    stopListen();

    delete tcpServer;
    delete fifoBuf;
}

void TCPServer::run()
{
    while(1)
    {
        usleep( 1 );
    }

    exec();
}

void TCPServer::acceptConnection()
{
    currentClient = tcpServer->nextPendingConnection();

    tcpClientList.append(currentClient);

    connect(currentClient, SIGNAL(readyRead()), this, SLOT(readPendingData()));
    connect(currentClient, SIGNAL(disconnected()), this, SLOT(removeConnection()));

    // Emit signals to notice connection changed
    emit connectionChanged();
    emit connectionIn(getClientInfo(tcpClientList.size() - 1));
}


void TCPServer::removeConnection()
{
    // Search and remove the client in UnconnectedState
    for(int i = (tcpClientList.length() - 1); i >= 0; i--)
    {
        if(tcpClientList[i]->state() == QAbstractSocket::UnconnectedState)
        {
            // Emit signals to notice connection changed
            emit connectionChanged();
            emit connectionOut(getClientInfo(i));

            tcpClientList.removeAt(i);
        }
    }
}

void TCPServer::readPendingData()
{
    // Search all clients
    for(int i = 0; i < tcpClientList.length(); i++)
    {
        QByteArray temp = tcpClientList[i]->readAll();

        if(temp.isEmpty())
        {
            continue;
        }
        else
        {
            {
                QMutexLocker locker(&mutex);
                fifoBuf->pushData(temp.data(), temp.size());
            }

            rxPacketCnt++;
            rxTotalBytesSize += temp.size();

            // Emit signal
            emit newDataReady(i);

#ifdef TCP_SERVER_DEBUG_TRACE
            QString ipPortStr;
            ipPortStr = currentClient->peerAddress().toString();
            ipPortStr.append(":");
            ipPortStr.append(QString::number(currentClient->peerPort()));

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

bool TCPServer::beginListen(const QHostAddress &address, uint16_t port)
{
    bool ret = tcpServer->listen(address, port);

    if(ret)
    {
        hostAddr = address;
        listenPort = port;
    }

    return ret;
}

void TCPServer::stopListen()
{
    //QMutexLocker locker(&mutex);
    QTcpSocket* socket;

    for(int i = (tcpClientList.length() - 1); i >= 0; i--)
    {
        socket = tcpClientList[i];
        socket->disconnectFromHost();
        bool ok = socket->waitForDisconnected(1000);
        if(!ok)
        {
            qDebug() << socket->errorString();
        }
    }

    tcpServer->close();
}

uint32_t TCPServer::getListenPort() const
{
    if(tcpServer->isListening())
    {
        return listenPort;
    }
}

QHostAddress TCPServer::getHostAddress() const
{
    return hostAddr;
}

uint32_t TCPServer::getConnectionCount() const
{
    return tcpClientList.size();
}

QString TCPServer::getClientInfo(uint32_t clientIndex)
{
    QString infoStr = "";
    if(clientIndex < (uint32_t)tcpClientList.size())
    {
        infoStr = tcpClientList[clientIndex]->peerAddress().toString();
        infoStr.append(":");
        infoStr.append(QString::number(tcpClientList[clientIndex]->peerPort()));
    }

    return infoStr;
}

QHostAddress TCPServer::getClientAddress(uint32_t clientIndex)
{
    QHostAddress clientAddress;
    if(clientIndex < (uint32_t)tcpClientList.size())
    {
        clientAddress = tcpClientList[clientIndex]->peerAddress();
    }

    return clientAddress;
}

quint16 TCPServer::getClientPort(uint32_t clientIndex)
{
    quint16 clientPort = 0;
    if(clientIndex < (uint32_t)tcpClientList.size())
    {
        clientPort = tcpClientList[clientIndex]->peerPort();
    }

    return clientPort;
}

bool TCPServer::getUndealData(char *dataP, uint32_t &len)
{
    bool ret = false;
    QMutexLocker locker(&mutex);

    // Pop data from FIFO
    ret = fifoBuf->popData((char *)dataP, len);

    return ret;
}

bool TCPServer::getUndealData(QByteArray &data)
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

void TCPServer::sendData(uint32_t clientIndex, const char *data, uint32_t len)
{
    if(clientIndex >= (uint32_t)tcpClientList.size())
    {
        return;
    }

    if(NULL == data || 0 == len)
    {
        return;
    }

    // If not in connected state then return
    if(tcpClientList[clientIndex]->state() != QAbstractSocket::ConnectedState)
    {
        return;
    }

    txPacketCnt++;
    txTotalBytesSize += len;

    tcpClientList[clientIndex]->write(data, len);

    // Emit signal
    emit newDataTx(getClientAddress(clientIndex), getClientPort(clientIndex), QByteArray(data, len));
}

void TCPServer::sendData(uint32_t clientIndex, QByteArray &data)
{
    sendData(clientIndex, data.constData(), data.size());
}

uint32_t TCPServer::getTxDiagramCnt() const
{
    return txPacketCnt;
}

uint32_t TCPServer::getRxDiagramCnt() const
{
    return rxPacketCnt;
}

uint32_t TCPServer::getTotalTxBytes() const
{
    return txTotalBytesSize;
}

uint32_t TCPServer::getTotalRxBytes() const
{
    return rxTotalBytesSize;
}

void TCPServer::resetTxRxCnt()
{
    txPacketCnt = 0;
    rxPacketCnt = 0;

    txTotalBytesSize = 0;
    rxTotalBytesSize = 0;
}

/**********************************************************************
PACKAGE:        Communication
FILE:           TcpServer.h
COPYRIGHT (C):  All rights reserved.

PURPOSE:        TCP Server interface
**********************************************************************/

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QHostAddress>
#include <QByteArray>
#include <QMutex>

#include "FifoBuffer.h"


class TCPServer : public QThread
{
    Q_OBJECT
public:
    explicit TCPServer(QObject *parent = 0);
    ~TCPServer();

    void run();

    // Begin Tcp listen
    bool beginListen(const QHostAddress &address = QHostAddress::Any, uint16_t port = 0);

    // Stop listen
    void stopListen();

    uint32_t getListenPort() const;
    QHostAddress getHostAddress() const;

    // Return the count of clients connected to the server
    uint32_t getConnectionCount() const;

    QString getClientInfo(uint32_t clientIndex);

    // True: if there is undeal data in buffer
    // False: no data in buffer
    bool getUndealData(char *dataP, uint32_t &len);
    bool getUndealData(QByteArray &data);

    // Send data to client
    // @para  clientIndex -- the index number of tcpClientList
    void sendData(uint32_t clientIndex, const char *data, uint32_t len);
    void sendData(uint32_t clientIndex, QByteArray &data);

    uint32_t getTxDiagramCnt() const;
    uint32_t getRxDiagramCnt() const;

    uint32_t getTotalTxBytes() const;
    uint32_t getTotalRxBytes() const;

    void resetTxRxCnt();

signals:
    void connectionChanged(void);
    void connectionIn(QString);
    void connectionOut(QString);
    void newDataReady(uint32_t clientIndex);
    void newDataTx(QHostAddress, uint16_t, QByteArray);

private:
    QTcpServer *tcpServer;
    QList<QTcpSocket*> tcpClientList;
    QTcpSocket *currentClient;

    FIFOBuffer *fifoBuf;

    QHostAddress hostAddr;
    uint16_t listenPort;

    uint32_t txPacketCnt;
    uint32_t rxPacketCnt;
    uint32_t txTotalBytesSize;
    uint32_t rxTotalBytesSize;

    QMutex mutex;   // locker

    QHostAddress getClientAddress(uint32_t clientIndex);
    quint16 getClientPort(uint32_t clientIndex);

private slots:
    void acceptConnection();
    void removeConnection();
    void readPendingData();
};

#endif // TCPSERVER_H

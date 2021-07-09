/**********************************************************************
PACKAGE:        Communication
FILE:           UdpServer.h
COPYRIGHT (C):  All rights reserved.

PURPOSE:        UDP Server interface
**********************************************************************/

#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>
#include <QList>
#include <QMutex>

#include "FifoBuffer.h"

class UDPServer : public QThread
{
    Q_OBJECT
public:
    explicit UDPServer(QObject *parent = 0);
    virtual ~UDPServer();

    void run();

    void initSocket(const QHostAddress &address = QHostAddress::Any, uint16_t port = 0);
    void closeSocket();

    QHostAddress getHostAddress() const;
    uint32_t getServerPort() const;

    // Return the count of clients connected to the server
    uint32_t getConnectionCount() const;

    QString getClientInfo(uint32_t clientIndex);

    // True: if there is undeal data in buffer
    // False: no data in buffer
    bool getUndealData(char *dataP, uint32_t &len);
    bool getUndealData(QByteArray &data);

    // Send data to client
    // @para  clientIndex -- the index number of clientList
    void sendData(uint32_t clientIndex, const char *data, uint32_t len);
    void sendData(uint32_t clientIndex, QByteArray &data);

    void sendData(QHostAddress &address, uint16_t port, const char *data, uint32_t len);
    void sendData(QHostAddress &address, uint16_t port, QByteArray &data);

    uint32_t getTxDiagramCnt() const;
    uint32_t getRxDiagramCnt() const;

    uint32_t getTotalTxBytes() const;
    uint32_t getTotalRxBytes() const;

    void resetTxRxCnt();

signals:
    void connectionIn(QString);
    void connectionOut(QString);
    void newDataReady(int);
    void newDataReady(int, QByteArray);
    void newDataTx(QHostAddress, int, QByteArray);
    void message(const QString& info);

private:

    struct INCOMING_CLIENT_INFO
    {
       QHostAddress address;
       uint16_t port;
    };

    QUdpSocket *udpSocket;

    FIFOBuffer *fifoBuf;

    QHostAddress hostAddr;      // Host IP
    uint16_t serverPort;        // Host port

    QHostAddress clientAddr;    // Incoming IP
    uint16_t clientPort;        // Incoming port

    QList<struct INCOMING_CLIENT_INFO> clientList;

    uint32_t txPacketCnt;
    uint32_t rxPacketCnt;
    uint32_t txTotalBytesSize;
    uint32_t rxTotalBytesSize;

    QMutex mutex;   // Mutex lock

    // Add new client to list
    void addClientToList(QHostAddress address, uint16_t port);

    // Remove client from list
    void removeClientFromList(QHostAddress address, uint16_t port);

    // According to address & port, get index from list
    int getClientIndex(QHostAddress address, uint16_t port);

private slots:
    void readPendingDatagrams();
};

#endif // UDPSERVER_H

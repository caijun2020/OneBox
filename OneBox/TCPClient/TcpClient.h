/**********************************************************************
PACKAGE:        Communication
FILE:           TcpClient.h
COPYRIGHT (C):  All rights reserved.

PURPOSE:        TCP Client interface
**********************************************************************/

#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>
#include <QMutex>

#include "FifoBuffer.h"


class TCPClient : public QThread
{
    Q_OBJECT
public:
    explicit TCPClient(QObject *parent = 0);
    ~TCPClient();

    void run();

    // Connect to Server
    bool connectToServer(const QHostAddress &ip = QHostAddress::Any, uint16_t port = 0);
    bool connectToServer(QString ip, uint16_t port = 0);

    // Disconnect from server
    void disconnectFromServer();

    uint32_t getListenPort() const;
    QHostAddress getHostAddress() const;

    // True: if there is undeal data in buffer
    // False: no data in buffer
    bool getUndealData(char *dataP, uint32_t &len);
    bool getUndealData(QByteArray &data);

    // Send data to server
    void sendData(const char *data, uint32_t len);
    void sendData(QByteArray &data);

    uint32_t getTxDiagramCnt() const;
    uint32_t getRxDiagramCnt() const;

    uint32_t getTotalTxBytes() const;
    uint32_t getTotalRxBytes() const;

    void resetTxRxCnt();

signals:
    void newDataReady(void);
    void connectionOut(void);
    void newDataTx(QHostAddress, uint16_t, QByteArray);

private:
    QTcpSocket *tcpClient;

    FIFOBuffer *fifoBuf;

    QHostAddress hostAddr;
    uint16_t listenPort;

    uint32_t txPacketCnt;
    uint32_t rxPacketCnt;
    uint32_t txTotalBytesSize;
    uint32_t rxTotalBytesSize;

    QMutex mutex; // Mutex locker

private slots:
    void readPendingData();
    void removeConnection();
    void readError(QAbstractSocket::SocketError);
};

#endif // CTCPCLIENT_H

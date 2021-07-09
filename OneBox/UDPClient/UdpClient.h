/**********************************************************************
PACKAGE:        Communication
FILE:           UdpClient.h
COPYRIGHT (C):  All rights reserved.

PURPOSE:        UDP Client interface
**********************************************************************/

#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>
#include <QMutex>

#include "FifoBuffer.h"

class UDPClient : public QThread
{
    Q_OBJECT
public:
    explicit UDPClient(QObject *parent = 0);
    ~UDPClient();

    void run();

    void initSocket(const QHostAddress &address = QHostAddress::Any, uint16_t port = 0);
    void closeSocket();
    
    QHostAddress getHostAddress() const;
    uint32_t getServerPort() const;

    // True: if there is undeal data in buffer
    // False: no data in buffer
    bool getUndealData(char *dataP, uint32_t &len);
    bool getUndealData(QByteArray &data);

    // Send data
    void sendData(QHostAddress &address, uint16_t port, const char *data, uint32_t len);
    void sendData(QHostAddress &address, uint16_t port, QByteArray &data);

    uint32_t getTxDiagramCnt() const;
    uint32_t getRxDiagramCnt() const;

    uint32_t getTotalTxBytes() const;
    uint32_t getTotalRxBytes() const;

    void resetTxRxCnt();

signals:
    void newDataReady(void);
    void newDataTx(QHostAddress, uint16_t, QByteArray);

public slots:

private:
    QUdpSocket *udpSocket;

    FIFOBuffer *fifoBuf;

    QHostAddress hostAddr;      // Host IP
    uint16_t serverPort;        // Host port

    QHostAddress clientAddr;    // Incoming IP
    uint16_t clientPort;        // Incoming port

    uint32_t txPacketCnt;
    uint32_t rxPacketCnt;
    uint32_t txTotalBytesSize;
    uint32_t rxTotalBytesSize;

    QMutex mutex;   // Mutex lock

private slots:
    void readPendingDatagrams();
};

#endif // UDPCLIENT_H

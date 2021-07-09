/**********************************************************************
PACKAGE:        Communication
FILE:           QSerialPort.h
COPYRIGHT (C):  All rights reserved.

PURPOSE:        Serial Port interface based on QextSerialBase
**********************************************************************/

#ifndef QSERIALPORT_H
#define QSERIALPORT_H
#include <QThread>
#include <QTimer>
#include <QMutex>
#include <QStringList>

#include "qextserialbase.h"
#include "ComInitData.h"
#include "LoopBuffer.h"


class QSerialPort : public QThread
{
    Q_OBJECT
public:
    QSerialPort(struct COM_PORT_INIT_DATA *initData);
    QSerialPort();
    virtual ~QSerialPort();

    void run();

    bool open(struct COM_PORT_INIT_DATA *initData);     // Init Serial Port
    void close();

    // Write data to printer
    int writeData(const char *txData, int len);
    int writeData(QByteArray &txData);

    bool isOpen();   // Check whether the COM port is opened successful

    void setBaudRate(BaudRateType baudrate);
    void setParity(ParityType parity);
    void setDataBits(DataBitsType databits);
    void setStopBits(StopBitsType stopbits);
    void setFlowControl(FlowType flowtype);

    QStringList getAvailablePorts();

    uint32_t getTotalTxBytes() const;
    uint32_t getTotalRxBytes() const;
    void resetTxRxCnt();

    // True: if there is undeal data in buffer
    // False: no data in buffer
    bool getUndealData(uint8_t *dataP, uint32_t &len);
    bool getUndealData(QByteArray &data);

signals:
    void newDataReady(QByteArray);
    void newDataTx(QByteArray);
    void startPolling();
    void stopPolling();

protected slots:
    void readDataFromCOM();      // Read data from COM port
    void startPollingTimer();    // Start polling timer
    void stopPollingTimer();    // Stop polling timer

private:
    enum
    {
        RX_BUF_SIZE  = 2000,
        TX_BUF_SIZE  = 2000
    };

    QextSerialBase *comPort;  // Serial COM port

    QTimer *timerForRx; // This timer is used to trig the com port to read data

    char *txBuffer;        // Transmit buffer
    struct COM_PORT_INIT_DATA *comInitData; // COM port init data
    LoopBuffer *rxLoopBuffer;

    QMutex mutex;   // locker

    uint32_t txTotalBytesSize;
    uint32_t rxTotalBytesSize;

    void init();     // Init Serial Port
    void deInit();   // Release/DeInit Serial Port

#ifdef Q_OS_WIN
    // Get com port info from register
    QString getComInfoFromReg(int index, QString keyorvalue);
#endif

};

#endif // QSERIALPORT_H

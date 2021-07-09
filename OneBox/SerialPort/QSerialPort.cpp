/**********************************************************************
PACKAGE:        Communication
FILE:           QSerialPort.cpp
COPYRIGHT (C):  All rights reserved.

PURPOSE:        Serial Port interface based on QextSerialBase
**********************************************************************/

#include "QSerialPort.h"
#include <QSettings>
#include <QMutexLocker>
#include <QDebug>

#ifdef Q_OS_WIN
    #include <qt_windows.h>
    #include "win_qextserialport.h"
#else
    #include "posix_qextserialport.h"
#endif


//#define __SERIAL_CONSOLE_DEBUG__

QSerialPort::QSerialPort(COM_PORT_INIT_DATA *initData) :
    comPort(NULL),
    timerForRx(NULL)
{
    init();

    open(initData);
}

QSerialPort::QSerialPort() :
    comPort(NULL),
    timerForRx(NULL)
{
    init();
}

QSerialPort::~QSerialPort()
{
    deInit();
}

void QSerialPort::run()
{
}


void QSerialPort::init()
{
    // Reset tx/rx bytes count
    resetTxRxCnt();

    comInitData = new struct COM_PORT_INIT_DATA;

    // Init Tx buffer
    txBuffer = new char [TX_BUF_SIZE];
    memset(txBuffer, 0, TX_BUF_SIZE);

    // Init Rx buffer
    rxLoopBuffer = new LoopBuffer(RX_BUF_SIZE);
    //rxLoopBuffer->setMsgEndChar(MSG_END_FLAG_1);

    connect(this, SIGNAL(startPolling()), this, SLOT(startPollingTimer()));
    connect(this, SIGNAL(stopPolling()), this, SLOT(stopPollingTimer()));
}

void QSerialPort::deInit()
{
    if(NULL != timerForRx)
    {
        timerForRx->stop();
        delete timerForRx;
        timerForRx = NULL;
    }

    if(comPort != NULL)
    {
        comPort->close();
    }

    delete comPort;
    delete comInitData;
    delete []txBuffer;
    delete rxLoopBuffer;
}

bool QSerialPort::open(struct COM_PORT_INIT_DATA *initData)
{
    bool ret = false;
    QString portName = "";

    // Load COM port setting
    memcpy(comInitData, initData, sizeof(struct COM_PORT_INIT_DATA));

    // Init Tx buffer
    memset(txBuffer, 0, TX_BUF_SIZE);

    portName.append(comInitData->port);
    portName.prepend("\\\\.\\");   // Which is used to open the port number >= COM10

    close();

#ifdef Q_OS_WIN
    comPort = new Win_QextSerialPort(portName, QextSerialBase::Polling); // Polling Mode
#else
    comPort = new Posix_QextSerialPort(portName, QextSerialBase::Polling); // Polling Mode
#endif

    comPort->open(QIODevice::ReadWrite); // Read Write Mode
    comPort->setBaudRate(comInitData->baudrate);  // Baudrate
    comPort->setDataBits(comInitData->databits);  // Data bits
    comPort->setParity(comInitData->parity);  // Parity
    comPort->setStopBits(comInitData->stopbits); // Stopbit
    comPort->setFlowControl(comInitData->flowtype); // Flow control, FLOW_OFF, FLOW_HARDWARE, FLOW_XONXOFF
    comPort->setTimeout(10);    // Set timeout=10ms

    // If COM port is open, then start to read data from COM port
    if(isOpen())
    {
        ret = true;
        emit startPolling();
    }

    return ret;
}

void QSerialPort::close()
{
    emit stopPolling();

    if(comPort != NULL)
    {
        comPort->close();
        delete comPort;
        comPort = NULL;
    }
}

bool QSerialPort::isOpen()
{
    bool ret = false;

    if(comPort != NULL)
    {
        ret = comPort->isOpen();
    }

    return ret;
}

int QSerialPort::writeData(const char* txData, int len)
{
    int ret = 0;

#ifdef __SERIAL_CONSOLE_DEBUG__
    for(int i = 0; i < len; i++)
    {
        qDebug() << "txData[" << i << "]=" << (quint8)txData[i];
    }
#endif

    if(comPort != NULL)
    {
        if(comPort->isOpen()
                && comPort->isWritable())
        {
            ret = comPort->write(txData, len);
            txTotalBytesSize += len;

            // Emit signal
            emit newDataTx(QByteArray(txData, len));
        }
        else
        {
            qDebug() << "comPort not open";
        }
    }

    return ret;
}

int QSerialPort::writeData(QByteArray &txData)
{
    return writeData(txData.constData(), txData.length());
}

void QSerialPort::readDataFromCOM()
{
    if(NULL == comPort)
    {
        return;
    }

    if(comPort->isOpen())
    {
        QByteArray temp = comPort->readAll();   //Read all the data to the serial port buffer

        if(!temp.isEmpty())
        {
            {
                QMutexLocker locker(&mutex);
                rxLoopBuffer->writeData(temp);
            }

            rxTotalBytesSize += temp.length();

            // Emit signal
            emit newDataReady(temp);
        }
    }
}

void QSerialPort::startPollingTimer()
{
    if(NULL != timerForRx)
    {
        timerForRx->stop();
        delete timerForRx;
    }

    timerForRx = new QTimer();
    connect(timerForRx, SIGNAL(timeout()), this, SLOT(readDataFromCOM()));
    timerForRx->start(50);  //timeOut = 50ms
}

void QSerialPort::stopPollingTimer()
{
    if(NULL != timerForRx)
    {
        timerForRx->stop();
    }
}

void QSerialPort::setBaudRate(BaudRateType baudrate)
{
    if(NULL == comPort)
    {
        return;
    }

    comInitData->baudrate = baudrate;
    comPort->setBaudRate(comInitData->baudrate);
}

void QSerialPort::setParity(ParityType parity)
{
    if(NULL == comPort)
    {
        return;
    }

    comInitData->parity = parity;
    comPort->setParity(comInitData->parity);
}

void QSerialPort::setDataBits(DataBitsType databits)
{
    if(NULL == comPort)
    {
        return;
    }

    comInitData->databits = databits;
    comPort->setDataBits(comInitData->databits);
}

void QSerialPort::setStopBits(StopBitsType stopbits)
{
    if(NULL == comPort)
    {
        return;
    }

    comInitData->stopbits = stopbits;
    comPort->setStopBits(comInitData->stopbits);
}

void QSerialPort::setFlowControl(FlowType flowtype)
{
    if(NULL == comPort)
    {
        return;
    }

    comInitData->flowtype = flowtype;
    comPort->setFlowControl(comInitData->flowtype); //flow control, FLOW_OFF, FLOW_HARDWARE, FLOW_XONXOFF
}

QStringList QSerialPort::getAvailablePorts()
{
    QStringList portList;
    portList.clear();

#ifdef Q_OS_WIN
    QString path = "HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM";
    QSettings settings(path, QSettings::NativeFormat);
    QStringList key = settings.allKeys();

    for(int i = 0; i < key.size(); i++)
    {
        QString port = getComInfoFromReg(i, "value");
        if(!port.isEmpty())
        {
            portList.append(port);
        }
    }
#endif

    return portList;
}

#ifdef Q_OS_WIN
QString QSerialPort::getComInfoFromReg(int index, QString keyorvalue)
{
    QString commresult = "";
    HKEY hKey;
    QString keymessage;
    QString valuemessage;
    DWORD keysize, type, valuesize;
    wchar_t keyname[256] = {0};
    char keyvalue[256] = {0};

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ |  KEY_QUERY_VALUE, &hKey) != 0)
    {
        //qDebug() << "RegOpenKeyEx failed";
        return commresult;
    }

    keysize = sizeof(keyname);
    valuesize = sizeof(keyvalue);

    if(RegEnumValue(hKey, index, keyname, &keysize, 0, &type, (BYTE*)keyvalue, &valuesize) == 0)
    {
        keymessage.append(QString::fromStdWString(keyname));

        for(int j = 0; j < (int)valuesize; j++)
        {
            if (keyvalue[j] != 0x00)
            {
                valuemessage.append(keyvalue[j]);
            }
        }

        //qDebug() << "keymessage = " << keymessage;
        //qDebug() << "valuemessage = " << valuemessage;

        if(keyorvalue == "key")
        {
            commresult = keymessage;
        }

        if(keyorvalue == "value")
        {
            commresult = valuemessage;
        }
    }

    RegCloseKey(hKey);  // Close reg

    return commresult;
}
#endif

uint32_t QSerialPort::getTotalTxBytes() const
{
    return txTotalBytesSize;
}

uint32_t QSerialPort::getTotalRxBytes() const
{
    return rxTotalBytesSize;
}

void QSerialPort::resetTxRxCnt()
{
    txTotalBytesSize = 0;
    rxTotalBytesSize = 0;
}

bool QSerialPort::getUndealData(uint8_t *dataP, uint32_t &len)
{
    bool ret = false;
    QByteArray temp;
    ret = getUndealData(temp);

    if(ret)
    {
        len = temp.size();
        memcpy(dataP, temp.constData(), temp.size());
    }

    return ret;
}

bool QSerialPort::getUndealData(QByteArray &data)
{
    bool ret = false;
    QMutexLocker locker(&mutex);

    data = rxLoopBuffer->readAll();
    if(!data.isEmpty())
    {
        ret = true;
    }

    return ret;
}

/**********************************************************************
PACKAGE:        Communication
FILE:           ModbusTCP.cpp
COPYRIGHT (C):  All rights reserved.

PURPOSE:        Modbus TCP interface
**********************************************************************/

#include "ModbusTCP.h"

//#define MODBUS_TCP_DEBUG_TRACE

ModbusTCP::ModbusTCP(QObject *parent) :
    QThread(parent),
    m_tcpClient(new TCPClient),
    rxLoopBuf(new LoopBuffer),
    fifoBuf(new FIFOBuffer),
    txBufLen(0),
    m_transactionID(0x0000),
    m_protocolID(0x0000),
    m_unitID(1),
    isRunning(false),
    m_periodTxTimeInMs(100),
    periodTxTmr(NULL),
    getResponseFlag(true),
    txRetryTimes(0),
    TX_RETRY_MAX_TIMES(1),
    txErrorCnt(0),
    TX_ERROR_MAX_CNT(10*TX_RETRY_MAX_TIMES),
    m_autoConnectToServerFlag(true)
{
    // Init Tx buffer for transmit
    m_comTxBuf = new char [TX_BUF_SIZE];
    memset(m_comTxBuf, 0, TX_BUF_SIZE);

    bindModel(m_tcpClient);

    // Signals & slots
    connect(this, SIGNAL(startTxTimer()), this, SLOT(initTxTimer()));
    connect(this, SIGNAL(stopTxTimer()), this, SLOT(deInitTxTimer()));

}

ModbusTCP::~ModbusTCP()
{
    delete m_tcpClient;
    delete rxLoopBuf;
    delete fifoBuf;

    if(m_comTxBuf != NULL)
    {
        delete []m_comTxBuf;
        m_comTxBuf = NULL;
    }
}

void ModbusTCP::run()
{
    while(1)
    {
        usleep( 1 );
    }

    exec();
}

void ModbusTCP::bindModel(TCPClient *clientP)
{
    if(NULL != clientP)
    {
        unbind();

        m_tcpClient = clientP;

        connect(m_tcpClient, SIGNAL(newDataReady(QByteArray)), this, SLOT(updateIncomingData(QByteArray)));
        connect(m_tcpClient, SIGNAL(connectionOut()), this, SLOT(diconnectedStatus()));
    }
}

void ModbusTCP::unbind()
{
    if(NULL != m_tcpClient)
    {
        disconnect(m_tcpClient, 0 , this , 0);
    }

    m_tcpClient = NULL;
}

bool ModbusTCP::connectToServer(const QHostAddress &ip, uint16_t port)
{
    bool ret = false;

    if(NULL != m_tcpClient)
    {
        ret = m_tcpClient->connectToServer(ip, port);
    }

    hostAddr = ip;
    serverPort = port;

    isRunning = ret;

    // Emit signal
    emit connectionChanged(isRunning);

    return ret;
}

bool ModbusTCP::connectToServer(QString ip, uint16_t port)
{
    return connectToServer(QHostAddress(ip), port);
}

void ModbusTCP::disconnectFromServer()
{
    if(NULL != m_tcpClient)
    {
        m_tcpClient->disconnectFromServer();
    }
}

void ModbusTCP::setTransactionID(uint16_t id)
{
    m_transactionID = id;
}

void ModbusTCP::setProtocolID(uint16_t id)
{
    m_protocolID = id;
}

void ModbusTCP::setUnitID(uint8_t id)
{
    m_unitID = id;
}

void ModbusTCP::setTxPeriod(uint32_t ms)
{
    m_periodTxTimeInMs = ms;
}

void ModbusTCP::setTxRetryTimes(uint32_t cnt)
{
    TX_RETRY_MAX_TIMES = cnt;
    TX_ERROR_MAX_CNT = 10 * TX_RETRY_MAX_TIMES;
}

void ModbusTCP::setAutoReconnect(bool autoConnectFlag)
{
    m_autoConnectToServerFlag = autoConnectFlag;
}

bool ModbusTCP::readInputRegisters(uint16_t regOffset, uint16_t regCnt)
{
    bool ret = false;
    uint32_t index = 0;
    struct MODBUS_READ_FEEDBACK tempFeedBackStruct;
    uint16_t len = 0;

    if(0 == regCnt)
    {
        return ret;
    }

    QMutexLocker locker(&mutex);

    // Clear buffer
    char txDataBuf[TX_BUF_SIZE] = {0};
    memset(txDataBuf, 0, TX_BUF_SIZE);
    memset(&tempFeedBackStruct, 0, sizeof(MODBUS_READ_FEEDBACK));

    // MBAP header - 7 bytes
    txDataBuf[index++] = (uint8_t)(m_transactionID >> 8);  // transaction ID high-8bit
    txDataBuf[index++] = (uint8_t)(m_transactionID & 0x00ff);  // transaction ID low-8bit

    txDataBuf[index++] = (uint8_t)(m_protocolID >> 8);  // protocol ID high-8bit
    txDataBuf[index++] = (uint8_t)(m_protocolID & 0x00ff);  // protocol ID low-8bit

    txDataBuf[index++] = (uint8_t)(len >> 8);  // packet length high-8bit
    txDataBuf[index++] = (uint8_t)(len & 0x00ff);  // packet length low-8bit
    len = index;
    txDataBuf[index++] = m_unitID;


    // Function code
    txDataBuf[index++] = MB_FUNC_READ_INPUT_REGISTER;

    // Note: For Modbus communication, data are big endian!
    txDataBuf[index++] = (uint8_t)(regOffset >> 8);        // reg address high-8bit
    txDataBuf[index++] = (uint8_t)(regOffset & 0x00ff);    // reg address low-8bit

    txDataBuf[index++] = (uint8_t)(regCnt >> 8);        // reg count high-8bit
    txDataBuf[index++] = (uint8_t)(regCnt & 0x00ff);    // reg count low-8bit

    // Calculate packet length
    len = index - len;
    txDataBuf[4] = (uint8_t)(len >> 8);  // packet length high-8bit
    txDataBuf[5] = (uint8_t)(len & 0x00ff);  // packet length low-8bit

    tempFeedBackStruct.address = regOffset;
    tempFeedBackStruct.len = regCnt;
    tempFeedBackStruct.rdwrFlag = MODBUS_RD_OPT;

    // Push data to FIFO
    fifoBuf->pushData((char *)&tempFeedBackStruct, sizeof(struct MODBUS_READ_FEEDBACK));
    // Push data to FIFO
    ret = fifoBuf->pushData(txDataBuf, index);

    return ret;
}

bool ModbusTCP::readHoldRegisters(uint16_t regOffset, uint16_t regCnt)
{
    bool ret = false;
    uint32_t index = 0;
    struct MODBUS_READ_FEEDBACK tempFeedBackStruct;
    uint16_t len = 0;

    if(0 == regCnt)
    {
        return ret;
    }

    QMutexLocker locker(&mutex);

    // Clear buffer
    char txDataBuf[TX_BUF_SIZE] = {0};
    memset(txDataBuf, 0, TX_BUF_SIZE);
    memset(&tempFeedBackStruct, 0, sizeof(MODBUS_READ_FEEDBACK));

    // MBAP header - 7 bytes
    txDataBuf[index++] = (uint8_t)(m_transactionID >> 8);  // transaction ID high-8bit
    txDataBuf[index++] = (uint8_t)(m_transactionID & 0x00ff);  // transaction ID low-8bit

    txDataBuf[index++] = (uint8_t)(m_protocolID >> 8);  // protocol ID high-8bit
    txDataBuf[index++] = (uint8_t)(m_protocolID & 0x00ff);  // protocol ID low-8bit

    txDataBuf[index++] = (uint8_t)(len >> 8);  // packet length high-8bit
    txDataBuf[index++] = (uint8_t)(len & 0x00ff);  // packet length low-8bit
    len = index;
    txDataBuf[index++] = m_unitID;


    // Function code
    txDataBuf[index++] = MB_FUNC_READ_HOLDING_REGISTER;

    // Note: For Modbus communication, data are big endian!
    txDataBuf[index++] = (uint8_t)(regOffset >> 8);        // reg address high-8bit
    txDataBuf[index++] = (uint8_t)(regOffset & 0x00ff);    // reg address low-8bit

    txDataBuf[index++] = (uint8_t)(regCnt >> 8);        // reg count high-8bit
    txDataBuf[index++] = (uint8_t)(regCnt & 0x00ff);    // reg count low-8bit

    // Calculate packet length
    len = index - len;
    txDataBuf[4] = (uint8_t)(len >> 8);  // packet length high-8bit
    txDataBuf[5] = (uint8_t)(len & 0x00ff);  // packet length low-8bit

    tempFeedBackStruct.address = regOffset;
    tempFeedBackStruct.len = regCnt;
    tempFeedBackStruct.rdwrFlag = MODBUS_RD_OPT;

    // Push data to FIFO
    fifoBuf->pushData((char *)&tempFeedBackStruct, sizeof(struct MODBUS_READ_FEEDBACK));
    // Push data to FIFO
    ret = fifoBuf->pushData(txDataBuf, index);

    return ret;
}

bool ModbusTCP::writeHoldRegister(uint16_t regOffset, uint16_t regValue)
{
    bool ret = false;
    uint32_t index = 0;
    struct MODBUS_READ_FEEDBACK tempFeedBackStruct;
    uint16_t len = 0;

    QMutexLocker locker(&mutex);

    // Clear buffer
    char txDataBuf[TX_BUF_SIZE] = {0};
    memset(txDataBuf, 0, TX_BUF_SIZE);
    memset(&tempFeedBackStruct, 0, sizeof(MODBUS_READ_FEEDBACK));

    // MBAP header - 7 bytes
    txDataBuf[index++] = (uint8_t)(m_transactionID >> 8);  // transaction ID high-8bit
    txDataBuf[index++] = (uint8_t)(m_transactionID & 0x00ff);  // transaction ID low-8bit

    txDataBuf[index++] = (uint8_t)(m_protocolID >> 8);  // protocol ID high-8bit
    txDataBuf[index++] = (uint8_t)(m_protocolID & 0x00ff);  // protocol ID low-8bit

    txDataBuf[index++] = (uint8_t)(len >> 8);  // packet length high-8bit
    txDataBuf[index++] = (uint8_t)(len & 0x00ff);  // packet length low-8bit
    len = index;
    txDataBuf[index++] = m_unitID;


    // Function code
    txDataBuf[index++] = MB_FUNC_WRITE_REGISTER;

    // Note: For Modbus communication, data are big endian!
    txDataBuf[index++] = (uint8_t)(regOffset >> 8);        // reg address high-8bit
    txDataBuf[index++] = (uint8_t)(regOffset & 0x00ff);    // reg address low-8bit

    txDataBuf[index++] = (uint8_t)(regValue >> 8);        // reg value high-8bit
    txDataBuf[index++] = (uint8_t)(regValue & 0x00ff);    // reg value low-8bit

    // Calculate packet length
    len = index - len;
    txDataBuf[4] = (uint8_t)(len >> 8);  // packet length high-8bit
    txDataBuf[5] = (uint8_t)(len & 0x00ff);  // packet length low-8bit

    tempFeedBackStruct.address = regOffset;
    tempFeedBackStruct.len = 1;
    tempFeedBackStruct.rdwrFlag = MODBUS_WR_OPT;

    // Push data to FIFO
    fifoBuf->pushData((char *)&tempFeedBackStruct, sizeof(struct MODBUS_READ_FEEDBACK));
    // Push data to FIFO
    ret = fifoBuf->pushData(txDataBuf, index);

    return ret;
}

bool ModbusTCP::writeMultiRegisters(uint16_t regOffset, const char *dataP, uint16_t regCnt)
{
    bool ret = false;
    uint32_t index = 0;
    struct MODBUS_READ_FEEDBACK tempFeedBackStruct;
    uint16_t len = 0;

    if(NULL == dataP || 0 == regCnt)
    {
        return ret;
    }

    QMutexLocker locker(&mutex);

    // Clear buffer
    char txDataBuf[TX_BUF_SIZE] = {0};
    memset(txDataBuf, 0, TX_BUF_SIZE);
    memset(&tempFeedBackStruct, 0, sizeof(MODBUS_READ_FEEDBACK));

    // MBAP header - 7 bytes
    txDataBuf[index++] = (uint8_t)(m_transactionID >> 8);  // transaction ID high-8bit
    txDataBuf[index++] = (uint8_t)(m_transactionID & 0x00ff);  // transaction ID low-8bit

    txDataBuf[index++] = (uint8_t)(m_protocolID >> 8);  // protocol ID high-8bit
    txDataBuf[index++] = (uint8_t)(m_protocolID & 0x00ff);  // protocol ID low-8bit

    txDataBuf[index++] = (uint8_t)(len >> 8);  // packet length high-8bit
    txDataBuf[index++] = (uint8_t)(len & 0x00ff);  // packet length low-8bit
    len = index;
    txDataBuf[index++] = m_unitID;

    // Function code
    txDataBuf[index++] = MB_FUNC_WRITE_MULTIPLE_REGISTERS;

    // Note: For Modbus communication, data are big endian!
    txDataBuf[index++] = (uint8_t)(regOffset >> 8);        // reg address high-8bit
    txDataBuf[index++] = (uint8_t)(regOffset & 0x00ff);    // reg address low-8bit

    txDataBuf[index++] = (uint8_t)(regCnt >> 8);        // reg count high-8bit
    txDataBuf[index++] = (uint8_t)(regCnt & 0x00ff);    // reg count low-8bit

    txDataBuf[index++] = (uint8_t)(regCnt * sizeof(uint16_t));   // length in bytes

    for(uint32_t i = 0; i < regCnt; i++)
    {
        uint16_t value = *((uint16_t *)dataP + i);

        txDataBuf[index++] = (uint8_t)(value >> 8);        // data high-8bit
        txDataBuf[index++] = (uint8_t)(value & 0x00ff);    // data low-8bit
    }

    // Calculate packet length
    len = index - len;
    txDataBuf[4] = (uint8_t)(len >> 8);  // packet length high-8bit
    txDataBuf[5] = (uint8_t)(len & 0x00ff);  // packet length low-8bit


    tempFeedBackStruct.address = regOffset;
    tempFeedBackStruct.len = regCnt;
    tempFeedBackStruct.rdwrFlag = MODBUS_WR_OPT;

    // Push data to FIFO
    fifoBuf->pushData((char *)&tempFeedBackStruct, sizeof(struct MODBUS_READ_FEEDBACK));
    // Push data to FIFO
    ret = fifoBuf->pushData(txDataBuf, index);

    return ret;
}

bool ModbusTCP::writeMultiRegistersInt32(uint16_t regOffset, uint32_t value)
{
    bool ret = false;

    uint16_t regValue[2] = {0};
    regValue[0] = (value >> 16) & 0x0000FFFF;
    regValue[1] = (value >> 0) & 0x0000FFFF;

    ret = writeMultiRegisters(regOffset, (char *)&regValue, 2);

    return ret;
}

bool ModbusTCP::writeMultiRegistersInt16(uint16_t regOffset, uint16_t value)
{
    bool ret = false;

    ret = writeMultiRegisters(regOffset, (char *)&value, 1);

    return ret;
}

void ModbusTCP::updateIncomingData(QByteArray data)
{
    QByteArray rxDataBuf;
    rxDataBuf.clear();

    if(NULL == m_tcpClient)
    {
        return;
    }

    if(!data.isEmpty())
    {
        // Update rx data to loop buffer, parse it later
        rxLoopBuf->writeData(data);

        // Emit signal
        emit newDataReady(data);

        // Parse packet
        if(parseResponsePacket(data))
        {
        }

        getResponseFlag = true;
    }
}

bool ModbusTCP::parseResponsePacket(QByteArray &responseData)
{
    bool ret = false;

    if(responseData.size() < MODBUS_RESPONSE_MSG_START_LEN)
    {
        return ret;
    }

    // Check transactionID matched
    if((m_transactionID >> 8) != responseData[0] ||
            (m_transactionID & 0x00ff) != responseData[1])
    {
        return ret;
    }

    MODBUS_RX_MSG_STRUCT *feedbackMsg = (MODBUS_RX_MSG_STRUCT *)(responseData.data() + MODBUS_RESPONSE_MSG_START_LEN);

    switch(feedbackMsg->functionCode)
    {
    case MB_FUNC_READ_HOLDING_REGISTER:
    case MB_FUNC_READ_INPUT_REGISTER:

        if(readFeedbackStruct.len != (feedbackMsg->data[0] / sizeof(uint16_t)))
        {
        #ifdef MODBUS_TCP_DEBUG_TRACE
            qDebug() << "invalid rx length! readFeedbackStruct.len =" << readFeedbackStruct.len
                     << ",(feedbackMsg->data[0]/sizeof(uint16_t)=" << feedbackMsg->data[0] / sizeof(uint16_t);
        #endif
        }
        else
        {
            memcpy((char *)readFeedbackStruct.buffer, (char *)&(feedbackMsg->data[1]), feedbackMsg->data[0]);
            ret = true;
        }

        break;
    case MB_FUNC_WRITE_REGISTER:
        ret = true;
        break;
    case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
        ret = true;
        break;
    default:
    #ifdef MODBUS_TCP_DEBUG_TRACE
        qDebug("Invalid function code = 0x%02x", feedbackMsg->functionCode);
    #endif
        break;
    }

    // Only read operation send feedback msg
    if(MODBUS_RD_OPT == readFeedbackStruct.rdwrFlag && true == ret)
    {
        // Emit signal
        emit newResponseMsg(readFeedbackStruct);

#ifdef MODBUS_TCP_DEBUG_TRACE
        qDebug() << "readFeedbackStruct.address" << readFeedbackStruct.address;
        qDebug() << "readFeedbackStruct.len" << readFeedbackStruct.len;

        for(int i = 0; i < readFeedbackStruct.len; i++)
        {
            qDebug("readFeedbackStruct.bufer[%d]=0x%04x", i, readFeedbackStruct.buffer[i]);
        }
#endif

    }

    return ret;
}

void ModbusTCP::diconnectedStatus(void)
{
    isRunning = false;

    // Emit signal
    emit connectionChanged(isRunning);
}

void ModbusTCP::initTxTimer()
{
    deInitTxTimer();

    if(NULL == periodTxTmr)
    {
        periodTxTmr = new QTimer;
        connect(periodTxTmr, SIGNAL(timeout()), this, SLOT(periodTxService()));
        periodTxTmr->start(m_periodTxTimeInMs);
    }
}

void ModbusTCP::deInitTxTimer()
{
    if(NULL != periodTxTmr)
    {
        periodTxTmr->stop();
        disconnect(periodTxTmr, 0, this, 0);
        delete periodTxTmr;
        periodTxTmr = NULL;
    }
}

void ModbusTCP::startPeriodTxService()
{
    emit startTxTimer();
}

void ModbusTCP::stopPeriodTxService()
{
    emit stopTxTimer();
}

bool ModbusTCP::getRunningStatus() const
{
    return isRunning;
}

void ModbusTCP::periodTxService()
{
    uint32_t len = 0;
    QMutexLocker locker(&mutex);

    //qDebug() << "ModbusTCP::periodTxService()";

    if(false == getResponseFlag)
    {
        if(++txRetryTimes > TX_RETRY_MAX_TIMES)
        {
            txRetryTimes = 0;

            // Tx error count increased
            txErrorCnt++;

            // If TxRetryTimes is too much
            // the connection should be lost, need to connect to server again!
            if(txErrorCnt >= TX_ERROR_MAX_CNT)
            {
                txErrorCnt = 0;

                // Auto connect to tcp server
                autoConnectToServer();
            }

        }
        else
        {
            retransmitTask();
            return;
        }
    }
    else
    {
        // 2020-Mar-21 add this logic
        // Once received msg from Modbus, then reset Tx error count
        // When tx msg > 10 and there's no feedback, then reInitModbusComm()
        txErrorCnt = 0;
    }

    // Clear buffer
    memset(m_comTxBuf, 0, TX_BUF_SIZE);

    // Pop data from FIFO
    if(true == fifoBuf->popData((char *)&readFeedbackStruct, len))
    {
    }

    if(true == fifoBuf->popData(m_comTxBuf, txBufLen))
    {
        // Reset response flag
        getResponseFlag = false;

        // Send data package to ModbusRTU
        writeDataToModbus(m_comTxBuf, txBufLen);
    }

}

void ModbusTCP::retransmitTask()
{
    // Reset response flag
    getResponseFlag = false;

    // If m_comTxBuf is not empty(0x00...)
    if(0 != txBufLen)
    {
        // Send data package to ModbusTCP
        writeDataToModbus(m_comTxBuf, txBufLen);
    }
}

int ModbusTCP::writeDataToModbus(const char* txData, int len)
{
    int ret = 0;

    if(NULL == m_tcpClient)
    {
        return ret;
    }

    if(NULL == txData || 0 == len)
    {
        return ret;
    }

    if(isRunning)
    {
        ret = len;
    }

    m_tcpClient->sendData(txData, len);

    // Emit signal
    QByteArray data(txData, len);
    emit newDataTx(data);

    return ret;
}

void ModbusTCP::autoConnectToServer()
{
    if(m_autoConnectToServerFlag)
    {
        if(!isRunning)
        {
            connectToServer(hostAddr, serverPort);
        }
    }
}


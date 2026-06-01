#include "ModbusRTU.h"
#include <QDebug>

#include "endian_proc.h"
#include "CRCUtility.h"
#include "QUtilityBox.h"

//#define MODBUSRTU_DEBUG_PRINT

ModbusRTU::ModbusRTU(ModbusCommBase *parent) :
    ModbusCommBase(parent),
    m_settingFile("config.ini"),
    m_devAddr(1),
    intervalTimeInMs(0),
    intervalTime(new QTime),
    isTxRxOkFlag(false),
    getResponseFlag(true),
    txRetryTimes(0),
    TX_RETRY_MAX_TIMES(1),
    txErrorCnt(0),
    TX_ERROR_MAX_CNT(10*TX_RETRY_MAX_TIMES),
    errorCheckFlag(true),
    periodTxMaxTimeInMs(250),
    periodTxTmr(new QTimer),
    logFile(new FileLog),
    logPath("./Log/"),
    txBufLen(0),
    modbusRTUReadOpt(false)
{
    // Prepend the exe absolute path
    m_settingFile.prepend(QUtilityBox::instance()->getAppDirPath());
    // Default setting file
    currentSetting = new QSettings(m_settingFile, QSettings::IniFormat);

    // Load Settings from ini file
    loadSettingFromIniFile();

    // Init Com Port for Modbus
    comPortInit();

    // Init FIFO buffer
    fifoBuf = new FIFOBuffer();

    // Init Tx buffer for transmit
    m_comTxBuf= new char [TX_BUF_SIZE];
    memset(m_comTxBuf, 0, TX_BUF_SIZE);

    // Init timeout response check
    connect(periodTxTmr, SIGNAL(timeout()), this, SLOT(periodTxService()));
    startPeriodTxService();
}

ModbusRTU::~ModbusRTU()
{
    qDebug() << "~ModbusRTU()";
    delete currentSetting;

    comPortDeInit();

    if(m_comTxBuf != NULL)
    {
        delete []m_comTxBuf;
        m_comTxBuf = NULL;
    }

    delete logFile;

    delete intervalTime;
    delete periodTxTmr;

    delete fifoBuf;
}

void ModbusRTU::loadSettingFromIniFile()
{
    // Load Font type and size
    currentSetting->beginGroup("SystemSetting");

    if(currentSetting->contains("LogPath"))
    {
        // Load Log Path
        logPath = currentSetting->value("LogPath").toString();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("LogPath", logPath);
    }
    // Init Log File Path
    logFile->setLogPath(logPath);

    currentSetting->endGroup();

    // Load Modbus setting
    currentSetting->beginGroup("ModbusSetting");

    if(currentSetting->contains("ResponseTime"))
    {
        // Load Period Tx Max time
        periodTxMaxTimeInMs = currentSetting->value("ResponseTime").toInt();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("ResponseTime", periodTxMaxTimeInMs);
    }

    if(currentSetting->contains("ResponseRetry"))
    {
        // Load Tx Retry count
        TX_RETRY_MAX_TIMES = currentSetting->value("ResponseRetry").toInt();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("ResponseRetry", TX_RETRY_MAX_TIMES);
    }

    if(currentSetting->contains("SlaveAddress"))
    {
        // Load slave address
        int slaveAddr = currentSetting->value("SlaveAddress").toInt();

        // Set modbus salve address
        setSlaveAddr(slaveAddr);
    }
    else
    {
        // Init the default value
        currentSetting->setValue("SlaveAddress", m_devAddr);
    }

    if(currentSetting->contains("ErrorCheck"))
    {
        // Load error check flag
        errorCheckFlag = currentSetting->value("ErrorCheck").toBool();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("ErrorCheck", errorCheckFlag);
    }

    currentSetting->endGroup();
}

bool ModbusRTU::comPortInit()
{
    bool ret = false;
    qDebug() << "comPortInit";

    // Load COM port setting
    loadComSettingFromFile();

    // Init Rx buffer
    rxLoopBuf = new LoopBuffer;

    comPort = new QSerialPort(comInitData);
    connect(comPort, SIGNAL(newDataReady(QByteArray)), this, SLOT(readDataFromModbus(QByteArray)));

    // check com port is open or not
    ret = isComPortOpen();

    return ret;
}

void ModbusRTU::comPortDeInit()
{
    QString logStr;

    if(comPort != NULL)
    {
        delete comPort;
        comPort = NULL;
    }

    if(comInitData != NULL)
    {
        delete comInitData;
        comInitData = NULL;
    }

    if(rxLoopBuf != NULL)
    {
        delete rxLoopBuf;
        rxLoopBuf = NULL;
    }

    qDebug() << "comPortDeInit";
    logStr.append("comPortDeInit");

    // Update log
    updateLogData(logStr);
}

bool ModbusRTU::isComPortOpen()
{
    static int cnt = 0;
    QString warningStr = tr("Open ");
    warningStr.append(comInitData->port);  // "COMXX"

    ++cnt;
    if(!comPort->isOpen())
    {
        warningStr.append(tr(" failed!"));
        if(1 == cnt)
        {
            // Update log
            updateLogData(warningStr);
        }
        else
        {
            // Update log
            updateLogData(warningStr);
        }

        return false;
    }
    else
    {
        return true;
    }
}

void ModbusRTU::loadComSettingFromFile()
{
    comInitData = new struct COM_PORT_INIT_DATA;
    QString comSetStr = "Modbus";

    memset((char *)comInitData, 0, sizeof(struct COM_PORT_INIT_DATA));

    initDefaultCOMSetting();

    currentSetting->beginGroup(comSetStr.append("ComSetting"));

    QString portName = currentSetting->value("Port").toString();
    memcpy(comInitData->port, portName.toLatin1().data(), portName.toLatin1().length());

    switch(currentSetting->value("BaudRate").toInt())
    {
    case 9600:
        comInitData->baudrate = BAUD9600;
        break;
    case 19200:
        comInitData->baudrate = BAUD19200;
        break;
    case 38400:
        comInitData->baudrate = BAUD38400;
        break;
    case 57600:
        comInitData->baudrate = BAUD57600;
        break;
    case 115200:
        comInitData->baudrate = BAUD115200;
        break;
    default:
        comInitData->baudrate = BAUD115200;
        break;
    }

    switch(currentSetting->value("StopBits").toInt())
    {
    case 1:
        comInitData->stopbits = STOP_1;
        break;
    case 2:
        comInitData->stopbits = STOP_2;
        break;
    default:
        comInitData->stopbits = STOP_1;
        break;
    }

    switch(currentSetting->value("DataBits").toInt())
    {
    case 5:
        comInitData->databits = DATA_5;
        break;
    case 6:
        comInitData->databits = DATA_6;
        break;
    case 7:
        comInitData->databits = DATA_7;
        break;
    case 8:
        comInitData->databits = DATA_8;
        break;
    default:
        comInitData->databits = DATA_8;
        break;
    }

    if(currentSetting->value("Parity").toString() == "None")
    {
        comInitData->parity = PAR_NONE;
    }
    else if(currentSetting->value("Parity").toString() == "Odd")
    {
        comInitData->parity = PAR_ODD;
    }
    else if(currentSetting->value("Parity").toString() == "Even")
    {
        comInitData->parity = PAR_EVEN;
    }
    else
    {
        comInitData->parity = PAR_NONE;
    }

    if(currentSetting->value("FlowControl").toString() == "None")
    {
        comInitData->flowtype = FLOW_OFF;
    }
    else if(currentSetting->value("FlowControl").toString() == "Hardware")
    {
        comInitData->flowtype = FLOW_HARDWARE;
    }
    else if(currentSetting->value("FlowControl").toString() == "XonXoff")
    {
        comInitData->flowtype = FLOW_XONXOFF;
    }
    else
    {
        comInitData->flowtype = FLOW_OFF;
    }

    currentSetting->endGroup();
}

void ModbusRTU::initDefaultCOMSetting()
{
    currentSetting->beginGroup("ModbusComSetting");

    if(!currentSetting->contains("Port"))
    {
        // Init the default value
        currentSetting->setValue("Port", "COM1");
    }

    if(!currentSetting->contains("BaudRate"))
    {
        // Init the default value
        currentSetting->setValue("BaudRate", 38400);
    }

    if(!currentSetting->contains("StopBits"))
    {
        // Init the default value
        currentSetting->setValue("StopBits", 1);
    }

    if(!currentSetting->contains("DataBits"))
    {
        // Init the default value
        currentSetting->setValue("DataBits", 8);
    }

    if(!currentSetting->contains("Parity"))
    {
        // Init the default value
        currentSetting->setValue("Parity", "Even");
    }

    if(!currentSetting->contains("FlowControl"))
    {
        // Init the default value
        currentSetting->setValue("FlowControl", "None");
    }

    currentSetting->endGroup();
}

int ModbusRTU::writeDataToModbus(const char* txData, int len)
{
    int ret = 0;

#ifdef __SERIAL_CONSOLE_DEBUG__
    for( int i = 0; i < len; i++ )
    {
        qDebug() << "txData[" << i << "]=" << (quint8)txData[i];
    }
#endif

    if(NULL == comPort)
    {
        return ret;
    }

    if(comPort->isOpen())
    {
        ret = comPort->writeData(txData, len);

        // Emit signal
        QByteArray data(txData, len);
        emit newDataTx(data);
    }
    else
    {
        // Open com port fail, which means com port is disconnected
        // ReInit com port again
        //reInitModbusComm();
    }

    // If TxRetryTimes is too much, re-init com port
    if(txErrorCnt >= TX_ERROR_MAX_CNT)
    {
        txErrorCnt = 0;

        // ReInit com port again
        reInitModbusComm();
    }

    return ret;
}


void ModbusRTU::readDataFromModbus(QByteArray data)
{
    if(NULL == comPort)
    {
        return;
    }

    if(comPort->isOpen())
    {
        QByteArray temp = data;

        if(!temp.isEmpty())
        {
            // Update rx data to loop buffer, parse it later
            rxLoopBuf->writeData(temp);

            // Emit signal
            emit newDataReady(data);

            // Parse Response Packet
            parseResponseDataFromCOM();
        }
    }
}

void ModbusRTU::parseResponseDataFromCOM()
{
    int len = 0;
    QByteArray temp;
    QByteArray dataPacketInput;

    temp.clear();
    dataPacketInput.clear();

    //qDebug() << "parseResponseDataFromCOM()";

    while(true == rxLoopBuf->isUndealDataExist())
    {
        dataPacketInput.append(rxLoopBuf->readData(1));

        //qDebug() << "dataPacketInput.at(0) = " << QString::number((uint8_t)dataPacketInput.at(0));

        // Check the 1st byte in Rx packet, it shall be slave address
        if(m_devAddr == dataPacketInput.at(0))
        {
            len = rxLoopBuf->getUndealDataSize();
            temp = rxLoopBuf->readAll();
            //qDebug() << "readAll temp = " << temp;
            dataPacketInput.append(temp.left(len));

            temp.clear();
            temp.append(dataPacketInput.mid(0, dataPacketInput.length()));

            parseResponsePacket(temp);
            break;
        }
        else
        {
            dataPacketInput.remove(0, 1);
        }
    }
}


void ModbusRTU::parseResponsePacket(QByteArray responseData)
{
    QString logStr;
    QByteArray rxNACKData = responseData;
    rxNACKData[1] = rxNACKData[1] - 0x80;

    // Calculate CRC16
    uint16_t crc16 = do_crc16(responseData.data(), responseData.size() - MODBUS_CRC_LENGTH);

    if(0 == memcmp((char *)responseData.right(2).data(), &crc16, MODBUS_CRC_LENGTH))
    {
#ifdef MODBUSRTU_DEBUG_PRINT
        qDebug("crc16 is ok = 0x%04X", crc16);
#endif
        isTxRxOkFlag = true;

        if(checkTxRxConformity(m_comTxBuf, responseData.data()) ||
                checkTxRxConformity(m_comTxBuf, rxNACKData.data()))
        {
            //qDebug("getResponseFlag");
            getResponseFlag = true;
        }
        else
        {
            qDebug("Invalid Response Data, checkTxRxConformity fail");
            getResponseFlag = false;
            return;
        }
    }
    else
    {
        getResponseFlag = false;

        qDebug("crc16 is bad = 0x%04X", crc16);
        isTxRxOkFlag = false;

        logStr.clear();
        logStr.append("crc16 is bad");
        // Update log
        updateLogData(logStr);

        return;
    }


    if(0 == memcmp((char *)responseData.data(), (char *)&m_mbRxCheckStruct, m_mbRxCheckStruct.len))
    {
        MODBUS_RX_MSG_STRUCT *feedbackMsg = (MODBUS_RX_MSG_STRUCT *)responseData.data();

        switch(feedbackMsg->functionCode)
        {
        case MB_FUNC_READ_HOLDING_REGISTER:

            if(readFeedbackStruct.len != (feedbackMsg->data[0] / sizeof(uint16_t)))
            {
                readFeedbackStruct.len = (feedbackMsg->data[0] / sizeof(uint16_t));
                qDebug() << "invalid rx length! readFeedbackStruct.len =" << readFeedbackStruct.len << ",(feedbackMsg->data[0]/sizeof(uint16_t)=" << feedbackMsg->data[0] / sizeof(uint16_t);
            }

            memcpy((char *)readFeedbackStruct.buffer, (char *)&(feedbackMsg->data[1]), feedbackMsg->data[0]);

            break;
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
            break;
        default:
            break;
        }

#ifdef MODBUSRTU_DEBUG_PRINT
        qDebug() << "readFeedbackStruct.len = " << readFeedbackStruct.len << "readFeedbackStruct.address = " << readFeedbackStruct.address;
        qDebug() << "emit(reportModbusResponseValue)";
#endif

        // Only read operation send out signal
        // Write operation do not need to notice host
        if(true == modbusRTUReadOpt)
        {
            emit(reportModbusResponseValue(readFeedbackStruct));
        }

    }
    else
    {
       qDebug() << "memcmp m_mbRxCheckStruct fail";
    }
}

uint16_t ModbusRTU::do_crc16(const char *addr, uint16_t len)
{
    uint16_t crcValue;

    crcValue = CRCUtility::instance()->modbus_crc16((uint8_t *)addr, len);
    //qDebug() << "crc modbus = 0x" << QString::number(crcValue, 16);

    return crcValue;
}

void ModbusRTU::setSlaveAddr(uint8_t addr)
{
    m_devAddr = addr;
}

uint8_t ModbusRTU::getSlaveAddr() const
{
    return m_devAddr;
}

bool ModbusRTU::getModbusCommOk()
{
    return isTxRxOkFlag;
}

void ModbusRTU::setModbusTimeOut(int timeoutInMs)
{
    periodTxMaxTimeInMs = timeoutInMs;

    // Restart period Tx timer
    startPeriodTxService();
}

void ModbusRTU::periodTxService()
{
    uint32_t len = 0;
    QMutexLocker locker(&mutex);

    if(false == getResponseFlag)
    {
        if(++txRetryTimes > TX_RETRY_MAX_TIMES)
        {
            txRetryTimes = 0;

            // Tx error count increased
            txErrorCnt++;
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
        // Once received msg from ModbusRTU, then reset Tx error count
        // When tx msg > 10 and there's no feedback, then reInitModbusComm()
        txErrorCnt = 0;
    }

    // Clear buffer
    memset(m_comTxBuf, 0, TX_BUF_SIZE);

    // Pop data from FIFO
    if(true == fifoBuf->popData((char *)&readFeedbackStruct, len))
    {
        if(MODBUS_WR_OPT == readFeedbackStruct.rdwrFlag)
        {
            modbusRTUReadOpt = false;
        }
        else
        {
            modbusRTUReadOpt = true;
        }

        //qDebug("modbusRTUReadOpt = %d len = %d\n", modbusRTUReadOpt, len);
    }

    if(true == fifoBuf->popData(m_comTxBuf, txBufLen))
    {
        // Reset response flag
        getResponseFlag = false;

    #ifdef MODBUSRTU_DEBUG_PRINT
        // Restart interval time
        intervalTime->restart();

        QString tmpStr;
        tmpStr.clear();

        for(uint32_t i = 0; i < txBufLen; i++)
        {
            #if 1
            tmpStr.append(QString::number((uint8_t)m_comTxBuf[i], 16).rightJustified(2, '0').toUpper());
            tmpStr.append(" ");
            //qDebug() << "m_comTxBuf[" << i << "] = " << QString::number((uint8_t)m_comTxBuf[i], 16).toUpper();
            #else
            char tmpBuf[TX_BUF_SIZE] = {0};
            sprintf(tmpBuf, "%02X ", (uint8_t)m_comTxBuf[i]);
            tmpStr.append(tmpBuf);
            #endif
        }

        qDebug() << tmpStr;

    #endif

        // Send data package to ModbusRTU
        writeDataToModbus(m_comTxBuf, txBufLen);
    }

}

void ModbusRTU::retransmitTask()
{
    QString logStr;

    // Reset response flag
    getResponseFlag = false;

    // If m_comTxBuf is not empty(0x00...)
    if(0 != txBufLen)
    {
        // Send data package to ModbusRTU
        writeDataToModbus(m_comTxBuf, txBufLen);

    #ifdef MODBUSRTU_DEBUG_PRINT
        // Restart interval time
        intervalTime->restart();

        QString tmpStr;
        tmpStr.clear();

        for(uint32_t i = 0; i < txBufLen; i++)
        {
            #if 1
            tmpStr.append(QString::number((uint8_t)m_comTxBuf[i], 16).rightJustified(2, '0').toUpper());
            tmpStr.append(" ");
            #else
            char tmpBuf[TX_BUF_SIZE] = {0};
            sprintf(tmpBuf, "%02X ", (uint8_t)m_comTxBuf[i]);
            tmpStr.append(tmpBuf);
            #endif
        }

        qDebug() << tmpStr;

    #endif

        logStr.clear();
        logStr.append("ModbusRTU retransmit");
        // Update log
        updateLogData(logStr);
        qDebug() << logStr;

    }
}

void ModbusRTU:: startPeriodTxService()
{
    // Start timer
    periodTxTmr->start(periodTxMaxTimeInMs);
}

void ModbusRTU::updateLogData(QString logStr)
{
    QDateTime time = QDateTime::currentDateTime();
    QString timeStr = time.toString("[yyyy-MM-dd hh:mm:ss.zzz] ");

    // Add time stamp
    logStr.prepend(timeStr);

    logFile->addLogToFile(logStr);
}

bool ModbusRTU::checkTxRxConformity(const char *txBufP, const char *rxBufP)
{
    bool retFlag = false;

    if(0 == memcmp(txBufP, rxBufP, MODBUS_RESPONSE_MSG_START_LEN))
    {
#ifdef MODBUSRTU_DEBUG_PRINT
        qDebug("checkTxRxConformity pass");
#endif
        retFlag = true;
    }
    else if(0 == memcmp(txBufP, rxBufP, MODBUS_RESPONSE_MSG_START_LEN_2))
    {
#ifdef MODBUSRTU_DEBUG_PRINT
        qDebug("checkTxRxConformity pass");
#endif
        retFlag = true;
    }
    else
    {
#ifdef MODBUSRTU_DEBUG_PRINT
        qDebug("checkTxRxConformity fail");
#endif
        retFlag = false;
    }

#ifdef MODBUSRTU_DEBUG_PRINT
    for(int i = 0; i < MODBUS_RESPONSE_MSG_START_LEN; i++)
    {
        qDebug("txBufP[%d] = 0x%02x, rxBufP[%d] = 0x%02x", i, (uint8_t)txBufP[i], i, (uint8_t)rxBufP[i]);
    }
#endif

    return retFlag;
}

bool ModbusRTU::reInitModbusComm()
{
    bool ret = false;
    QString logStr;
    QString warningStr = tr("ModbusRTU Communication Error!");

    // If do not check error, return directly
    if(!errorCheckFlag)
    {
        return ret;
    }

    logStr = warningStr;
    logStr.append("reInitModbusComm");

    isTxRxOkFlag = false;
    txErrorCnt = 0;

    // ReInit com port again
    comPortDeInit();

    // Init Com Port for Modbus
    ret = comPortInit();

    // Update log
    updateLogData(logStr);

    return ret;
}

bool ModbusRTU::readHoldRegisters(uint16_t regOffset, uint16_t regCnt)
{
    bool ret = false;
    uint32_t index = 0;
    uint16_t crc = 0;
    struct MODBUS_READ_FEEDBACK tempFeedBackStruct;
    memset(&tempFeedBackStruct, 0, sizeof(struct MODBUS_READ_FEEDBACK));

    if(0 == regCnt)
    {
        return ret;
    }

    QMutexLocker locker(&mutex);

    // Clear buffer
    memset(m_comTxBuf, 0, TX_BUF_SIZE);
    memset((char *)&m_mbRxCheckStruct, 0, sizeof(struct MODBUS_RX_MSG_STRUCT));

    m_comTxBuf[index++] = m_devAddr;
    m_comTxBuf[index++] = MB_FUNC_READ_HOLDING_REGISTER;

    // Note: For Modbus RTU communication, data are big endian!
    // CRC16 is little endian!
    m_comTxBuf[index++] = (uint8_t)(regOffset >> 8);        // reg address high-8bit
    m_comTxBuf[index++] = (uint8_t)(regOffset & 0x00ff);    // reg address low-8bit

    m_comTxBuf[index++] = (uint8_t)(regCnt >> 8);        // reg count high-8bit
    m_comTxBuf[index++] = (uint8_t)(regCnt & 0x00ff);    // reg count low-8bit

    crc = CRCUtility::instance()->modbus_crc16((uint8_t *)m_comTxBuf, index);

    m_comTxBuf[index++] = (uint8_t)(crc & 0x00ff);    // CRC low-8bit
    m_comTxBuf[index++] = (uint8_t)(crc >> 8);        // CRC high-8bit

    tempFeedBackStruct.address = regOffset;
    tempFeedBackStruct.len = regCnt;
    tempFeedBackStruct.rdwrFlag = MODBUS_RD_OPT;

    // Fill rx msg check format
    m_mbRxCheckStruct.address = m_devAddr;
    m_mbRxCheckStruct.functionCode = MB_FUNC_READ_HOLDING_REGISTER;
    m_mbRxCheckStruct.data[0] = regCnt * sizeof(uint16_t);

    // This is a bug, readHoldRegisters() will be call several times sequently
    // m_mbRxCheckStruct will be overwrite, so here just fill the check len as 2
    m_mbRxCheckStruct.len = 2;

    // Push data to FIFO
    fifoBuf->pushData((char *)&tempFeedBackStruct, sizeof(struct MODBUS_READ_FEEDBACK));
    // Push data to FIFO
    ret = fifoBuf->pushData(m_comTxBuf, index);

    return ret;
}

bool ModbusRTU::writeMultiRegisters(uint16_t regOffset, const char *dataP, uint16_t regCnt)
{
    bool ret = false;
    uint32_t index = 0;
    uint16_t crc = 0;
    struct MODBUS_READ_FEEDBACK tempFeedBackStruct;
    memset(&tempFeedBackStruct, 0, sizeof(struct MODBUS_READ_FEEDBACK));

    if(NULL == dataP || 0 == regCnt)
    {
        return ret;
    }

    QMutexLocker locker(&mutex);

    // Clear buffer
    memset(m_comTxBuf, 0, TX_BUF_SIZE);
    memset((char *)&m_mbRxCheckStruct, 0, sizeof(struct MODBUS_RX_MSG_STRUCT));

    m_comTxBuf[index++] = m_devAddr;
    m_comTxBuf[index++] = MB_FUNC_WRITE_MULTIPLE_REGISTERS;

    // Note: For Modbus RTU communication, data are big endian!
    // CRC16 is little endian!
    m_comTxBuf[index++] = (uint8_t)(regOffset >> 8);        // reg address high-8bit
    m_comTxBuf[index++] = (uint8_t)(regOffset & 0x00ff);    // reg address low-8bit

    m_comTxBuf[index++] = (uint8_t)(regCnt >> 8);        // reg count high-8bit
    m_comTxBuf[index++] = (uint8_t)(regCnt & 0x00ff);    // reg count low-8bit

    m_comTxBuf[index++] = (uint8_t)(regCnt * sizeof(uint16_t));   // length in bytes

    for(uint32_t i = 0; i < regCnt; i++)
    {
        uint16_t value = *((uint16_t *)dataP + i);

        m_comTxBuf[index++] = (uint8_t)(value >> 8);        // data high-8bit
        m_comTxBuf[index++] = (uint8_t)(value & 0x00ff);    // data low-8bit
    }

    crc = CRCUtility::instance()->modbus_crc16((uint8_t *)m_comTxBuf, index);

    m_comTxBuf[index++] = (uint8_t)(crc & 0x00ff);    // CRC low-8bit
    m_comTxBuf[index++] = (uint8_t)(crc >> 8);        // CRC high-8bit

    tempFeedBackStruct.address = regOffset;
    tempFeedBackStruct.len = regCnt;
    tempFeedBackStruct.rdwrFlag = MODBUS_WR_OPT;

    // Fill rx msg check format
    m_mbRxCheckStruct.address = m_devAddr;
    m_mbRxCheckStruct.functionCode = MB_FUNC_WRITE_MULTIPLE_REGISTERS;
    m_mbRxCheckStruct.data[0] = (uint8_t)(regOffset >> 8);        // reg address high-8bit
    m_mbRxCheckStruct.data[1] = (uint8_t)(regOffset & 0x00ff);    // reg address low-8bit
    m_mbRxCheckStruct.data[2] = (uint8_t)(regCnt >> 8);        // reg count high-8bit
    m_mbRxCheckStruct.data[3] = (uint8_t)(regCnt & 0x00ff);    // reg count low-8bit
    m_mbRxCheckStruct.len = 6;

    // Push data to FIFO
    fifoBuf->pushData((char *)&tempFeedBackStruct, sizeof(struct MODBUS_READ_FEEDBACK));
    // Push data to FIFO
    ret = fifoBuf->pushData(m_comTxBuf, index);

    return ret;
}

#ifndef MODBUSRTU_H
#define MODBUSRTU_H

#include <QWidget>

#include <QSettings>
#include <QTimer>
#include <QTime>
#include <QMutex>

#include "ModbusCommBase.h"

#include "QSerialPort.h"
#include "FifoBuffer.h"
#include "LoopBuffer.h"
#include "FileLog.h"

namespace Ui {
class ModbusRTU;
}

class ModbusRTU : public ModbusCommBase
{
    Q_OBJECT
    
public:
    explicit ModbusRTU(ModbusCommBase *parent = 0);
    virtual ~ModbusRTU();

    // True: Modbus commumication is ok
    bool getModbusCommOk();

    // Set Modbus communication timeout, Unit:ms
    void setModbusTimeOut(int timeoutInMs);

    // Re-Init Modbus communication
    bool reInitModbusComm();

    // Set modbus slave address
    void setSlaveAddr(uint8_t addr);

    // Get modbus slave address
    uint8_t getSlaveAddr() const;

    /*-----------------------------------------------------------------------
    FUNCTION:       readHoldRegisters
    PURPOSE:        Read holding registers from modbusRTU slave device
    ARGUMENTS:      uint16_t regOffset  -- register offset address
                    uint16_t regCnt     -- count of registers
    RETURNS:        true - read successful, false - failed
    -----------------------------------------------------------------------*/
    virtual bool readHoldRegisters(uint16_t regOffset, uint16_t regCnt);

    /*-----------------------------------------------------------------------
    FUNCTION:       writeMultiRegisters
    PURPOSE:        Write multiple registers
    ARGUMENTS:      uint16_t regOffset      -- register offset address
                    const uint16_t *dataP   -- data pointer
                    uint16_t regCnt         -- count of registers
    RETURNS:        true - write successful, false - failed
    -----------------------------------------------------------------------*/
    virtual bool writeMultiRegisters(uint16_t regOffset, const char *dataP, uint16_t regCnt);

signals:
    void newDataReady(QByteArray);
    void newDataTx(QByteArray);

protected slots:
    void readDataFromModbus(QByteArray data);      //Read data from PLC

private slots:
    void periodTxService();
    void retransmitTask();

private:

    enum
    {
        RX_BUF_SIZE  = 1000,
        TX_BUF_SIZE  = 300
    };

    QString m_settingFile;
    QSettings *currentSetting;  // Store current setting with ini file

    // Serial COM port used to communicate with PLC
    QSerialPort *comPort;

    LoopBuffer *rxLoopBuf;
    FIFOBuffer *fifoBuf;
    char *m_comTxBuf;        // Transmit buffer

    struct COM_PORT_INIT_DATA *comInitData; // Printer COM port init data

    uint8_t m_devAddr;  // Modbus slave address

    int intervalTimeInMs;   // The time between Tx and Rx, Unit:ms
    QTime *intervalTime;    // Used to calculate elapsed time

    bool isTxRxOkFlag;   // This flag is used to indicate COM port Rx data CRC Ok or bad

    // This flag is used to indicate modbus client response the TX CMD, and send back the response
    bool getResponseFlag;
    int txRetryTimes;   // Used to calculate retry times
    int TX_RETRY_MAX_TIMES;   // Maximum tx retry times

    int txErrorCnt; // Transmit error count
    int TX_ERROR_MAX_CNT;   // Maximum tx error
    bool errorCheckFlag;    // Flag used to enable/disable error check

    int periodTxMaxTimeInMs;    // Period Tx time, Unit:ms
    QTimer *periodTxTmr; // This timer is used to trigger period Tx service

    QMutex mutex;   // locker

    FileLog *logFile;   // Log File
    QString logPath;    // Log Path

    uint32_t txBufLen;  // Modbus Tx buffer length

    // Modbus rx data from client
    struct MODBUS_RX_MSG_STRUCT m_mbRxCheckStruct;

    // Modbus read back strutct report to host
    struct MODBUS_READ_FEEDBACK readFeedbackStruct;

    // Flag used to indicate read/write operation of ModbusRTU communication
    // True: read operation, False: write operation
    bool modbusRTUReadOpt;

    // Write data to modbus
    int writeDataToModbus(const char *txData, int len);

    bool comPortInit();     // Init Com Port to Printer
    void comPortDeInit();   // Release/DeInit Com Port to Printer

    bool isComPortOpen();   //check whether the COM port is opened successful

    void loadSettingFromIniFile();  // Load setting from ini file
    void loadComSettingFromFile(); // Load COM port setting from ini file
    void initDefaultCOMSetting(); // Init the default COM port setting if setting not exist in ini file

    // CRC16 operation
    uint16_t do_crc16(const char *addr, uint16_t len);

    // Parse Rx Packet and show Reg data value
    void parseResponsePacket(QByteArray responseData);

    // Parse Response Data
    void parseResponseDataFromCOM();

    // Start timer to Tx CMD from FIFO buffer
    void startPeriodTxService();

    // Update Log to file
    void updateLogData(QString logStr);

    // Check Rx msg is the right feedback for Tx msg
    bool checkTxRxConformity(const char *txBufP, const char *rxBufP);

};

#endif // MODBUSRTU_H

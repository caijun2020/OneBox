/**********************************************************************
PACKAGE:        Communication
FILE:           ModbusTCP.h
COPYRIGHT (C):  All rights reserved.

PURPOSE:        Modbus TCP interface
**********************************************************************/

#ifndef MODBUSTCP_H
#define MODBUSTCP_H

#include "TcpClient.h"
#include "ModbusData.h"
#include "LoopBuffer.h"
#include "FifoBuffer.h"
#include <QThread>
#include <QMutex>
#include <QTimer>


class ModbusTCP : public QThread
{
    Q_OBJECT
public:
    explicit ModbusTCP(QObject *parent = 0);
    ~ModbusTCP();

    enum
    {
        RX_BUF_SIZE  = 1000,
        TX_BUF_SIZE  = 300
    };

    void run();

    // Connect to Server
    bool connectToServer(const QHostAddress &ip = QHostAddress::Any, uint16_t port = 0);
    bool connectToServer(QString ip, uint16_t port = 0);

    // Disconnect from server
    void disconnectFromServer();

    /*-----------------------------------------------------------------------
    FUNCTION:		bindModel
    PURPOSE:		Bind a TCPClient model
    ARGUMENTS:		TCPClient *clientP -- TCPClient clientP
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void bindModel(TCPClient *clientP);

    /*-----------------------------------------------------------------------
    FUNCTION:		unbind
    PURPOSE:		Unbind the TCPClient model
    ARGUMENTS:		None
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void unbind();

    /*-----------------------------------------------------------------------
    FUNCTION:       setTransactionID
    PURPOSE:        Set transaction ID
    ARGUMENTS:      uint16_t id  -- transaction ID of MBPA
    RETURNS:        None
    -----------------------------------------------------------------------*/
    void setTransactionID(uint16_t id);

    /*-----------------------------------------------------------------------
    FUNCTION:       setProtocolID
    PURPOSE:        Set protocol ID
    ARGUMENTS:      uint16_t id  -- protocol ID of MBPA
    RETURNS:        None
    -----------------------------------------------------------------------*/
    void setProtocolID(uint16_t id);

    /*-----------------------------------------------------------------------
    FUNCTION:       setUnitID
    PURPOSE:        Set Unit ID
    ARGUMENTS:      uint16_t id  -- unit ID of MBPA
    RETURNS:        None
    -----------------------------------------------------------------------*/
    void setUnitID(uint8_t id);

    /*-----------------------------------------------------------------------
    FUNCTION:       setTxPeriod
    PURPOSE:        Set Tx period
    ARGUMENTS:      uint32_t ms  -- milliseconds
    RETURNS:        None
    -----------------------------------------------------------------------*/
    void setTxPeriod(uint32_t ms);

    /*-----------------------------------------------------------------------
    FUNCTION:       setTxRetryTimes
    PURPOSE:        Set Tx retransmit times
    ARGUMENTS:      uint32_t cnt  -- retransmit count
    RETURNS:        None
    -----------------------------------------------------------------------*/
    void setTxRetryTimes(uint32_t cnt);

    /*-----------------------------------------------------------------------
    FUNCTION:       setAutoReconnect
    PURPOSE:        Set Auto reconnect to server flag
    ARGUMENTS:      bool, true: auto-reconnect, false: do not auto reconnect
    RETURNS:        None
    -----------------------------------------------------------------------*/
    void setAutoReconnect(bool autoConnectFlag);

    /*-----------------------------------------------------------------------
    FUNCTION:       readInputRegisters
    PURPOSE:        Read input registers from modbusRTU slave device
    ARGUMENTS:      uint16_t regOffset  -- register offset address
                    uint16_t regCnt     -- count of registers
    RETURNS:        true - read successful, false - failed
    -----------------------------------------------------------------------*/
    bool readInputRegisters(uint16_t regOffset, uint16_t regCnt);

    /*-----------------------------------------------------------------------
    FUNCTION:       readHoldRegisters
    PURPOSE:        Read holding registers from modbusRTU slave device
    ARGUMENTS:      uint16_t regOffset  -- register offset address
                    uint16_t regCnt     -- count of registers
    RETURNS:        true - read successful, false - failed
    -----------------------------------------------------------------------*/
    bool readHoldRegisters(uint16_t regOffset, uint16_t regCnt);

    /*-----------------------------------------------------------------------
    FUNCTION:       writeHoldRegisters
    PURPOSE:        Write single holding register from modbusRTU slave device
    ARGUMENTS:      uint16_t regOffset  -- register offset address
                    uint16_t regValue   -- value of registers
    RETURNS:        true - read successful, false - failed
    -----------------------------------------------------------------------*/
    bool writeHoldRegister(uint16_t regOffset, uint16_t regValue);

    /*-----------------------------------------------------------------------
    FUNCTION:       writeMultiRegisters
    PURPOSE:        Write multiple registers
    ARGUMENTS:      uint16_t regOffset      -- register offset address
                    const uint16_t *dataP   -- data pointer
                    uint16_t regCnt         -- count of registers
    RETURNS:        true - write successful, false - failed
    -----------------------------------------------------------------------*/
    bool writeMultiRegisters(uint16_t regOffset, const char *dataP, uint16_t regCnt);

    /*-----------------------------------------------------------------------
    FUNCTION:       writeMultiRegistersInt32
    PURPOSE:        Write 32bit value for reg via multiple registers(0x10 cmd)
    ARGUMENTS:      uint16_t regOffset      -- register offset address
                    uint32_t value         -- data value
    RETURNS:        true - write successful, false - failed
    -----------------------------------------------------------------------*/
    bool writeMultiRegistersInt32(uint16_t regOffset, uint32_t value);

    /*-----------------------------------------------------------------------
    FUNCTION:       writeMultiRegistersInt16
    PURPOSE:        Write 16bit value for reg via multiple registers(0x10 cmd)
    ARGUMENTS:      uint16_t regOffset      -- register offset address
                    uint16_t value         -- data value
    RETURNS:        true - write successful, false - failed
    -----------------------------------------------------------------------*/
    bool writeMultiRegistersInt16(uint16_t regOffset, uint16_t value);

    // Start timer to Tx CMD from FIFO buffer
    void startPeriodTxService();
    void stopPeriodTxService();

    bool getRunningStatus() const;

signals:
    void connectionChanged(bool connected);
    void newDataReady(QByteArray);
    void newDataTx(QByteArray);
    void startTxTimer();
    void stopTxTimer();
    void newResponseMsg(MODBUS_READ_FEEDBACK msg);

protected slots:
    void updateIncomingData(QByteArray data);
    void diconnectedStatus(void);
    void initTxTimer();
    void deInitTxTimer();

    void periodTxService();
    void retransmitTask();

private:

    QMutex mutex; // Mutex locker

    TCPClient *m_tcpClient;

    LoopBuffer *rxLoopBuf;
    FIFOBuffer *fifoBuf;
    char *m_comTxBuf;        // Transmit buffer
    uint32_t txBufLen;  // Modbus Tx buffer length

    uint16_t m_transactionID;   // ModbusTCP transaction ID
    uint16_t m_protocolID;  // ModbusTCP protocol ID, always 0x0000
    uint8_t m_unitID;  // ModbusTCP unit ID

    bool isRunning;   // Flag to indicate tcp client is running or not

    // Modbus read back strutct report to host
    struct MODBUS_READ_FEEDBACK readFeedbackStruct;

    uint32_t m_periodTxTimeInMs;    // Period tx time in MS
    QTimer *periodTxTmr; // This timer is used to trigger period Tx service

    // This flag is used to indicate modbus client response the TX CMD, and send back the response
    bool getResponseFlag;
    int txRetryTimes;   // Used to calculate retry times
    int TX_RETRY_MAX_TIMES;   // Maximum tx retry times

    int txErrorCnt; // Transmit error count
    int TX_ERROR_MAX_CNT;   // Maximum tx error

    // This flag is used to detect connection lost and auto re-connect
    bool m_autoConnectToServerFlag;

    QHostAddress hostAddr;      // Host IP
    uint16_t serverPort;        // Host port

    // Write data to modbus
    int writeDataToModbus(const char *txData, int len);

    // Parse Rx Packet and show Reg data value
    bool parseResponsePacket(QByteArray &responseData);

    // Auto connect to tcp server
    void autoConnectToServer();

private slots:

};

#endif // MODBUSTCP_H

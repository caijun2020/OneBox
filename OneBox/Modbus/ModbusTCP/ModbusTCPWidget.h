/**********************************************************************
PACKAGE:        UI
FILE:           ModbusTCPWidget.h
COPYRIGHT (C):  All rights reserved.

PURPOSE:        Modbus TCP UI widget
**********************************************************************/

#ifndef MODBUSTCPWIDGET_H
#define MODBUSTCPWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QTimer>
#include <QTime>
#include <QMutex>

#include "ModbusTCP.h"
#include "FifoBuffer.h"
#include "LoopBuffer.h"
#include "FileLog.h"

namespace Ui {
class ModbusTCPWidget;
}

class ModbusTCPWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ModbusTCPWidget(QWidget *parent = 0);
    virtual ~ModbusTCPWidget();

    /*-----------------------------------------------------------------------
    FUNCTION:		bindModel
    PURPOSE:		Bind a ModbusTCP model
    ARGUMENTS:		ModbusTCP *clientP -- ModbusTCP clientP
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void bindModel(ModbusTCP *clientP);

    /*-----------------------------------------------------------------------
    FUNCTION:		unbind
    PURPOSE:		Unbind the ModbusTCP model
    ARGUMENTS:		None
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void unbind();

    // Set ModbusTcp unit ID
    void setUnitID(uint8_t id);

    // Return tcp connection status, true: connected, false: disconnected
    bool getConnectionStatus() const;

public:
    typedef struct
    {
        int value;
        char text[50];
    }COMBOX_LIST;

signals:

public slots:
    virtual void updateUI();

private slots:

    void updateConnectionStatus(bool connected);
    void readDataFromModbus(QByteArray data);
    void updateTxDataToLog(QByteArray data);

    void on_pushButton_connect_clicked();

    void on_pushButton_clear_clicked();

    void on_pushButton_send_clicked();

    void on_checkBox_hex_clicked(bool checked);

    void on_checkBox_autoClear_clicked(bool checked);

    void on_lineEdit_serverIP_editingFinished();

    void on_lineEdit_serverPort_editingFinished();

    void on_comboBox_functionCode_currentIndexChanged(int index);

    void on_lineEdit_transactionID_editingFinished();

    void on_lineEdit_protocolID_editingFinished();

    void on_lineEdit_unitID_editingFinished();

    void on_lineEdit_data_textChanged(const QString &arg1);

    void on_checkBox_showTxRx_clicked(bool checked);

    void on_checkBox_autoReconnect_clicked(bool checked);

private:

    enum
    {
        RX_BUF_SIZE  = 1000,
        TX_BUF_SIZE  = 300
    };

    Ui::ModbusTCPWidget *ui;

    QTimer *refreshTimer;
    int refreshInMs;
    QMutex m_mutex; // Mutex

    QString m_settingFile;

    ModbusTCP *m_modbusTCP;
    QSettings *currentSetting;  // Store current setting with ini file

    uint16_t m_transactionID;   // ModbusTCP transaction ID
    uint16_t m_protocolID;  // ModbusTCP protocol ID, always 0x0000
    uint8_t m_unitID;  // ModbusTCP unit ID

    int intervalTimeInMs;   // The time between Tx and Rx, Unit:ms
    QTime *intervalTime;    // Used to calculate elapsed time

    int periodTxTimeInMs;    // Period Tx time, Unit:ms
    int TX_RETRY_MAX_TIMES;   // Maximum tx retry times

    bool isRunning;  // This falg is used to indicate tcp connection status
    bool hexFormatFlag; // This flag is used to enable hex format show
    bool autoClearRxFlag; // This flag is used to clear rx buffer automatically

    QString m_serverIP;
    int m_serverPort;

    // This flag is used to detect connection lost and auto re-connect
    bool m_autoConnectToServerFlag;

    bool showTxRxFlag;  // flag used to enable Tx/Rx packet display in log area

    // Load Settings from ini file
    void loadSettingFromIniFile();

    // Update setting to ini file
    void updateSettingToFile();

    void initWidgetFont();  // Init the Font type and size of the widget
    void initWidgetStyle(); // Init Icon of the widget

    // Update Log to file
    void updateLogData(QString logStr);

    // Update log to display buffer textEdit area
    void updateToUILog(QString logStr, bool timeStamp = true);

    // Auto connect to tcp server
    void autoConnectToServer();

};

#endif // MODBUSTCPWIDGET_H

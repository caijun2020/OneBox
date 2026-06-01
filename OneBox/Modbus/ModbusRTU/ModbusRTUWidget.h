#ifndef MODBUSRTUWIDGET_H
#define MODBUSRTUWIDGET_H

#include <QWidget>

#include <QSettings>
#include <QTimer>
#include <QTime>
#include <QMutex>

#include "ModbusRTU.h"
#include "FileLog.h"

namespace Ui {
class ModbusRTUWidget;
}

class ModbusRTUWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ModbusRTUWidget(QWidget *parent = 0);
    virtual ~ModbusRTUWidget();

    /*-----------------------------------------------------------------------
    FUNCTION:		bindModel
    PURPOSE:		Bind a ModbusRTU model
    ARGUMENTS:		ModbusRTU *modbusRTUP -- ModbusRTU modbusRTUP
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void bindModel(ModbusRTU *modbusRTUP);

    /*-----------------------------------------------------------------------
    FUNCTION:		unbind
    PURPOSE:		Unbind the ModbusRTU model
    ARGUMENTS:		None
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void unbind();

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

public:
    typedef struct
    {
        int value;
        char text[50];
    }COMBOX_LIST;

signals:

public slots:
    virtual void updateUI();

protected slots:

private slots:

    void on_pushButton_clear_clicked();

    void on_pushButton_send_clicked();

    void on_checkBox_autoClear_clicked(bool checked);

    void on_checkBox_hex_clicked(bool checked);

    void on_comboBox_functionCode_currentIndexChanged(int index);

    void on_lineEdit_data_textEdited(const QString &arg1);

    void on_lineEdit_address_editingFinished();

    void readDataFromModbus(QByteArray data);
    void updateTxDataToLog(QByteArray data);
    void handleModbusResponseValue(MODBUS_READ_FEEDBACK data);

private:

    enum
    {
        RX_BUF_SIZE  = 1000,
        TX_BUF_SIZE  = 300
    };

    Ui::ModbusRTUWidget *ui;

    QTimer *refreshTimer;
    int refreshInMs;
    QString m_settingFile;

    ModbusRTU *m_modbusRTU;

    QSettings *currentSetting;  // Store current setting with ini file

    QString widgetFontType; // Store the font type of widget
    int widgetFontSize;     // Store the font size of widget

    int intervalTimeInMs;   // The time between Tx and Rx, Unit:ms
    QTime *intervalTime;    // Used to calculate elapsed time
    QMutex mutex;   // locker

    FileLog *logFile;   // Log File
    QString logPath;    // Log Path

    uint32_t txBufLen;  // Modbus Tx buffer length

    uint8_t m_devAddr;  // Modbus slave address

    // Modbus rx data from client
    struct MODBUS_RX_MSG_STRUCT m_mbRxCheckStruct;

    // Modbus read back strutct report to host
    struct MODBUS_READ_FEEDBACK readFeedbackStruct;

    bool hexFormatFlag; // This flag is used to enable hex format show
    bool autoClearRxFlag; // This flag is used to clear rx buffer automatically

    void loadSettingFromIniFile();  // Load setting from ini file

    void initWidgetFont();  // Init the Font type and size of the widget
    void initWidgetStyle(); // Init Icon of the widget

    // Update Log to file
    void updateLogData(QString logStr);

    // Update log to display buffer textEdit area
    void updateToUILog(QString logStr, bool timeStamp = true);

};

#endif // MODBUSRTUWIDGET_H

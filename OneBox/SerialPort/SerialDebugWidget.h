/**********************************************************************
PACKAGE:        Communication
FILE:           SerialDebugWidget.h
COPYRIGHT (C):  All rights reserved.

PURPOSE:        Serial Port Debug Widget UI
**********************************************************************/
#ifndef SERIALDEBUGWIDGET_H
#define SERIALDEBUGWIDGET_H

#include <QWidget>
#include "QSerialPort.h"

namespace Ui {
class SerialDebugWidget;
}

class SerialDebugWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit SerialDebugWidget(QSerialPort *portHandler = NULL, QWidget *parent = 0);
    ~SerialDebugWidget();

    /*-----------------------------------------------------------------------
    FUNCTION:		bindModel
    PURPOSE:		Bind a QSerialPort model
    ARGUMENTS:		QSerialPort *portHandler -- QSerialPort pointer
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void bindModel(QSerialPort *portHandler);

    /*-----------------------------------------------------------------------
    FUNCTION:		unbind
    PURPOSE:		Unbind the QSerialPort model
    ARGUMENTS:		None
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void unbind();

    /*-----------------------------------------------------------------------
    FUNCTION:		sendData
    PURPOSE:		Send data to serial port
    ARGUMENTS:		QByteArray &data     -- tx data buffer
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void sendData(QByteArray &data);

    /*-----------------------------------------------------------------------
    FUNCTION:		sendData
    PURPOSE:		Send data to serial port
    ARGUMENTS:		const char *data     -- tx data buffer pointer
                    uint32_t len         -- tx data length
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void sendData(const char *data, uint32_t len);

public:
    typedef struct
    {
        int value;
        char text[50];
    }COMBOX_LIST;

signals:
    void newDataReady(QByteArray);

protected:
    void resizeEvent(QResizeEvent *e);

private slots:
    void on_pushButton_send_clicked();

    void on_pushButton_open_clicked();

    void updateUI();

    void on_pushButton_reset_clicked();

    void on_pushButton_clear_clicked();

    void updateIncomingData(QByteArray data);

    void autoSendData();

    void on_checkBox_autoSend_clicked(bool checked);

    void on_checkBox_timeStamp_clicked(bool checked);

    void on_checkBox_autoClear_clicked(bool checked);

    void on_checkBox_hex_clicked(bool checked);

private:

    Ui::SerialDebugWidget *ui;

    QSerialPort *serialPort;    // COM port handler
    struct COM_PORT_INIT_DATA *comInitData; // COM port init data

    QString widgetFontType; // Store the font type of widget
    int widgetFontSize;     // Store the font size of widget

    bool isOpenFlag;    // True: COM port is open
    bool timeStampFlag; // This flag is used to enable timestamp info
    bool hexFormatFlag; // This flag is used to enable hex format show
    bool autoClearRxFlag; // This flag is used to clear rx buffer automatically

    QTimer refreshUITimer;  // Timer used to refresh tx/rx bytes
    QTimer autoSendTimer;   // Timer used to auto send data

    void initWidgetFont();  // Init the Font type and size
    void initWidgetStyle(); // Init widget style

    // Enable/Disable funtion UI
    void setFunctionUI(bool enable);

    // Update Rx data in textBrowser
    void updateLogData(QString logStr);
};

#endif // SERIALDEBUGWIDGET_H

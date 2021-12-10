/**********************************************************************
PACKAGE:        Communication
FILE:           UdpClientWidget.h
COPYRIGHT (C):  All rights reserved.

PURPOSE:        UDP Client Widget UI
**********************************************************************/

#ifndef UDPCLIENTWIDGET_H
#define UDPCLIENTWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QSettings>
#include "UdpClient.h"


namespace Ui {
class UdpClientWidget;
}

class UdpClientWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit UdpClientWidget(QWidget *parent = 0);
    ~UdpClientWidget();

    /*-----------------------------------------------------------------------
    FUNCTION:		bindModel
    PURPOSE:		Bind a UDPClient model
    ARGUMENTS:		UDPClient *clientP -- UDPClient pointer
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void bindModel(UDPClient *clientP);

    /*-----------------------------------------------------------------------
    FUNCTION:		unbind
    PURPOSE:		Unbind the UDPServer model
    ARGUMENTS:		None
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void unbind();

    /*-----------------------------------------------------------------------
    FUNCTION:		sendData
    PURPOSE:		Send data to a server
    ARGUMENTS:		QHostAddress address -- IP address
                    uint16_t port        -- port No.
                    QByteArray &data     -- tx data buffer
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void sendData(QHostAddress address, uint16_t port, QByteArray &data);

protected:
    void resizeEvent(QResizeEvent *e);

signals:
    void newDataReady(QByteArray data);

private slots:
    void on_pushButton_send_clicked();

    void on_pushButton_clear_clicked();

    void updateIncomingData();

    void on_pushButton_connect_clicked();

    void updateTxDataToLog(QHostAddress address, int port, QByteArray data);

    void updateUI();

    void on_pushButton_reset_clicked();

    void on_checkBox_hex_clicked(bool checked);

    void on_checkBox_autoClear_clicked(bool checked);

    void updateServerInfo(QHostAddress address, int port);

    void updateConnectionStatus(bool connected);

    void on_lineEdit_IP_editingFinished();

    void on_lineEdit_serverPort_editingFinished();

    void on_lineEdit_localPort_editingFinished();

    void on_checkBox_showTx_clicked(bool checked);

    void on_checkBox_showRx_clicked(bool checked);

private:
    Ui::UdpClientWidget *ui;

    QSettings *currentSetting;  // Store current setting with ini file

    UDPClient *udpClient;
    bool isRunning;
    bool hexFormatFlag; // This flag is used to enable hex format show
    bool autoClearRxFlag; // This flag is used to clear rx buffer automatically

    QByteArray rxDataBuf;   // Rx data buffer

    QString serverIP;
    uint16_t serverPort;
    uint16_t localPort;

    QTimer *refreshTimer;
    int refreshInMs;

    bool showTxPacketFlag;  // flag used to enable Tx packet display in log area
    bool showRxPacketFlag;  // flag used to enable Rx packet display in log area

    void initWidgetFont();  // Init the Font type and size of the widget
    void initWidgetStyle(); // Init Icon of the widget

    void loadSettingFromIniFile();  // Load setting from ini file

    // Update setting to ini file
    void updateSettingToFile();

    // Update log in TextEdit area
    void updateLogData(QString logStr);
};

#endif // UDPCLIENTWIDGET_H

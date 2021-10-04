/**********************************************************************
PACKAGE:        Communication
FILE:           TcpClientWidget.h
COPYRIGHT (C):  All rights reserved.

PURPOSE:        TCP Client Widget UI
**********************************************************************/

#ifndef TCPCLIENTWIDGET_H
#define TCPCLIENTWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QSettings>

#include "TcpClient.h"

namespace Ui {
class TcpClientWidget;
}

class TcpClientWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit TcpClientWidget(QWidget *parent = 0);
    ~TcpClientWidget();

    bool getTcpConnectionStatus();
    bool connectToServer(QString ip, uint16_t port);

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
    FUNCTION:		sendData
    PURPOSE:		Send data to client
    ARGUMENTS:		QByteArray &data     -- tx data buffer
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void sendData(QByteArray &data);

signals:
    void newDataReady(QByteArray data);

protected:
    void resizeEvent(QResizeEvent *e);

private slots:
    void on_pushButton_connect_clicked();

    void on_pushButton_send_clicked();

    void updateIncomingData();

    void diconnectedStatus();

    void updateTxDataToLog(QHostAddress address, uint16_t port, QByteArray data);

    void updateUI();

    void on_pushButton_reset_clicked();

    void on_pushButton_clear_clicked();

    void on_checkBox_hex_clicked(bool checked);

    void on_checkBox_autoClear_clicked(bool checked);

    void updateServerInfo(QHostAddress address, uint16_t port);

    void updateConnectionStatus(bool connected);

    void on_lineEdit_IP_editingFinished();

    void on_lineEdit_listenPort_editingFinished();

private:
    Ui::TcpClientWidget *ui;

    QSettings *currentSetting;  // Store current setting with ini file

    TCPClient *tcpClient;
    bool isRunning;
    bool hexFormatFlag; // This flag is used to enable hex format show
    bool autoClearRxFlag; // This flag is used to clear rx buffer automatically

    QByteArray rxDataBuf;   // Rx data buffer

    QTimer refreshUITimer;

    QString serverIP;
    uint16_t serverPort;

    void initWidgetFont();  // Init the Font type and size of the widget
    void initWidgetStyle(); // Init Icon of the widget

    void loadSettingFromIniFile();  // Load setting from ini file

    // Update setting to ini file
    void updateSettingToFile();

    // Update log in TextEdit area
    void updateLogData(QString logStr);
};

#endif // TCPCLIENTWIDGET_H

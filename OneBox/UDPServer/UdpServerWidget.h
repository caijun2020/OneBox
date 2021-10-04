/**********************************************************************
PACKAGE:        Communication
FILE:           UdpServerWidget.h
COPYRIGHT (C):  All rights reserved.

PURPOSE:        UDP Server Widget UI
**********************************************************************/

#ifndef UDPSERVERWIDGET_H
#define UDPSERVERWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QHostAddress>
#include <QSettings>

#include "UdpServer.h"

namespace Ui {
class UdpServerWidget;
}

class UdpServerWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit UdpServerWidget(QWidget *parent = 0);
    virtual ~UdpServerWidget();

    /*-----------------------------------------------------------------------
    FUNCTION:		bindModel
    PURPOSE:		Bind a UDPServer model
    ARGUMENTS:		UDPServer *serverP -- UDPServer pointer
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void bindModel(UDPServer *serverP);

    /*-----------------------------------------------------------------------
    FUNCTION:		unbind
    PURPOSE:		Unbind the UDPServer model
    ARGUMENTS:		None
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void unbind();

    /*-----------------------------------------------------------------------
    FUNCTION:		sendData
    PURPOSE:		Send data to client
    ARGUMENTS:		uint32_t clientIndex -- the index number of clientList
                    QByteArray &data     -- tx data buffer
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void sendData(uint32_t clientIndex, QByteArray &data);

    /*-----------------------------------------------------------------------
    FUNCTION:		sendData
    PURPOSE:		Send data to client
    ARGUMENTS:		uint32_t clientIndex -- the index number of clientList
                    char *data           -- tx data buffer pointer
                    uint32_t len         -- tx data length
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void sendData(uint32_t clientIndex, const char *data, uint32_t len);

signals:
    void newDataReady(int clientIndex, QByteArray data);

protected:
    void resizeEvent(QResizeEvent *e);

private slots:
    void on_pushButton_listen_clicked();

    void on_pushButton_send_clicked();

    void on_pushButton_clear_clicked();

    void addIncomingClient(QString str);
    void removeIncomingClient(QString str);
    void updateIncomingData(int clientIndex);
    void updateIncomingData(int clientIndex, QByteArray data);

    void updateTxDataToLog(QHostAddress address, int port, QByteArray data);
    void updateMessageToLog(const QString &msg);

    void on_pushButton_reset_clicked();

    void updateUI();

    void on_checkBox_hex_clicked(bool checked);

    void on_checkBox_autoClear_clicked(bool checked);

    void updateServerInfo(QHostAddress address, uint16_t port);

    void updateConnectionStatus(bool connected);

    void on_lineEdit_IP_editingFinished();

    void on_lineEdit_listenPort_editingFinished();

private:
    Ui::UdpServerWidget *ui;

    QSettings *currentSetting;  // Store current setting with ini file

    UDPServer *udpServer;
    bool isRunning;
    bool hexFormatFlag; // This flag is used to enable hex format show
    bool autoClearRxFlag; // This flag is used to clear rx buffer automatically

    QByteArray rxDataBuf;   // Rx data buffer

    QTimer refreshUITimer;  // Timer used to refresh tx/rx cnt

    QString serverIP;
    uint16_t listenPort;

    void initWidgetFont();  // Init the Font type and size of the widget
    void initWidgetStyle(); // Init Icon of the widget

    void loadSettingFromIniFile();  // Load setting from ini file

    // Update setting to ini file
    void updateSettingToFile();

    // Update log in TextEdit area
    void updateLogData(QString logStr);

};

#endif // UDPSERVERWIDGET_H

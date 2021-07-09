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

    void updateTxDataToLog(QHostAddress address, uint16_t port, QByteArray data);

    void updateUI();

    void on_pushButton_reset_clicked();

    void on_checkBox_hex_clicked(bool checked);

    void on_checkBox_autoClear_clicked(bool checked);

private:
    Ui::UdpClientWidget *ui;

    UDPClient *udpClient;
    bool isRunning;
    bool hexFormatFlag; // This flag is used to enable hex format show
    bool autoClearRxFlag; // This flag is used to clear rx buffer automatically

    QByteArray rxDataBuf;   // Rx data buffer

    QTimer refreshUITimer;

    void initWidgetFont();  // Init the Font type and size of the widget
    void initWidgetStyle(); // Init Icon of the widget

    // Update log in TextEdit area
    void updateLogData(QString logStr);
};

#endif // UDPCLIENTWIDGET_H

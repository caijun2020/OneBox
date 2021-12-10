#ifndef TCPSERVERWIDGET_H
#define TCPSERVERWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QSettings>
#include "TcpServer.h"

namespace Ui {
class TcpServerWidget;
}

class TcpServerWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit TcpServerWidget(QWidget *parent = 0);
    ~TcpServerWidget();

    /*-----------------------------------------------------------------------
    FUNCTION:		bindModel
    PURPOSE:		Bind a TCPServer model
    ARGUMENTS:		TCPServer *serverP -- TCPServer pointer
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void bindModel(TCPServer *serverP);

    /*-----------------------------------------------------------------------
    FUNCTION:		unbind
    PURPOSE:		Unbind the TCPServer model
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
    void newDataReady(uint32_t clientIndex, QByteArray data);

protected:
    void resizeEvent(QResizeEvent *e);

private slots:
    void on_pushButton_listen_clicked();

    void on_pushButton_send_clicked();

    void on_pushButton_clear_clicked();

    void addIncomingClient(QString str);
    void removeIncomingClient(QString str);
    void updateIncomingData(uint32_t clientIndex);
    void updateTxDataToLog(QHostAddress address, uint16_t port, QByteArray data);

    void updateUI();

    void on_pushButton_reset_clicked();

    void on_checkBox_hex_clicked(bool checked);

    void on_checkBox_autoClear_clicked(bool checked);

    void updateServerInfo(QHostAddress address, uint16_t port);

    void updateConnectionStatus(bool connected);

    void on_lineEdit_IP_editingFinished();

    void on_lineEdit_listenPort_editingFinished();

    void on_checkBox_showTx_clicked(bool checked);

    void on_checkBox_showRx_clicked(bool checked);

private:
    Ui::TcpServerWidget *ui;

    QSettings *currentSetting;  // Store current setting with ini file

    TCPServer *tcpServer;
    bool isRunning;
    bool hexFormatFlag; // This flag is used to enable hex format show
    bool autoClearRxFlag; // This flag is used to clear rx buffer automatically

    QByteArray rxDataBuf;   // Rx data buffer

    QString serverIP;
    uint16_t listenPort;

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

#endif // TCPSERVERWIDGET_H

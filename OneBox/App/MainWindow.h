#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "TcpServerWidget.h"
#include "TcpClientWidget.h"
#include "UdpServerWidget.h"
#include "UdpClientWidget.h"
#include "SerialDebugWidget.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;

    struct TAB_WIDGET_CONFIG
    {
        int index;
        char label[40];
        QWidget *widget;
    };

    enum TAB_WIDGET_INDEX
    {
        TCP_SERVER_TAB_INDEX = 0,
        TCP_CLIENT_TAB_INDEX,
        UDP_SERVER_TAB_INDEX,
        UDP_CLIENT_TAB_INDEX,
        SERIAL_TAB_INDEX,
        TAB_WIDGET_CNT
    };

    // Setting used to config the tab widget
    struct TAB_WIDGET_CONFIG m_tabWidgetSetting[TAB_WIDGET_CNT];

    // Communication models
    TCPServer *m_tcpServer;
    TCPClient *m_tcpClient;
    UDPServer *m_udpServer;
    UDPClient *m_udpClient;
    QSerialPort *m_serialPort;

    // UIs
    TcpServerWidget *m_tcpServerW;
    TcpClientWidget *m_tcpClientW;
    UdpServerWidget *m_udpServerW;
    UdpClientWidget *m_udpClientW;
    SerialDebugWidget *m_serialW;

    void initWidgetFont();  // Init the Font type and size of the widget
    void initWidgetStyle(); // Init Icon of the widget
};

#endif // MAINWINDOW_H

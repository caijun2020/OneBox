#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_tcpServer(new TCPServer),
    m_tcpClient(new TCPClient),
    m_udpServer(new UDPServer),
    m_udpClient(new UDPClient),
    m_serialPort(new QSerialPort),
    m_tcpServerW(new TcpServerWidget),
    m_tcpClientW(new TcpClientWidget),
    m_udpServerW(new UdpServerWidget),
    m_udpClientW(new UdpClientWidget),
    m_serialW(new SerialDebugWidget)
{
    ui->setupUi(this);

    // Init Widget Font type and size
    initWidgetFont();

    // Init Widget Style
    initWidgetStyle();

    // UI bind model
    m_tcpServerW->bindModel(m_tcpServer);
    m_tcpClientW->bindModel(m_tcpClient);
    m_udpServerW->bindModel(m_udpServer);
    m_udpClientW->bindModel(m_udpClient);
    m_serialW->bindModel(m_serialPort);

    // Set Window Title
    this->setWindowTitle(tr("All in One ToolBox"));

    // Set Menu Bar Version Info
    ui->menuVersion->addAction("V1.0 2021-Sep-17");
}

MainWindow::~MainWindow()
{
    delete ui;

    delete m_tcpServer;
    delete m_tcpServerW;

    delete m_tcpClient;
    delete m_tcpClientW;

    delete m_udpServer;
    delete m_udpServerW;

    delete m_udpClient;
    delete m_udpClientW;

    delete m_serialPort;
    delete m_serialW;
}

void MainWindow::initWidgetFont()
{
}

void MainWindow::initWidgetStyle()
{
    // When add new tab, update the following struct list
    struct TAB_WIDGET_CONFIG tmpSetting[] =
    {
        {SERIAL_TAB_INDEX, "SerialPort", m_serialW},
        {UDP_SERVER_TAB_INDEX, "UDP Server", m_udpServerW},
        {UDP_CLIENT_TAB_INDEX, "UDP Client", m_udpClientW},
        {TCP_SERVER_TAB_INDEX, "TCP Server", m_tcpServerW},
        {TCP_CLIENT_TAB_INDEX, "TCP Client", m_tcpClientW},
    };

    // Init struct setting of tabWidget
    memset(m_tabWidgetSetting, 0, sizeof(TAB_WIDGET_CONFIG) * TAB_WIDGET_CNT);
    if(sizeof(tmpSetting) > sizeof(TAB_WIDGET_CONFIG) * TAB_WIDGET_CNT)
    {
        memcpy(m_tabWidgetSetting, tmpSetting, sizeof(TAB_WIDGET_CONFIG) * TAB_WIDGET_CNT);
    }
    else
    {
        memcpy(m_tabWidgetSetting, tmpSetting, sizeof(tmpSetting));
    }

    for(int i = 0; i < TAB_WIDGET_CNT; i++)
    {
        for(int cnt = 0; cnt < TAB_WIDGET_CNT; cnt++)
        {
            if(i == m_tabWidgetSetting[cnt].index && NULL != m_tabWidgetSetting[cnt].widget)
            {
                ui->tabWidget->insertTab(m_tabWidgetSetting[cnt].index,
                                         m_tabWidgetSetting[cnt].widget,
                                         QString(m_tabWidgetSetting[cnt].label));
                break;
            }
        }
    }

}

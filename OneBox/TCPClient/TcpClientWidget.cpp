/**********************************************************************
PACKAGE:        Communication
FILE:           TcpClientWidget.cpp
COPYRIGHT (C):  All rights reserved.

PURPOSE:        TCP Client Widget UI
**********************************************************************/

#include "TcpClientWidget.h"
#include "ui_TcpClientWidget.h"

#include <QNetworkInterface>
#include <QDateTime>
#include <QStringList>
#include <QDebug>

#include "QtBaseType.h"
#include "QUtilityBox.h"

TcpClientWidget::TcpClientWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcpClientWidget),
    tcpClient(NULL),
    isRunning(false),
    hexFormatFlag(false),
    autoClearRxFlag(true),
    refreshTimer(new QTimer),
    refreshInMs(1000),
    serverIP("127.0.0.1"),
    serverPort(50000),
    showTxPacketFlag(true),
    showRxPacketFlag(true)
{
    ui->setupUi(this);

    // Default setting file
    currentSetting = new QSettings("config.ini", QSettings::IniFormat);

    // Load Settings from ini file
    loadSettingFromIniFile();

    // Init Widget Font type and size
    initWidgetFont();

    // Init Widget Style
    initWidgetStyle();

    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(updateUI()));
    refreshTimer->start(refreshInMs);  //1s

    // Set Window Title
    this->setWindowTitle( tr("Tcp Client Widget") );
}

TcpClientWidget::~TcpClientWidget()
{
    delete ui;
    delete currentSetting;
    delete refreshTimer;
}

void TcpClientWidget::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    QWidget *pWidget = static_cast<QWidget*>(this->parent());

    if(pWidget != NULL)
    {
        this->resize(pWidget->size());
    }
}

void TcpClientWidget::bindModel(TCPClient *clientP)
{
    if(NULL != clientP)
    {
        unbind();

        tcpClient = clientP;

        connect(tcpClient, SIGNAL(newDataReady()), this, SLOT(updateIncomingData()));
        connect(tcpClient, SIGNAL(connectionOut()), this, SLOT(diconnectedStatus()));
        connect(tcpClient, SIGNAL(newDataTx(QHostAddress,uint16_t,QByteArray)), this, SLOT(updateTxDataToLog(QHostAddress,uint16_t,QByteArray)));

        connect(tcpClient, SIGNAL(serverChanged(QHostAddress,uint16_t)), this, SLOT(updateServerInfo(QHostAddress,uint16_t)));
        connect(tcpClient, SIGNAL(connectionChanged(bool)), this, SLOT(updateConnectionStatus(bool)));

        isRunning = tcpClient->getRunningStatus();
        updateConnectionStatus(isRunning);

        // If client is not running, start listen
        if(!isRunning)
        {
            // Enable Listen with delay
            QTimer::singleShot(refreshInMs, this, SLOT(on_pushButton_connect_clicked()));
        }
    }
}

void TcpClientWidget::unbind()
{
    if(NULL != tcpClient)
    {
        disconnect(tcpClient, 0 , this , 0);
    }

    tcpClient = NULL;
}

void TcpClientWidget::initWidgetFont()
{
}

void TcpClientWidget::initWidgetStyle()
{
    // Update Status Color
    ui->label_status->setText("");
    updateConnectionStatus(isRunning);

    // Get host IP address
    //ui->lineEdit_IP->setText(QNetworkInterface().allAddresses().at(1).toString());
    qDebug() << "All IP Address: " << QNetworkInterface().allAddresses();

    ui->checkBox_hex->setChecked(hexFormatFlag);
    ui->checkBox_autoClear->setChecked(autoClearRxFlag);

    ui->checkBox_showRx->setChecked(showRxPacketFlag);
    ui->checkBox_showTx->setChecked(showTxPacketFlag);
}

void TcpClientWidget::loadSettingFromIniFile()
{
    currentSetting->beginGroup("TCPClient");

    if(currentSetting->contains("ServerIP"))
    {
        // Load IP
        serverIP = currentSetting->value("ServerIP").toString();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("ServerIP", serverIP);
    }
    ui->lineEdit_IP->setText(serverIP);

    if(currentSetting->contains("ServerPort"))
    {
        // Load Server port
        serverPort = currentSetting->value("ServerPort").toInt();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("ServerPort", serverPort);
    }
    ui->lineEdit_listenPort->setText(QString::number(serverPort));

    currentSetting->endGroup();
}

void TcpClientWidget::on_pushButton_connect_clicked()
{
    QString logStr;

    // Toggle flag
    isRunning = !isRunning;

    if(true == isRunning)
    {
        // Connect to server
        isRunning = tcpClient->connectToServer(ui->lineEdit_IP->text(), ui->lineEdit_listenPort->text().toInt());
        logStr.append(tr("Connect to %1:").arg(ui->lineEdit_IP->text()));
        logStr.append(ui->lineEdit_listenPort->text());
        if(isRunning)
        {
            logStr.append(" succeed");
        }
        else
        {
            logStr.append(" failed");
        }

        // Update log
        updateLogData(logStr);
    }
    else
    {
        // Disconnect from server
        tcpClient->disconnectFromServer();
    }
}

bool TcpClientWidget::connectToServer(QString ip, uint16_t port)
{
    ui->lineEdit_IP->setText(ip);
    ui->lineEdit_listenPort->setText(QString::number(port));
    on_pushButton_connect_clicked();

    return isRunning;
}

void TcpClientWidget::on_pushButton_send_clicked()
{
    uint32_t txLen = 0;
    uint8_t tempBuf[10000] = {0};
    QByteArray tempTxBuf;
    QUtilityBox utilityBox;

    // If there is no server connected
    if(!isRunning)
    {
        return;
    }

    // Hex format
    if(hexFormatFlag)
    {
        txLen = utilityBox.convertHexStringToDataBuffer(tempBuf, ui->textEdit_txData->toPlainText());
        tempTxBuf = QByteArray((char *)tempBuf, txLen);
    }
    else
    {
        tempTxBuf = ui->textEdit_txData->toPlainText().toLatin1();
    }

    // Send msg
    sendData(tempTxBuf);
}

void TcpClientWidget::sendData(QByteArray &data)
{
    QString logStr;

    if(NULL == tcpClient)
    {
        return;
    }

    tcpClient->sendData(data);
}

void TcpClientWidget::updateLogData(QString logStr)
{
    QDateTime time = QDateTime::currentDateTime();
    QString timeStr = time.toString("[yyyy-MM-dd hh:mm:ss:zzz] ");

    // Add time stamp
    logStr.prepend(timeStr);

    if(autoClearRxFlag)
    {
        // Rx buffer > 200k bytes, clear and reset
        if(ui->textEdit_log->toPlainText().size() > 200000)
        {
            ui->textEdit_log->clear();
        }
    }

    //logFile->addLogToFile(logStr);
    ui->textEdit_log->insertPlainText(logStr.append("\n")); //Display the log in the textBrowse
    ui->textEdit_log->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
}

void TcpClientWidget::updateIncomingData()
{
    QString logStr;

    rxDataBuf.clear();
    if(tcpClient->getUndealData(rxDataBuf))
    {
        if(showRxPacketFlag)
        {
            logStr.append(tr("Rx data from %1:").arg(ui->lineEdit_IP->text()));
            logStr.append(ui->lineEdit_listenPort->text());
            // Update log
            updateLogData(logStr);

            logStr.clear();
            logStr.append(tr("Rx Data:"));
            for(int i = 0; i < rxDataBuf.size(); i++)
            {
                logStr.append(QString::number((uint8_t)rxDataBuf.at(i), 16).rightJustified(2, '0').toUpper());
                logStr.append(" ");
            }
            logStr.append("(");
            logStr.append(rxDataBuf);
            logStr.append(")");
            // Update log
            updateLogData(logStr);
        }

        // Emit signal, new comming data received
        emit newDataReady(rxDataBuf);
    }
}

void TcpClientWidget::diconnectedStatus()
{
    QString logStr;

    isRunning = false;

    logStr.append(tr("Disconnect from %1:").arg(ui->lineEdit_IP->text()));
    logStr.append(ui->lineEdit_listenPort->text());

    // Update log
    updateLogData(logStr);

    // Update UI status
    updateConnectionStatus(isRunning);
}

bool TcpClientWidget::getTcpConnectionStatus()
{
    return isRunning;
}

void TcpClientWidget::updateTxDataToLog(QHostAddress address, uint16_t port, QByteArray data)
{
    QString logStr;

    if(showTxPacketFlag)
    {
        logStr.append(tr("Send data to %1:%2")
                      .arg(address.toString())
                      .arg(port));
        // Update log
        updateLogData(logStr);

        logStr.clear();
        logStr.append(tr("Tx Data:"));
        for(int i = 0; i < data.size(); i++)
        {
            logStr.append(QString::number((uint8_t)data.at(i), 16).rightJustified(2, '0').toUpper());
            logStr.append(" ");
        }
        logStr.append("(");
        logStr.append(data);
        logStr.append(")");
        // Update log
        updateLogData(logStr);
    }
}

void TcpClientWidget::updateUI()
{
    static uint32_t lastRxPacketCnt = 0;
    static uint32_t lastTxPacketCnt = 0;

    if(NULL == tcpClient)
    {
        return;
    }

    ui->label_rxPacketCnt->setText(QString::number(tcpClient->getRxDiagramCnt()));
    ui->label_txPacketCnt->setText(QString::number(tcpClient->getTxDiagramCnt()));
    ui->label_rxPacketCntAvg->setText(QString::number(tcpClient->getRxDiagramCnt() - lastRxPacketCnt));
    ui->label_txPacketCntAvg->setText(QString::number(tcpClient->getTxDiagramCnt() - lastTxPacketCnt));
    ui->label_rxBytesCnt->setText(QString::number(tcpClient->getTotalRxBytes()));
    ui->label_txBytesCnt->setText(QString::number(tcpClient->getTotalTxBytes()));

    lastRxPacketCnt = tcpClient->getRxDiagramCnt();
    lastTxPacketCnt = tcpClient->getTxDiagramCnt();
}

void TcpClientWidget::on_pushButton_reset_clicked()
{
    if(NULL == tcpClient)
    {
        return;
    }

    tcpClient->resetTxRxCnt();
}

void TcpClientWidget::on_pushButton_clear_clicked()
{
    ui->textEdit_log->clear();
}

void TcpClientWidget::on_checkBox_hex_clicked(bool checked)
{
    hexFormatFlag = checked;
}

void TcpClientWidget::on_checkBox_autoClear_clicked(bool checked)
{
    autoClearRxFlag = checked;
}

void TcpClientWidget::updateServerInfo(QHostAddress address, uint16_t port)
{
    ui->lineEdit_IP->setText(address.toString());
    ui->lineEdit_listenPort->setText(QString::number(port));
}

void TcpClientWidget::updateConnectionStatus(bool connected)
{
    isRunning = connected;

    if(connected)
    {
        ui->pushButton_connect->setText(tr("Disconnect"));

        // Update Status Color
        ui->label_status->setStyleSheet(BG_COLOR_GREEN);
    }
    else
    {
        ui->pushButton_connect->setText(tr("Connect"));

        // Update Status Color
        ui->label_status->setStyleSheet(BG_COLOR_RED);
    }
}

void TcpClientWidget::updateSettingToFile()
{
    currentSetting->beginGroup("TCPClient");
    currentSetting->setValue("ServerIP", serverIP);
    currentSetting->setValue("ServerPort", serverPort);
    currentSetting->endGroup();
}

void TcpClientWidget::on_lineEdit_IP_editingFinished()
{
    serverIP = ui->lineEdit_IP->text();

    // Update setting to ini file
    updateSettingToFile();
}

void TcpClientWidget::on_lineEdit_listenPort_editingFinished()
{
    serverPort = ui->lineEdit_listenPort->text().toInt();

    // Update setting to ini file
    updateSettingToFile();
}

void TcpClientWidget::on_checkBox_showTx_clicked(bool checked)
{
    showTxPacketFlag = checked;
}

void TcpClientWidget::on_checkBox_showRx_clicked(bool checked)
{
    showRxPacketFlag = checked;
}

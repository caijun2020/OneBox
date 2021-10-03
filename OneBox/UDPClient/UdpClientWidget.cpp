/**********************************************************************
PACKAGE:        Communication
FILE:           UdpClientWidget.cpp
COPYRIGHT (C):  All rights reserved.

PURPOSE:        UDP Client Widget UI
**********************************************************************/

#include "UdpClientWidget.h"
#include "ui_UdpClientWidget.h"

#include <QNetworkInterface>
#include <QDateTime>
#include <QStringList>
#include <QDebug>
#include "QtBaseType.h"
#include "QUtilityBox.h"


UdpClientWidget::UdpClientWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UdpClientWidget),
    udpClient(NULL),
    isRunning(false),
    hexFormatFlag(false),
    autoClearRxFlag(true)
{
    ui->setupUi(this);

    // Init Widget Font type and size
    initWidgetFont();

    // Init Widget Style
    initWidgetStyle();

    connect(&refreshUITimer, SIGNAL(timeout()), this, SLOT(updateUI()));
    refreshUITimer.start(1000);  //1s

    // Set Window Title
    this->setWindowTitle( tr("Udp Client Widget") );
}

UdpClientWidget::~UdpClientWidget()
{
    delete ui;
}

void UdpClientWidget::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    QWidget *pWidget = static_cast<QWidget*>(this->parent());

    if(pWidget != NULL)
    {
        this->resize(pWidget->size());
    }
}

void UdpClientWidget::bindModel(UDPClient *clientP)
{
    if(NULL != clientP)
    {
        unbind();

        udpClient = clientP;

        connect(udpClient, SIGNAL(newDataReady()), this, SLOT(updateIncomingData()));
        connect(udpClient, SIGNAL(newDataTx(QHostAddress,uint16_t,QByteArray)), this, SLOT(updateTxDataToLog(QHostAddress,uint16_t,QByteArray)));
        connect(udpClient, SIGNAL(serverChanged(QHostAddress,uint16_t)), this, SLOT(updateServerInfo(QHostAddress,uint16_t)));
        connect(udpClient, SIGNAL(connectionChanged(bool)), this, SLOT(updateConnectionStatus(bool)));

        isRunning = udpClient->getRunningStatus();
        updateConnectionStatus(isRunning);
    }
}

void UdpClientWidget::unbind()
{
    if(NULL != udpClient)
    {
        disconnect(udpClient, 0 , this , 0);
    }

    udpClient = NULL;
}

void UdpClientWidget::initWidgetFont()
{
}

void UdpClientWidget::initWidgetStyle()
{
    ui->label_status->setText("");
    updateConnectionStatus(isRunning);

    // Get host IP address
    ui->lineEdit_IP->setText(QNetworkInterface().allAddresses().at(1).toString());
    qDebug() << "All IP Address: " << QNetworkInterface().allAddresses();

    ui->lineEdit_serverPort->setText(QString::number(8080));
    ui->lineEdit_localPort->setText(QString::number(54321));

    ui->checkBox_hex->setChecked(hexFormatFlag);
    ui->checkBox_autoClear->setChecked(autoClearRxFlag);
}

void UdpClientWidget::on_pushButton_send_clicked()
{
    uint32_t txLen = 0;
    uint8_t tempBuf[10000] = {0};
    QByteArray tempTxBuf;
    QUtilityBox utilityBox;

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
    if(ui->comboBox_clients->currentText().toLower().contains("unicast"))
    {
        sendData(QHostAddress(ui->lineEdit_IP->text()), ui->lineEdit_serverPort->text().toInt(), tempTxBuf);
    }
    else if(ui->comboBox_clients->currentText().toLower().contains("broadcast"))
    {
        sendData(QHostAddress::Broadcast, ui->lineEdit_serverPort->text().toInt(), tempTxBuf);
    }
}

void UdpClientWidget::on_pushButton_clear_clicked()
{
    ui->textEdit_log->clear();
}


void UdpClientWidget::updateLogData(QString logStr)
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


void UdpClientWidget::sendData(QHostAddress address, uint16_t port, QByteArray &data)
{
    QString logStr;

    if(NULL == udpClient)
    {
        return;
    }

    udpClient->sendData(address, port, data);
}

void UdpClientWidget::updateIncomingData()
{
    QString logStr;

    rxDataBuf.clear();
    if(udpClient->getUndealData(rxDataBuf))
    {
        logStr.append(tr("Rx data from %1:").arg(ui->lineEdit_IP->text()));
        logStr.append(ui->lineEdit_serverPort->text());
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

        // Emit signal, new comming data received
        emit newDataReady(rxDataBuf);
    }
}

void UdpClientWidget::on_pushButton_connect_clicked()
{
    if(NULL == udpClient)
    {
        return;
    }

    // Toggle flag
    isRunning = !isRunning;

    if(true == isRunning)
    {
        udpClient->initSocket(QHostAddress::Any, ui->lineEdit_localPort->text().toInt());
        udpClient->setServerAddressPort(QHostAddress(ui->lineEdit_IP->text()), ui->lineEdit_serverPort->text().toInt());
    }
    else
    {
        udpClient->closeSocket();
    }
}

void UdpClientWidget::updateTxDataToLog(QHostAddress address, uint16_t port, QByteArray data)
{
    QString logStr;

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

void UdpClientWidget::updateUI()
{
    static uint32_t lastRxPacketCnt = 0;
    static uint32_t lastTxPacketCnt = 0;

    if(NULL == udpClient)
    {
        return;
    }

    ui->label_rxPacketCnt->setText(QString::number(udpClient->getRxDiagramCnt()));
    ui->label_txPacketCnt->setText(QString::number(udpClient->getTxDiagramCnt()));
    ui->label_rxPacketCntAvg->setText(QString::number(udpClient->getRxDiagramCnt() - lastRxPacketCnt));
    ui->label_txPacketCntAvg->setText(QString::number(udpClient->getTxDiagramCnt() - lastTxPacketCnt));
    ui->label_rxBytesCnt->setText(QString::number(udpClient->getTotalRxBytes()));
    ui->label_txBytesCnt->setText(QString::number(udpClient->getTotalTxBytes()));

    lastRxPacketCnt = udpClient->getRxDiagramCnt();
    lastTxPacketCnt = udpClient->getTxDiagramCnt();
}

void UdpClientWidget::on_pushButton_reset_clicked()
{
    if(NULL == udpClient)
    {
        return;
    }

    udpClient->resetTxRxCnt();
}

void UdpClientWidget::on_checkBox_hex_clicked(bool checked)
{
    hexFormatFlag = checked;
}

void UdpClientWidget::on_checkBox_autoClear_clicked(bool checked)
{
    autoClearRxFlag = checked;
}

void UdpClientWidget::updateServerInfo(QHostAddress address, uint16_t port)
{
    ui->lineEdit_IP->setText(address.toString());
    ui->lineEdit_serverPort->setText(QString::number(port));
}

void UdpClientWidget::updateConnectionStatus(bool connected)
{
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



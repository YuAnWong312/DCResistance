#include "dcrwidget.h"
#include "ui_dcrwidget.h"
#include <QMessageBox>
#include  "dcrprocotol.h"
#include <QDateTime>

DCResistance::DCResistance(
    QWidget *parent ,
    QString portName,
    int wirelessChannel
        ) : QWidget(parent),
    ui (new Ui::DCResistance)
{
    m_COM = NULL;
    ui_statusBar_deviceStatus          = NULL;
    ui_statusBar_portName              = NULL;
    ui_statusBar_portStatus             = NULL;
    ui_statusBar_wirelessChannel     = NULL;

    initSerialPortSettings(portName );
    bool bResult = serialConnect();
    if(!bResult)
    {
        QMessageBox::critical(
                    this,
                    QString::fromUtf8("错误"),
                    QString::fromUtf8("端口") + m_portName + QString::fromUtf8("初始化错误，") + QString::fromUtf8("将弹出对话框显示发送报文。")
                    );
        m_state.reserved_enable = 1;
    }

    ui->setupUi(this);
    connect(m_COM , SIGNAL(readyRead()) , this , SLOT(findMessage()) );
    connect(ui->cobOutputCurrent, SIGNAL(currentIndexChanged(QString)) , this, SLOT(setCurrentShow()) );
    setCurrentShow();

    initStatusBar();

    //无线电台通道设置
    m_wirelessChannel.setChannel(wirelessChannel);
    QByteArray setChannel = m_wirelessChannel.TxDSwitchChannel();
    m_state.status = DCResistance::ST_RESERVED;
    qDebug() <<m_state.status << DCResistance::ST_RESERVED;
    write(setChannel);
    uiStateRefresh();
    refreshStatusBar();

}


DCResistance::~DCResistance()
{
    /*
    delete ui;
    delete m_clkRequest;

    if(m_COM)
    {
        if(m_COM->isOpen() )
        {
            m_COM->close();
        }
        delete m_COM;
    }
    */
    freePointer(m_clkRequest )
    closeAndFreeCOM();
    delete ui;
}

void DCResistance::initSerialPortSettings(QString portName )
{
    closeAndFreeCOM();
    m_portName = portName ;
    m_portSettings.BaudRate = BAUD9600;
    m_portSettings.DataBits = DATA_8;
    m_portSettings.Parity = PAR_NONE;
    m_portSettings.StopBits = STOP_1;
    m_portSettings.FlowControl = FLOW_OFF;
    m_portSettings.Timeout_Millisec = 100;
}


bool DCResistance::serialConnect()
{
    closeAndFreeCOM();
    m_COM = new QextSerialPort(
                m_portName,
                m_portSettings,
                QextSerialBase::EventDriven
                );
    if(!m_COM ){
        return false;
    }

    bool bResult = m_COM->open(QIODevice::ReadWrite);
    if(bResult ){
        qDebug()<< "Succeed in opening serial port: " << m_portName;
    }
    else{
        qDebug()<< "Failed to open serial port: " << m_portName;
    }

    return bResult;
}

bool DCResistance::write(QByteArray code, bool hide)
{
    QString hexCode = code.toHex();
    bool bSuccess = false;

    if(m_COM && m_COM->isOpen() )
    {
        m_COM->write(code);
        bSuccess = true;
    }

    if(bSuccess)
    {
        qDebug() << "TXD(" << m_portName << "): " << hexCode;
    }
    else {
        qDebug() << "TXD(" << m_portName << "): " << hexCode << " Failed! ";
    }

    if(!hide && !bSuccess)
    {
        QMessageBox::warning(NULL,QString::fromUtf8("模拟发送"), hexCode);
    }

    return bSuccess;
}

//ui刷新
void DCResistance::uiStateRefresh()
{

    if (m_state.reserved_enable)
    {
        ui->btnCurrentSetting->setEnabled(true);
        ui->btnDeviceReset->setEnabled(true);
        ui->btnExit->setEnabled(true);
        ui->btnHandshaking->setEnabled(true);
        ui->btnPrintReport->setEnabled(true);
        ui->btnTesting->setEnabled(true);
    }
    else switch(m_state.status)
    {
        case    ST_RESERVED:
        case    ST_RESET:
        ui->btnCurrentSetting->setEnabled(true);
        ui->btnDeviceReset->setEnabled(true);
        ui->btnTesting->setEnabled(true);
        ui->btnPrintReport->setEnabled(false);
        break;

    case    ST_CHARGING:
    case    ST_TESTING:
    case    ST_SMALLCURRENT:
    case    ST_HIGHCURRENT:
    case    ST_OVERHEATING:
        ui->btnCurrentSetting->setEnabled(false);
        ui->btnTesting->setEnabled(false);
        ui->btnDeviceReset->setEnabled(true);
        ui->btnPrintReport->setEnabled(false);
        break;
    case    ST_DISCHARGING:
        ui->btnCurrentSetting->setEnabled(false);
        ui->btnTesting->setEnabled(true);
        ui->btnDeviceReset->setEnabled(true);
        ui->btnPrintReport->setEnabled(false);
        break;
    case    ST_FINISHED:
        ui->btnCurrentSetting->setEnabled(false);
        ui->btnTesting->setEnabled(true);
        ui->btnDeviceReset->setEnabled(true);
        ui->btnPrintReport->setEnabled(true);
        break;
    default:
        qDebug() << "switch error in DCResistance::uiStateRefresh().";
    }
    refreshStatusBar();
}

//处理串口的接收缓冲区，处理报文
void DCResistance::findMessage()
{
    m_lockMessageBuff.lockForWrite();
    QByteArray appendBytes = m_COM->readAll();
    if(appendBytes.size() ==0)
    {
        m_lockMessageBuff.unlock();
        return ;
    }

    if(m_messageBuff.size() + appendBytes.size() >= DCR_LEN_MAXBUFF )
    {
        m_messageBuff.clear();
    }

    if (m_messageBuff.size() >= 3 && m_state.status != DCResistance::ST_RESERVED)
    {
        if (    m_messageBuff.at(0) != DCR_RET_BEGIN
            ||  m_messageBuff.at(1) != DCR_RET_ADDRH_DEFAULT
            ||  m_messageBuff.at(2) != DCR_RET_ADDRL_DEFAULT)
        {
            m_messageBuff.clear();
        }
    }
    m_messageBuff.append(appendBytes);
    m_lockMessageBuff.unlock();

    QByteArray message;
    m_lockMessageBuff.lockForRead();
    message = m_messageBuff;
    m_lockMessageBuff.unlock();

    //无线电台通道

    //qDebug() <<m_state.status;
    if (m_state.status == DCResistance::ST_RESERVED)
    {
        if (message.size() < m_wirelessChannel.RxDSwitchChannelConfirm().size() )
        {
            return ;
        }
        m_lockMessageBuff.lockForWrite();
        m_messageBuff.remove(
                    0,
                    m_wirelessChannel.RxDSwitchChannelConfirm().size()
                    );

        m_lockMessageBuff.unlock();
        if (message == m_wirelessChannel.RxDSwitchChannelConfirm() )
        {
            m_state.status = DCResistance::ST_RESET;
            m_clkRequest = new QTimer(this);
            connect(m_clkRequest, SIGNAL(timeout() ) , this , SLOT(ON_clkRequest_timeout() ) );
            m_clkRequest->start(DCR_CLK_REQUEST);
        }

        qDebug() <<"RxD(" <<m_portName << "): " <<message.toHex() << "Wireless Channel Changed";
        return ;
    }

    if (message.size() < DCR_LEN_MSGWITHOUTDATA)
    {
        return ;
    }
    int len;
    len = message.at(3) - '0';
    len = len * 10 + message.at(4) - '0' - 1;
    len = len + DCR_LEN_MSGWITHOUTDATA;
    if (len > message.size())
    {
        return ;
    }
    message.remove(len, DCR_LEN_MAXBUFF);

    m_lockMessageBuff.lockForWrite();
    m_messageBuff.remove(0, len);
    m_lockMessageBuff.unlock();

    DcrReceive::ERRTYPE err;
    DcrReceive::RETTYPE ret;
    DcrReceive msg(message);
    ret = msg.decode(err);
    qDebug() <<"RxD(" << m_portName <<"): " << message.toHex()
        << "error: " << err;

    switch (msg.status)
    {
    case DCR_RET_CMD_RESET:
        m_state.status = ST_RESET;
        break;
    case DCR_RET_CMD_TESTING:
        m_state.status = ST_TESTING;
        break;
    case DCR_RET_CMD_CHARGING:
        m_state.status = ST_CHARGING;
        break;
    case DCR_RET_CMD_DISCHARGING:
        m_state.status = ST_DISCHARGING;
        break;
    case DCR_RET_CMD_OVERHEATING:
        m_state.status = ST_OVERHEATING;
        break;
    case DCR_RET_CMD_FINISHED:
        m_state.status = ST_FINISHED;
        break;
     case DCR_RET_CMD_SMALLCURRENT:
        m_state.status = ST_SMALLCURRENT;
        break;
     case DCR_RET_CMD_HIGHCURRENT:
       m_state.status = ST_HIGHCURRENT;
       break;
    }
    uiStateRefresh();

    if (msg.lengthH != '0') //应改为msg.data.count != 0
    {
        DcrReceive::DATA_CURRENT_RVALUE data;
        if(msg.getData(data) )
        {
            ui->lneRValueShow->setText(data.R7);
        }
    }
}

void DCResistance::setCurrentShow()
{
    QString strC;
    strC = ui->cobOutputCurrent->currentText();
    ui->lneCurrentShow->setText(strC);
}

//联机
void DCResistance::on_btnHandshaking_clicked()
{
    DcrSend message(DcrSend::TYPE_DataRequest);
    QByteArray code = message.encode();
    write(code);
}

//电流设置
void DCResistance::on_btnCurrentSetting_clicked()
{
    DcrSend message(DcrSend::TYPE_CurrentSetting);
    message.data.clear();

    char    outputCurrent;
    if(ui->cobOutputCurrent->currentText() == "10A")
    {
        outputCurrent = '\x30';
    }
    else if (ui->cobOutputCurrent->currentText() =="5A")
    {
        outputCurrent = '\x31';
//        ui->lneCurrentShow->displayText() = "5A";
    }
    else if (ui->cobOutputCurrent->currentText() =="1A")
    {
        outputCurrent = '\x32';
    }
    else if( ui->cobOutputCurrent->currentText() == "100mA")
    {
        outputCurrent = '\x33';
    }
    else if(ui->cobOutputCurrent->currentText() =="5mA")
    {
        outputCurrent = '\x34';
    }

    message.data.append(outputCurrent);

    QByteArray code = message.encode();
    write(code);
}

//测试
void DCResistance::on_btnTesting_clicked()
{
    DcrSend message(DcrSend::TYPE_Test);
    QByteArray code = message.encode();
    write(code);
}

//复位
void DCResistance::on_btnDeviceReset_clicked()
{
    DcrSend message(DcrSend::TYPE_Reset);
    QByteArray code = message.encode();
    write(code);

    ui->lneRValueA->setText("");
    ui->lneRValueB->setText("");
    ui->lneRValueC->setText("");

}

//打印
void DCResistance::on_btnPrintReport_clicked()
{
    DcrSend message(DcrSend::TYPE_Print);
    QByteArray code = message.encode();
    write(code);
    /*
    QMessageBox::warning(
        this,
        QString::fromUtf8("打印报告"),
        QString::fromUtf8("打印报告：这个功能ToDo！")
    );
    */
    QTime  time = QTime::currentTime();
    QDate  date =  QDate::currentDate();
    QString TestTime = time.toString("hh:mm:ss");
    QString TestDate = date.toString("yyyy-MM-dd");

    ui->lneMeasuringDate->setText(TestDate);
    ui->lneMeasuringTime->setText(TestTime);

}

//返回
void DCResistance::on_btnExit_clicked()
{
    exit(0);
}

//! 定时器“查询设备状态”时间到
void DCResistance::ON_clkRequest_timeout()
{
    // TODO:
    DcrSend message(DcrSend::TYPE_DataRequest);
    QByteArray code = message.encode();
    write(code, true);
}

//! 检验整数有效性
bool DCResistance::dcrSetNum(int &iValue, int srcValue,
    int minValue, int maxValue)
{

    bool notInRange = !(minValue <= srcValue && srcValue < maxValue);
    if (notInRange)
    {
        iValue = minValue;
    }
    else
    {
        iValue = srcValue;
    }
    return notInRange;
}

//! 格式化输出
void DCResistance::dcrSetNum(QString &strValue, double srcValue,
    int fracBits, int maxBits)
{
    strValue.setNum(srcValue, 'f', fracBits);
    if (strValue.size() > maxBits)
    {
        strValue.resize(maxBits);
    }
    if (strValue.at(strValue.size() - 1) == '.')
    {
        strValue.resize(strValue.size() - 1);
    }
    while (strValue.size()<maxBits)
    {
        strValue = '\x20' + strValue;
    }
    Q_ASSERT(strValue.size() == maxBits);
}

//! 状态栏初始化
void DCResistance::initStatusBar()
{
    clearStatusBar();

    ui_statusBar_portName = new QLabel(ui->dcrStatusBar);
    ui->dcrStatusBar->addWidget(ui_statusBar_portName);

    ui_statusBar_portStatus = new QLabel(ui->dcrStatusBar);
    ui->dcrStatusBar->addWidget(ui_statusBar_portStatus);

    ui_statusBar_wirelessChannel = new QLabel(ui->dcrStatusBar);
    ui->dcrStatusBar->addWidget(ui_statusBar_wirelessChannel);

    ui_statusBar_deviceStatus = new QLabel(ui->dcrStatusBar);
    ui->dcrStatusBar->addPermanentWidget(ui_statusBar_deviceStatus);
}

//! 状态栏信息刷新
void DCResistance::refreshStatusBar()
{
    ui_statusBar_portName->setText(QString("Port: ") + m_portName);

    if (m_COM && m_COM->isOpen())
    {
        ui_statusBar_portStatus->setText(QString::fromUtf8("串口已打开"));
    }
    else
    {
        ui_statusBar_portStatus->setText(QString::fromUtf8("串口未打开"));
    }

    ui_statusBar_wirelessChannel->setText(QString("Channel: ")
        + QString::number(m_wirelessChannel.channel()));

    switch(m_state.status)
    {
    case DCResistance::ST_RESERVED:
        ui_statusBar_deviceStatus->setText(QString::fromUtf8("无线通道切换"));
        break;
    case DCResistance::ST_RESET:
        ui_statusBar_deviceStatus->setText(QString::fromUtf8("复位状态"));
        break;
    case DCResistance::ST_TESTING:
        ui_statusBar_deviceStatus->setText(QString::fromUtf8("正在测试状态"));
        break;
    case DCResistance::ST_CHARGING:
        ui_statusBar_deviceStatus->setText(QString::fromUtf8("正在充电状态"));
        break;
    case DCResistance::ST_DISCHARGING:
        ui_statusBar_deviceStatus->setText(QString::fromUtf8("正在放电状态"));
        break;
    case DCResistance::ST_OVERHEATING:
        ui_statusBar_deviceStatus->setText(QString::fromUtf8("过热保护状态"));
        break;
    case DCResistance::ST_SMALLCURRENT:
        ui_statusBar_deviceStatus->setText(QString::fromUtf8("请换小电流"));
        break;
    case DCResistance::ST_HIGHCURRENT:
        ui_statusBar_deviceStatus->setText(QString::fromUtf8("请换大电流"));
        break;
    case DCResistance::ST_FINISHED:
        ui_statusBar_deviceStatus->setText(QString::fromUtf8("测试完成循环测试状态"));
        break;
    }
}

//! 状态栏clear
void DCResistance::clearStatusBar()
{

    ui->dcrStatusBar->removeWidget(ui_statusBar_portName);
    ui->dcrStatusBar->removeWidget(ui_statusBar_portStatus);
    ui->dcrStatusBar->removeWidget(ui_statusBar_wirelessChannel);
    ui->dcrStatusBar->removeWidget(ui_statusBar_deviceStatus);
    freePointer(ui_statusBar_portName);
    freePointer(ui_statusBar_portStatus);
    freePointer(ui_statusBar_wirelessChannel);
    freePointer(ui_statusBar_deviceStatus);
}
void DCResistance::closeAndFreeCOM()
{
    if(m_COM)
    {
        if(m_COM->isOpen() )
        {
            m_COM->close();
        }
        delete m_COM;
    }
}

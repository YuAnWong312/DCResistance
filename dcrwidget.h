#ifndef DCRWIDGET_H
#define DCRWIDGET_H

#include <QWidget>
#include <QString>
#include <QReadWriteLock>
#include <QTimer>
#include "ui_dcrwidget.h"
#include "qextserialport.h"
#include "wirelessChannel.h"
#include <QDebug>
#define DCR_LEN_MAXBUFF         100
#define DCR_CLK_REQUEST          400    //主机每400毫秒访问一次从机或是发送命令

#define DCR_COM_PORTNAME        "COM3"
#define DCR_WIRELESS_CHANNEL        5

#ifndef freePointer
#define freePointer(x)    \
{                                   \
    if (x)                          \
    {                                \
        delete x;                   \
    }                                   \
}
#endif
namespace Ui {
    class DCResistance;
}
// DCResistance类是直阻测试程序主界面类
class DCResistance : public QWidget
{
    Q_OBJECT
    
    
private:
    Ui::DCResistance *ui;

public:
    //从机状态
    enum STATE_DEF
    {
        ST_RESET = 0,
        ST_TESTING =1,             //正在测试
        ST_CHARGING,
        ST_OVERHEATING,     /*过热保护 */
        ST_FINISHED,
        ST_DISCHARGING,
        ST_SMALLCURRENT,
        ST_HIGHCURRENT,
        ST_RESERVED             //内部保留，用于无线电台通道控制
    };

    //ui状态
    union SState{
        struct{
        uchar status:4;
        uchar reserved:3;
        uchar reserved_enable:1;
        };
        char value;
        SState()
        {
            value = 0;
#            ifdef   DCR_ENABLE_ALL
                    reserved_enable = 1;
#            endif
        }
    };


protected:
    SState  m_state;
    QextSerialPort  *m_COM;
    QString m_portName;
    PortSettings m_portSettings;
    QByteArray m_messageBuff;
    QReadWriteLock m_lockMessageBuff;
    QTimer * m_clkRequest;

    CWirelessChannel m_wirelessChannel;

    QLabel  *ui_statusBar_portName;         //ui状态栏—串口号
    QLabel  *ui_statusBar_portStatus;
    QLabel  *ui_statusBar_wirelessChannel;
    QLabel  *ui_statusBar_deviceStatus;

public:
    explicit    DCResistance(
            QWidget *parent = 0,
            QString portName = DCR_COM_PORTNAME,
            int wirelessChannel = DCR_WIRELESS_CHANNEL
            );
    ~DCResistance();

    void initSerialPortSettings(QString portName );
    bool serialConnect();
    bool write(QByteArray code, bool hide = false);

private:
    bool dcrSetNum(int &iValue, int srcValue, int minValue, int maxValue);
    void dcrSetNum(QString &strValue, double srcValue, int fracBits, int maxBits);

    void  initStatusBar();
    void  refreshStatusBar();
    void  clearStatusBar();

    void closeAndFreeCOM();

protected slots:
    void uiStateRefresh();
    void setCurrentShow();
    void findMessage();

    void on_btnHandshaking_clicked();
    void on_btnCurrentSetting_clicked();
    void on_btnDeviceReset_clicked();
    void on_btnPrintReport_clicked();
    void on_btnTesting_clicked();
    void on_btnExit_clicked();
    void ON_clkRequest_timeout();
private slots:
 //   void on_btnHandshaking_customContextMenuRequested(const QPoint &pos);
};

#endif // MAINWINDOW_H

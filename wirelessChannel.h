/*--------------------------------------------------------------------------
 *
 *  Identifier  :   [IS16xx-S00] Wireless Channel
 *  File        :   [IS16xx-S00-00] wirelessChannel.h
 *  Brief       :   无线电台报文
 *  Description :   CWirelessChannel 类用于产生与保定思瑞特电气有限公司无线电台
 *                  （简称无线电台）通信的部分报文————通道切换报文。
 *                  具体描述和使用方法见类前注释或代码文档。
 *  Version     :   1.0.0
 *  Update Date :   20160105 (wucz)
 *  ------------
 *  File List   <   [IS16xx-S00]
 *              :   [IS16xx-S00-00] wirelessChannel.h
 *              :   [IS16xx-S00-01] wirelessChannel.cpp
 *              :   [IS16xx-S00-02] wirelessChannel.pri
 *
 *------------------------------------------------------------------------*/

#ifndef __WIRELESSCHANNEL_H__
#define __WIRELESSCHANNEL_H__

#include <QByteArray>

/*  为缩短校验位生成时间，校验位的前8字节进行预处理，并将处理结果定义为常数  */
/*! 无线发送校验 - 前8字节校验  */
#define WIRELESS_SEND_CS                148
/*! 无线接收校验 - 前8字节校验  */
#define WIRELESS_RECEIVE_CS             20

/*! \class CWirelessChannel
 *  \brief CWirelessChannel 类用于产生与保定思瑞特电气有限公司（简称思瑞特）无线电台
 *  通信的部分报文————通道切换报文.
 *
 *  可以使用带参数的 CWirelessChannel() 构造函数设置通道号，也可使用 setChannel()
 *  函数设置通道号。\n
 *  通道号设置完成后，可以调用 TxDSwitchChannel() 函数获得通道切换发送报文，使用
 *  RxDSwitchChannelConfirm() 函数获得通道切换命令接收（确认）报文。\n
 *  \n
 *  报文格式依据《YL-500系列无线数传模块规格书》中AT指令格式编写，具体文档可以在网站
 *  http://www.rf-module.cn/ 下载。
 */
class CWirelessChannel
{
protected:
    int m_channel;      /*!<  无线电台通道号  */
public:
    CWirelessChannel(const CWirelessChannel &srcObject);
    explicit CWirelessChannel(int channel = 1);
    ~CWirelessChannel();

    int channel() const;
    void setChannel(int channel);
    static QByteArray TxDSwitchChannel(int channel);
    QByteArray TxDSwitchChannel();
    static QByteArray RxDSwitchChannelConfirm(int channel);
    QByteArray RxDSwitchChannelConfirm();
};

#endif  //  __WIRELESSCHANNEL_H__

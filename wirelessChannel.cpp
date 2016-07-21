/*--------------------------------------------------------------------------
 *
 *  Identifier  :   [IS16xx-S00] Wireless Channel
 *  File        :   [IS16xx-S00-01] wirelessChannel.cpp
 *  Brief       :   无线电台报文
 *  Description :   CWirelessChannel 类用于产生与保定思瑞特电气有限公司无线电台
 *                  （简称无线电台）通信的部分报文————通道切换报文。
 *                  具体描述和使用方法见类前注释或代码文档。
 *  Version     :   1.0.0
 *  Update Date :   20160105 (wucz)
 *
 *------------------------------------------------------------------------*/

#include "wirelessChannel.h"
#include <QDebug>

//! 拷贝构造函数
/*!
 *  \param [in] srcObject 源对象
 */
CWirelessChannel::CWirelessChannel(const CWirelessChannel &srcObject)
{
    setChannel(srcObject.channel());
}

//! 构造函数
/*!
 *  \param [in] channel [默认值重载] 通道号，默认为通道1
 *  \warning 通道号为1至32，请根据实际使用通道进行设置并确保通道号的正确性，
 *  通道号不在此范围内时，程序的后续行为是不可预见的。
 */
CWirelessChannel::CWirelessChannel(int channel/* = 1*/)
{
    setChannel(channel);
}

//! 析构函数
CWirelessChannel::~CWirelessChannel()
{

}

//! 通道号
/*!
 *  获取通道号。
 *  \return 通道号
 *  \warning 返回的是 CWirelessChannel 对象的通道号，而不是无线电台设备的通道状态。
 */
int CWirelessChannel::channel() const
{
    return m_channel;
}

//! 设置通道号
/*!
 *  \param [in] channel [默认值重载] 通道号，默认为通道1
 *  \warning 通道号为1至32，请根据实际使用通道进行设置并确保通道号的正确性，
 *  通道号不在此范围内时，程序的后续行为是不可预见的。
 */
void CWirelessChannel::setChannel(int channel/* = 1*/)
{
    m_channel = channel;
}

//! 编码发送报文 - 切换通道 \a channel
/*!
 *  \param [in] channel 通道号
 *  \sa TxDSwitchChannel()
 */
QByteArray CWirelessChannel::TxDSwitchChannel(int channel)
{
    //  采用append方式逐位构造报文的原因是(char *)串会在'\0'处截断。
    QByteArray ret;
    ret.clear();
    ret.append('\xAF');
    ret.append('\xAF');
    ret.append('\x00');
    ret.append('\x00');
    ret.append('\xAF');
    ret.append('\x80');
    ret.append('\x05');
    ret.append('\x02');
    ret.append((char)channel);
    ret.append('\x00');
    ret.append((char)(WIRELESS_SEND_CS + channel));
    ret.append('\x0D');
    ret.append('\x0A');
    return ret;
}

//! 编码发送报文 - 切换通道
/*!
 *  使用之前对象设置的通道号来编码发送“切换通道”报文。
 */
QByteArray CWirelessChannel::TxDSwitchChannel()
{
    return CWirelessChannel::TxDSwitchChannel(m_channel);
}

//! 编码接收报文 - 切换通道验证 \a channel
/*!
 *  使用本函数生成的报文与接收的无线电台返回的报文比较，若相同表示无线通道切换成功。
 *  \param [in] channel 通道号
 *  \sa RxDSwitchChannelConfirm()
 */
QByteArray CWirelessChannel::RxDSwitchChannelConfirm(int channel)
{
    //  采用append方式逐位构造报文的原因是(char *)串会在'\0'处截断。
    QByteArray ret;
    ret.clear();
    ret.append('\xAF');
    ret.append('\xAF');
    ret.append('\x00');
    ret.append('\x00');
    ret.append('\xAF');
    ret.append('\x00');
    ret.append('\x05');
    ret.append('\x02');
    ret.append((char)channel);
    ret.append('\x00');
    ret.append((char)(WIRELESS_RECEIVE_CS + channel));
    ret.append('\x0D');
    ret.append('\x0A');
    return ret;
}

//! 编码接收报文 - 切换通道验证 \a channel
/*!
 *  使用本函数生成的报文与接收的无线电台返回的报文比较，若相同表示无线通道切换成功。
 */
QByteArray CWirelessChannel::RxDSwitchChannelConfirm()
{
    return CWirelessChannel::RxDSwitchChannelConfirm(m_channel);
}

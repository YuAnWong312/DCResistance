#include "dcrprocotol.h"
#include <QDebug>

//!  构造函数
/*!
 *  初始化默认的报文头、报文尾、从机地址高、从机地址低。
 *   DcrSend(CMDTYPE type)
 */
DcrSend::DcrSend() :
    begin(DCR_MSG_BEGIN),
    addrH(DCR_MSG_ADDRH_DEFAULT),
    addrL(DCR_MSG_ADDRL_DEFAULT),
    end(DCR_MSG_END)
{
}

//!  构造函数
/*!
 *  根据报文类型初始化报文结构。
 *  @param[in] type 报文类型
 *  \sa CMDTYPE, TfSend()
 */
DcrSend::DcrSend(CMDTYPE type)
{
    new (this) DcrSend();
    switch (type)
    {
    /*
    case TYPE_AutoTestAllPhases:
        lengthH = '0';
        lengthL = '2';
        command = TF_MSG_CMD_TEST;
        data.clear();
        data.append(TF_MSG_SUBCMD_AUTOTESTALLPHASES);
        break;
    case TYPE_TestAllPhases:
        lengthH = '0';
        lengthL = '2';
        command = TF_MSG_CMD_TEST;
        data.clear();
        data.append(TF_MSG_SUBCMD_TESTALLPHASES);
        break;
    case TYPE_TestOnePhase:
        lengthH = '0';
        lengthL = '2';
        command = TF_MSG_CMD_TEST;
        data.clear();
        data.append(TF_MSG_SUBCMD_TESTONEPHASE);
        break;
    case TYPE_TestZ:
        lengthH = '0';
        lengthL = '2';
        command = TF_MSG_CMD_TEST;
        data.clear();
        data.append(TF_MSG_SUBCMD_TESTZ);
        break;
        */
    case TYPE_CurrentSetting:
        lengthH = '0';
        lengthL = '2';
        command = DCR_MSG_CMD_PARAM;
        break;
    case TYPE_Reset:
        lengthH = '0';
        lengthL = '1';
        command = DCR_MSG_CMD_RESET;
//        data.clear();
//        data.append(DCR_MSG_SUBCMD_RESET);
        break;
    case TYPE_Test:
        lengthH = '0';
        lengthL = '1';
        command = DCR_MSG_CMD_TEST;
//        data.clear();
//        data.append(DCR_MSG_SUBCMD_TEST);
        break;
    case TYPE_Print:
        lengthH = '0';
        lengthL = '1';
        command = DCR_MSG_CMD_PRINT;
//        data.clear();
//        data.append(DCR_MSG_SUBCMD_PRINT);
        break;
    case TYPE_DataRequest:
        lengthH = '0';
        lengthL = '1';
        command = DCR_MSG_CMD_STATECHECK;
//        data.clear();
        //data.append(DCR_MSG_SUBCMD_STATECHECK);
        break;
    }
}

//!  报文编码
/*!
 *  \return 报文，以 QByteArray 类型存储。
 */
QByteArray DcrSend::encode()
{
    QByteArray ret;
    char check;
    ret.clear();
    ret.append(begin);          //  报文头
    ret.append(addrH);          //  从机地址高
    ret.append(addrL);          //  从机地址低
    ret.append(lengthH);        //  数据和命令长度高
    ret.append(lengthL);        //  数据和命令长度低
    ret.append(command);        //  主机命令
    ret.append(data);           //  数据
    check = 0;
    foreach (char ch, ret)
    {
        check = check ^ ch;
    }
    ret.append(check);          //  异或校验
    ret.append(end);            //  报文尾

    return ret;
}

//!  构造函数
DcrReceive::DcrReceive() :
    code(m_code),
    begin(m_begin),
    addrH(m_addrH),
    addrL(m_addrL),
    lengthH(m_lengthH),
    lengthL(m_lengthL),
    status(m_status),
    data(m_data),
    checkByte(m_checkByte),
    end(m_end)
{
}

//!  构造函数
/*!
 *   @param[in] lpszCode 接收报文
 */
DcrReceive::DcrReceive(const char *lpszCode) :
    code(m_code),
    begin(m_begin),
    addrH(m_addrH),
    addrL(m_addrL),
    lengthH(m_lengthH),
    lengthL(m_lengthL),
    status(m_status),
    data(m_data),
    checkByte(m_checkByte),
    end(m_end)
{
    setCode(lpszCode);
}

//!  构造函数
/*!
 *   @param[in] code 接收报文
 */
DcrReceive::DcrReceive(const QByteArray &code):
    code(m_code),
    begin(m_begin),
    addrH(m_addrH),
    addrL(m_addrL),
    lengthH(m_lengthH),
    lengthL(m_lengthL),
    status(m_status),
    data(m_data),
    checkByte(m_checkByte),
    end(m_end)
{
    setCode(code);
}

//!  获取报文类型
/*!
 *   \return 报文类型
 *   \sa TfReceive::RETTYPE
 */
DcrReceive::RETTYPE DcrReceive::getLastReturnType()
{
    return m_retType;
}

//!  获取报文译码状态
/*!
 *  \return 报文译码状态
 *  \sa TfReceive::ERRTYPE
 */
DcrReceive::ERRTYPE DcrReceive::getLastErrorType()
{
    return m_errType;
}

//!  报文译码
/*!
 *  \return 报文类型
 *  \sa TfReceive::ERRTYPE
 */
DcrReceive::RETTYPE DcrReceive::decode()
{
    ERRTYPE err;
    return decode(err);
}

//!  报文译码
/*!
 *  @param[out] errType 报文译码状态
 *  \return 报文类型
 *  \sa TfReceive::ERRTYPE
 */
DcrReceive::RETTYPE DcrReceive::decode(ERRTYPE &errType)
{
    m_retType = TYPE_Error;

    //  检验报文长度
    int len = m_code.size();
    if (len < DCR_LEN_MSGWITHOUTDATA)
    {
        m_errType = ERR_SizeNotMatch;
        errType = m_errType;
        return m_retType;
    }

    //  解码 - 报文结构
    m_begin     =   m_code.at(0);
    m_addrH     =   m_code.at(1);
    m_addrL     =   m_code.at(2);
    m_lengthH   =   m_code.at(3);
    m_lengthL   =   m_code.at(4);
    m_status    =   m_code.at(5);
    m_checkByte =   m_code.at(len - 2);
    m_end       =   m_code.at(len - 1);
    int datalen;
    datalen = m_lengthH - '0';
    datalen = datalen * 10 + m_lengthL - '0' - 1;
    if (len != datalen + DCR_LEN_MSGWITHOUTDATA)
    {
        m_errType = ERR_SizeNotMatch;
        errType = m_errType;
        return m_retType;
    }
    m_data.clear();
    for (int i = 6; i < len-2; i++)
    {
        m_data.append(m_code.at(i));
    }

    //  校验 - 确认报文头
    if (m_begin != DCR_RET_BEGIN)
    {
        m_errType = ERR_Begin;
        errType = m_errType;
        return m_retType;
    }

    //  校验 - 确认报文尾
    if (m_end != DCR_RET_END)
    {
        m_errType = ERR_End;
        errType = m_errType;
        return m_retType;
    }

    //  校验 - 校验位
    char xcheck = 0;
    foreach (char ch, m_code)
    {
        xcheck = xcheck ^ ch;
    }
    if (xcheck != m_end)
    {
        qDebug() << "CheckByte: " << (xcheck ^ m_checkByte ^ m_end);
        m_errType = ERR_CheckByte;
        errType = m_errType;
        return m_retType;
    }

    //  解码 - 从机状态
    switch (m_status)
    {
    case DCR_RET_CMD_RESET:
        m_retType = TYPE_Reset;
        m_errType = ERR_Success;
        break;
    case DCR_RET_CMD_CHARGING:
        m_retType = TYPE_Charging;
        m_errType = ERR_Success;
        break;
    case DCR_RET_CMD_TESTING:
        m_retType = TYPE_Testing;
        m_errType = ERR_Success;
        break;
    case DCR_RET_CMD_OVERHEATING:
        m_retType = TYPE_OverHeating;
        m_errType = ERR_Success;
        break;
    case DCR_RET_CMD_SMALLCURRENT:
        m_retType = TYPE_SmallCurrent;
        m_errType = ERR_Success;
        break;
    case DCR_RET_CMD_HIGHCURRENT:
        m_retType = TYPE_HighCurrent;
        m_errType = ERR_Success;
        break;
    case DCR_RET_CMD_DISCHARGING:
        m_retType = TYPE_DisCharging;
        m_errType = ERR_Success;
        break;
    case DCR_RET_CMD_FINISHED:
        m_retType = TYPE_Finished;
        m_errType = ERR_Success;
        break;
    default:
        m_retType = TYPE_Error;
        m_errType = ERR_Status;
        errType = m_errType;
        return m_retType;
    }

    errType = m_errType;
    return m_retType;
}

//!  设置报文
/*!
 *  @param[in] lpszCode 报文
 */
void DcrReceive::setCode(const char *lpszCode)
{
    m_code = lpszCode;
}

//!  设置报文
/*!
 *  @param[in] code 报文
 */
void DcrReceive::setCode(const QByteArray &code)
{
    m_code = code;
}

//!  获取单相数据
/*
   @param[out] data 单相数据
  \return 数据解析结果。（成功(true)/失败(false)）

bool TfReceive::getData(TfReceive::DATA_ONEPHASE &data)
{
    if (m_data.size() != TF_LEN_DATAONEPHASE)
    {
        return false;
    }
    if (m_data.at(0) != 'd')
    {
        return false;
    }
*/
    /*-------------------------------*
     * 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 *
     * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 *
     *-------------------------------*
     * | K1          err1      | fjd *
     * 'd'                     jx    *
     *-------------------------------*/
/*
    data.K1 = m_data;
    data.K1.remove(7, TF_LEN_DATAONEPHASE);
    data.K1.remove(0, 1);
    data.err1 = m_data;
    data.err1.remove(12, TF_LEN_DATAONEPHASE);
    data.err1.remove(0, 7);
    data.jx = m_data;
    data.jx.remove(13, TF_LEN_DATAONEPHASE);
    data.jx.remove(0, 12);
    data.fjd = m_data;
    data.fjd.remove(0, 13);

    return true;
}

*/
bool DcrReceive::getData(DcrReceive::DATA_CURRENT_RVALUE &data)
{
    if (m_data.size() != DCR_LEN_DATACURRENT_RVALUE)
    {
        return false;
    }
    if(m_status != 'H')
    {
        return false;
    }
    data.C1 = m_data.mid(0, 1);
    data.R7 = m_data.mid(1 ,7);
    return true;
}



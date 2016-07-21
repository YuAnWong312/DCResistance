#ifndef __DCRPROCOTOL_H__
#define __DCRPROCOTOL_H__

#include <qbytearray>

#ifndef __DCRSEND_H__
#define __DCRSEND_H__

#define DCR_MSG_BEGIN            '\x7E'      //  报文头 定义
#define DCR_MSG_END              '\x0D'      //  报文尾 定义
#define DCR_MSG_ADDRH_DEFAULT    '\x36'    //  从机地址高 默认
#define DCR_MSG_ADDRL_DEFAULT    '\x36'    //  从机地址低 默认
#define DCR_MSG_CMD_TEST         'A'         //  主机命令-测试/复测A(41H)
#define DCR_MSG_CMD_PARAM        'B'         //  主机命令-参数设置B(42H)
#define DCR_MSG_CMD_RESET        'C'         //  主机命令-复位命令C(43H)
#define DCR_MSG_CMD_PRINT         'D'         //  主机命令-打印命令D(44H)
// #define DCR_MSG_CMD_PRINT        'E'         //  主机命令-打印命令E(45H)
#define DCR_MSG_CMD_STATECHECK   'F'         //  主机命令-主机请求数据报文F(46H)
/*
#define DCR_MSG_SUBCMD_TEST     '\x3E'          //数据首字节--测试命令
#define DCR_MSG_SUBCMD_RESET    '\x3C'          //数据首字节--复位
#define DCR_MSG_SUBCMD_PRINT    '\x3B'          //数据首字节--打印
#define DCR_MSG_SUBCMD_STATECHECK   '\x39' //数据首字节--请求数据
*/
/*
#define TF_MSG_SUBCMD_AUTOTESTALLPHASES '\x31'  //  数据首字节-自动测试
#define TF_MSG_SUBCMD_TESTALLPHASES     '\x32'  //  数据首字节-三相测试
#define TF_MSG_SUBCMD_TESTONEPHASE      '\x33'  //  数据首字节-单相测试
#define TF_MSG_SUBCMD_TESTZ             '\x34'  //  数据首字节-自动测试（Z型变）
*/
//! DcrSend 类用于生成测试仪的发送报文. DcrSend类
/*!
 *  DcrSend 类根据参数构造对应的功能信息，使用 encode() 函数生成报文。\n
 *  在生成报文前，可以修改对应公共成员变量的值来修改功能。
 */
class DcrSend
{
public:
    char begin;         /*!<  报文头  */
    char addrH;         /*!<  从机地址高  */
    char addrL;         /*!<  从机地址低  */
    char lengthH;       /*!<  数据和命令长度高  */
    char lengthL;       /*!<  数据和命令长度低  */
    char command;       /*!<  主机命令  */
    QByteArray data;    /*!<  数据  */
    char checkByte;     /*!<  异或校验  */
    char end;           /*!<  报文尾  */

public:
    enum CMDTYPE        /*!  报文类型 - 构造函数依此编码报文  */
    {


        TYPE_CurrentSetting       =   20,     /*!<  参数设置  */
        TYPE_Reset              =   30,     /*!<  复位  */
        TYPE_Test               =   40,     /*!<  存储  */
        TYPE_Print              =   50,     /*!<  打印  */
        TYPE_DataRequest        =   60      /*!<  主机请求数据  */
    };

public:
    explicit DcrSend();
    explicit DcrSend(CMDTYPE type);
    QByteArray encode();
public:

};

#endif  //  __DCRSEND_H__

#ifndef __DCRRECEIVE_H__
#define __DCRRECEIVE_H__

#define DCR_RET_BEGIN            DCR_MSG_BEGIN            //  报文头 定义
#define DCR_RET_END              DCR_MSG_END              //  报文尾 定义
#define DCR_RET_ADDRH_DEFAULT    DCR_MSG_ADDRH_DEFAULT    //  从机地址高 默认
#define DCR_RET_ADDRL_DEFAULT    DCR_MSG_ADDRL_DEFAULT    //  从机地址低 默认
#define DCR_RET_CMD_RESET        'A'     //  从机状态 - 复位状态
#define DCR_RET_CMD_CHARGING      'B'     //  从机状态 - 正在充电状态
#define DCR_RET_CMD_OVERHEATING   'C'     //  从机状态 - 过热保护状态
#define DCR_RET_CMD_SMALLCURRENT   'D'     //  从机状态 - 请换小电流
#define DCR_RET_CMD_HIGHCURRENT     'E'     //  从机状态 - 请换大电流
#define DCR_RET_CMD_DISCHARGING     'F'     //从机状态 - 正在放电状态
#define DCR_RET_CMD_TESTING         'G'     //从机状态 - 正在测试
#define DCR_RET_CMD_FINISHED        'H'     //从机状态 - 测试完成循环测试状态

#define DCR_LEN_MSGWITHOUTDATA  8
#define DCR_RET_SUBCMD_CURRENT      'c'      //数据头- 电流信息
#define DCR_RET_SUBCMD_RVALUE       'r'         //数据头- 阻值数据
#define DCR_LEN_DATACURRENT_RVALUE     8    //测试完成状态的数据字节
//#define DCR_LEN_DATARVALUE      7
/*
#define TF_RET_SUBCMD_ONEPHASE  'd'     //  数据头 - 单相数据
#define TF_RET_SUBCMD_ALLPHASES 's'     //  数据头 - 三相数据
#define TF_LEN_MSGWITHOUTDATA   8       \
    //  无数据报文的长度
    //  8 = 1(begin) + 2(addr) + 2(length) + 1(status) + 1(check) + 1(end)
#define TF_LEN_DATAONEPHASE     15      \
    //  单相数据报文数据的长度
    //  1('d') + 6(K) + 5(err) + 1(jx) + 2(fjd)
#define TF_LEN_DATAALLPHASES    40      \
    //  三相数据报文数据的长豆
    //  1('s') + [6(K) + 5(err)]*3 + 1(H) + 1(L) + 2(zbx) + 2(fjd)
*/
//! Receive 类用于分析从仪接收的报文. DcrReceive
/*!
 *  使用重载的 TfReceive(const QByteArray &data) 等构造函数设置接收报文，
 *  或者使用 setCode 函数设置接收报文。\n
 *  使用 decode() 及相关函数报文译码。
 *  如果报文包含额外的数据信息，
 *  使用 getData(TfReceive::DATA_ONEPHASE &data) 函数获取单相信息，
 *  getData(TfReceive::DATA_ALLPHASES &data) 函数获取三相信息。\n
 *  \warning getData 函数必须使用在一条 decode 函数之后。
 *
 *  调用 decode 函数译码后可以使用公有的成员变量访问译码结果。
 */
class DcrReceive
{
public:
    /*!  报文类型  */
    enum RETTYPE    /*!  报文类型  */
    {
        TYPE_Error                  =   0,  /*!<  报文解码失败，包括校验错误  */
        TYPE_Reset                  =   10, /*!<  复位状态  */
        TYPE_Charging                =   20, /*!<  正在充电状态  */
        TYPE_OverHeating            =   30, /*!<  过保护状态  热*/
        TYPE_SmallCurrent  =   40, /*!<  小电流  */
        TYPE_HighCurrent           =   50, /*!<  大电流  */
        TYPE_DisCharging         =   60, /*!<  正在放电  */
        TYPE_Testing        =   70 , /*!<  正在测试  */
        TYPE_Finished      =   80
    };

public:
    /*!  报文译码结果  */
    enum ERRTYPE
    {
        ERR_Success         =   0,  /*!<  报文解析成功  */
        ERR_SizeNotMatch  = 1 ,       /*!<  报文长度不匹配  */
        ERR_Begin           ,       /*!<  错误的报文头  */
        ERR_End             ,       /*!<  错误的报文尾  */
        ERR_CheckByte  = 2     ,       /*!<  校验错误  */
        ERR_Status                 /*!<  从机状态无法识别  */
    };

public:
    struct  DATA_CURRENT_RVALUE
    {
        QByteArray C1;      //测得的当前电流信息
        QByteArray R7;
    };
/*
public:
    struct  DATA_RVALUE
    {
        QByteArray R7;
    };
*/
protected:
    QByteArray m_code;      /*!<  原始报文（内部）  */
public:
    const QByteArray &code;  /*!<  原始报文  */

protected:
    char m_begin;               /*!<  报文头（内部）  */
    char m_addrH;               /*!<  从机地址高（内部）  */
    char m_addrL;               /*!<  从机地址低（内部）  */
    char m_lengthH;             /*!<  数据和命令长度高（内部）  */
    char m_lengthL;             /*!<  数据和命令长度低（内部）  */
    char m_status;              /*!<  从机状态（内部）  */
    QByteArray m_data;          /*!<  数据（内部）  */
    char m_checkByte;           /*!<  异或校验（内部）  */
    char m_end;                 /*!<  报文尾（内部）  */

public:
    const char &begin;          /*!<  报文头  */
    const char &addrH;          /*!<  从机地址高  */
    const char &addrL;          /*!<  从机地址低  */
    const char &lengthH;        /*!<  数据和命令长度高  */
    const char &lengthL;        /*!<  数据和命令长度低  */
    const char &status;         /*!<  从机状态  */
    const QByteArray &data;     /*!<  数据  */
    const char &checkByte;      /*!<  异或校验  */
    const char &end;            /*!<  报文尾  */

public:
    explicit DcrReceive();
    explicit DcrReceive(const char *lpszData);
    explicit DcrReceive(const QByteArray &data);

    //  报文解码 - 解析报文结构
protected:
    DcrReceive::RETTYPE m_retType;   /*!<  报文类型  */
    DcrReceive::ERRTYPE m_errType;   /*!<  报文译码结果  */
public:
    DcrReceive::RETTYPE getLastReturnType(); //  = m_retType
    DcrReceive::ERRTYPE getLastErrorType();  //  = m_errType
    DcrReceive::RETTYPE decode();
    DcrReceive::RETTYPE decode(ERRTYPE &errType);
    void setCode(const char *lpszCode);
    void setCode(const QByteArray &code);
    bool    getData(DcrReceive::DATA_CURRENT_RVALUE &data);
//    bool    getData(DcrReceive::DATA_RVALUE &data);
};

#endif  //  __DCRRECEIVE_H__

#endif  //  __DCRPROCOTOL_H__

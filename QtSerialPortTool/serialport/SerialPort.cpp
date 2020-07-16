
#include "serialport.h"
#if WIN32
#include <stdio.h>
#include <iostream>
#include <WinSock2.h>
#include <QDebug>
SerialPort::SerialPort():
    hcom(NULL),
    baud_rate_(Baud115200),
    data_bits_(Data8),
    parity_(NoParity),
    stop_bits_(OneStop),
    is_overlapped_(true),
    is_open_(false)

{

}

SerialPort::~SerialPort()
{
    is_open_=false;   

}

void SerialPort::setBaudRate(SerialPort::BaudRate baud_rate)
{
    baud_rate_=baud_rate;
}

void SerialPort::setDataBits(SerialPort::DataBits data_bits)
{
    data_bits_=data_bits;
}

void SerialPort::setParity(SerialPort::Parity parity)
{
    parity_=parity;
}

void SerialPort::setStopBits(SerialPort::StopBits stop_bits)
{
    stop_bits_=stop_bits;
}

void SerialPort::setOverlapped(bool is_overlapped)
{
    is_overlapped_=is_overlapped;
}

//在Windows上，要打开COM10以后的串口（包括COM10），串口名称不再是“COM10”，
//而是“\\.\COM10”，因此在open函数中，如果是COM10及以后的串口，
//串口名（假设当前要打开COM10）应该写“\\\\.\\COM10”，其中“\\\\.\\”为“\\.\”的转义。
bool SerialPort::openPort(std::string port_name)
{

    is_open_=false;
    hcom=NULL;
    std::cout<<"SerialPort open"<<std::endl;

    //解决10以后串口的问题

    std::string s_port_num=port_name.substr(3);
    int port_num=stoi(s_port_num,0,10);
    if(port_num>=10)
    {
        port_name="\\\\.\\"+port_name;
    }

    if (is_overlapped_)
	{
        //异步方式
        hcom = CreateFileA(port_name.c_str(), //串口名
                                    GENERIC_READ | GENERIC_WRITE, //支持读写
                                    0, //独占方式，串口不支持共享
                                    NULL,//安全属性指针，默认值为NULL
                                    OPEN_EXISTING, //打开现有的串口文件
                                    FILE_FLAG_OVERLAPPED, //0：同步方式，FILE_FLAG_OVERLAPPED：异步方式
                                    NULL);//用于复制文件句柄，默认值为NULL，对串口而言该参数必须置为NULL

	}
	else
	{
        //同步方式
        hcom = CreateFileA(port_name.c_str(), //串口名
                                    GENERIC_READ | GENERIC_WRITE, //支持读写
                                    0, //独占方式，串口不支持共享
                                    NULL,//安全属性指针，默认值为NULL
                                    OPEN_EXISTING, //打开现有的串口文件
                                    0, //0：同步方式，FILE_FLAG_OVERLAPPED：异步方式
                                    NULL);//用于复制文件句柄，默认值为NULL，对串口而言该参数必须置为NULL

	}
	
    if(hcom == INVALID_HANDLE_VALUE)
    {
        std::cout<<"CreateFile error"<<std::endl;
        closePort();
		return false;
	}

	//配置缓冲区大小 
    if(!SetupComm(hcom,1024, 1024))
	{
        std::cout<<"SetupComm error "<<std::endl;
        closePort();
		return false;
	}


    // 配置参数 winbase.h #define ONESTOPBIT 0
    DCB dcb;
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);

    if(!GetCommState(hcom,&dcb))
    {
        std::cout<<"GetCommState error "<<std::endl;
        closePort();
        return false;

    }

    dcb.BaudRate =baud_rate_; // 波特率
    dcb.ByteSize = data_bits_; // 数据位
    dcb.Parity=parity_;
    dcb.StopBits=stop_bits_;
    // 串口流控制
    //dcb.fRtsControl = RTS_CONTROL_ENABLE;
    //dcb.fDtrControl = DTR_CONTROL_ENABLE;

   // std::cout<<"baud_rate_"<<baud_rate_<<std::endl;

    //std::cout<<"StopBits"<<stop_bits_<<std::endl;

    if(!SetCommState(hcom,&dcb))
	{
		// 设置参数失败
        int ret=GetLastError();
        std::cout<<"SetCommState error "<<ret<<std::endl;
        closePort();
		return false;
	}

	//超时处理,单位：毫秒
	//总超时＝时间系数×读或写的字符数＋时间常量
	COMMTIMEOUTS TimeOuts;
    TimeOuts.ReadIntervalTimeout = 10; //读间隔超时
    TimeOuts.ReadTotalTimeoutMultiplier = 0; //读时间系数
    TimeOuts.ReadTotalTimeoutConstant = 0; //读时间常量
    TimeOuts.WriteTotalTimeoutMultiplier = 10; // 写时间系数
    TimeOuts.WriteTotalTimeoutConstant = 50; //写时间常量


    if(!SetCommTimeouts(hcom,&TimeOuts))
    {
        std::cout<<"SetCommTimeouts error "<<std::endl;
        closePort();
        return false;
    }

    PurgeComm(hcom,PURGE_TXCLEAR|PURGE_RXCLEAR| PURGE_RXABORT | PURGE_TXABORT);//清空串口缓冲区
    is_open_=true;
    return true;
}

bool SerialPort::isOpen()
{
    return is_open_;
}

void SerialPort::closePort()
{
    //std::cout<<"CloseHandle hCom "<<hCom<<std::endl;
    if(hcom==INVALID_HANDLE_VALUE)
    {
        hcom=NULL;
        std::cout<<"CloseHandle INVALID_HANDLE_VALUE"<<std::endl;
        return;
    }
    if(hcom==0)
    {
        return;
    }


    if(CloseHandle(hcom))
    {
       std::cout<<"CloseHandle ok "<<std::endl;
    }
    else
    {
       std::cout<<"CloseHandle error "<<std::endl;
    }
}

int SerialPort::send(const uint8_t *data,int len)
{


    if (is_overlapped_)
	{

        //异步方式
        unsigned long dwBytesWrite = len; //成功写入的数据字节数
        unsigned long dwErrorFlags; //错误标志
        COMSTAT comStat; //通讯状态
        OVERLAPPED m_osWrite; //异步输入输出结构体

        //创建一个用于OVERLAPPED的事件处理，不会真正用到，但系统要求这么做
        memset(&m_osWrite, 0, sizeof(m_osWrite));
        m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, L"WriteEvent");

        ClearCommError(hcom, &dwErrorFlags, &comStat); //清除通讯错误，获得设备当前状态
        int bWriteStat = WriteFile(hcom, //串口句柄
            data, //数据首地址
            dwBytesWrite, //要发送的数据字节数
            &dwBytesWrite, //DWORD*，用来接收返回成功发送的数据字节数
            &m_osWrite); //NULL为同步发送，OVERLAPPED*为异步发送
        if (!bWriteStat)
        {
            if (GetLastError() == ERROR_IO_PENDING) //如果串口正在写入
            {
                WaitForSingleObject(m_osWrite.hEvent, 1000); //等待写入事件1秒钟
            }
            else
            {
                ClearCommError(hcom, &dwErrorFlags, &comStat); //清除通讯错误
                CloseHandle(m_osWrite.hEvent); //关闭并释放hEvent内存
                return -1;
            }
        }
        return dwBytesWrite;


	}
	else
	{
        // 同步方式
        unsigned long dwBytesWrite = len; //成功写入的数据字节数
        int bWriteStat = WriteFile(hcom, //串口句柄
                                    data, //数据首地址
                                    dwBytesWrite, //要发送的数据字节数
                                    &dwBytesWrite, //DWORD*，用来接收返回成功发送的数据字节数
                                    NULL); //NULL为同步发送，OVERLAPPED*为异步发送
        if (!bWriteStat)
        {
            return -2;
        }
        return dwBytesWrite;
	}
}

int SerialPort::receive(uint8_t *data, int max_len)
{

    unsigned long wCount = max_len;//成功读取的数据字节数
    if (is_overlapped_)
	{
        //异步方式
        unsigned long dwErrorFlags; //错误标志
        COMSTAT comStat; //通讯状态
        OVERLAPPED m_osRead; //异步输入输出结构体

        //创建一个用于OVERLAPPED的事件处理，不会真正用到，但系统要求这么做
        memset(&m_osRead, 0, sizeof(m_osRead));
        m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, L"ReadEvent");

        ClearCommError(hcom, &dwErrorFlags, &comStat); //清除通讯错误，获得设备当前状态
        if (!comStat.cbInQue)return -1; //如果输入缓冲区字节数为0，则返回false

        BOOL bReadStat = ReadFile(hcom, //串口句柄
            data, //数据首地址
            wCount, //要读取的数据最大字节数
            &wCount, //DWORD*,用来接收返回成功读取的数据字节数
            &m_osRead); //NULL为同步发送，OVERLAPPED*为异步发送
        if (!bReadStat)
        {
            if (GetLastError() == ERROR_IO_PENDING) //如果串口正在读取中
            {
                //GetOverlappedResult函数的最后一个参数设为TRUE
                //函数会一直等待，直到读操作完成或由于错误而返回
                GetOverlappedResult(hcom, &m_osRead, &wCount, TRUE);
            }
            else
            {
                ClearCommError(hcom, &dwErrorFlags, &comStat); //清除通讯错误
                CloseHandle(m_osRead.hEvent); //关闭并释放hEvent的内存
                return -2;
            }
        }
        return wCount;
	}
	else
    {

        //同步方式
        int bReadStat = ReadFile(hcom, //串口句柄
                                    data, //数据首地址
                                    wCount, //要读取的数据最大字节数
                                    &wCount, //DWORD*,用来接收返回成功读取的数据字节数
                                    NULL); //NULL为同步发送，OVERLAPPED*为异步发送
        if (!bReadStat)
        {
            return -3;
        }
        return wCount;
	}
}

#endif

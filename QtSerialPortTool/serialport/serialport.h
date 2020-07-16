
#ifndef _SERIALPORT_H
#define _SERIALPORT_H
#include <string>

#if WIN32
#include <windows.h>
#endif
/*
    作者：李科威
    日期：2020-12-06
    SerialPort
	用途：串口读写	
*/

class SerialPort
{
#if WIN32
public:
    enum BaudRate
    {
        Baud1200 = 1200,
        Baud2400 = 2400,
        Baud4800 = 4800,
        Baud9600 = 9600,
        Baud19200 = 19200,
        Baud38400 = 38400,
        Baud57600 = 57600,
        Baud115200 = 115200,
        UnknownBaud = -1
    };
    enum DataBits
    {
        Data5 = 5,
        Data6 = 6,
        Data7 = 7,
        Data8 = 8,
        UnknownDataBits = -1
    };

    enum Parity
    {
        NoParity = 0,//无校验
        OddParity = 1,//奇校验
        EvenParity = 2,//偶校验
        MarkParity = 3,//标记校验(仅适用于windows)
        SpaceParity = 4,
        UnknownParity = -1
    };
    enum StopBits
    {
        OneStop = 0,
        One5StopBits=1,
        TwoStop = 2,
        UnknownStopBits = -1
    };
    SerialPort();
    ~SerialPort();	

    // isOverlapped_(同步、异步,仅适用与windows): true为异步，flase为同步
    void setBaudRate(BaudRate baud_rate);
    void setDataBits(DataBits data_bits);
    void setParity(Parity parity);
    void setStopBits(StopBits stop_bits);
    void setOverlapped(bool is_overlapped);//flase 同步 true 异步，默认异步;

    bool isOpen();

    bool openPort(std::string port_name);
    //写数据，成功返回发送数据长度，异步写入成功也返回0，失败返回0
    int send(const uint8_t *data,int len);
    //读数据，成功返回读取实际数据的长度，失败返回0
    int receive(uint8_t *data,int max_len);
    void closePort();

private:		
    HANDLE hcom;
    unsigned long baud_rate_;
    uint8_t data_bits_;
    uint8_t parity_;
    uint8_t stop_bits_;
    bool is_overlapped_;
    bool is_open_;
#endif
};

#endif

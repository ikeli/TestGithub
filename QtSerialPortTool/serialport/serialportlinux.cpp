#include "serialportlinux.h"
#if !WIN32
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <iostream>
SerialPortLinux::SerialPortLinux():
    baud_rate_(Baud115200),
    data_bits_(Data8),
    parity_(NoParity),
    stop_bits_(OneStop),
    is_open_(false)
{

}
void SerialPortLinux::setBaudRate(SerialPortLinux::BaudRate baud_rate)
{
    baud_rate_=baud_rate;
}

void SerialPortLinux::setDataBits(SerialPortLinux::DataBits data_bits)
{
    data_bits_=data_bits;
}

void SerialPortLinux::setParity(SerialPortLinux::Parity parity)
{
    parity_=parity;
}

void SerialPortLinux::setStopBits(SerialPortLinux::StopBits stop_bits)
{
    stop_bits_=stop_bits;
}

/*打开串口
   在Linux下串口文件是位于/dev下的
   串口一为/dev/ttyS0
   串口二为/dev/ttyS1
*/
bool SerialPortLinux::openPort(std::string port_name)
{

   is_open_=false;

   /*以读写方式打开串口*/
   fd=open(port_name.c_str(),O_RDWR|O_NOCTTY|O_NONBLOCK);
   if(fd==-1)
   {
       //不能打开串口
       return false;
   }

   if(!setPortConfig(fd))
   {
       std::cout<<"setPortConfig error"<<std::endl;
       return false;
   }
   return true;

}

void SerialPortLinux::closePort()
{
     is_open_=false;
     close(fd);
}
bool SerialPortLinux::setPortConfig(int fd)
{
//    Baud1200 = 1200,
//    Baud2400 = 2400,
//    Baud4800 = 4800,
//    Baud9600 = 9600,
//    Baud19200 = 19200,
//    Baud38400 = 38400,
//    Baud57600 = 57600,
//    Baud115200 = 115200,
//    UnknownBaud = -1



    int baud_rate=-1;
    struct termios opt;

    //获得当前设备模式，与终端相关的参数。fd=0标准输入
    if(tcgetattr(fd,&opt)!=0)
    {
        return false;
    }

    switch (baud_rate_)
    {
        case Baud9600:
            baud_rate=B9600;
            break;
        case Baud115200:
            baud_rate=B115200;
            break;
        default:
            baud_rate=-1;
            break;
    }


    if(baud_rate==-1)
    {
       return false;

    }

    tcflush(fd,TCIOFLUSH);//设置前flush
    cfsetispeed(&opt,baud_rate);//设置结构termios输入波特率
    cfsetospeed(&opt,baud_rate);//fd应该是文件描述的意思
    int status=tcsetattr(fd,TCSANOW,&opt);//设置终端参数，TCANOW修改立即发生
    if(status!=0)
    {
        //tcsetattr::set baud rate failed
        return false;
    }

    //修改控制模式，保证程序不会占用串口？
    opt.c_cflag |= CLOCAL;

    //修改控制模式，使得能够从串口读取输入数据
    opt.c_cflag |= CREAD;

    opt.c_cflag |= HUPCL;

    //no control-flow
     opt.c_cflag &=~CRTSCTS;

     //避免发送的数据是0x0D,接收变成了0x0A
     opt.c_iflag &= ~ICRNL;

    //设置数据位
    opt.c_cflag &=~CSIZE;
    switch (data_bits_)
    {
        case 5:
            opt.c_cflag &= ~CSIZE;//屏蔽其它标志位
            opt.c_cflag |= CS5;
            break;
        case 6:
            opt.c_cflag &= ~CSIZE;//屏蔽其它标志位
            opt.c_cflag |=CS6;
            break;
        case 7:
            opt.c_cflag &= ~CSIZE;//屏蔽其它标志位
            opt.c_cflag |=CS7;
            break;
        case 8:
            opt.c_cflag &= ~CSIZE;//屏蔽其它标志位
            opt.c_cflag |=CS8;
            break;
        default:
            std::cout <<" open failed , unkown databit ." << std::endl;
            return false;
    }

//    NoParity = 0,//无校验
//    OddParity = 1,//奇校验
//    EvenParity = 2,//偶校验
//    MarkParity = 3,//标记校验(仅适用于windows)
//    SpaceParity = 4,
//    UnknownParity = -1
    //校验
    switch (parity_)
    {
        case 0://NoParity
            opt.c_cflag &=~PARENB;//clear parity enable
            opt.c_iflag &=~INPCK;
            break;
        case 1://奇校验
            opt.c_cflag |=(PARODD|PARENB);//奇校验
            opt.c_iflag |=INPCK;//disable parity checking
            break;
        case 2://偶校验
            opt.c_cflag |=PARENB;//开启奇偶校验
            opt.c_cflag &=~PARODD;//转换为偶效验
            opt.c_iflag |=INPCK;//disable parity checking
            break;
        case 3:
            break;
        case 4:
            opt.c_cflag &=~PARENB;//no parity
            opt.c_cflag &=~CSTOPB;
            break;
        default:
            return false;
    }



    if(parity_!=0)
    {
        opt.c_iflag |=INPCK;
    }

//    OneStop = 0,
//    One5StopBits=1,
//    TwoStop = 2,
//    UnknownStopBits = -1

    //设置停止位
    switch (stop_bits_)
    {
        case 0:
            opt.c_cflag &=~CSTOPB;//CSTOPB：使用1位停止位
            break;
        case 1:
            opt.c_cflag &=~CSTOPB;
            break;
        case 2:
            opt.c_cflag |=CSTOPB;//CSTOPB：使用2位停止位
            break;
        default:
            return false;
    }


    //修改输出模式：原始数据输出(raw 模式)
    opt.c_lflag &= ~(ICANON | ECHO | ISIG);                /*Input*/
    opt.c_oflag &= ~OPOST;                                /*Output*/

    opt.c_cc[VTIME]=150;//设置超时15seconds
    opt.c_cc[VMIN]=0;//update the option and do it

    //如果发生数据溢出，接收数据，但是不再读取
     tcflush(fd,TCIFLUSH);

    status=tcsetattr(fd,TCSANOW,&opt);//设置终端参数，TCANOW修改立即发生
    if(status!=0)
    {
        //Cannot set the serial port parameters
        return false;
    }
    tcflush(fd,TCIOFLUSH);//设置后flush

    return true;
}
int SerialPortLinux::send(const uint8_t *data,int len)
{

    int write_size=write(fd,data,len);
    return write_size;
}
int SerialPortLinux::receive(uint8_t *data,int max_len)
{
    int read_size=read(fd,data,max_len);
    return read_size;
}

#endif

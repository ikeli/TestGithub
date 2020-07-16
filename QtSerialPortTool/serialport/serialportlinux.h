#ifndef SERIALPORTLINUX_H
#define SERIALPORTLINUX_H
#include <string>

class SerialPortLinux
{
#if !WIN32
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
public:
    SerialPortLinux();

    void setBaudRate(BaudRate baud_rate);
    void setDataBits(DataBits data_bits);
    void setParity(Parity parity);
    void setStopBits(StopBits stop_bits);

    bool openPort(std::string port_name);
    int send(const uint8_t *data,int len);
    int receive(uint8_t *data,int max_len);
    void closePort();
private:
     int fd;
     bool is_open_;
     unsigned long baud_rate_;
     uint8_t data_bits_;
     uint8_t parity_;
     uint8_t stop_bits_;
     bool setPortConfig(int fd);
#endif
};

#endif // SERIALPORTLINUX_H

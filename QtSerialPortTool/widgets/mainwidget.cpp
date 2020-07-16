#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QDateTime>
#include <QDebug>

const int LOG_MAX_SIZE=400;
const int RECEIVED_BUFFER_SIZE=1024;

MainWidget::MainWidget(QWidget *parent)
    :QWidget(parent)
    ,ui(new Ui::MainWidget)
    ,isConnectPort_(false)
    ,isExitRece_(false)
    ,logCount_(0)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/images/wapp.png"));
    this->setWindowTitle("Serial Port 1.0.0");

    initUI();
}

MainWidget::~MainWidget()
{
    delete ui;
}
void MainWidget::initUI()
{

#if WIN32
    serialPort_=new SerialPort;
#else
    serialPort_=new SerialPortLinux;
#endif



    serialPortWidget_=new SerialPortWidget(this);
    teMsg_=new QTextEdit(this);

    connect(serialPortWidget_,SIGNAL(sigConnectPort(QString,QString)),SLOT(slotConnectPort(QString,QString)));
    connect(serialPortWidget_,SIGNAL(sigCloseConnect()),SLOT(slotCloseConnect()));
    connect(this,SIGNAL(sigConnectStatus(SerialPortWidget::ConnectStatus)),
            serialPortWidget_,SLOT(slotConnectStatus(SerialPortWidget::ConnectStatus)));

//    connect(this,SIGNAL(sigShowMsg(MainWidget::MsgType,QString)),
//            SLOT(slotShowMsg(MainWidget::MsgType,QString)));

    connect(this,SIGNAL(sigShowMsg(MainWidget::MsgType,QString)),
            SLOT(slotShowMsg(MainWidget::MsgType,QString)),Qt::BlockingQueuedConnection);


    for(int i=0;i<BUTTON_COUNT;i++)
    {
        QPushButton *pb=new QPushButton(this);
        setButtonStyle(pb,60);
        pb->setText(ButtonText[i]);
        connect(pb,&QPushButton::clicked,[=](){

                   slotClickedButton(MainWidget::ButtonType(i));
               });
        pbList_.append(pb);
    }

    QHBoxLayout *h1=new QHBoxLayout();
    QVBoxLayout *v1=new QVBoxLayout();
    for(int i=0;i<pbList_.size();i++)
    {
        v1->addWidget(pbList_[i]);
    }
    v1->addStretch();

    h1->addWidget(teMsg_);
    h1->addLayout(v1);

    QVBoxLayout *mainLayout=new QVBoxLayout(this);
    mainLayout->addWidget(serialPortWidget_);
    mainLayout->addLayout(h1);
    setLayout(mainLayout);

}

void MainWidget::runReceived()
{
    receThread_= std::thread([&](){

            while(!isExitRece_)
            {

                  uint8_t*data=new uint8_t[RECEIVED_BUFFER_SIZE];
                  int read_size= serialPort_->receive(data,RECEIVED_BUFFER_SIZE);
                  if(read_size>0)
                  {
                       for(int i=0;i<read_size;i++)
                       {
                           qDebug()<<"i:"<<i<<" data:"<<data[i];
                           recData_.push_back(data[i]);
                           //\r\n 0x0D 0x0A
//                           if(data[i]==0x0A)
//                           {
//                              uint size=recData_.size();
//                              if(recData_[size-2]==0x0D)
//                              {
//                                emit sigShowMsg(MsgType::MSG_OK,binToHexString(recData_));
//                                recData_.clear();

//                              }
//                           }


                       }//end for

                       emit sigShowMsg(MsgType::MSG_OK,binToHexString(recData_));
                       recData_.clear();

                  }
                  delete [] data;
                  data=nullptr;

                  std::this_thread::sleep_for(std::chrono:: milliseconds (1));

            }//end while

        });

        receThread_.detach();
}

void MainWidget::setButtonStyle(QPushButton *pb, int minW)
{
    pb->setMinimumWidth(minW);
    pb->setMinimumHeight(30);
}
void MainWidget::slotConnectPort(QString portName, QString baud)
{
#if WIN32
    if(baud=="9600")
    {
      serialPort_->setBaudRate(SerialPort::Baud9600);
    }
    else
    {
      serialPort_->setBaudRate(SerialPort::Baud115200);
    }
#else
    if(baud=="9600")
    {
      serialPort_->setBaudRate(SerialPortLinux::Baud9600);
    }
    else
    {
      serialPort_->setBaudRate(SerialPortLinux::Baud115200);
    }
#endif



    if(serialPort_->openPort(portName.toStdString()))
    {
       isConnectPort_=true;
       isExitRece_=false;
       runReceived();
       emit sigConnectStatus(SerialPortWidget::ConnectStatus::OK);

    }
    else
    {
       isExitRece_=true;
       isConnectPort_=false;
       serialPort_->closePort();
       emit sigConnectStatus(SerialPortWidget::ConnectStatus::Error);
    }

}
void MainWidget::slotCloseConnect()
{
    isExitRece_=true;
    isConnectPort_=false;
    serialPort_->closePort();
    emit sigConnectStatus(SerialPortWidget::ConnectStatus::Close);
}
void MainWidget::showRecMsg(MainWidget::MsgType type,QString msg)
{

    if(type==MSG_ERROR)
    {
       msg="<font color=\"#ff0000\">"+msg+"</font>";
    }

    teMsg_->append(QDateTime::currentDateTime ().toString ("[yyyy/MM/dd hh:mm:ss zzz] ")+msg);

    logCount_++;
    if(logCount_>LOG_MAX_SIZE)
    {
        teMsg_->clear();
        logCount_=0;
        //qDebug()<<"clear";
    }

    teMsg_->moveCursor(QTextCursor::End);
}

void MainWidget::slotShowMsg(MainWidget::MsgType type,QString msg)
{   
   showRecMsg(type,msg);
}
void MainWidget::slotClickedButton(MainWidget::ButtonType type)
{
    switch (type)
    {
        case PB_SEND:
        {
            uint8_t *data=new uint8_t[20];
            for(int i=0;i<18;i++)
            {
                data[i]=i;
            }
            data[18]=0x0D;
            data[19]=0x0A;
            serialPort_->send(data,20);
            delete [] data;
            data=nullptr;
            break;
        }
        case PB_CLEAR:
        {          
            teMsg_->clear();
            break;
        }

    }

}
QString MainWidget::binToHexString(std::vector<uint8_t> data)
{
    std::ostringstream strHex;
    strHex << std::hex << std::setfill('0');    
    uint size=data.size();
    for(uint i=0;i<size;i++)
    {
       strHex << std::setw(2)<<std::setiosflags(std::ios::uppercase)<<static_cast<unsigned int>(data[i])<<' ';
    }
    return QString::fromStdString(strHex.str());
}
QString MainWidget::binToHexString(const uint8_t *data,size_t size)
{
    std::ostringstream strHex;
    strHex << std::hex << std::setfill('0');
    for (size_t i = 0; i < size; ++i)
    {
        strHex << std::setw(2)<<std::setiosflags(std::ios::uppercase)<<static_cast<unsigned int>(data[i])<<' ';
    }
    return QString::fromStdString(strHex.str());
}

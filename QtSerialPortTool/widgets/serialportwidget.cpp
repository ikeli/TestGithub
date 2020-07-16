#include "serialportwidget.h"
#include <QSerialPortInfo>
#include <QStyleOption>
#include <QStylePainter>
#include<QAbstractItemView>
#include <QMessageBox>
#include <QDebug>
SerialPortWidget::SerialPortWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
}
void SerialPortWidget::paintEvent(QPaintEvent *event)
{
    QStylePainter painter(this);
    QStyleOption opt;
    opt.initFrom(this);
    opt.rect=rect();
    painter.drawPrimitive(QStyle::PE_Widget, opt);
    QWidget::paintEvent(event);
}

void SerialPortWidget::initUI()
{

    lblConnectStatus=new QLabel(this);
    lblConnectStatus->setMinimumSize(20,20);
    lblConnectStatus->setMaximumSize(20,20);

    QLabel *lblPortName=new QLabel(this);
    lblPortName->setObjectName("lbl");
    lblPortName->setText("Port:");
    cmbPortName=new QComboBox(this);
#if WIN32
    cmbPortName->setMinimumSize(70,25);
    cmbPortName->setMaximumSize(70,25);
#else
    cmbPortName->setMinimumSize(120,25);
    cmbPortName->setMaximumSize(120,25);
    cmbPortName->setEditable(true);
#endif

    //cmbPortName->addItem("/dev/pts/1");


    QLabel *lblBaud=new QLabel(this);
    lblBaud->setObjectName("lbl");
    lblBaud->setText("Baud:");
    cmbBaud=new QComboBox(this);   
    cmbBaud->setMinimumSize(80,25);

    btnRefresh=new QPushButton(this);
    btnRefresh->setObjectName("btnRefresh");
    btnRefresh->setMinimumSize(20,20);
    btnRefresh->setMaximumSize(20,20);
    btnConnect=new QPushButton(this);
    btnConnect->setObjectName("btnConnect");
    btnConnect->setMinimumSize(80,25);
    btnConnect->setMaximumSize(80,25);
    btnConnect->setText("Connect");

    QHBoxLayout *hLayout=new QHBoxLayout(this);
    hLayout->addWidget(lblConnectStatus);
    hLayout->addWidget(lblPortName);
    hLayout->addWidget(cmbPortName);
    hLayout->addWidget(btnRefresh);
    hLayout->addWidget(lblBaud);
    hLayout->addWidget(cmbBaud);
    hLayout->addSpacing(5);
    hLayout->addWidget(btnConnect);
    hLayout->addStretch();
    hLayout->addSpacing(5);


    hLayout->addSpacing(5);
    hLayout->setMargin(5);

    setLayout(hLayout);


    initPortName();
    initBaud();
    showConnectStatus(false);

    connect(btnConnect,SIGNAL(clicked()),this,SLOT(slotConnect()));
    connect(btnRefresh,SIGNAL(clicked()),this,SLOT(slotRefreshPortName()));


}
void SerialPortWidget::initPortName()
{
    QString port_name="";
    QString space="";
#if WIN32
    foreach(const QSerialPortInfo &portinfo,QSerialPortInfo::availablePorts())
    {
       port_name=portinfo.portName();      
       space.fill(' ',12-port_name.length());
       cmbPortName->addItem(port_name+space+portinfo.description());

       qDebug()<<"QSerialPortInfo description:"<<portinfo.description();
       qDebug()<<"QSerialPortInfo productIdentifier :"<<portinfo.productIdentifier();
       qDebug()<<"QSerialPortInfo vendorIdentifier :"<<portinfo.vendorIdentifier();
        qDebug()<<"QSerialPortInfo serialNumber :"<<portinfo.serialNumber();

    }

    comboxDropDownFitWidth();
#else
    foreach(const QSerialPortInfo &portinfo,QSerialPortInfo::availablePorts())
    {
       port_name=portinfo.portName();

       cmbPortName->addItem("/dev/"+port_name);
    }
#endif

}
void SerialPortWidget::comboxDropDownFitWidth()
{
    int maxLen=10;
    int count=cmbPortName->count();
    for(int i=0;i <count;i++)
    {
        if(maxLen < cmbPortName->itemText(i).length())
        {
           maxLen = cmbPortName->itemText(i).length();
        }

    }

    int ptVal = this->font().pointSize();
    cmbPortName->view()->setFixedWidth(maxLen*ptVal*0.75);

}

void SerialPortWidget::setControlEnable(bool enable)
{
    btnRefresh->setEnabled(enable);
    cmbPortName->setEnabled(enable);
    cmbBaud->setEnabled(enable);
}
void SerialPortWidget::initBaud()
{
     cmbBaud->addItem("9600");
     cmbBaud->addItem("115200");
     cmbBaud->setCurrentIndex(1);
}

void SerialPortWidget::showConnectStatus(bool isConnected)
{
    showConnectStatusImg(isConnected);
    lblConnectStatus->style()->unpolish(lblConnectStatus);
    lblConnectStatus->style()->polish(lblConnectStatus);
    lblConnectStatus->update();
}
void SerialPortWidget::slotConnect()
{
    if(btnConnect->text()=="Connect")
    {
        //btnConnect->setText("Disconnect");
        QString portNameText=cmbPortName->currentText();
        QString portName=portNameText.mid(0,portNameText.indexOf(" "));
        emit sigConnectPort(portName,cmbBaud->currentText());
    }
    else
    {        
        emit sigCloseConnect();
        btnConnect->setText("Connect");
    }

}
void SerialPortWidget::slotConnectStatus(SerialPortWidget::ConnectStatus status)
{

   qDebug()<<"slotConnectStatus";
    if(status==ConnectStatus::OK)
    {
        setControlEnable(false);
        btnConnect->setText("Disconnect");
        showConnectStatus(true);
    }
    else if(status==ConnectStatus::Close)
    {      
        setControlEnable(true);
        btnConnect->setText("Connect");
        showConnectStatus(false);
    }
    else if(status==ConnectStatus::Error)
    {
        setControlEnable(true);
        btnConnect->setText("Connect");
        showConnectStatus(false);
        lblConnectStatus->update();
        QMessageBox::critical(this, "error", "连接错误",
                                 QMessageBox::Ok, QMessageBox::Ok);
    }
}
void SerialPortWidget::slotRefreshPortName()
{
    cmbPortName->clear();
    initPortName();
}
void SerialPortWidget::showConnectStatusImg(bool isConnected)
{

    QPixmap pixConnected;
    isConnected? pixConnected.load(":/images/green.png"):pixConnected.load(":/images/gray.png");
    pixConnected.scaled(lblConnectStatus->size(),Qt::KeepAspectRatio);
    lblConnectStatus->setScaledContents(true);
    lblConnectStatus->setPixmap(pixConnected);
}

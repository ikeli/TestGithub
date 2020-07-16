#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <thread>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <stdio.h>
#include <list>
#include <vector>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QHBoxLayout>
#include "serialport/serialport.h"
#include "serialport/serialportlinux.h"
#include "widgets/serialportwidget.h"
namespace Ui {
class MainWidget;
}
const int BUTTON_COUNT=2;
class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();
public:
    enum MsgType{
        MSG_ERROR,
        MSG_OK,
        MSG_INFO
    };
    enum ButtonType{
        PB_SEND=0,
        PB_CLEAR
    };

    QString ButtonText[BUTTON_COUNT]={"Send","Clear"};
signals:
    void sigConnectStatus(SerialPortWidget::ConnectStatus);

    void sigShowMsg(MainWidget::MsgType type,QString msg);
private slots:
    void slotClickedButton(MainWidget::ButtonType type);
    void slotConnectPort(QString portName, QString baud);
    void slotCloseConnect();

    void slotShowMsg(MainWidget::MsgType type,QString msg);
private:
    Ui::MainWidget *ui;
private:
    bool isConnectPort_;
    bool isExitRece_; 
    int  logCount_;
#if WIN32
    SerialPort *serialPort_;
#else
    SerialPortLinux *serialPort_;
#endif


    std::thread receThread_;
    std::vector<uint8_t> recData_;


    SerialPortWidget *serialPortWidget_;
    QTextEdit *teMsg_;
    QList<QPushButton*> pbList_;

    void initUI();
    void runReceived();
    void showRecMsg(MainWidget::MsgType type,QString msg);
    void setButtonStyle(QPushButton *pb,int minW);
    QString binToHexString(std::vector<uint8_t> data);
    QString binToHexString(const uint8_t *data, size_t size);
};

#endif // MAINWIDGET_H

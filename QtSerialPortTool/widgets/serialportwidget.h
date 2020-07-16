#ifndef SERIALPORTWIDGET_H
#define SERIALPORTWIDGET_H
#pragma execution_character_set("utf-8");
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QComboBox>
class SerialPortWidget : public QWidget
{
    Q_OBJECT
public:
    enum ConnectStatus{
        OK,
        Close,
        Error,
    };
    explicit SerialPortWidget(QWidget *parent = nullptr);

signals:
   void sigConnectPort(QString portName,QString baud);
   void sigCloseConnect();
public slots:
   void slotConnectStatus(SerialPortWidget::ConnectStatus);
protected:
    void paintEvent(QPaintEvent *event);
public:
private slots:
    void slotConnect(); 
    void slotRefreshPortName();   
private:
    QLabel *lblConnectStatus;
    QComboBox *cmbPortName;
    QComboBox *cmbBaud;
    QPushButton *btnConnect;
    QPushButton *btnRefresh;

    void initUI();
    void initPortName();
    void initBaud();
    void showConnectStatus(bool isConnected);
    void showConnectStatusImg(bool isConnected);
    void comboxDropDownFitWidth();
    void setControlEnable(bool enable);
};

#endif // SERIALPORTWIDGET_H

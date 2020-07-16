#include <QFile>
#include <QApplication>
#include "widgets/mainwidget.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile qss_file(":/default.qss");
    if(!qss_file.open(QFile::ReadOnly))
    {
      //qDebug()<<"qss file open error";
    }
    qApp->setStyleSheet(qss_file.readAll());
    qss_file.close();

    qRegisterMetaType<MainWidget::MsgType>("MainWidget::MsgType");

    //MainWindow w;
    MainWidget w;
    w.show();
    return a.exec();
}

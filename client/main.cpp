#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include "CFramelessWindowBase.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString appSkin;
    QString appSkinPath = ":/Resource/Resource/windowsXP.qss";
    QFile file(appSkinPath);
    if(file.open(QIODevice::ReadOnly))
    {
        appSkin = QString(file.readAll().data());
        file.close();
    }
    MainWindow w;
    w.setStyleSheet(appSkin);
    w.show();

    return a.exec();
}

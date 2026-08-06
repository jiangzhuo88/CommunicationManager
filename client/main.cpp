#include "mainwindow.h"
#include "LoginDialog.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 加载样式表
    QString appSkin;
    QString appSkinPath = ":/Resource/Resource/windowsXP.qss";
    QFile file(appSkinPath);
    if(file.open(QIODevice::ReadOnly))
    {
        appSkin = QString(file.readAll().data());
        file.close();
    }

    // 显示登录界面
    LoginDialog loginDlg;
    loginDlg.setStyleSheet(appSkin);
    if (loginDlg.exec() != QDialog::Accepted)
    {
        return 0;  // 用户取消登录，退出程序
    }

    MainWindow w;
    w.setStyleSheet(appSkin);
    w.show();

    return a.exec();
}

#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QGraphicsOpacityEffect>
#include <QStyleFactory>
#include "CFramelessWindowBase.h"
#include "LoginDialog.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Windows"));
    QString appSkin;
    QString appSkinPath = ":/Resource/Resource/windowsXP.qss";
    QFile file(appSkinPath);
    if(file.open(QIODevice::ReadOnly))
    {
        appSkin = QString(file.readAll().data());
        file.close();
    }
//    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect;
//    effect->setOpacity(0.2);
    a.setStyleSheet(appSkin);
    // 显示登录界面
    LoginDialog loginDlg;
    if (loginDlg.exec() != QDialog::Accepted)
    {
        return 0;  // 用户取消登录，退出程序
    }
    MainWindow w;
    w.setCurrentUser(loginDlg.userName(), loginDlg.role());
//    w.setGraphicsEffect(effect);
//    w.setDisabled(true);
    w.show();

    return a.exec();
}

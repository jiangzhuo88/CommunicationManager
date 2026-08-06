#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLineEdit>
#include <QTableWidget>
#include <QListWidget>
#include <QMenuBar>
#include "CFramelessWindowBase.h"
#include "ZToolButton.h"
#include "CenterWidget.h"
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private:
    void initUi();
    void initMenuBar();
    // 按钮创建辅助函数
    ZToolButton* createRibbonButton(const QString &text, const QIcon &icon = QIcon());
    QFrame* createRibbonGroup(const QString &title, const QList<ZToolButton*> &buttons);
private slots:
    void slotAddUser();
    void slotDeleteUser();
    void slotUserList();
private:
    // 顶部Ribbon栏
    QWidget* m_ribbonBar;
    QHBoxLayout* m_ribbonLayout;
    CenterWidget* m_centralWidget;
    QMenuBar* m_menuBar;
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLineEdit>
#include <QTableWidget>
#include <QListWidget>
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
    // 按钮创建辅助函数
    ZToolButton* createRibbonButton(const QString &text, const QIcon &icon = QIcon());
    QFrame* createRibbonGroup(const QString &title, const QList<ZToolButton*> &buttons);
private:
    // 顶部Ribbon栏
    QWidget* m_ribbonBar;
    QHBoxLayout* m_ribbonLayout;
    CenterWidget* m_centralWidget;


};

#endif // MAINWINDOW_H

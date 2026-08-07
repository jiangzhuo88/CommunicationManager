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
#include "ImportDataDialog.h"
#include "localdatabase.h"
#include "dmdatabase.h"
#include "qt_ipcnotify.h"
#include "UserStore.h"
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setCurrentUser(const QString& userName, UserStore::Role role);
private slots:
    void btnImportClicked();
    void btnClearClicked();
    void btnUserManagerClicked();
private:
    void initUi();
    // 按钮创建辅助函数
    ZToolButton* createRibbonButton(const QString &text, const QIcon &icon = QIcon());
    QFrame* createRibbonGroup(const QString &title, const QList<ZToolButton*> &buttons);
    int dialogExec(QDialog* dlg);
    void loadConfig();
    void saveConfig();
    void loadLocalData();
    void loadIPC();
    void saveLocalData(const QString &tableName, QList<QMap<QString, QString> > mapList);
//    void saveTPLocalData(QList<QMap<QString, QString> > mapList);
    void onConnectDb();
    void loadTableData(const QString &tableName, QTableWidget *table);
    bool dataIsValid(const QString &tableName,QVariantMap map);
    QList<QMap<QString, QString> > loadDP();
    QList<QMap<QString, QString> > loadTP();
    void updteTable(QString localDBTableName, QTableWidget* table);
    void onAddLine(QTableWidget* table, const QString& tableName, QString key = QString());
    void onRemoveLine(QTableWidget* table,const QString& tableName);
    void onUpdateTable(int row, int column,const QString& oldVal,const QString& newVal, QTableWidget* tableWidget, const QString& tableName);
    void onPasteTable(CenterWidget* cenWidget,const QString& tableName);
    QString getStr(int number,QString str);
    QMap<int, QStringList> parseStrToMapList(const QString& src);
    QString extractFreqOnly(const QString &text);
private:
    // 顶部Ribbon栏
    QWidget* m_ribbonBar;
    QHBoxLayout* m_ribbonLayout;
    CenterWidget* m_DPWidget;
    CenterWidget* m_TPWidget;
    QWidget* m_maskWidget;
    QTabWidget* m_tabWidget;
private:
    QString m_dmHost;
    int m_dmPort;
    QString m_dmDatabaseName;
    QString m_dmUsername;
    QString m_dmPassword;
    QString m_dmSchema;

    QString m_localDBPath;

    LocalDatabase *m_localDatabase;
    // 数据库对象
    DmDatabase *m_dmDatabase;
    QtIpcNotify* ipc = nullptr;

    // 当前登录用户
    QString        m_currentUserName;
    UserStore::Role m_currentRole = UserStore::RoleNormal;
};

#endif // MAINWINDOW_H

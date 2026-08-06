#include "mainwindow.h"
#include "UserManagementDialog.h"

#include <QPushButton>
#include <QLabel>
#include <QHeaderView>
#include <QAction>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 无边框窗口，需要在创建子控件前设置
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    initUi();
}

void MainWindow::initUi()
{
    this->setObjectName("MainWindow");

    resize(1700, 900);

    // --------------------------
    // 1. 顶部Ribbon栏（和截图一模一样的分组按钮）
    // --------------------------
    m_ribbonBar = new QWidget(this);
    m_ribbonBar->setObjectName("ribbonBar");
    m_ribbonLayout = new QHBoxLayout(m_ribbonBar);
    m_ribbonLayout->setSpacing(2);
    m_ribbonLayout->setContentsMargins(2,2,2,2);

    // --- 分组1：Database Management ---
    QList<ZToolButton*> dbBtns;
    auto btnLoad = createRibbonButton("Load\nDatabase", QIcon(":/Resource/Resource/Icon/加载.png"));
    auto btnClear = createRibbonButton("Clear\nDatabase", QIcon(":/Resource/Resource/Icon/清屏-清空.png"));
    auto btnBackup = createRibbonButton("Backup\nDatabase", QIcon(":/Resource/Resource/Icon/备份.png"));
    auto btnRestore = createRibbonButton("Restore\nDatabase", QIcon(":/Resource/Resource/Icon/恢复.png"));
    auto btnWipe = createRibbonButton("Wipe\nDatabase", QIcon(":/Resource/Resource/Icon/擦除.png"));
    dbBtns << btnLoad << btnClear << btnBackup << btnRestore << btnWipe;
    auto groupDb = createRibbonGroup("Database Management", dbBtns);
    m_ribbonLayout->addWidget(groupDb);

    // --- 分组2：Data Management ---
    QList<ZToolButton*> dataBtns;
    auto btnImport = createRibbonButton("Import\nData", QIcon(":/Resource/Resource/Icon/导入.png"));
    auto btnJamming = createRibbonButton("Jamming Style\nEditing", QIcon(":/Resource/Resource/Icon/干扰样式编辑.png"));
    auto btnDict = createRibbonButton("Dictionary\nManagement", QIcon(":/Resource/Resource/Icon/字典管理.png"));
    dataBtns << btnImport << btnJamming << btnDict;
    auto groupData = createRibbonGroup("Data Management", dataBtns);
    m_ribbonLayout->addWidget(groupData);

    // --- 分组3：Notify Update ---
    QList<ZToolButton*> notifyBtns;
    auto btnNotify = createRibbonButton("Notify\nUpdate", QIcon(":/Resource/Resource/Icon/通知.png"));
    notifyBtns << btnNotify;
    auto groupNotify = createRibbonGroup("Notify Update", notifyBtns);
    m_ribbonLayout->addWidget(groupNotify);

    // 占位，让分组靠左边
    m_ribbonLayout->addStretch(1);


    // --------------------------
    // 4. 整体布局组装
    // --------------------------
    QWidget* tabBarWidget = new QWidget(this);
    tabBarWidget->setObjectName("tabBarWidget");
    QTabWidget* tabWidget = new QTabWidget(this);

    QVBoxLayout* mainLayout = new QVBoxLayout(tabBarWidget);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    // 自定义标题栏
    CFramelessWindowBase* CFrameless = new CFramelessWindowBase;
    CFrameless->setWindowTitleText("Communication Identify Manager Client");
    CFrameless->setWindowTitleIcon(QIcon(":/Resource/Resource/Icon/通信.png"));
    mainLayout->addWidget(CFrameless);

    // 菜单栏（用户管理）
    initMenuBar();
    mainLayout->addWidget(m_menuBar);

    // 顶部Ribbon
    tabWidget->addTab(m_ribbonBar,"Menu Function");
    mainLayout->addWidget(tabWidget);

    m_centralWidget = new CenterWidget;
    m_centralWidget->setObjectName("centralWidget");
    mainLayout->addWidget(m_centralWidget,1);
    setCentralWidget(tabBarWidget);

    // 安装边框缩放处理器（必须在标题栏被添加到布局后调用）
    CFrameless->installFramelessHandler();
}

void MainWindow::initMenuBar()
{
    m_menuBar = new QMenuBar(this);
    m_menuBar->setObjectName("appMenuBar");

    QMenu* userMenu = m_menuBar->addMenu("用户管理");
    QAction* actAddUser = userMenu->addAction("添加用户");
    QAction* actDelUser = userMenu->addAction("删除用户");
    QAction* actUserList = userMenu->addAction("用户列表");

    connect(actAddUser, &QAction::triggered, this, &MainWindow::slotAddUser);
    connect(actDelUser, &QAction::triggered, this, &MainWindow::slotDeleteUser);
    connect(actUserList, &QAction::triggered, this, &MainWindow::slotUserList);
}

// 创建Ribbon按钮（带图标+文字，和截图样式一致）
ZToolButton* MainWindow::createRibbonButton(const QString &text, const QIcon &icon)
{
    ZToolButton* btn = new ZToolButton();
    btn->setObjectName("tabAction");
    btn->setIcon(icon,QSize(24,24));
    btn->setText(text);
    btn->setMinimumHeight(80); // 固定大小，保持整齐
    return btn;
}

// 创建Ribbon分组（带标题，按钮水平排列）
QFrame* MainWindow::createRibbonGroup(const QString &title, const QList<ZToolButton*> &buttons)
{
    QFrame* group = new QFrame();
    group->setObjectName("category");
    group->setFrameShape(QFrame::Box);
    group->setLineWidth(1);


    QVBoxLayout* groupLayout = new QVBoxLayout(group);
    groupLayout->setSpacing(0);
    groupLayout->setContentsMargins(0,0,0,0);

    // 按钮行
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(2);
    for (auto btn : buttons) {
        btnLayout->addWidget(btn);
    }
    groupLayout->addLayout(btnLayout);

    // 分组标题
    QLabel* labelTitle = new QLabel(title, group);
    labelTitle->setObjectName("categoryLabel");
    labelTitle->setAlignment(Qt::AlignCenter);
    labelTitle->setContentsMargins(6,0,6,0);
    groupLayout->addWidget(labelTitle);

    return group;
}

void MainWindow::slotAddUser()
{
    UserManagementDialog dlg(this);
    dlg.exec();
}

void MainWindow::slotDeleteUser()
{
    UserManagementDialog dlg(this);
    dlg.exec();
}

void MainWindow::slotUserList()
{
    UserManagementDialog dlg(this);
    dlg.exec();
}

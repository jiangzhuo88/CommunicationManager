#include "mainwindow.h"
#include "UserManagementDialog.h"
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>
#include <QDialog>
#include <QCoreApplication>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <thread>
#define PrimaryKey "TZCSXH"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_dmDatabase(new DmDatabase(this))
    , m_localDatabase(new LocalDatabase(this))
    , ipc(new QtIpcNotify(IpcNotifyRaw::ROLE_CLIENT))
{
    initUi();
    loadIPC();
    loadConfig();
    loadLocalData();
    connect(m_DPWidget, &CenterWidget::userCellEdited,[this](int row,int column,const QString& oldVal,const QString& newVal){
        onUpdateTable(row,column,oldVal,newVal,m_DPWidget->getTableWidget(),"DP_MSG");
    });
    connect(m_TPWidget, &CenterWidget::userCellEdited,[this](int row,int column,const QString& oldVal,const QString& newVal){
        onUpdateTable(row,column,oldVal,newVal,m_TPWidget->getTableWidget(),"TP_MSG");
    });
}

void MainWindow::btnImportClicked()
{

//    ImportDataDialog dlg(this);
//    dlg.setWindowTitle("Import Data");
//    //    dlg.resize(400,300);
//    dialogExec(&dlg);

    if(QMessageBox::Yes != QMessageBox::question(this,"Hint","Data will tobe imported and covered,Are you continue?",QMessageBox::Yes,QMessageBox::No))
    {
        return;
    }

    QList<QMap<QString,QString>> dpList = loadDP();
    QList<QMap<QString,QString>> tpList = loadTP();

    for(QMap<QString,QString>& map : dpList)
    {
        QString str = map["Platform"];
        QMap<int,QStringList> MBMCMap = parseStrToMapList(str);
        map["PlatformMsg"] = MBMCMap[30].join(";");
        map["PlatformFeature"] = MBMCMap[156].join(";");
    }

    saveLocalData("DP_MSG",dpList);
    saveLocalData("TP_MSG",tpList);

    updteTable("DP_MSG",m_DPWidget->getTableWidget());
    updteTable("TP_MSG",m_TPWidget->getTableWidget());
    ipc->publish("REFRESH");


}

void MainWindow::btnClearClicked()
{
    if(QMessageBox::Yes != QMessageBox::question(this,"Hint","Data will clear,Are you continue?",QMessageBox::Yes,QMessageBox::No))
    {
        return;
    }
    m_DPWidget->clear();
    m_TPWidget->clear();
    m_localDatabase->dropData("DP_MSG");
    m_localDatabase->dropData("TP_MSG");
    ipc->publish("REFRESH");
}

void MainWindow::btnUserManagerClicked()
{
    UserManagementDialog dlg(this);
    dlg.exec();
}



void MainWindow::initUi()
{
    this->setObjectName("MainWindow");

    resize(1700, 900);
    m_maskWidget = new QWidget(this);
    // --------------------------
    // 1. 顶部Ribbon栏（和截图一模一样的分组按钮）
    // --------------------------
    m_ribbonBar = new QWidget(this);
    m_ribbonBar->setObjectName("ribbonBar");
    m_ribbonLayout = new QHBoxLayout(m_ribbonBar);
    m_ribbonLayout->setSpacing(2);
    m_ribbonLayout->setContentsMargins(2,2,2,2);

//    // --- 分组1：Database Management ---
//    QList<ZToolButton*> dbBtns;
//    ZToolButton* btnLoad = createRibbonButton("Load\nDatabase", QIcon(":/Resource/Resource/Icon/加载.png"));
//    ZToolButton* btnClear = createRibbonButton("Clear\nDatabase", QIcon(":/Resource/Resource/Icon/清屏-清空.png"));
//    ZToolButton* btnBackup = createRibbonButton("Backup\nDatabase", QIcon(":/Resource/Resource/Icon/备份.png"));
//    ZToolButton* btnRestore = createRibbonButton("Restore\nDatabase", QIcon(":/Resource/Resource/Icon/恢复.png"));
//    ZToolButton* btnWipe = createRibbonButton("Wipe\nDatabase", QIcon(":/Resource/Resource/Icon/擦除.png"));
//    dbBtns << btnLoad << btnClear << btnBackup << btnRestore << btnWipe;
//    auto groupDb = createRibbonGroup("Database Management", dbBtns);
//    m_ribbonLayout->addWidget(groupDb);

    // --- 分组2：Data Management ---
    QList<ZToolButton*> dataBtns;
    ZToolButton* btnImport = createRibbonButton("Import\nData", QIcon(":/Resource/Resource/Icon/导入.png"));
    ZToolButton* btnClear = createRibbonButton("Clear\nData", QIcon(":/Resource/Resource/Icon/清屏-清空.png"));
//    ZToolButton* btnJamming = createRibbonButton("Jamming Style\nEditing", QIcon(":/Resource/Resource/Icon/干扰样式编辑.png"));
    ZToolButton* btnDict = createRibbonButton("Dictionary\nManagement", QIcon(":/Resource/Resource/Icon/字典管理.png"));
//    dataBtns << btnImport << btnJamming << btnDict;
    dataBtns << btnImport << btnClear << btnDict;
    auto groupData = createRibbonGroup("Data Management", dataBtns);
    m_ribbonLayout->addWidget(groupData);

    // --- 分组3：Notify Update ---
    QList<ZToolButton*> notifyBtns;
    ZToolButton* btnNotify = createRibbonButton("Notify\nUpdate", QIcon(":/Resource/Resource/Icon/通知.png"));
    notifyBtns << btnNotify;
    auto groupNotify = createRibbonGroup("Notify Update", notifyBtns);
    m_ribbonLayout->addWidget(groupNotify);

    QList<ZToolButton*> usersBtns;
    ZToolButton* btnUserManager = createRibbonButton("User\nManager", QIcon(":/Resource/Resource/Icon/清屏-清空.png"));
    usersBtns << btnUserManager;
    auto groupUser = createRibbonGroup("User Manager", usersBtns);
    m_ribbonLayout->addWidget(groupUser);
    // 占位，让分组靠左边
    m_ribbonLayout->addStretch(1);


    // --------------------------
    // 4. 整体布局组装
    // --------------------------
    QWidget* tabBarWidget = new QWidget(this);
    tabBarWidget->setObjectName("tabBarWidget");
    QTabWidget* tabWidget = new QTabWidget(this);
    tabWidget->setObjectName("mainTab");
    tabWidget->tabBar()->setObjectName("mainTabBar");
    QVBoxLayout* mainLayout = new QVBoxLayout(tabBarWidget);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);



    tabWidget->addTab(m_ribbonBar,"Menu Function");

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    CFramelessWindowBase* CFrameless = new CFramelessWindowBase;
    CFrameless->setWindowTitleText("Communication Identify Manager Client");
    CFrameless->setWindowTitleIcon(QIcon(":/Resource/Resource/Icon/通信.png"));
    mainLayout->addWidget(CFrameless);
    // 顶部Ribbon
    mainLayout->addWidget(tabWidget);

    m_tabWidget = new QTabWidget();
    m_tabWidget->setObjectName("centralTabWidget");
    m_tabWidget->tabBar()->setObjectName("centralTabWidgetBar");
    QStringList dpList;
    dpList<<"Contry"<<"DeviceName"<<"ThreatLevel"<<"PlatformMsg"<<"PlatformFeature"<<"Frequency(MHz)"<<"Modulation"<<"SymbolRate"<<"CharacterFeature";
    QStringList tpList;
    tpList<<"Contry"<<"DeviceName"<<"ThreatLevel"<<"PlatformMsg"<<"PlatformFeature"<<"Frequency(MHz)"<<"Modulation"<<"SymbolRate"<<"CharacterFeature";
    m_DPWidget = new CenterWidget(dpList);
    m_TPWidget = new CenterWidget(tpList);
    m_tabWidget->setObjectName("centralWidget");
    m_tabWidget->addTab(m_DPWidget,"FFA");
    m_tabWidget->addTab(m_TPWidget,"FFH");
    mainLayout->addWidget(m_tabWidget,1);
    setCentralWidget(tabBarWidget);

    m_maskWidget->setGeometry(this->rect());
    m_maskWidget->setStyleSheet("background-color:rgba(0,0,0,0.5);");
    m_maskWidget->raise();
    m_maskWidget->hide();

    // 安装边框缩放处理器（必须在标题栏被添加到布局后调用）
    CFrameless->installFramelessHandler();
    connect(btnImport,&ZToolButton::clicked,this,&MainWindow::btnImportClicked);
    connect(btnClear,&ZToolButton::clicked,this,&MainWindow::btnClearClicked);
    connect(btnUserManager,&ZToolButton::clicked,this,&MainWindow::btnUserManagerClicked);
    connect(m_DPWidget,&CenterWidget::sigAddALine,[&](){onAddLine(m_DPWidget->getTableWidget(),"DP_MSG");});
    connect(m_DPWidget,&CenterWidget::sigRemoveALine,[&](){onRemoveLine(m_DPWidget->getTableWidget(),"DP_MSG");});
    connect(m_TPWidget,&CenterWidget::sigAddALine,[&](){onAddLine(m_TPWidget->getTableWidget(),"TP_MSG");});
    connect(m_TPWidget,&CenterWidget::sigRemoveALine,[&](){onRemoveLine(m_TPWidget->getTableWidget(),"TP_MSG");});
    connect(m_DPWidget,&CenterWidget::sigPasteLine,[&](){onPasteTable(m_DPWidget,"DP_MSG");});
    connect(m_TPWidget,&CenterWidget::sigPasteLine,[&](){onPasteTable(m_TPWidget,"TP_MSG");});
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

int MainWindow::dialogExec(QDialog *dlg)
{
    if(dlg == nullptr)
    {
        return -1;
    }
    m_maskWidget->setGeometry(this->rect());
    m_maskWidget->show();
    int ret = dlg->exec();
    m_maskWidget->hide();
    return ret;
}

void MainWindow::loadConfig()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    QSettings settings(configPath, QSettings::IniFormat);

    settings.beginGroup("Database");
    m_dmHost = settings.value("Host", "localhost").toString();
    m_dmPort = settings.value("Port", 5236).toInt();
    m_dmDatabaseName = settings.value("Database", "").toString();
    m_dmUsername = settings.value("Username", "").toString();
    m_dmPassword = settings.value("Password", "").toString();
    m_dmSchema = settings.value("Schema", "").toString();
    settings.endGroup();
    settings.beginGroup("LocalDB");
    m_localDBPath = settings.value("path", "").toString();
    settings.endGroup();
}

void MainWindow::saveConfig()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    QSettings settings(configPath, QSettings::IniFormat);

    settings.beginGroup("Database");
    settings.setValue("Host", m_dmHost);
    settings.setValue("Port", m_dmPort);
    settings.setValue("Database", m_dmDatabaseName);
    settings.setValue("Username", m_dmUsername);
    settings.setValue("Password", m_dmPassword);
    settings.setValue("Schema", m_dmSchema);
    settings.endGroup();

    settings.sync();
}

void MainWindow::loadLocalData()
{
//    QString binDir = QCoreApplication::applicationDirPath();
    QDir().mkpath(m_localDBPath);

    QString dbPath = m_localDBPath + "/local_mapping.db";
    if (!m_localDatabase->initialize(dbPath)) {
        QMessageBox::critical(this, "错误", "本地数据库初始化失败: " + m_localDatabase->lastError());
    }
    m_localDatabase->createTable("DP_MSG",m_DPWidget->getTableColHeaders()<<PrimaryKey);
    m_localDatabase->createTable("TP_MSG",m_TPWidget->getTableColHeaders()<<PrimaryKey);

    //    QStringList tables = m_localDatabase->getTableNames();
    loadTableData("TP_MSG",m_TPWidget->getTableWidget());
    loadTableData("DP_MSG",m_DPWidget->getTableWidget());
}

void MainWindow::loadIPC()
{
    QObject::connect(ipc,&QtIpcNotify::sigConnected,[](){
        qDebug()<<"IPC已连接";
    });
    QObject::connect(ipc,&QtIpcNotify::sigMessage,[](const QString& msg){
        qDebug()<<"收到消息："<<msg;
        if(msg == "REFRESH")
        {
            qDebug()<<"执行刷新业务";
        }
    });
    ipc->start();
}

void MainWindow::saveLocalData(const QString &tableName,QList<QMap<QString,QString>> mapList)
{
    //    QList<QMap<QString,QString>> dpList = loadDP();
    if (!m_localDatabase->syncData(tableName, PrimaryKey,mapList)) {
        QMessageBox::critical(this, "错误", "同步数据失败: " + m_localDatabase->lastError());
    }

}

//void MainWindow::saveTPLocalData(QList<QMap<QString, QString> > mapList)
//{
//    if (!m_localDatabase->syncData("TP_MSG", PrimaryKey,mapList)) {
//        QMessageBox::critical(this, "错误", "同步数据失败: " + m_localDatabase->lastError());
//    }
//}

void MainWindow::onConnectDb()
{
    if (m_dmDatabase->isConnected()) {
        //        QMessageBox::information(this, "提示", "已经连接到达梦数据库");
        //        qDebug()<<"已经连接到达梦数据库";
        return;
    }

    if (m_dmHost.isEmpty() || m_dmUsername.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先配置数据库连接参数");
        //数据库配置不开放，自己写死配置文件
        //        onConfigureDb();
        return;
    }

    if (!m_dmDatabase->connect(m_dmHost, m_dmPort, m_dmDatabaseName, m_dmUsername, m_dmPassword, m_dmSchema)) {
        QMessageBox::critical(this, "错误", "连接失败: " + m_dmDatabase->lastError());
        return;
    }

    QString successMsg = "成功连接到达梦数据库";
    if (!m_dmSchema.isEmpty()) {
        successMsg += QString("\n当前模式: %1").arg(m_dmSchema);
    }
    //    QMessageBox::information(this, "成功", successMsg);
    qDebug()<<successMsg;

    QStringList dmTables = m_dmDatabase->getTableNames();

    if (dmTables.isEmpty()) {
        //        QMessageBox::warning(this, "警告", "达梦数据库中没有找到表");
        qDebug()<<"达梦数据库中没有找到表";
        return;
    }
}

void MainWindow::loadTableData(const QString &tableName,QTableWidget* table)
{
    if (tableName.isEmpty()) {
        return;
    }
    QStringList columns = m_localDatabase->getTableColumns(tableName);
    QStringList pkColumns = m_localDatabase->getPrimaryKey(tableName);

    QList<QVariantMap> data = m_localDatabase->getTableData(tableName);
    table->clearContents();
//    table->setRowCount(data.size());
    for (int i = 0,row = 0; i < data.size(); ++i) {
        const QVariantMap &rowData = data[i];

        if(!dataIsValid(tableName,rowData))
        {
            continue;
        }
        table->setRowCount(table->rowCount() + 1);
        //        QVariantMap pkValues;
        QString key = rowData.value(PrimaryKey).toString();
        for(int col = 0;col<table->columnCount();col++)
        {
            QString colName = table->horizontalHeaderItem(col)->text();
            QString value = rowData.value(colName).toString();
            QTableWidgetItem* item = new QTableWidgetItem(value);
            table->setItem(row,col, item);
            item->setData(Qt::UserRole,key);
        }
        row++;
    }

    //    table->resizeColumnsToContents();
    qDebug() << "表数据加载成功:" << tableName << "共" << data.size() << "行"
             << "主键:" << pkColumns.join(",");
}

bool MainWindow::dataIsValid(const QString &tableName, QVariantMap map)
{
    if(tableName == "DP_MSG")
    {
        if(map["PlatformFeature"].toString().isEmpty())
        {
            return false;
        }
    }
    return true;
}

QList<QMap<QString, QString> > MainWindow::loadDP()
{
    QList<QMap<QString, QString> > dpMap;
    onConnectDb();
    if (!m_dmDatabase->isConnected())
    {
        qDebug()<<"数据库未连接";
        return dpMap;
    }
    //寻找定频表2-6g的数据，第一个匹配2-5，后三个分别匹配0-9，再包含6000
//    QString sql = "select * FROM KZCSJDB.QBZB_ZBMB_TXMB_TZCS_DZBPP where REGEXP_LIKE(DZBPL,'(^|;)([2-5][0-9]{3}|6000)(;|$)');";
    //2026.08.03 临时修改：因为没有2-6G数据，查询6000以下的频率
    QString sql = "select * FROM KZCSJDB.QBZB_ZBMB_TXMB_TZCS_DZBPP where DZBPL <= '6000';";
    QList<QVariantMap> dpKeys = m_dmDatabase->readTableDataForSqlOrder(sql);
    for(QVariantMap keys : dpKeys)
    {
        QMap<QString, QString> mapTemp;
        QString TZCSXH = keys["TZCSXH"].toString();
        sql = QString("select * FROM KZCSJDB.QBZB_ZBMB_TXMB_TZCS_DZBTZ where TZCSXH='%1';").arg(TZCSXH);
        QList<QVariantMap> DZBTZList = m_dmDatabase->readTableDataForSqlOrder(sql);
        if(DZBTZList.count() > 0)
        {
            QStringList TZYSList = DZBTZList[0]["TZYS"].toString().split(",");
            QString FHSL = DZBTZList[0]["FHSL"].toString();
            QString TZYS;
            for(QString TZYSVar : TZYSList)
            {
                if(TZYS.count() > 0)
                {
                    TZYS += ";";
                }
                sql = QString("select * FROM KZCSJDB.ZZBZ_S_XZ_ZJZB_XHTZYS where XHTZYSNM='%1';").arg(TZYSVar);
                QList<QVariantMap> list = m_dmDatabase->readTableDataForSqlOrder(sql);
                if(list.count() <= 0)
                {
                    TZYS += TZYSVar;
                    continue;
                }
                TZYS += list[0]["ASCDM"].toString();
            }
            mapTemp["Modulation"] = TZYS;
            mapTemp["SymbolRate"] = FHSL;
        }
        sql = QString("select * FROM KZCSJDB.QBZB_ZBMB_TXMB_TZCS_DZBPP where TZCSXH='%1';").arg(TZCSXH);
        QList<QVariantMap> DZPBBList = m_dmDatabase->readTableDataForSqlOrder(sql);
        if(DZPBBList.count() > 0)
        {
            QString DZBPL = DZPBBList[0]["DZBPL"].toString();
            mapTemp["Frequency"] = DZBPL;
        }

        sql = QString("select * FROM KZCSJDB.QBZB_ZBMB_TXMB_TZCS where TZCSXH='%1';").arg(TZCSXH);
        QList<QVariantMap> TZCSList = m_dmDatabase->readTableDataForSqlOrder(sql);
        QString TXMBID = TZCSList[0]["TXMBID"].toString();
        mapTemp[PrimaryKey] = TZCSXH;
        mapTemp["SBID"] = TXMBID;
        QStringList TXTZList = TZCSList[0]["TXTZ"].toString().split(";");
        QString MC;
        for(QString var : TXTZList)
        {
            int TXTZ = var.toInt();
            sql = QString("select * FROM KZCSJDB.ZZBZ_S_XZ_ZJZB_CSXTDL where CSXTDLNM='%1';").arg(TXTZ);
            QList<QVariantMap> CSXTDLNMList = m_dmDatabase->readTableDataForSqlOrder(sql);
            MC += CSXTDLNMList[0]["MC"].toString();
        }
        mapTemp["CharacterFeature"] = MC;
        //找不到MBID这条数据就接不下去了
        if(TZCSList.count() <= 0)
        {
            continue;
        }
        sql = QString("select * FROM KZCSJDB.QBCL_QBZB_XHSB where SBID='%1';").arg(TXMBID);
        //信号识别表
        QList<QVariantMap> XHSBList = m_dmDatabase->readTableDataForSqlOrder(sql);
        if(XHSBList.count() > 0)
        {
            QVariantMap XHSBMap = XHSBList[0];
            QString GJDQNM = XHSBMap["GJDQNM"].toString();
            QString SBXH = XHSBMap["SBXH"].toString();
            int WXCDNM = XHSBMap["WXCDNM"].toInt();
            QString strWXCDNM = QString("%1").arg(WXCDNM,2,10,QChar('0'));
            mapTemp["Contry"] = GJDQNM;
            mapTemp["DeviceName"] = SBXH;
            mapTemp["ThreatLevel"] = strWXCDNM;
            sql = QString("select MC FROM KZCSJDB.ZZBZ_S_TY_GJ where GJDQNM='%1';").arg(GJDQNM);
            //信号识别表
            QList<QVariantMap> GJDQNMTrans = m_dmDatabase->readTableDataForSqlOrder(sql);
            if(GJDQNMTrans.count() > 0)
            {
                mapTemp["Contry"] = GJDQNMTrans[0]["MC"].toString();
            }
            sql = QString("select MC FROM KZCSJDB.ZZBZ_S_XZ_QB_MBWXCD where WXCDNM='%1';").arg(strWXCDNM);
            QList<QVariantMap> WXCDList = m_dmDatabase->readTableDataForSqlOrder(sql);
            if(WXCDList.count() > 0)
            {
                mapTemp["ThreatLevel"] = WXCDList[0]["MC"].toString();
            }
        }
        sql = QString("select * FROM KZCSJDB.QBCL_QBZB_DZMB where MBID='%1';").arg(TXMBID);
        //平台信息
        QList<QVariantMap> PTXXList = m_dmDatabase->readTableDataForSqlOrder(sql);
        if(PTXXList.count() > 0)
        {
            QVariantMap PTXXMap = PTXXList[0];
            int DZMBLBNM = PTXXMap["DZMBLBNM"].toInt();
            QString MBMC = PTXXMap["MBMC"].toString();
            mapTemp["Platform"] += getStr(DZMBLBNM,MBMC);

        }

        sql = QString("select * from QBZB_ZBMB_TXMB_GZCS        where TXMBID = '%1';").arg(TXMBID);
        QList<QVariantMap> GZCSXHList = m_dmDatabase->readTableDataForSqlOrder(sql);
        for(QVariantMap map : GZCSXHList)
        {
            QString GZCSXH = map["GZCSXH"].toString();
            sql = QString("select * from KZCSJDB.QBZB_GX_PTMBPBTXSBQK where TXSBDXGZCSID = '%1';").arg(GZCSXH);
            QList<QVariantMap> PTMBIDList = m_dmDatabase->readTableDataForSqlOrder(sql);
            for(QVariantMap PTMTIDmap : PTMBIDList)
            {
                QString PTMBID = PTMTIDmap["PTMBID"].toString();
                if(PTMBID.isEmpty())
                {
                    continue;
                }
                sql = QString("select * from KZCSJDB.QBCL_QBZB_DZMB where MBID = '%1';").arg(PTMBID);
                QList<QVariantMap> PTList = m_dmDatabase->readTableDataForSqlOrder(sql);
                for(QVariantMap varmap : PTList)
                {
                    int DZMBLBNM = varmap["DZMBLBNM"].toInt();
                    QString MBMC = varmap["MBMC"].toString();
                    mapTemp["Platform"] += getStr(DZMBLBNM,MBMC);
                }
            }
        }

        dpMap.push_back(mapTemp);
    }
    return dpMap;
}
# if 0
QList<QMap<QString, QString> > MainWindow::loadTP()
{
    QList<QMap<QString, QString> > dpMap;
    onConnectDb();
    if (!m_dmDatabase->isConnected())
    {
        qDebug()<<"数据库未连接";
        return dpMap;
    }
    // 寻找定频表2-6g的数据，第一个匹配2-5，后三个分别匹配0-9，再包含6000
    QString sql = R"(WITH split_data AS(
                  select TZCSXH,TPPL,REGEXP_SUBSTR(TPPL,'[^;]+',1,LEVEL)
                  AS seg
                  FROM KZCSJDB.QBZB_ZBMB_TXMB_TZCS_TPPLXX
                  WHERE TPPL IS NOT NULL
                  CONNECT BY LEVEL <= REGEXP_COUNT(TPPL,';') + 1
                  AND PRIOR TZCSXH = TZCSXH
                  AND PRIOR DBMS_RANDOM.VALUE IS NOT NULL
                  )
                  SELECT DISTINCT TZCSXH,TPPL
                  FROM split_data
                  WHERE REGEXP_LIKE(seg,'(^|;)([2-5][0-9]{3}|6000)(;|$)')
                  OR
                  (REGEXP_LIKE(seg,'^[0-9]+[-~][0-9]+$')
                  AND TO_NUMBER(REGEXP_SUBSTR(seg,'[0-9]+',1,1))<=6000
                  AND TO_NUMBER(REGEXP_SUBSTR(seg,'[0-9]+',1,2))>=2000);)";
    QList<QVariantMap> dpKeys = m_dmDatabase->readTableDataForSqlOrder(sql);
    for(QVariantMap keys : dpKeys)
    {
        QMap<QString, QString> mapTemp;
        QString TZCSXH = keys["TZCSXH"].toString();
        mapTemp[PrimaryKey] = TZCSXH;
        sql = QString("select * FROM KZCSJDB.QBZB_ZBMB_TXMB_TZCS_TPTZ where TZCSXH='%1';").arg(TZCSXH);
        QList<QVariantMap> TZYSList = m_dmDatabase->readTableDataForSqlOrder(sql);
        if(TZYSList.count() > 0)
        {
            QVariantMap map = TZYSList[0];
            QString TZYS = map["TZYS"].toString();
            QString FHSL = map["FHSL"].toString();
            QString TS = map["TS"].toString();
            QString ZLSJ = map["ZLSJ"].toString();
            QString ZKB = map["ZKB"].toString();
            mapTemp["TZYS"] = TZYS;
            mapTemp["FHSL"] = FHSL;
            mapTemp["TS"] = TS;
            mapTemp["ZLSJ"] = ZLSJ;
            mapTemp["ZKB"] = ZKB;
        }
        sql = QString("select * FROM KZCSJDB.QBZB_ZBMB_TXMB_TZCS_TPPLXX where TZCSXH='%1';").arg(TZCSXH);
        QList<QVariantMap> TPPLList = m_dmDatabase->readTableDataForSqlOrder(sql);
        if(TPPLList.count() > 0)
        {
            QString TPPL = TPPLList[0]["TPPL"].toString();
            mapTemp["TPPL"] = TPPL;
        }

        sql = QString("select * FROM KZCSJDB.QBZB_ZBMB_TXMB_TZCS where TZCSXH='%1';").arg(TZCSXH);
        QList<QVariantMap> TZCSList = m_dmDatabase->readTableDataForSqlOrder(sql);
        QString TXMBID = TZCSList[0]["TXMBID"].toString();
        mapTemp["SBID"] = TXMBID;
        QString TXTZ = TZCSList[0]["TXTZ"].toString();
        //找不到MBID这条数据就接不下去了
        if(TZCSList.count() <= 0)
        {
            continue;
        }
        sql = QString("select * FROM KZCSJDB.QBCL_QBZB_XHSB where SBID='%1';").arg(TXMBID);
        //信号识别表
        QList<QVariantMap> XHSBList = m_dmDatabase->readTableDataForSqlOrder(sql);
        if(XHSBList.count() > 0)
        {
            QVariantMap XHSBMap = XHSBList[0];
            QString GJDQNM = XHSBMap["GJDQNM"].toString();
            QString SBXH = XHSBMap["SBXH"].toString();
            QString SBMC = XHSBMap["SBMC"].toString();
            QString WXCDNM = XHSBMap["WXCDNM"].toString();
            QString MBID = XHSBMap["MBID"].toString();
            mapTemp["GJDQNM"] = GJDQNM;
            mapTemp["SBXH"] = SBXH;
            mapTemp["SBMC"] = SBMC;
            mapTemp["WXCDNM"] = WXCDNM;
            mapTemp["MBID"] = MBID;
            sql = QString("select MC FROM KZCSJDB.ZZBZ_S_TY_GJ where GJDQNM='%1';").arg(GJDQNM);
            //信号识别表
            QList<QVariantMap> GJDQNMTrans = m_dmDatabase->readTableDataForSqlOrder(sql);
            if(GJDQNMTrans.count() > 0)
            {
                mapTemp["GJDQNM"] = GJDQNMTrans[0]["MC"].toString();
            }
        }
        sql = QString("select * FROM KZCSJDB.QBCL_QBZB_DZMB where MBID='%1';").arg(TXMBID);
        //平台信息
        QList<QVariantMap> PTXXList = m_dmDatabase->readTableDataForSqlOrder(sql);
        if(PTXXList.count() > 0)
        {
            QVariantMap PTXXMap = PTXXList[0];
            QString MBMC = PTXXMap["MBMC"].toString();
            mapTemp["MBMC"] = MBMC;
        }
        dpMap.push_back(mapTemp);
    }
    return dpMap;
}
#else
QList<QMap<QString, QString> > MainWindow::loadTP()
{
    QList<QMap<QString, QString> > dpMap;
    onConnectDb();
    if (!m_dmDatabase->isConnected())
    {
        qDebug()<<"数据库未连接";
        return dpMap;
    }
    QString sql = "select * FROM KZCSJDB.QBZB_ZBMB_TXMB_TZCS_TPPLXX;";
    QList<QVariantMap> TPPLList = m_dmDatabase->readTableDataForSqlOrder(sql);
    for(QVariantMap TPPLMap : TPPLList)
    {
        QMap<QString, QString> mapTemp;
        QString TZCSXH = TPPLMap["TZCSXH"].toString();
        QString TPPL = TPPLMap["TPPL"].toString();
        TPPLMap["Frequency"] = TPPL;
        sql = QString("select * FROM KZCSJDB.QBZB_ZBMB_TXMB_TZCS_TPTZ where TZCSXH='%1';").arg(TZCSXH);
        QList<QVariantMap> TPTZList = m_dmDatabase->readTableDataForSqlOrder(sql);
        if(TPTZList.count() > 0)
        {
            QStringList TZYSList = TPTZList[0]["TZYS"].toString().split(",");
            QString FHSL = TPTZList[0]["FHSL"].toString();
            QString TZYS;
            for(QString TZYSVar : TZYSList)
            {
                if(TZYS.count() > 0)
                {
                    TZYS += ";";
                }
                sql = QString("select * FROM KZCSJDB.ZZBZ_S_XZ_ZJZB_XHTZYS where XHTZYSNM='%1';").arg(TZYSVar);
                QList<QVariantMap> list = m_dmDatabase->readTableDataForSqlOrder(sql);
                if(list.count() <= 0)
                {
                    TZYS += TZYSVar;
                    continue;
                }
                TZYS += list[0]["ASCDM"].toString();
            }
            mapTemp["Modulation"] = TZYS;
            mapTemp["SymbolRate"] = FHSL;
        }

        sql = QString("select * FROM KZCSJDB.QBZB_ZBMB_TXMB_TZCS where TZCSXH='%1';").arg(TZCSXH);
        QList<QVariantMap> TZCSList = m_dmDatabase->readTableDataForSqlOrder(sql);
        QString TXMBID = TZCSList[0]["TXMBID"].toString();
        mapTemp[PrimaryKey] = TZCSXH;
        mapTemp["SBID"] = TXMBID;
        QStringList TXTZList = TZCSList[0]["TXTZ"].toString().split(";");
        QString MC;
        for(QString var : TXTZList)
        {
            int TXTZ = var.toInt();
            sql = QString("select * FROM KZCSJDB.ZZBZ_S_XZ_ZJZB_CSXTDL where CSXTDLNM='%1';").arg(TXTZ);
            QList<QVariantMap> CSXTDLNMList = m_dmDatabase->readTableDataForSqlOrder(sql);
            MC += CSXTDLNMList[0]["MC"].toString();
        }
        mapTemp["CharacterFeature"] = MC;
        //找不到MBID这条数据就接不下去了
        if(TZCSList.count() <= 0)
        {
            continue;
        }
        sql = QString("select * FROM KZCSJDB.QBCL_QBZB_XHSB where SBID='%1';").arg(TXMBID);
        //信号识别表
        QList<QVariantMap> XHSBList = m_dmDatabase->readTableDataForSqlOrder(sql);
        if(XHSBList.count() > 0)
        {
            QVariantMap XHSBMap = XHSBList[0];
            QString GJDQNM = XHSBMap["GJDQNM"].toString();
            QString SBXH = XHSBMap["SBXH"].toString();
            int WXCDNM = XHSBMap["WXCDNM"].toInt();
            QString strWXCDNM = QString("%1").arg(WXCDNM,2,10,QChar('0'));
            mapTemp["Contry"] = GJDQNM;
            mapTemp["DeviceName"] = SBXH;
            mapTemp["ThreatLevel"] = strWXCDNM;
            sql = QString("select MC FROM KZCSJDB.ZZBZ_S_TY_GJ where GJDQNM='%1';").arg(GJDQNM);
            //信号识别表
            QList<QVariantMap> GJDQNMTrans = m_dmDatabase->readTableDataForSqlOrder(sql);
            if(GJDQNMTrans.count() > 0)
            {
                mapTemp["Contry"] = GJDQNMTrans[0]["MC"].toString();
            }
            sql = QString("select MC FROM KZCSJDB.ZZBZ_S_XZ_QB_MBWXCD where WXCDNM='%1';").arg(strWXCDNM);
            QList<QVariantMap> WXCDList = m_dmDatabase->readTableDataForSqlOrder(sql);
            if(WXCDList.count() > 0)
            {
                mapTemp["ThreatLevel"] = WXCDList[0]["MC"].toString();
            }
        }
        sql = QString("select * FROM KZCSJDB.QBCL_QBZB_DZMB where MBID='%1';").arg(TXMBID);
        //平台信息
        QList<QVariantMap> PTXXList = m_dmDatabase->readTableDataForSqlOrder(sql);
        if(PTXXList.count() > 0)
        {
            QVariantMap PTXXMap = PTXXList[0];
            int DZMBLBNM = PTXXMap["DZMBLBNM"].toInt();
            QString MBMC = PTXXMap["MBMC"].toString();
            mapTemp["Platform"] += getStr(DZMBLBNM,MBMC);

        }

        sql = QString("select * from QBZB_ZBMB_TXMB_GZCS        where TXMBID = '%1';").arg(TXMBID);
        QList<QVariantMap> GZCSXHList = m_dmDatabase->readTableDataForSqlOrder(sql);
        for(QVariantMap map : GZCSXHList)
        {
            QString GZCSXH = map["GZCSXH"].toString();
            sql = QString("select * from KZCSJDB.QBZB_GX_PTMBPBTXSBQK where TXSBDXGZCSID = '%1';").arg(GZCSXH);
            QList<QVariantMap> PTMBIDList = m_dmDatabase->readTableDataForSqlOrder(sql);
            for(QVariantMap PTMTIDmap : PTMBIDList)
            {
                QString PTMBID = PTMTIDmap["PTMBID"].toString();
                if(PTMBID.isEmpty())
                {
                    continue;
                }
                sql = QString("select * from KZCSJDB.QBCL_QBZB_DZMB where MBID = '%1';").arg(PTMBID);
                QList<QVariantMap> PTList = m_dmDatabase->readTableDataForSqlOrder(sql);
                for(QVariantMap varmap : PTList)
                {
                    int DZMBLBNM = varmap["DZMBLBNM"].toInt();
                    QString MBMC = varmap["MBMC"].toString();
                    mapTemp["Platform"] += getStr(DZMBLBNM,MBMC);
                }
            }
        }

        dpMap.push_back(mapTemp);
    }
    return dpMap;
}
#endif
void MainWindow::updteTable(QString localDBTableName,  QTableWidget *table)
{
    //    table->clearContents();
    //    table->setRowCount(0);

    QList<QVariantMap> mapList = m_localDatabase->getTableData(localDBTableName);
    QMap<QVariant,int> key2Row;
    for(int r = 0;r<table->rowCount();r++)
    {
        QTableWidgetItem* keyItem = table->item(r,0);
        if(!keyItem)
        {
            continue;
        }
        QVariant uid = keyItem->data(Qt::UserRole);
        if(!uid.isNull())
        {
            key2Row[uid] = r;
        }
    }
    for(const QVariantMap& map : mapList)
    {
        if(!dataIsValid(localDBTableName,map))
        {
            continue;
        }
        QVariant dbUid = map[PrimaryKey];
        if(dbUid.isNull())
        {
            continue;
        }
        int row = 0;
        if(key2Row.contains(dbUid))
        {
            row = key2Row[dbUid];
        }
        else
        {
            row = table->rowCount();
            QString key = map[PrimaryKey].toString();
            onAddLine(table,localDBTableName,key);
        }

        for(int col = 0;col<table->columnCount();col++)
        {
            QString header = table->horizontalHeaderItem(col)->text();
            QString newVar = map[header].toString();
            if(table->item(row,col)->text() != newVar)
            {
                table->item(row,col)->setText(newVar);
            }
        }

    }
}


void MainWindow::onAddLine(QTableWidget *table, const QString &tableName,QString key)
{
    //    QTableWidget* table = m_DPWidget->getTableWidget();
    int row = table->rowCount();
    table->setRowCount(row + 1);
    QList<QMap<QString,QString> > mapList;
    if(key.isEmpty())
    {
        key = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
    }
    for(int i = 0;i<table->columnCount();i++)
    {
        QTableWidgetItem* item = new QTableWidgetItem;
        item->setData(Qt::UserRole,key);
        table->setItem(row,i,item);
    }
    QMap<QString,QString> map;
    map.insert(PrimaryKey,key);
    mapList.push_back(map);
    saveLocalData(tableName,mapList);
    return;
}

void MainWindow::onRemoveLine(QTableWidget *table, const QString &tableName)
{
    QSet<int> rowSet;
    for(QTableWidgetItem* item : table->selectedItems())
    {
        rowSet.insert(item->row());
    }
    QList<int> rows = rowSet.values();
    std::sort(rows.rbegin(),rows.rend());
    for(int row : rows)
    {
        QTableWidgetItem* item = table->item(row,0);
        if(item)
        {
            m_localDatabase->dropData(tableName,PrimaryKey,QStringList()<<item->data(Qt::UserRole).toString());
        }
        table->removeRow(row);

    }
    ipc->publish("REFRESH");
}

void MainWindow::onUpdateTable(int row, int column, const QString &oldVal, const QString &newVal, QTableWidget *tableWidget, const QString &tableName)
{
    QTableWidgetItem *item = tableWidget->item(row, column);
    if (!item) {
        return;
    }
    if(QMessageBox::Yes != QMessageBox::question(this,"Hint","Data will be update,Are you continue?",QMessageBox::Yes,QMessageBox::No))
    {
        item->setText(oldVal);
        return;
    }

    QString key = item->data(Qt::UserRole).toString();
    QString columnName = tableWidget->horizontalHeaderItem(column)->text();
    QVariant value = item->text();
    QVariantMap pkValues;
    pkValues.insert(PrimaryKey,key);
    if (m_localDatabase->updateCell(tableName, columnName, pkValues, value)) {
        qDebug() << "单元格数据已保存:" << tableName << columnName << pkValues << value;
    } else {
        QMessageBox::warning(this, "警告", "保存失败: " + m_localDatabase->lastError());
        loadTableData(tableName,tableWidget);
    }
    ipc->publish("REFRESH");
}

void MainWindow::onPasteTable(CenterWidget *cenWidget, const QString &tableName)
{
    QTableWidget *table = cenWidget->getTableWidget();
    QItemSelectionModel* selModel = table->selectionModel();
    QModelIndexList idxList = selModel->selectedIndexes();
    QSet<int> rowSet;
    for(const auto& idx : idxList)
    {
        rowSet.insert(idx.row());
    }
    QList<int> rows = rowSet.values();

    //如果是未找到选中行的话，那么直接追加复制行数和内容
    if(rows.count() <= 0)
    {
        for(QMap<int,QString> map:cenWidget->getCacheData())
        {
            int row = table->rowCount();
            onAddLine(table,tableName);
            //            table->item(row,col);
            for(auto it = map.begin();it!=map.end();it++)
            {
                table->item(row,it.key())->setText(it.value());
            }
        }
    }
    else
    {
        int count = qMin(rows.count(),cenWidget->getCacheData().count());
        for(int i = 0;i<count;i++)
        {
            int row = rows[i];
            QMap<int,QString> map = cenWidget->getCacheData()[i];
            for(auto it = map.begin();it!=map.end();it++)
            {
                table->item(row,it.key())->setText(it.value());
            }
        }
    }
}

QString MainWindow::getStr(int number, QString str)
{
    return QString("\"%1\"\"%2\";").arg(number).arg(str);
}

QMap<int, QStringList> MainWindow::parseStrToMapList(const QString &src)
{
    QMap<int, QStringList> result;
    if(src.isEmpty())
    {
        return result;
    }
    QRegularExpression re("\"([^\"]+)\"\"([^\"]+)\";");
    QRegularExpressionMatchIterator iter = re.globalMatch(src);
    while(iter.hasNext())
    {
        auto match = iter.next();
        int keyStr = match.captured(1).toInt();
        QString val = match.captured(2);
        result[keyStr].push_back(val);
    }
    return result;
}

QString MainWindow::extractFreqOnly(const QString &text)
{
    QString str = text;
    QRegularExpression re(R"(^([0-9.]+))");
    auto match = re.match(text.trimmed());
    if(match.hasMatch())
    {
        str = match.captured(1);
    }
    return str;
}

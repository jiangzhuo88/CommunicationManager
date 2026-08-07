#include "CenterWidget.h"
#include "ui_CenterWidget.h"
#include "TableEditDelegate.h"
#include <QMenu>
CenterWidget::CenterWidget(QStringList dbHeaders, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CenterWidget)
{
    ui->setupUi(this);
    auto *del = new TableEditDelegate(this);
    ui->tableWidget->setItemDelegate(del);
    ui->tabWidgetLibrary->tabBar()->setObjectName("tabWidgetLibraryBar");
    ui->tableWidget->setColumnCount(dbHeaders.count());
    ui->tableWidget->setHorizontalHeaderLabels(dbHeaders);
//    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->installEventFilter(this);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(ui->tableWidget,&QTableWidget::customContextMenuRequested,this,&CenterWidget::slotTableRightMenu);
    connect(del,&TableEditDelegate::userCellEdited,this,&CenterWidget::userCellEdited);
}

CenterWidget::~CenterWidget()
{
    delete ui;
}

QTableWidget *CenterWidget::getTableWidget()
{
    return ui->tableWidget;
}

QStringList CenterWidget::getTableColHeaders()
{
    QStringList list;
    for(int i = 0;i<ui->tableWidget->columnCount();i++)
    {
        QString colName = ui->tableWidget->horizontalHeaderItem(i)->text();
        colName = extractFreqOnly(colName);
        list<<colName;
    }
    return list;
}

QList<QMap<int, QString> > CenterWidget::getCacheData()
{
    return mapList;
}

void CenterWidget::clear()
{
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
}

void CenterWidget::slotTableRightMenu(const QPoint &pos)
{
    QMenu menu(this);
//    QTableWidgetItem* clickItem = ui->tableWidget->itemAt(pos);
//    int clickRow = -1;
//    if(clickItem)
//    {
//        clickRow = clickItem->row();
//    }
    QAction *addAction = menu.addAction("添加行");
    QAction *removeAction = menu.addAction("删除行");
    QAction *copyAction = menu.addAction("复制行");
    QAction *pasteAction = menu.addAction("粘贴行");
    QAction* selectedAct = menu.exec(ui->tableWidget->viewport()->mapToGlobal(pos));
    if(!selectedAct)
    {
        return;
    }
    if(selectedAct == addAction)
    {
        emit sigAddALine();
    }
    else if(selectedAct == removeAction)
    {
        emit sigRemoveALine();
    }
    //复制行
    else if(selectedAct == copyAction)
    {
//        emit sigRemoveALine();
        QItemSelectionModel* selModel = ui->tableWidget->selectionModel();
        QModelIndexList idxList = selModel->selectedIndexes();
        QSet<int> rowSet;
        for(const auto& idx : idxList)
        {
            rowSet.insert(idx.row());
        }
        QList<int> rows = rowSet.values();
        std::sort(rows.begin(),rows.end());
        int colCnt = ui->tableWidget->columnCount();
        mapList.clear();
        for(int r : rows)
        {
//            RowData
            QMap<int,QString> map;
            for(int c = 0;c<colCnt;c++)
            {
                QTableWidgetItem* it = ui->tableWidget->item(r,c);
//                QString headName = ui->tableWidget->horizontalHeaderItem(c)->text();
                QString text;
//                QVariant userData;
                if(it)
                {
                    text = it->text();
//                    userData = it->data(Qt::UserRole);
                }
                map[c] = text;
            }
            mapList.push_back(map);
        }
        ui->tableWidget->clearSelection();
    }
    else if(selectedAct == pasteAction)
    {
        emit sigPasteLine();
    }
}

void CenterWidget::initAutoSize()
{
    if(ui->tableWidget == nullptr)
    {
        return;
    }
//    m_table->resizeColumnsToContents();
//    QHeaderView*header = m_table->horizontalHeader();
//    header->setSectionResizeMode(QHeaderView::Interactive);
    int displayCol = 0;
    for(int i = 0;i<ui->tableWidget->columnCount();i++)
    {
        displayCol+= ui->tableWidget->columnWidth(i);
    }
//    qDebug()<<"m_table->width():"<<m_table->width()<<" displayCol:"<<displayCol;
    float width = ui->tableWidget->width() - ui->tableWidget->columnCount();
    float proportion = displayCol == 0? 0.0:width / displayCol;
    int defaultCol = ui->tableWidget->width() / ui->tableWidget->columnCount();
    int columnCount = ui->tableWidget->columnCount();
    int occupiedLen = 0;
    for(int i = 0;i<columnCount;i++)
    {
        if(proportion != 0)
        {
            defaultCol = ui->tableWidget->columnWidth(i) * proportion;
        }
        if(i == (columnCount-1))
        {
            defaultCol = width - occupiedLen;
        }
        ui->tableWidget->setColumnWidth(i,defaultCol);
        occupiedLen+=defaultCol;
    }

}

bool CenterWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->tableWidget && event->type() == QEvent::Resize)
    {
//        handleResizeEvent(static_cast<QResizeEvent*>(event));
        initAutoSize();
    }
    return QObject::eventFilter(watched,event);
}

QString CenterWidget::extractFreqOnly(const QString &text)
{
    QString str = text;
    QRegularExpression re(R"(^(.*?)\()");
    auto match = re.match(text.trimmed());
    if(match.hasMatch())
    {
        str = match.captured(1);
    }
    return str;
}

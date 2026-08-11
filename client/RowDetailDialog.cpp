#include "RowDetailDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

#define PK_COL "SBID"

RowDetailDialog::RowDetailDialog(const QString& tableName, const QString& pkValue,
                                 LocalDatabase* db, QWidget* parent)
    : CDialogBase(parent)
    , m_tableName(tableName)
    , m_pkValue(pkValue)
    , m_db(db)
    , m_subTable(nullptr)
    , m_platformEqEdit(nullptr)
{
    // 参数字段：主表中除子表字段、PlatformEQ、SBID 之外的列
    m_paramFields << "Contry" << "DeviceName" << "ThreatLevel"
                  << "PlatformName" << "CharacterFeature";

    setWindowTitleText("Row Detail Editor");
    buildAndInitContent();
    loadData();
    initSize();
}

void RowDetailDialog::buildAndInitContent()
{
    QWidget* content = contentsWidget();
    content->setMinimumSize(700, 600);

    QVBoxLayout* mainLay = new QVBoxLayout(content);
    mainLay->setContentsMargins(12, 12, 12, 12);
    mainLay->setSpacing(10);

    // ========== 上部分：参数 ==========
    QGroupBox* paramGroup = new QGroupBox("Parameters");
    QFormLayout* paramLay = new QFormLayout(paramGroup);
    paramLay->setLabelAlignment(Qt::AlignRight);
    for (const QString& field : m_paramFields) {
        QLineEdit* edit = new QLineEdit;
        m_paramEdits[field] = edit;
        paramLay->addRow(field + ":", edit);
    }
    mainLay->addWidget(paramGroup);

    // ========== 中部分：子表 ==========
    QGroupBox* subGroup = new QGroupBox("Characteristic Parameters (Sub Table)");
    QVBoxLayout* subLay = new QVBoxLayout(subGroup);

    // 按钮行
    QHBoxLayout* subBtnLay = new QHBoxLayout();
    QPushButton* btnAddRow = new QPushButton("Add Row");
    QPushButton* btnRemoveRow = new QPushButton("Remove Row");
    subBtnLay->addWidget(btnAddRow);
    subBtnLay->addWidget(btnRemoveRow);
    subBtnLay->addStretch();
    subLay->addLayout(subBtnLay);

    m_subTable = new QTableWidget();
    m_subTable->setColumnCount(3);
    m_subTable->setHorizontalHeaderLabels(QStringList() << "Frequency" << "Modulation" << "SymbolRate");
    m_subTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_subTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    subLay->addWidget(m_subTable);

    mainLay->addWidget(subGroup);

    // ========== 下部分：PlatformEQ ==========
    QGroupBox* eqGroup = new QGroupBox("Platform EQ");
    QVBoxLayout* eqLay = new QVBoxLayout(eqGroup);
    m_platformEqEdit = new QPlainTextEdit();
    m_platformEqEdit->setMaximumBlockCount(0);
    eqLay->addWidget(m_platformEqEdit);
    mainLay->addWidget(eqGroup);

    // ========== 底部按钮 ==========
    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->addStretch();
    QPushButton* btnConfirm = new QPushButton("Confirm");
    QPushButton* btnCancel = new QPushButton("Cancel");
    btnConfirm->setMinimumWidth(80);
    btnCancel->setMinimumWidth(80);
    btnLay->addWidget(btnConfirm);
    btnLay->addWidget(btnCancel);
    mainLay->addLayout(btnLay);

    // 信号连接
    connect(btnConfirm, &QPushButton::clicked, this, &RowDetailDialog::onConfirm);
    connect(btnCancel, &QPushButton::clicked, this, &RowDetailDialog::onCancel);
    connect(btnAddRow, &QPushButton::clicked, this, &RowDetailDialog::onAddSubRow);
    connect(btnRemoveRow, &QPushButton::clicked, this, &RowDetailDialog::onRemoveSubRow);
}

void RowDetailDialog::loadData()
{
    if (!m_db || m_pkValue.isEmpty()) {
        return;
    }

    // --- 读取主表行数据 ---
    QString mainSql = QString("SELECT * FROM %1 WHERE %2 = ?").arg(m_tableName).arg(PK_COL);
    QList<QVariantMap> mainRows = m_db->queryRows(mainSql, QVariantList() << m_pkValue);
    if (mainRows.isEmpty()) {
        qWarning() << "RowDetailDialog: 主表无此记录:" << m_tableName << m_pkValue;
        return;
    }

    QVariantMap mainRow = mainRows[0];

    // 填充参数
    for (const QString& field : m_paramFields) {
        if (m_paramEdits[field]) {
            m_paramEdits[field]->setText(mainRow.value(field).toString());
        }
    }

    // 填充 PlatformEQ
    m_platformEqEdit->setPlainText(mainRow.value("PlatformEQ").toString());

    // --- 读取子表数据 ---
    QString subTable = m_tableName + "_SUB";
    QString subSql = QString("SELECT * FROM %1 WHERE %2 = ?").arg(subTable).arg(PK_COL);
    QList<QVariantMap> subRows = m_db->queryRows(subSql, QVariantList() << m_pkValue);

    m_subTable->setRowCount(subRows.size());
    for (int i = 0; i < subRows.size(); ++i) {
        const QVariantMap& row = subRows[i];
        m_subTable->setItem(i, 0, new QTableWidgetItem(row.value("Frequency").toString()));
        m_subTable->setItem(i, 1, new QTableWidgetItem(row.value("Modulation").toString()));
        m_subTable->setItem(i, 2, new QTableWidgetItem(row.value("SymbolRate").toString()));
    }

    if (subRows.isEmpty()) {
        // 至少显示一行空行
        m_subTable->setRowCount(1);
        for (int c = 0; c < 3; ++c) {
            m_subTable->setItem(0, c, new QTableWidgetItem(""));
        }
    }
}

void RowDetailDialog::onAddSubRow()
{
    int row = m_subTable->rowCount();
    m_subTable->setRowCount(row + 1);
    for (int c = 0; c < 3; ++c) {
        m_subTable->setItem(row, c, new QTableWidgetItem(""));
    }
}

void RowDetailDialog::onRemoveSubRow()
{
    QSet<int> rowSet;
    for (QTableWidgetItem* item : m_subTable->selectedItems()) {
        rowSet.insert(item->row());
    }
    QList<int> rows = rowSet.values();
    std::sort(rows.rbegin(), rows.rend());
    for (int r : rows) {
        m_subTable->removeRow(r);
    }
}

void RowDetailDialog::saveData()
{
    // --- 1. 更新主表参数字段 ---
    QVariantMap pkValues;
    pkValues.insert(PK_COL, m_pkValue);

    for (const QString& field : m_paramFields) {
        if (m_paramEdits[field]) {
            m_db->updateCell(m_tableName, field, pkValues, m_paramEdits[field]->text());
        }
    }

    // --- 2. 更新 PlatformEQ ---
    m_db->updateCell(m_tableName, "PlatformEQ", pkValues, m_platformEqEdit->toPlainText());

    // --- 3. 重建子表数据 ---
    QString subTable = m_tableName + "_SUB";

    // 删除旧子表记录
    m_db->dropData(subTable, PK_COL, QStringList() << m_pkValue);

    // 收集子表行数据
    QStringList freqs, mods, syms;
    QString insertSql = QString("INSERT INTO %1 (%2, Frequency, Modulation, SymbolRate) "
                                "VALUES (?, ?, ?, ?)").arg(subTable).arg(PK_COL);

    for (int r = 0; r < m_subTable->rowCount(); ++r) {
        QString freq = m_subTable->item(r, 0) ? m_subTable->item(r, 0)->text().trimmed() : "";
        QString mod  = m_subTable->item(r, 1) ? m_subTable->item(r, 1)->text().trimmed() : "";
        QString sym  = m_subTable->item(r, 2) ? m_subTable->item(r, 2)->text().trimmed() : "";

        // 跳过全空行
        if (freq.isEmpty() && mod.isEmpty() && sym.isEmpty()) {
            continue;
        }

        m_db->executeSql(insertSql, QVariantList() << m_pkValue << freq << mod << sym);

        if (!freq.isEmpty()) freqs << freq;
        if (!mod.isEmpty())  mods  << mod;
        if (!sym.isEmpty())  syms  << sym;
    }

    // --- 4. 更新主表中的拼接字段 ---
    m_db->updateCell(m_tableName, "Frequency", pkValues, freqs.join(";"));
    m_db->updateCell(m_tableName, "Modulation", pkValues, mods.join(";"));
    m_db->updateCell(m_tableName, "SymbolRate", pkValues, syms.join(";"));

    qDebug() << "RowDetailDialog: 数据保存完成," << m_tableName << m_pkValue;
}

void RowDetailDialog::onConfirm()
{
    saveData();
    accept();
}

void RowDetailDialog::onCancel()
{
    reject();
}

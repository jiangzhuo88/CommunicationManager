#ifndef ROWDETAILDIALOG_H
#define ROWDETAILDIALOG_H

#include "CDialogBase.h"
#include "localdatabase.h"
#include <QTableWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>

class RowDetailDialog : public CDialogBase
{
    Q_OBJECT
public:
    explicit RowDetailDialog(const QString& tableName, const QString& pkValue,
                             LocalDatabase* db, QWidget* parent = nullptr);
    ~RowDetailDialog() = default;

    void initUi();
    void loadData();

private slots:
    void onConfirm();
    void onCancel();
    void onAddSubRow();
    void onRemoveSubRow();

private:
    QString m_tableName;
    QString m_pkValue;
    LocalDatabase* m_db;

    // 上部分：参数
    QMap<QString, QLineEdit*> m_paramEdits;

    // 中部分：子表
    QTableWidget* m_subTable;

    // 下部分：PlatformEQ
    QPlainTextEdit* m_platformEqEdit;

    // 参数字段（除子表字段和PlatformEQ之外的主表列）
    QStringList m_paramFields;

    void saveData();
    void buildAndInitContent();
};

#endif // ROWDETAILDIALOG_H

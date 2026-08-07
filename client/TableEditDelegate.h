#ifndef TABLEEDITDELEGATE_H
#define TABLEEDITDELEGATE_H

#include <QStyledItemDelegate>
#include <QPersistentModelIndex>
class TableEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TableEditDelegate(QObject *parent = nullptr);
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setModelData(QWidget* editor,QAbstractItemModel*model,const QModelIndex &index) const override;
signals:
    void userCellEdited(int row,int col,const QString& oldVal,const QString& newVal)const ;
private:
    mutable QPersistentModelIndex m_editingIndex;
    mutable QString m_oldValue;
};

#endif // TABLEEDITDELEGATE_H

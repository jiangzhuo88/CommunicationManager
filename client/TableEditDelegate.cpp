#include "TableEditDelegate.h"

TableEditDelegate::TableEditDelegate(QObject *parent):QStyledItemDelegate(parent)
{

}

QWidget *TableEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    m_editingIndex = QPersistentModelIndex(index);
    m_oldValue = index.data(Qt::DisplayRole).toString();
    return QStyledItemDelegate::createEditor(parent,option,index);
}

void TableEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor,model,index);
    QString newVal = index.data(Qt::DisplayRole).toString();
    if(m_oldValue != newVal)
    {
        emit userCellEdited(index.row(),index.column(),m_oldValue,newVal);
    }
}

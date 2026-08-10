#ifndef CENTERWIDGET_H
#define CENTERWIDGET_H

#include <QWidget>
#include <QTableWidget>
namespace Ui {
class CenterWidget;
}

class CenterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CenterWidget(QStringList dbHeaders,QWidget *parent = 0);
    ~CenterWidget();
    QTableWidget* getTableWidget();
    QStringList getTableColHeaders();
    QList<QMap<int, QString> > getCacheData();
    void clear();
    static QString extractFreqOnly(const QString &text);
signals:
    void sigAddALine();
    void sigRemoveALine();
    void sigPasteLine();
    void userCellEdited(int row,int col,const QString& oldVal,const QString& newVal);
private slots:
    void slotTableRightMenu(const QPoint& pos);
private:
    void initAutoSize();
    bool eventFilter(QObject *watched, QEvent *event);
private:
    Ui::CenterWidget *ui;
    QList<QMap<int,QString>> mapList;
};

#endif // CENTERWIDGET_H

#ifndef USERMANAGEMENTDIALOG_H
#define USERMANAGEMENTDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>

class UserManagementDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserManagementDialog(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void slotAddUser();
    void slotDeleteUser();
    void slotSelectionChanged();

private:
    void initUi();
    void loadUsers();

    // 自定义标题栏拖动
    bool m_isDrag = false;
    QPoint m_dragStartPos;

    // 控件
    QLabel*      m_titleLab;
    QPushButton* m_btnCloseDlg;
    QTableWidget* m_table;
    QPushButton* m_btnAdd;
    QPushButton* m_btnDelete;
};

#endif // USERMANAGEMENTDIALOG_H

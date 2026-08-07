#ifndef USERMANAGEMENTDIALOG_H
#define USERMANAGEMENTDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include "UserStore.h"

class UserManagementDialog : public QDialog
{
    Q_OBJECT
public:
    // currentUserName：当前登录用户（不能删除自己）
    // currentRole：当前登录用户角色（普通用户不能删除管理员账号）
    explicit UserManagementDialog(const QString& currentUserName,
                                 UserStore::Role currentRole,
                                 QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void slotAddUser();
    void slotDeleteUser();
    void slotSelectionChanged();
    void slotApplyChanges();

private:
    void initUi();
    void loadUsers();
    // 把当前用户信息存进对话框（用于 apply）
    struct EditableUser {
        QString userName;
        QString password;   // 明文（仅对话框内存中，关闭后丢弃）
        UserStore::Role role;
        bool    isNew = false;
        bool    removed = false;
    };

    // 自定义标题栏拖动
    bool m_isDrag = false;
    QPoint m_dragStartPos;

    // 当前登录上下文
    QString        m_currentUser;
    UserStore::Role m_currentRole;

    // 控件
    QLabel*      m_titleLab;
    QPushButton* m_btnCloseDlg;
    QTableWidget* m_table;
    QPushButton* m_btnAdd;
    QPushButton* m_btnDelete;
    QPushButton* m_btnApply;

    // 内存编辑状态
    QList<EditableUser> m_editList;
};

#endif // USERMANAGEMENTDIALOG_H

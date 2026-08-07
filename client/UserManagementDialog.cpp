#include "UserManagementDialog.h"
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QComboBox>

UserManagementDialog::UserManagementDialog(const QString& currentUserName,
                                         UserStore::Role currentRole,
                                         QWidget *parent)
    : QDialog(parent)
    , m_currentUser(currentUserName)
    , m_currentRole(currentRole)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("userMgmtDialog");
    setFixedSize(560, 440);

    initUi();
    loadUsers();
}

void UserManagementDialog::initUi()
{
    QVBoxLayout* mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0,0,0,0);
    mainLay->setSpacing(0);

    // 标题栏
    QWidget* titleBar = new QWidget(this);
    titleBar->setObjectName("loginTitleBar");
    titleBar->setFixedHeight(40);
    titleBar->installEventFilter(this);
    QHBoxLayout* titleLay = new QHBoxLayout(titleBar);
    titleLay->setContentsMargins(10,0,0,0);
    titleLay->setSpacing(0);

    m_titleLab = new QLabel("用户管理");
    QFont fnt = m_titleLab->font();
    fnt.setPointSize(10);
    m_titleLab->setFont(fnt);
    m_titleLab->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_btnCloseDlg = new QPushButton("");
    m_btnCloseDlg->setObjectName("btnClose");
    m_btnCloseDlg->setFixedSize(40,40);

    titleLay->addSpacing(10);
    titleLay->addWidget(m_titleLab);
    titleLay->addStretch();
    titleLay->addWidget(m_btnCloseDlg);
    mainLay->addWidget(titleBar);

    // 内容区
    QWidget* content = new QWidget(this);
    content->setObjectName("loginFormFrame");
    QVBoxLayout* contentLay = new QVBoxLayout(content);
    contentLay->setContentsMargins(12,12,12,12);
    contentLay->setSpacing(10);

    // 用户表格：用户名 / 密码（明文显示，仅对话框内存） / 角色 / 操作
    m_table = new QTableWidget(0, 3);
    m_table->setHorizontalHeaderLabels({"用户名", "密码", "角色"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    // 用户名、角色可编辑，密码可编辑（用于新增/改密）
    m_table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    contentLay->addWidget(m_table);

    // 按钮区
    QHBoxLayout* btnLay = new QHBoxLayout;
    btnLay->addStretch();
    m_btnAdd = new QPushButton("添加用户");
    m_btnAdd->setObjectName("loginBtn");
    m_btnAdd->setMinimumSize(90,32);
    m_btnDelete = new QPushButton("删除用户");
    m_btnDelete->setObjectName("loginCancelBtn");
    m_btnDelete->setMinimumSize(90,32);
    m_btnDelete->setEnabled(false);
    m_btnApply = new QPushButton("应用");
    m_btnApply->setObjectName("loginBtn");
    m_btnApply->setMinimumSize(90,32);
    btnLay->addWidget(m_btnAdd);
    btnLay->addSpacing(10);
    btnLay->addWidget(m_btnDelete);
    btnLay->addSpacing(10);
    btnLay->addWidget(m_btnApply);
    contentLay->addLayout(btnLay);

    mainLay->addWidget(content, 1);

    // 信号
    connect(m_btnCloseDlg, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_btnAdd, &QPushButton::clicked, this, &UserManagementDialog::slotAddUser);
    connect(m_btnDelete, &QPushButton::clicked, this, &UserManagementDialog::slotDeleteUser);
    connect(m_btnApply, &QPushButton::clicked, this, &UserManagementDialog::slotApplyChanges);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &UserManagementDialog::slotSelectionChanged);

    // 普通用户限制
    if (m_currentRole != UserStore::RoleAdmin) {
        m_btnAdd->setEnabled(false);
        m_btnApply->setEnabled(false);
        m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
}

void UserManagementDialog::loadUsers()
{
    // 从 UserStore 加载到内存编辑列表
    UserStore store;
    QList<UserStore::UserInfo> list = store.users();
    m_editList.clear();
    for (const auto& u : list) {
        EditableUser eu;
        eu.userName = u.userName;
        eu.role     = u.role;
        // 已存在用户的密码不回显明文，置空表示"未修改"
        eu.password = "";
        eu.isNew    = false;
        eu.removed  = false;
        m_editList.append(eu);
    }

    // 填表
    m_table->setRowCount(0);
    for (int i = 0; i < m_editList.size(); ++i) {
        const EditableUser& u = m_editList[i];
        m_table->insertRow(i);
        m_table->setItem(i, 0, new QTableWidgetItem(u.userName));
        // 密码列：已有用户显示 "*"，新增用户显示空（等待输入）
        QTableWidgetItem* pwdItem = new QTableWidgetItem(u.isNew ? u.password : QString("*"));
        pwdItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 1, pwdItem);
        // 角色列：使用 QComboBox
        QComboBox* combo = new QComboBox;
        combo->addItem("普通用户", static_cast<int>(UserStore::RoleNormal));
        combo->addItem("管理员",   static_cast<int>(UserStore::RoleAdmin));
        combo->setCurrentIndex(u.role == UserStore::RoleAdmin ? 1 : 0);
        // 普通用户不能改角色
        if (m_currentRole != UserStore::RoleAdmin) {
            combo->setEnabled(false);
        }
        m_table->setCellWidget(i, 2, combo);
    }
}

void UserManagementDialog::slotAddUser()
{
    int row = m_table->rowCount();
    m_table->insertRow(row);

    // 默认用户名
    m_table->setItem(row, 0, new QTableWidgetItem("new_user"));
    // 密码列为空，等待输入
    QTableWidgetItem* pwdItem = new QTableWidgetItem("");
    pwdItem->setTextAlignment(Qt::AlignCenter);
    m_table->setItem(row, 1, pwdItem);
    // 角色
    QComboBox* combo = new QComboBox;
    combo->addItem("普通用户", static_cast<int>(UserStore::RoleNormal));
    combo->addItem("管理员",   static_cast<int>(UserStore::RoleAdmin));
    m_table->setCellWidget(row, 2, combo);

    // 同步到内存编辑列表
    EditableUser u;
    u.userName = "new_user";
    u.password = "";
    u.role     = UserStore::RoleNormal;
    u.isNew    = true;
    m_editList.append(u);

    m_table->editItem(m_table->item(row, 0));
}

void UserManagementDialog::slotDeleteUser()
{
    int row = m_table->currentRow();
    if (row < 0) return;

    QString name = m_table->item(row, 0)->text().trimmed();

    // 规则1：不能删除自己
    if (name == m_currentUser) {
        QMessageBox::information(this, "提示", "不能删除当前登录的账号");
        return;
    }
    // 规则2：普通用户不能删除管理员账号
    if (m_currentRole != UserStore::RoleAdmin) {
        // 取角色列 combo 当前值
        QComboBox* combo = qobject_cast<QComboBox*>(m_table->cellWidget(row, 2));
        if (combo && combo->currentData().toInt() == static_cast<int>(UserStore::RoleAdmin)) {
            QMessageBox::information(this, "提示", "普通用户不能删除管理员账号");
            return;
        }
    }

    // 规则3：删除后至少保留一个账号（这里先允许在表上删除，应用时再校验）
    m_table->removeRow(row);
    // 从内存列表中移除
    if (row < m_editList.size()) {
        if (m_editList[row].isNew) {
            m_editList.removeAt(row);
        } else {
            m_editList[row].removed = true;
        }
    }
}

void UserManagementDialog::slotSelectionChanged()
{
    m_btnDelete->setEnabled(m_table->currentRow() >= 0);
}

void UserManagementDialog::slotApplyChanges()
{
    // 仅管理员可应用
    if (m_currentRole != UserStore::RoleAdmin) {
        QMessageBox::information(this, "提示", "普通用户无权修改用户列表");
        return;
    }

    // 1. 校验表格内容；同时构建"目标用户列表"
    struct TargetUser {
        QString name;
        QString pwd;     // 明文（空表示不改）
        UserStore::Role role;
        bool isNew;
    };
    QList<TargetUser> targets;
    QSet<QString> seen;

    for (int i = 0; i < m_table->rowCount(); ++i) {
        QTableWidgetItem* nameItem = m_table->item(i, 0);
        QTableWidgetItem* pwdItem = m_table->item(i, 1);
        QComboBox* combo = qobject_cast<QComboBox*>(m_table->cellWidget(i, 2));
        if (!nameItem || !pwdItem || !combo) continue;

        QString name = nameItem->text().trimmed();
        QString pwd  = pwdItem->text();
        UserStore::Role role = static_cast<UserStore::Role>(combo->currentData().toInt());

        if (name.isEmpty()) {
            QMessageBox::warning(this, "提示", QString("第 %1 行用户名不能为空").arg(i+1));
            return;
        }
        if (seen.contains(name)) {
            QMessageBox::warning(this, "提示", QString("用户名 %1 重复").arg(name));
            return;
        }

        TargetUser t;
        t.name  = name;
        t.pwd   = pwd;
        t.role  = role;
        // 判断是否为新增：原内存列表里是否包含该名字（且未被移除）
        bool existed = false;
        for (const EditableUser& eu : m_editList) {
            if (eu.userName == name && !eu.removed) { existed = true; break; }
        }
        t.isNew = !existed;
        // 新增用户密码不能为空
        if (t.isNew && pwd.isEmpty()) {
            QMessageBox::warning(this, "提示", QString("新增用户 %1 必须设置密码").arg(name));
            return;
        }
        targets.append(t);
        seen.insert(name);
    }

    // 2. 校验至少保留一个账号
    if (targets.isEmpty()) {
        QMessageBox::warning(this, "提示", "至少需要保留一个账号");
        return;
    }

    // 3. 校验不能删除自己（自己必须在最终列表里）
    bool selfKept = false;
    for (const TargetUser& t : targets) {
        if (t.name == m_currentUser) { selfKept = true; break; }
    }
    if (!selfKept) {
        QMessageBox::warning(this, "提示", "不能删除当前登录的账号");
        return;
    }

    // 4. 写回 UserStore
    UserStore store;
    // 先删除不在 targets 里的旧用户
    QList<UserStore::UserInfo> oldList = store.users();
    for (const UserStore::UserInfo& old : oldList) {
        bool keep = false;
        for (const TargetUser& t : targets) {
            if (t.name == old.userName) { keep = true; break; }
        }
        if (!keep) {
            store.deleteUser(old.userName);
        }
    }
    // 再添加/更新
    for (const TargetUser& t : targets) {
        if (t.isNew) {
            store.addUser(t.name, t.pwd, t.role);
        } else {
            // 已存在用户：密码不为空才改密码
            if (!t.pwd.isEmpty()) {
                store.updatePassword(t.name, t.pwd);
            }
            store.updateRole(t.name, t.role);
        }
    }

    QMessageBox::information(this, "提示", "用户列表已更新");
    // 重新加载
    loadUsers();
}

void UserManagementDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->pos().y() <= 40) {
        m_isDrag = true;
        m_dragStartPos = event->globalPos() - frameGeometry().topLeft();
    }
    QDialog::mousePressEvent(event);
}

void UserManagementDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDrag) {
        move(event->globalPos() - m_dragStartPos);
    }
    QDialog::mouseMoveEvent(event);
}

void UserManagementDialog::mouseReleaseEvent(QMouseEvent *event)
{
    m_isDrag = false;
    QDialog::mouseReleaseEvent(event);
}

bool UserManagementDialog::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton) {
            m_isDrag = true;
            m_dragStartPos = me->globalPos() - frameGeometry().topLeft();
        }
        break;
    }
    case QEvent::MouseMove: {
        if (m_isDrag) {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            move(me->globalPos() - m_dragStartPos);
        }
        break;
    }
    case QEvent::MouseButtonRelease:
        m_isDrag = false;
        break;
    default:
        break;
    }
    return QDialog::eventFilter(watched, event);
}

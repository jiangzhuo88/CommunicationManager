#include "UserManagementDialog.h"
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>

UserManagementDialog::UserManagementDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("userMgmtDialog");
    setFixedSize(520, 400);

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

    // 用户表格
    m_table = new QTableWidget(0, 3);
    m_table->setHorizontalHeaderLabels({"用户名", "角色", "密码"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
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
    btnLay->addWidget(m_btnAdd);
    btnLay->addSpacing(10);
    btnLay->addWidget(m_btnDelete);
    contentLay->addLayout(btnLay);

    mainLay->addWidget(content, 1);

    // 信号
    connect(m_btnCloseDlg, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_btnAdd, &QPushButton::clicked, this, &UserManagementDialog::slotAddUser);
    connect(m_btnDelete, &QPushButton::clicked, this, &UserManagementDialog::slotDeleteUser);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &UserManagementDialog::slotSelectionChanged);
}

void UserManagementDialog::loadUsers()
{
    // 初始用户列表
    m_table->setRowCount(3);
    m_table->setItem(0, 0, new QTableWidgetItem("admin"));
    m_table->setItem(0, 1, new QTableWidgetItem("管理员"));
    m_table->setItem(0, 2, new QTableWidgetItem("admin"));

    m_table->setItem(1, 0, new QTableWidgetItem("user1"));
    m_table->setItem(1, 1, new QTableWidgetItem("普通用户"));
    m_table->setItem(1, 2, new QTableWidgetItem("123456"));

    m_table->setItem(2, 0, new QTableWidgetItem("user2"));
    m_table->setItem(2, 1, new QTableWidgetItem("普通用户"));
    m_table->setItem(2, 2, new QTableWidgetItem("123456"));
}

void UserManagementDialog::slotAddUser()
{
    m_table->insertRow(m_table->rowCount());
    m_table->setItem(m_table->rowCount()-1, 0, new QTableWidgetItem("new_user"));
    m_table->setItem(m_table->rowCount()-1, 1, new QTableWidgetItem("普通用户"));
    m_table->setItem(m_table->rowCount()-1, 2, new QTableWidgetItem("123456"));
    m_table->editItem(m_table->item(m_table->rowCount()-1, 0));
}

void UserManagementDialog::slotDeleteUser()
{
    int row = m_table->currentRow();
    if (row < 0) return;
    // 不允许删除 admin 账号
    if (m_table->item(row, 0) && m_table->item(row, 0)->text() == "admin") {
        QMessageBox::information(this, "提示", "不能删除管理员账号 admin");
        return;
    }
    m_table->removeRow(row);
}

void UserManagementDialog::slotSelectionChanged()
{
    m_btnDelete->setEnabled(m_table->currentRow() >= 0);
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

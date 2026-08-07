#include "LoginDialog.h"
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    // 无边框
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("loginDialog");
    setFixedSize(380, 260);

    initUi();
}

void LoginDialog::initUi()
{
    QVBoxLayout* mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0,0,0,0);
    mainLay->setSpacing(0);

    // ========== 标题栏 ==========
    QWidget* titleBar = new QWidget(this);
    titleBar->setObjectName("loginTitleBar");
    titleBar->setFixedHeight(40);
    titleBar->installEventFilter(this);  // 用于标题栏拖动
    QHBoxLayout* titleLay = new QHBoxLayout(titleBar);
    titleLay->setContentsMargins(10,0,0,0);
    titleLay->setSpacing(0);

    m_iconLab = new QLabel;
    m_iconLab->setFixedSize(20,20);
    m_iconLab->setPixmap(QIcon(":/Resource/Resource/Icon/通信.png").pixmap(20,20));
    m_iconLab->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_titleLab = new QLabel("Communication Identify Manager");
    QFont fnt = m_titleLab->font();
    fnt.setPointSize(10);
    m_titleLab->setFont(fnt);
    m_titleLab->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_btnClose = new QPushButton("");
    m_btnClose->setObjectName("btnClose");
    m_btnClose->setFixedSize(40,40);

    titleLay->addWidget(m_iconLab);
    titleLay->addSpacing(6);
    titleLay->addWidget(m_titleLab);
    titleLay->addStretch();
    titleLay->addWidget(m_btnClose);

    mainLay->addWidget(titleBar);

    // ========== 表单区 ==========
    QFrame* formFrame = new QFrame(this);
    formFrame->setObjectName("loginFormFrame");
    QVBoxLayout* formLay = new QVBoxLayout(formFrame);
    formLay->setContentsMargins(30,20,30,20);
    formLay->setSpacing(12);

    // 用户名
    m_userEdit = new QLineEdit;
    m_userEdit->setPlaceholderText("请输入用户名");
    m_userEdit->setMinimumHeight(32);

    // 密码
    m_pwdEdit = new QLineEdit;
    m_pwdEdit->setPlaceholderText("请输入密码");
    m_pwdEdit->setEchoMode(QLineEdit::Password);
    m_pwdEdit->setMinimumHeight(32);

    QFormLayout* form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setSpacing(10);
    form->addRow("用户名:", m_userEdit);
    form->addRow("密  码:", m_pwdEdit);
    formLay->addLayout(form);

    // 按钮区
    QHBoxLayout* btnLay = new QHBoxLayout;
    btnLay->addStretch();
    m_btnLogin = new QPushButton("登录");
    m_btnLogin->setObjectName("loginBtn");
    m_btnLogin->setMinimumSize(80,32);
    m_btnCancel = new QPushButton("取消");
    m_btnCancel->setObjectName("loginCancelBtn");
    m_btnCancel->setMinimumSize(80,32);
    btnLay->addWidget(m_btnLogin);
    btnLay->addSpacing(10);
    btnLay->addWidget(m_btnCancel);
    formLay->addLayout(btnLay);

    mainLay->addWidget(formFrame, 1);

    // 默认填充 admin/admin（首次启动 store 自动创建了该账号）
    if (m_store.exists("admin")) {
        m_userEdit->setText("admin");
        m_pwdEdit->setText("admin");
    }

    // 信号
    connect(m_btnClose, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_btnLogin, &QPushButton::clicked, this, &LoginDialog::slotLogin);
    connect(m_pwdEdit, &QLineEdit::returnPressed, this, &LoginDialog::slotLogin);
}

QString LoginDialog::userName() const { return m_userEdit->text().trimmed(); }
QString LoginDialog::password() const { return m_pwdEdit->text(); }

void LoginDialog::slotLogin()
{
    if (userName().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名");
        m_userEdit->setFocus();
        return;
    }
    if (password().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入密码");
        m_pwdEdit->setFocus();
        return;
    }
    UserStore::Role role = UserStore::RoleNormal;
    if (!m_store.validate(userName(), password(), role)) {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误");
        m_pwdEdit->clear();
        m_pwdEdit->setFocus();
        return;
    }
    m_role = role;
    accept();
}

void LoginDialog::slotCancel()
{
    reject();
}

void LoginDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->pos().y() <= 40) {
        m_isDrag = true;
        m_dragStartPos = event->globalPos() - frameGeometry().topLeft();
    }
    QDialog::mousePressEvent(event);
}

void LoginDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDrag) {
        move(event->globalPos() - m_dragStartPos);
    }
    QDialog::mouseMoveEvent(event);
}

void LoginDialog::mouseReleaseEvent(QMouseEvent *event)
{
    m_isDrag = false;
    QDialog::mouseReleaseEvent(event);
}

bool LoginDialog::eventFilter(QObject *watched, QEvent *event)
{
    // 处理标题栏的拖动（标题栏是子部件，其鼠标事件不会传递到 QDialog）
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

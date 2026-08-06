#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);

    // 获取登录信息
    QString userName() const;
    QString password() const;
    bool isAdmin() const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void slotLogin();
    void slotCancel();

private:
    void initUi();

    // 自定义标题栏拖动
    bool m_isDrag = false;
    QPoint m_dragStartPos;

    // 控件
    QLabel*      m_iconLab;
    QLabel*      m_titleLab;
    QPushButton* m_btnClose;
    QLineEdit*   m_userEdit;
    QLineEdit*   m_pwdEdit;
    QComboBox*   m_roleCombo;
    QPushButton* m_btnLogin;
    QPushButton* m_btnCancel;
};

#endif // LOGINDIALOG_H

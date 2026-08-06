#ifndef ZTOOLBUTTON_H
#define ZTOOLBUTTON_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QAbstractButton>

class ZToolButton : public QAbstractButton
{
    Q_OBJECT
public:
    explicit ZToolButton(QWidget *parent = nullptr);

    // 接口
    void setIcon(const QIcon &icon, const QSize &iconSize);
    void setText(const QString &text);
    void setMiddleSpace(int spaceHeight);
protected:
    void paintEvent(QPaintEvent *e);
private:
    QLabel* m_iconLab = nullptr;
    QLabel* m_textLab = nullptr;
    QVBoxLayout* m_layout = nullptr;
};

#endif // ZTOOLBUTTON_H

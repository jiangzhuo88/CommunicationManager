#include "ZToolButton.h"
//#include "ui_ZToolButton.h"

#include <QSpacerItem>
#include <QSizePolicy>
#include <QPaintEvent>
#include <QDebug>
#include <QPainter>
#include <QStyleOptionToolButton>
ZToolButton::ZToolButton(QWidget *parent)
    : QAbstractButton(parent)
{
    // 整体布局
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(4,4,4,4);
    m_layout->setSpacing(0);

    // 图标
    m_iconLab = new QLabel;
    m_iconLab->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    // 文字
    m_textLab = new QLabel;
    m_textLab->setAlignment(Qt::AlignCenter);

    // 组装：图标 -> 中间弹簧留白 -> 文字
    m_layout->addWidget(m_iconLab);

    // 中间伸缩弹簧，核心：拉开间距
    QSpacerItem* spacer = new QSpacerItem(
        20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_layout->addSpacerItem(spacer);

    m_layout->addWidget(m_textLab);

    // 鼠标手型
    setCursor(Qt::PointingHandCursor);
}

void ZToolButton::setIcon(const QIcon &icon, const QSize &iconSize)
{
    m_iconLab->setPixmap(icon.pixmap(iconSize));
    m_iconLab->setFixedHeight(iconSize.height());
}

void ZToolButton::setText(const QString &text)
{
    m_textLab->setText(text);
}

// 动态修改中间留白高度
void ZToolButton::setMiddleSpace(int spaceHeight)
{
    // 移除旧弹簧
    if (m_layout->count() >= 2)
    {
        auto item = m_layout->itemAt(1);
        if (item->spacerItem())
        {
            m_layout->removeItem(item);
            delete item;
        }
    }
    // 新建指定高度弹簧
    QSpacerItem* spacer = new QSpacerItem(
        20, spaceHeight, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_layout->insertSpacerItem(1, spacer);
}

void ZToolButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QStyleOptionToolButton opt;
    opt.initFrom(this);

    opt.state = QStyle::State_None;
    if(isEnabled())
    {
        opt.state |= QStyle::State_Enabled;
    }
    if(isDown())
    {
        opt.state |= QStyle::State_Sunken;
    }
    if(underMouse())
    {
        opt.state |= QStyle::State_MouseOver;
    }
    if(hasFocus())
    {
        opt.state |= QStyle::State_HasFocus;
    }
    style()->drawComplexControl(QStyle::CC_ToolButton,&opt,&p,this);

}


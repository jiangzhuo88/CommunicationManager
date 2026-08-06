#include "CFramelessWindowBase.h"
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QDebug>
#include <QStyle>
CFramelessWindowBase::CFramelessWindowBase(QWidget *parent)
    : QWidget(parent)
{
    // 无边框
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
//    setAttribute(Qt::WA_Hover);
    setAttribute(Qt::WA_StyledBackground);

//    // ========== 中心容器 + 整体布局 ==========
//    QWidget* centralWgt = new QWidget(this);
//    QVBoxLayout* mainLayout = new QVBoxLayout(centralWgt);
//    mainLayout->setContentsMargins(0,0,0,0);
//    mainLayout->setSpacing(0);
//    setCentralWidget(centralWgt);

    // ========== 自定义标题栏 高度40px ==========
//    m_titleBar = new QWidget;
    this->setFixedHeight(40);
    this->setObjectName("TitleBar");

    QHBoxLayout* titleLay = new QHBoxLayout(this);
    titleLay->setContentsMargins(0,0,0,0);
    titleLay->setSpacing(0);

    // 图标
    m_iconLab = new QLabel;
    m_iconLab->setFixedSize(20,20);

    // 标题文字
    m_titleLab = new QLabel;
    QFont fnt = m_titleLab->font();
    fnt.setPointSize(10);
    m_titleLab->setFont(fnt);

    // 弹簧
    titleLay->addWidget(m_iconLab);
    titleLay->addWidget(m_titleLab);
    titleLay->addStretch();

    // ========== 三个按钮 尺寸28px ==========
    m_btnMin   = new QPushButton("");
    m_btnMin->setObjectName("btnMin");
    m_btnMax   = new QPushButton("");
    m_btnMax->setObjectName("btnMax");
    m_btnClose = new QPushButton("");
    m_btnClose->setObjectName("btnClose");

    m_btnMin->setFixedSize(40,40);
    m_btnMax->setFixedSize(40,40);
    m_btnClose->setFixedSize(40,40);

    titleLay->addWidget(m_btnMin);
    titleLay->addWidget(m_btnMax);
    titleLay->addWidget(m_btnClose);
    // 按钮事件
    connect(m_btnMin, &QPushButton::clicked, this, &CFramelessWindowBase::slotMin);
    connect(m_btnMax, &QPushButton::clicked, this, &CFramelessWindowBase::slotMaxRestore);
    connect(m_btnClose, &QPushButton::clicked, this, &CFramelessWindowBase::slotClose);
}

void CFramelessWindowBase::setWindowTitleText(const QString &text)
{
    m_titleLab->setText(text);
}

void CFramelessWindowBase::setWindowTitleIcon(const QIcon &icon, int size)
{
    m_iconLab->setPixmap(icon.pixmap(size, size));
}

void CFramelessWindowBase::slotMin()
{
//    qDebug()<<this->parentWidget()->parentWidget();
    QWidget* parent = getParentWidget();
    parent->showMinimized();
}

void CFramelessWindowBase::slotMaxRestore()
{
    QWidget* parent = getParentWidget();
    if(parent->isMaximized())
    {
        parent->showNormal();
        m_btnMax->setObjectName("btnMax");
    }
    else
    {
        parent->showMaximized();
        m_btnMax->setObjectName("btnNomal");
    }
//    m_btnMax->show();
    m_btnMax->style()->unpolish(m_btnMax);
    m_btnMax->style()->polish(m_btnMax);
}

void CFramelessWindowBase::slotClose()
{
    QWidget* parent = getParentWidget();
    parent->close();
}

bool CFramelessWindowBase::isInTitleBar(const QPoint &pos)
{
    return pos.y() <= 40;
}

CFramelessWindowBase::ResizeDir CFramelessWindowBase::getResizeDirection(const QPoint &pos)
{
    int x = pos.x();
    int y = pos.y();
    int w = width();
    int h = height();
    int m = m_resizeMargin;

    bool left = x <= m;
    bool right = x >= w - m;
    bool top = y <= m;
    bool bottom = y >= h - m;

    if(top && left) return DIR_TOPLEFT;
    if(top && right) return DIR_TOPRIGHT;
    if(bottom && left) return DIR_BOTTOMLEFT;
    if(bottom && right) return DIR_BOTTOMRIGHT;
    if(left) return DIR_LEFT;
    if(right) return DIR_RIGHT;
    if(top) return DIR_TOP;
    if(bottom) return DIR_BOTTOM;

    return DIR_NONE;
}

void CFramelessWindowBase::setCursorByDir(ResizeDir dir)
{
    switch (dir)
    {
    case DIR_LEFT: case DIR_RIGHT:
        setCursor(Qt::SizeHorCursor); break;
    case DIR_TOP: case DIR_BOTTOM:
        setCursor(Qt::SizeVerCursor); break;
    case DIR_TOPLEFT: case DIR_BOTTOMRIGHT:
        setCursor(Qt::SizeFDiagCursor); break;
    case DIR_TOPRIGHT: case DIR_BOTTOMLEFT:
        setCursor(Qt::SizeBDiagCursor); break;
    default:
        setCursor(Qt::ArrowCursor); break;
    }
}

void CFramelessWindowBase::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        QWidget* parent = getParentWidget();
        QPoint pos = event->pos();
        m_mouseStartPos = event->globalPos();
        m_winStartRect = parent->geometry();

        m_resizeDir = getResizeDirection(pos);
        if(m_resizeDir != DIR_NONE)
        {
            m_isResize = true;
        }
        else if(isInTitleBar(pos))
        {
            m_isDrag = true;
            m_dragStartPos = event->globalPos() - parent->frameGeometry().topLeft();
        }
    }
    QWidget::mousePressEvent(event);
}

void CFramelessWindowBase::mouseMoveEvent(QMouseEvent *event)
{
    QPoint globalPos = event->globalPos();

    QWidget* parent = getParentWidget();
    // 缩放
    if(m_isResize && m_resizeDir != DIR_NONE)
    {
        QRect rc = m_winStartRect;
        QPoint delta = globalPos - m_mouseStartPos;

        switch (m_resizeDir)
        {
        case DIR_LEFT: rc.setLeft(rc.left()+delta.x()); break;
        case DIR_RIGHT: rc.setRight(rc.right()+delta.x()); break;
        case DIR_TOP: rc.setTop(rc.top()+delta.y()); break;
        case DIR_BOTTOM: rc.setBottom(rc.bottom()+delta.y()); break;
        case DIR_TOPLEFT: rc.setLeft(rc.left()+delta.x());rc.setTop(rc.top()+delta.y());break;
        case DIR_TOPRIGHT:rc.setRight(rc.right()+delta.x());rc.setTop(rc.top()+delta.y());break;
        case DIR_BOTTOMLEFT:rc.setLeft(rc.left()+delta.x());rc.setBottom(rc.bottom()+delta.y());break;
        case DIR_BOTTOMRIGHT:rc.setRight(rc.right()+delta.x());rc.setBottom(rc.bottom()+delta.y());break;
        default:break;
        }

        if(rc.width() < 400) rc.setWidth(400);
        if(rc.height() < 300) rc.setHeight(300);
        parent->setGeometry(rc);
    }
    // 拖动
    else if(m_isDrag && !parent->isMaximized())
    {
        parent->move(globalPos - m_dragStartPos);
    }
    // 悬浮改光标
    else
    {
        ResizeDir d = getResizeDirection(event->pos());
        setCursorByDir(d);
    }

    QWidget::mouseMoveEvent(event);

}

void CFramelessWindowBase::mouseReleaseEvent(QMouseEvent *event)
{
    m_isDrag = false;
    m_isResize = false;
    m_resizeDir = DIR_NONE;
    QWidget::mouseReleaseEvent(event);
}

void CFramelessWindowBase::leaveEvent(QEvent *event)
{
    setCursor(Qt::ArrowCursor);
    QWidget::leaveEvent(event);
}

QWidget *CFramelessWindowBase::getParentWidget()
{
    QWidget* parent = this;
    while (parent && parent->parentWidget()) {
        parent = parent->parentWidget();
    }
    return parent;
}

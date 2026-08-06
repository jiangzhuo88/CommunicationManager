#include "CFramelessWindowBase.h"
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QDebug>
#include <QStyle>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>

CFramelessWindowBase::CFramelessWindowBase(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);
    setFixedHeight(40);
    setObjectName("TitleBar");
    setMouseTracking(true);

    QHBoxLayout* titleLay = new QHBoxLayout(this);
    titleLay->setContentsMargins(0,0,0,0);
    titleLay->setSpacing(0);

    // 图标（设置鼠标穿透，使点击可以传递到标题栏以触发拖动/双击）
    m_iconLab = new QLabel;
    m_iconLab->setFixedSize(20,20);
    m_iconLab->setContentsMargins(10,0,0,0);
    m_iconLab->setAttribute(Qt::WA_TransparentForMouseEvents);

    // 标题文字（同样设置鼠标穿透）
    m_titleLab = new QLabel;
    QFont fnt = m_titleLab->font();
    fnt.setPointSize(10);
    m_titleLab->setFont(fnt);
    m_titleLab->setContentsMargins(6,0,0,0);
    m_titleLab->setAttribute(Qt::WA_TransparentForMouseEvents);

    // 弹簧
    titleLay->addWidget(m_iconLab);
    titleLay->addWidget(m_titleLab);
    titleLay->addStretch();

    // ========== 三个按钮 尺寸40px ==========
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

void CFramelessWindowBase::installFramelessHandler()
{
    QWidget* top = getTopWindow();
    if (!top) return;

    // 开启鼠标追踪，使无按键按下时也能收到 MouseMove（用于光标变化）
    top->setMouseTracking(true);
    const auto children = top->findChildren<QWidget*>();
    for (QWidget* w : children) {
        w->setMouseTracking(true);
    }

    // 在 qApp 上安装事件过滤器，拦截主窗口内所有子部件的鼠标事件以实现边框缩放
    qApp->installEventFilter(this);
}

QWidget* CFramelessWindowBase::getTopWindow()
{
    QWidget* w = this;
    while (w && w->parentWidget()) {
        w = w->parentWidget();
    }
    return w;
}

bool CFramelessWindowBase::isDescendantOf(QWidget* w, QWidget* ancestor)
{
    QWidget* p = w;
    while (p) {
        if (p == ancestor) return true;
        p = p->parentWidget();
    }
    return false;
}

CFramelessWindowBase::ResizeDir CFramelessWindowBase::getResizeDirection(const QPoint& pos, QWidget* top)
{
    int x = pos.x();
    int y = pos.y();
    int w = top->width();
    int h = top->height();
    int m = m_resizeMargin;

    bool left = x <= m;
    bool right = x >= w - m;
    bool topEdge = y <= m;
    bool bottom = y >= h - m;

    // 四个角优先
    if (topEdge && left) return DIR_TOPLEFT;
    if (topEdge && right) return DIR_TOPRIGHT;
    if (bottom && left) return DIR_BOTTOMLEFT;
    if (bottom && right) return DIR_BOTTOMRIGHT;

    // 在标题栏区域(高度40)内，左右边不触发缩放，避免拖动标题栏时误触缩放导致页面缩小
    if (y > 40) {
        if (left) return DIR_LEFT;
        if (right) return DIR_RIGHT;
    }
    if (topEdge) return DIR_TOP;
    if (bottom) return DIR_BOTTOM;

    return DIR_NONE;
}

QCursor CFramelessWindowBase::cursorForDir(ResizeDir dir)
{
    switch (dir) {
    case DIR_LEFT: case DIR_RIGHT:
        return Qt::SizeHorCursor;
    case DIR_TOP: case DIR_BOTTOM:
        return Qt::SizeVerCursor;
    case DIR_TOPLEFT: case DIR_BOTTOMRIGHT:
        return Qt::SizeFDiagCursor;
    case DIR_TOPRIGHT: case DIR_BOTTOMLEFT:
        return Qt::SizeBDiagCursor;
    default:
        return Qt::ArrowCursor;
    }
}

void CFramelessWindowBase::doResize(const QPoint& globalPos)
{
    QWidget* top = getTopWindow();
    if (!top) return;

    QRect rc = m_winStartRect;
    QPoint delta = globalPos - m_mouseStartPos;

    switch (m_resizeDir) {
    case DIR_LEFT: rc.setLeft(rc.left()+delta.x()); break;
    case DIR_RIGHT: rc.setRight(rc.right()+delta.x()); break;
    case DIR_TOP: rc.setTop(rc.top()+delta.y()); break;
    case DIR_BOTTOM: rc.setBottom(rc.bottom()+delta.y()); break;
    case DIR_TOPLEFT: rc.setLeft(rc.left()+delta.x()); rc.setTop(rc.top()+delta.y()); break;
    case DIR_TOPRIGHT: rc.setRight(rc.right()+delta.x()); rc.setTop(rc.top()+delta.y()); break;
    case DIR_BOTTOMLEFT: rc.setLeft(rc.left()+delta.x()); rc.setBottom(rc.bottom()+delta.y()); break;
    case DIR_BOTTOMRIGHT: rc.setRight(rc.right()+delta.x()); rc.setBottom(rc.bottom()+delta.y()); break;
    default: break;
    }

    // 最小尺寸约束：正确固定对应边
    const int minW = 400;
    const int minH = 300;
    if (rc.width() < minW) {
        if (m_resizeDir == DIR_LEFT || m_resizeDir == DIR_TOPLEFT || m_resizeDir == DIR_BOTTOMLEFT)
            rc.setLeft(rc.right() - minW);
        else
            rc.setRight(rc.left() + minW);
    }
    if (rc.height() < minH) {
        if (m_resizeDir == DIR_TOP || m_resizeDir == DIR_TOPLEFT || m_resizeDir == DIR_TOPRIGHT)
            rc.setTop(rc.bottom() - minH);
        else
            rc.setBottom(rc.top() + minH);
    }

    top->setGeometry(rc);
}

void CFramelessWindowBase::updateMaxButtonIcon()
{
    m_btnMax->style()->unpolish(m_btnMax);
    m_btnMax->style()->polish(m_btnMax);
    m_btnMax->update();
}

void CFramelessWindowBase::slotMin()
{
    QWidget* top = getTopWindow();
    if (top) top->showMinimized();
}

void CFramelessWindowBase::slotMaxRestore()
{
    QWidget* top = getTopWindow();
    if (!top) return;

    if (m_isMaximized) {
        // 还原
        top->setGeometry(m_normalGeometry);
        m_btnMax->setObjectName("btnMax");
        m_isMaximized = false;
    } else {
        // 最大化：保存当前几何，设置为屏幕大小
        m_normalGeometry = top->geometry();
        QRect screenRect = top->screen()->availableGeometry();
        top->setGeometry(screenRect);
        m_btnMax->setObjectName("btnNomal");  // 与 QSS 中保持一致
        m_isMaximized = true;
    }
    updateMaxButtonIcon();
}

void CFramelessWindowBase::slotClose()
{
    QWidget* top = getTopWindow();
    if (top) top->close();
}

void CFramelessWindowBase::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QWidget* top = getTopWindow();
        if (top) {
            // 记录拖动起始位置，实际拖动逻辑在 mouseMoveEvent 中处理
            // （避免双击时第二次 press 误触发还原逻辑）
            m_dragStartPos = event->globalPos() - top->geometry().topLeft();
            m_isDrag = true;
        }
    }
    QWidget::mousePressEvent(event);
}

void CFramelessWindowBase::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDrag) {
        QWidget* top = getTopWindow();
        if (top) {
            // 最大化状态下拖动标题栏：先还原窗口再跟随鼠标移动
            if (m_isMaximized) {
                int cursorX = event->globalPos().x();
                double relX = (double)(cursorX - top->geometry().left())
                              / qMax(1, top->geometry().width());
                // 还原到正常几何
                top->setGeometry(m_normalGeometry);
                m_btnMax->setObjectName("btnMax");
                m_isMaximized = false;
                updateMaxButtonIcon();
                QRect normalRect = top->geometry();
                int newX = cursorX - (int)(normalRect.width() * relX);
                int newY = event->globalPos().y() - event->pos().y();
                top->move(newX, newY);
                // 重新计算拖动起始位置
                m_dragStartPos = event->globalPos() - top->geometry().topLeft();
            }
            top->move(event->globalPos() - m_dragStartPos);
        }
    }
    QWidget::mouseMoveEvent(event);
}

void CFramelessWindowBase::mouseReleaseEvent(QMouseEvent *event)
{
    m_isDrag = false;
    QWidget::mouseReleaseEvent(event);
}

void CFramelessWindowBase::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 双击标题栏切换最大化/还原
    if (event->button() == Qt::LeftButton) {
        slotMaxRestore();
    }
    QWidget::mouseDoubleClickEvent(event);
}

bool CFramelessWindowBase::eventFilter(QObject *watched, QEvent *event)
{
    QWidget* top = getTopWindow();
    if (!top) return QWidget::eventFilter(watched, event);

    QWidget* w = qobject_cast<QWidget*>(watched);
    if (!w || !isDescendantOf(w, top)) {
        return QWidget::eventFilter(watched, event);
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() != Qt::LeftButton) break;
        // 最大化时不允许边框缩放
        if (m_isMaximized) break;
        // 鼠标在按钮上时不触发缩放，避免误拦截按钮点击
        if (qobject_cast<QAbstractButton*>(w)) break;

        QPoint globalPos = me->globalPos();
        QPoint localPos = top->mapFromGlobal(globalPos);
        ResizeDir dir = getResizeDirection(localPos, top);
        if (dir != DIR_NONE) {
            m_isResize = true;
            m_resizeDir = dir;
            m_mouseStartPos = globalPos;
            m_winStartRect = top->geometry();
            return true;  // 拦截，防止子部件/标题栏处理
        }
        break;
    }
    case QEvent::MouseMove: {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        QPoint globalPos = me->globalPos();
        QPoint localPos = top->mapFromGlobal(globalPos);

        // 正在缩放
        if (m_isResize && m_resizeDir != DIR_NONE) {
            doResize(globalPos);
            return true;
        }

        // 根据是否靠近边缘更新光标
        if (!m_isMaximized && !qobject_cast<QAbstractButton*>(w)) {
            ResizeDir dir = getResizeDirection(localPos, top);
            if (dir != DIR_NONE) {
                w->setCursor(cursorForDir(dir));
            } else {
                w->unsetCursor();
            }
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        if (m_isResize) {
            m_isResize = false;
            m_resizeDir = DIR_NONE;
            return true;
        }
        break;
    }
    default:
        break;
    }

    return QWidget::eventFilter(watched, event);
}

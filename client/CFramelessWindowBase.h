#ifndef CFramelessWindowBase_H
#define CFramelessWindowBase_H

#include <QWidget>
#include <QPoint>
#include <QLabel>
#include <QPushButton>

class CFramelessWindowBase : public QWidget
{
    Q_OBJECT
public:
    explicit CFramelessWindowBase(QWidget *parent = nullptr);
    ~CFramelessWindowBase() override = default;

    // 外部设置标题、图标
    void setWindowTitleText(const QString& text);
    void setWindowTitleIcon(const QIcon& icon, int size = 20);

    // 在标题栏被添加到主窗口布局后调用，安装全局边框缩放处理器
    void installFramelessHandler();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QWidget* getTopWindow();
    bool isDescendantOf(QWidget* w, QWidget* ancestor);

    // 窗口缩放方向
    enum ResizeDir
    {
        DIR_NONE=0,DIR_LEFT,DIR_RIGHT,DIR_TOP,DIR_BOTTOM,
        DIR_TOPLEFT,DIR_TOPRIGHT,DIR_BOTTOMLEFT,DIR_BOTTOMRIGHT
    };

    ResizeDir getResizeDirection(const QPoint& posInTop, QWidget* top);
    QCursor cursorForDir(ResizeDir dir);
    void doResize(const QPoint& globalPos);
    void updateMaxButtonIcon();

private slots:
    void slotMin();
    void slotMaxRestore();
    void slotClose();

private:
    // 标题栏UI
    QLabel*      m_iconLab;
    QLabel*      m_titleLab;
    QPushButton* m_btnMin;
    QPushButton* m_btnMax;
    QPushButton* m_btnClose;

    // 窗口拖动
    bool m_isDrag = false;
    QPoint m_dragStartPos;

    // 窗口缩放
    const int m_resizeMargin = 6;
    bool m_isResize = false;
    ResizeDir m_resizeDir = DIR_NONE;
    QPoint m_mouseStartPos;
    QRect  m_winStartRect;

    // 最大化/还原
    QRect m_normalGeometry;
//    bool m_isMaximized = false;
};

#endif // CFramelessWindowBase_H

#ifndef CFramelessWindowBase_H
#define CFramelessWindowBase_H

#include <QWidget>
#include <QPoint>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

class CFramelessWindowBase : public QWidget
{
    Q_OBJECT
public:
    explicit CFramelessWindowBase(QWidget *parent = nullptr);
    ~CFramelessWindowBase() override = default;

    // 外部设置标题、图标
    void setWindowTitleText(const QString& text);
    void setWindowTitleIcon(const QIcon& icon, int size = 20);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
private:
    QWidget* getParentWidget();
private slots:
    void slotMin();
    void slotMaxRestore();
    void slotClose();

private:
    // 布局UI
//    QWidget*     m_titleBar;
    QLabel*      m_iconLab;
    QLabel*      m_titleLab;
    QPushButton* m_btnMin;
    QPushButton* m_btnMax;
    QPushButton* m_btnClose;

    // 窗口拖动
    bool m_isDrag = false;
    QPoint m_dragStartPos;

    // 窗口缩放
    const int m_resizeMargin = 8;
    enum ResizeDir
    {
        DIR_NONE=0,DIR_LEFT,DIR_RIGHT,DIR_TOP,DIR_BOTTOM,
        DIR_TOPLEFT,DIR_TOPRIGHT,DIR_BOTTOMLEFT,DIR_BOTTOMRIGHT
    };
    ResizeDir m_resizeDir = DIR_NONE;
    bool m_isResize = false;
    QPoint m_mouseStartPos;
    QRect  m_winStartRect;

    ResizeDir getResizeDirection(const QPoint& pos);
    void setCursorByDir(ResizeDir dir);
    bool isInTitleBar(const QPoint& pos);
};

#endif // CFramelessWindowBase_H

#include "qt_ipcnotify.h"
static void rawConnCb(bool ok,void *u)
{
    auto ptr = reinterpret_cast<QtIpcNotify*>(u);
    QPointer<QtIpcNotify> guard(ptr);
    QMetaObject::invokeMethod(ptr,[guard,ok]()
    {
        if(guard)
        {
            emit guard->sigConnected(ok);
        }
    },Qt::QueuedConnection);
}
static void rawMsgCb(const std::string& msg,void *u)
{
    auto ptr = reinterpret_cast<QtIpcNotify*>(u);
    QPointer<QtIpcNotify> guard(ptr);
    QMetaObject::invokeMethod(ptr,[guard,msg]()
    {
        if(guard)
        {
            emit guard->sigMessage(QString::fromStdString(msg));
        }
    },Qt::QueuedConnection);
}

QtIpcNotify::QtIpcNotify(IpcNotifyRaw::Role role, QObject *parent)
    :QObject(parent),m_raw(role)
{
    m_raw.setConnCallback(rawConnCb,this);
    m_raw.setMsgCallback(rawMsgCb,this);
}
QtIpcNotify::~QtIpcNotify()
{
    m_raw.waitStop();
}

bool QtIpcNotify::start()
{
    return m_raw.start();
}

void QtIpcNotify::stop()
{
    m_raw.waitStop();
}

bool QtIpcNotify::publish(const QString &msg)
{
    return m_raw.publish(msg.toStdString());
}

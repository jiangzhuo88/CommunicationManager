#ifndef QT_IPCNOTIFY_H
#define QT_IPCNOTIFY_H
#pragma once
#include <QObject>
#include <QString>
#include <functional>
#include "ipc_notify.h"

class QtIpcNotify : public QObject
{
    Q_OBJECT
public:
    explicit QtIpcNotify(IpcNotifyRaw::Role role, QObject *parent = nullptr);
    ~QtIpcNotify() override;

    bool start();
    void stop();   // 注意：此函数会阻塞直到工作线程结束，勿在GUI主线程频繁调用
    bool publish(const QString& msg);

signals:
    void sigConnected(bool ok);
    void sigMessage(const QString& msg);

private:
    IpcNotifyRaw m_raw;
};

#endif // QT_IPCNOTIFY_H

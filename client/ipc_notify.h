#ifndef IPCNOTIFY_H
#define IPCNOTIFY_H

#include <QObject>
#include <QPointer>
#include <string>
#include <atomic>
#include <thread>

class IpcNotifyRaw
{
public:
    enum Role { ROLE_CLIENT, ROLE_SERVER };

    explicit IpcNotifyRaw(Role r);
    ~IpcNotifyRaw();

    bool start();
    void stop();
    void waitStop();

    bool publish(const std::string& msg);

    using ConnCallback = void(*)(bool connected, void* user);
    using MsgCallback = void(*)(const std::string& msg, void* user);

    void setConnCallback(ConnCallback cb, void* u);
    void setMsgCallback(MsgCallback cb, void* u);

private:
    void workThread();
    bool tryAsClient();
    bool becomeServer();
    void unpackBuffer();
    void handleDisconnect();          // 统一处理断开连接

    Role m_role;
    std::atomic<bool> m_running{false};

    // 以下成员仅在工作线程中访问，无需额外保护
    int m_peerFd = -1;
    bool m_connected = false;
    std::string m_recvBuf;

    ConnCallback m_connCb = nullptr;
    void* m_connUser = nullptr;
    MsgCallback m_msgCb = nullptr;
    void* m_msgUser = nullptr;

    std::thread* m_th = nullptr;

    static constexpr const char* IPC_SOCK = "/tmp/ipc_ab_notify.sock";
    static constexpr int MAX_RECV_BUF = 8192;
};


#endif // IPCNOTIFY_H

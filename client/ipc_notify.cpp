#include "ipc_notify.h"
#include <thread>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <arpa/inet.h>      // for htons/ntohs
#include <QDebug>
#include <chrono>

// 构造函数
IpcNotifyRaw::IpcNotifyRaw(Role r)
    : m_role(r)
{
}

// 析构函数
IpcNotifyRaw::~IpcNotifyRaw()
{
    waitStop();
}

// 启动工作线程
bool IpcNotifyRaw::start()
{
    if (m_running.load())
        return false;
    m_running.store(true);
    m_th = new std::thread([this](){ workThread(); });
    return true;
}

// 请求停止（非阻塞）
void IpcNotifyRaw::stop()
{
    m_running.store(false);
}

// 等待工作线程结束并清理资源
void IpcNotifyRaw::waitStop()
{
    stop();
    if (m_th && m_th->joinable()) {
        m_th->join();
    }
    delete m_th;
    m_th = nullptr;

    // 注意：此时工作线程已结束，m_peerFd 应由工作线程自行关闭，
    // 此处不再重复关闭，避免 double-close
}

// 设置连接状态回调
void IpcNotifyRaw::setConnCallback(ConnCallback cb, void *u)
{
    m_connCb = cb;
    m_connUser = u;
}

// 设置消息回调
void IpcNotifyRaw::setMsgCallback(MsgCallback cb, void *u)
{
    m_msgCb = cb;
    m_msgUser = u;
}

// 发送消息（带长度前缀）
bool IpcNotifyRaw::publish(const std::string &msg)
{
    if (!m_connected || m_peerFd < 0)
        return false;

    uint16_t lenNet = htons(static_cast<uint16_t>(msg.size()));
    std::string pkg;
    pkg.append(reinterpret_cast<const char*>(&lenNet), 2);
    pkg.append(msg);

    size_t total = pkg.size();
    size_t sent = 0;
    while (sent < total) {
        ssize_t w = send(m_peerFd, pkg.data() + sent, total - sent, MSG_NOSIGNAL);
        if (w <= 0) {
            // 发送失败，断开连接
            handleDisconnect();
            return false;
        }
        sent += static_cast<size_t>(w);
    }
    return true;
}

// 从接收缓冲区中解析完整消息
void IpcNotifyRaw::unpackBuffer()
{
    while (m_recvBuf.size() >= 2) {
        uint16_t lenNet;
        memcpy(&lenNet, m_recvBuf.data(), 2);
        uint16_t bodyLen = ntohs(lenNet);

        if (m_recvBuf.size() < (2U + bodyLen))
            break;  // 尚未收到完整消息

        std::string body(m_recvBuf.data() + 2, bodyLen);
        m_recvBuf.erase(0, 2 + bodyLen);

        if (m_msgCb) {
            m_msgCb(body, m_msgUser);
        }
    }
}

// 统一处理连接断开
void IpcNotifyRaw::handleDisconnect()
{
    if (m_peerFd >= 0) {
        close(m_peerFd);
        m_peerFd = -1;
    }
    m_connected = false;
    m_recvBuf.clear();
    if (m_connCb) {
        m_connCb(false, m_connUser);
    }
}

// 尝试作为客户端连接服务器
bool IpcNotifyRaw::tryAsClient()
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        return false;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, IPC_SOCK, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return false;
    }

    m_peerFd = fd;
    m_connected = true;
    if (m_connCb) m_connCb(true, m_connUser);

    char buf[MAX_RECV_BUF];
    while (m_running.load() && m_connected) {
        ssize_t r = recv(m_peerFd, buf, sizeof(buf), 0);
        if (r <= 0) {
            break;  // 连接断开或出错
        }
        m_recvBuf.append(buf, static_cast<size_t>(r));
        unpackBuffer();
    }

    handleDisconnect();
    return false;  // 返回 false 表示连接结束（无论正常还是异常）
}

// 成为服务器，等待客户端连接
bool IpcNotifyRaw::becomeServer()
{
    // 尝试绑定，如果地址已存在则先 unlink
    int srvFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (srvFd < 0) return false;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, IPC_SOCK, sizeof(addr.sun_path) - 1);

    // 先尝试 bind，如果失败且原因是地址已存在，则 unlink 后重试
    if (bind(srvFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        if (errno == EADDRINUSE) {
            unlink(IPC_SOCK);
            if (bind(srvFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                close(srvFd);
                return false;
            }
        } else {
            close(srvFd);
            return false;
        }
    }

    listen(srvFd, 1);

    int cliFd = accept(srvFd, nullptr, nullptr);
    close(srvFd);   // 关闭监听套接字，只保留客户端连接

    if (cliFd < 0)
        return false;

    m_peerFd = cliFd;
    m_connected = true;
    if (m_connCb) m_connCb(true, m_connUser);

    char buf[MAX_RECV_BUF];
    while (m_running.load() && m_connected) {
        ssize_t r = recv(m_peerFd, buf, sizeof(buf), 0);
        if (r <= 0) {
            break;
        }
        m_recvBuf.append(buf, static_cast<size_t>(r));
        unpackBuffer();
    }

    handleDisconnect();
    return false;
}

// 工作线程主循环
void IpcNotifyRaw::workThread()
{
    // 指数退避参数
    unsigned int retryDelayMs = 100;       // 初始延迟 100ms
    const unsigned int maxRetryDelayMs = 5000; // 最大延迟 5s

    while (m_running.load()) {
        bool ok = false;
        if (m_role == ROLE_CLIENT) {
            ok = tryAsClient();
        } else {
            ok = becomeServer();
        }
        // tryAsClient/becomeServer 返回 false 表示连接结束（断开或失败）
        // 需要重试
        if(!m_running.load()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(retryDelayMs));
        retryDelayMs = std::min(retryDelayMs * 2,maxRetryDelayMs);
    }
}


#ifndef USERSTORE_H
#define USERSTORE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>

/**
 * @brief 用户账号本地存储（密文）
 *
 * 存储规则：
 * - 账号信息保存在本地 JSON 文件（QSettings 兼容格式）
 * - 密码使用 SHA-256 + 固定 salt 加密后存储，明文不落盘
 * - 配置文件不存在或为空时，自动创建默认 admin/admin 账号（管理员）
 * - 始终保证至少存在一个账号
 */
class UserStore : public QObject
{
    Q_OBJECT
public:
    // 角色枚举
    enum Role {
        RoleNormal = 0,   // 普通用户
        RoleAdmin  = 1    // 管理员
    };
    Q_ENUM(Role)

    struct UserInfo {
        QString userName;
        QString passwordHash;   // 密文
        Role    role;
    };

    explicit UserStore(QObject *parent = nullptr);

    // 获取全部用户列表
    QList<UserInfo> users() const;

    // 校验登录：成功返回 true 并填充 role
    bool validate(const QString& userName, const QString& password, Role& outRole) const;

    // 查询某个用户是否存在
    bool exists(const QString& userName) const;

    // 查询某个用户的角色（不存在返回 RoleNormal）
    Role roleOf(const QString& userName) const;

    // 添加用户；用户名已存在或为空返回 false
    bool addUser(const QString& userName, const QString& password, Role role);

    // 删除用户；规则：
    //  - 删除最后一个账号：拒绝（保证至少一个账号）
    //  - 不存在的账号：返回 false
    bool deleteUser(const QString& userName);

    // 修改密码
    bool updatePassword(const QString& userName, const QString& newPassword);

    // 修改角色
    bool updateRole(const QString& userName, Role newRole);

    // 当前用户数量
    int count() const;


    // 明文密码 -> 密文
    //    static QString hashPassword(const QString& plain);
    // 明文密码 -> 密文
    static QString encryPassword(const QString& plain);
    // 密文密码 -> 明文
    static QString decryPassword(const QString& encryted);
private:
    // 配置文件全路径
    QString configPath() const;
    // 从配置加载到内存
    void load();
    // 把内存写回配置
    void save() const;

    // 内存中的用户表：userName -> UserInfo
    QMap<QString, UserInfo> m_users;
};

#endif // USERSTORE_H

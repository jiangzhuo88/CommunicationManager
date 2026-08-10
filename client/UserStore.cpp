#include "UserStore.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QCryptographicHash>

// 固定 salt，避免单纯 SHA256 被彩虹表反查
static const char* kSalt = "CIM_2026_SALT";

UserStore::UserStore(QObject *parent)
    : QObject(parent)
{
    load();
    // 关键规则：完全无账号时，默认创建 admin/admin 管理员
    if (m_users.isEmpty()) {
        UserInfo admin;
        admin.userName     = "admin";
        admin.passwordHash = encryPassword("admin");
        admin.role         = RoleAdmin;
        m_users.insert(admin.userName, admin);
        save();
    }
}

//QString UserStore::hashPassword(const QString& plain)
//{
//    // SHA-256(plain + salt)
//    QByteArray input = plain.toUtf8() + QByteArray(kSalt);
//    QByteArray hash = QCryptographicHash::hash(input, QCryptographicHash::Sha256);
//    return QString::fromLatin1(hash.toHex());
//}

QString UserStore::configPath() const
{
    // 存放在可执行文件同目录下的 config/users.json
    QString dir = QCoreApplication::applicationDirPath() + "/config";
    QDir().mkpath(dir);
    return dir + "/users.json";
}

void UserStore::load()
{
    QFile f(configPath());
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
        return; // 配置文件不存在 -> m_users 为空，构造函数会补默认 admin
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    f.close();
    if (err.error != QJsonParseError::NoError) {
        return;
    }
    if (!doc.isObject()) {
        return;
    }
    QJsonObject root = doc.object();
    if (!root.contains("users") || !root.value("users").isArray()) {
        return;
    }
    QJsonArray arr = root.value("users").toArray();
    for (const QJsonValue& v : arr) {
        if (!v.isObject()) continue;
        QJsonObject o = v.toObject();
        UserInfo u;
        u.userName     = o.value("name").toString();
        u.passwordHash = o.value("pwd").toString();
        u.role         = static_cast<Role>(o.value("role").toInt(RoleNormal));
        if (!u.userName.isEmpty() && !u.passwordHash.isEmpty()) {
            m_users.insert(u.userName, u);
        }
    }
}

void UserStore::save() const
{
    QJsonObject root;
    QJsonArray arr;
    for (const UserInfo& u : m_users) {
        QJsonObject o;
        o.insert("name", u.userName);
        o.insert("pwd",  u.passwordHash);
        o.insert("role", static_cast<int>(u.role));
        arr.append(o);
    }
    root.insert("users", arr);

    QFile f(configPath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QJsonDocument doc(root);
        f.write(doc.toJson(QJsonDocument::Indented));
        f.close();
    }
}

QList<UserStore::UserInfo> UserStore::users() const
{
    return m_users.values();
}

bool UserStore::validate(const QString& userName, const QString& password, Role& outRole) const
{
    auto it = m_users.constFind(userName);
    if (it == m_users.constEnd()) return false;
    UserInfo info = it.value();
    QString realPassword = decryPassword(info.passwordHash);
    if (realPassword != password) return false;
    outRole = it.value().role;
    return true;
}

bool UserStore::exists(const QString& userName) const
{
    return m_users.contains(userName);
}

UserStore::Role UserStore::roleOf(const QString& userName) const
{
    auto it = m_users.constFind(userName);
    if (it == m_users.constEnd()) return RoleNormal;
    return it.value().role;
}

bool UserStore::addUser(const QString& userName, const QString& password, Role role)
{
    if (userName.isEmpty() || password.isEmpty()) return false;
    if (m_users.contains(userName)) return false;
    UserInfo u;
    u.userName     = userName;
    u.passwordHash = encryPassword(password);
    u.role         = role;
    m_users.insert(userName, u);
    save();
    return true;
}

bool UserStore::deleteUser(const QString& userName)
{
    if (!m_users.contains(userName)) return false;
    // 至少保留一个账号
    if (m_users.size() <= 1) return false;
    m_users.remove(userName);
    save();
    return true;
}

bool UserStore::updatePassword(const QString& userName, const QString& newPassword)
{
    if (newPassword.isEmpty()) return false;
    auto it = m_users.find(userName);
    if (it == m_users.end()) return false;
    it.value().passwordHash = encryPassword(newPassword);
    save();
    return true;
}

bool UserStore::updateRole(const QString& userName, Role newRole)
{
    auto it = m_users.find(userName);
    if (it == m_users.end()) return false;
    it.value().role = newRole;
    save();
    return true;
}

int UserStore::count() const
{
    return m_users.size();
}

QString UserStore::encryPassword(const QString &plain)
{
    QByteArray data = plain.toUtf8();
    QByteArray key(kSalt);
    for(int i = 0;i < data.size();i++)
    {
        data[i] = data[i] ^ key[i % key.size()];
    }
    return QString::fromLatin1(data.toBase64());
}

QString UserStore::decryPassword(const QString &encryted)
{
    QByteArray data = QByteArray::fromBase64(encryted.toLatin1());
    QByteArray key(kSalt);
    for(int i = 0;i < data.size();i++)
    {
        data[i] = data[i] ^ key[i % key.size()];
    }
    return QString::fromUtf8(data);
}

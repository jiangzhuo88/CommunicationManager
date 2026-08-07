#ifndef DMDATABASE_H
#define DMDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantList>
#include <QStringList>

/**
 * @brief 达梦数据库操作类
 * 负责连接达梦数据库、读取表数据
 */
class DmDatabase : public QObject
{
    Q_OBJECT

public:
    explicit DmDatabase(QObject *parent = nullptr);
    ~DmDatabase();

    /**
     * @brief 连接到达梦数据库
     * @param host 主机地址
     * @param port 端口
     * @param databaseName 数据库名
     * @param username 用户名
     * @param password 密码
     * @param schema 模式名(为空则使用当前用户对应的模式)
     * @return 连接是否成功
     */
    bool connect(const QString &host, int port, const QString &databaseName,
                 const QString &username, const QString &password,
                 const QString &schema = QString());

    /**
     * @brief 断开连接
     */
    void disconnect();

    /**
     * @brief 测试连接是否正常
     */
    bool isConnected();

    /**
     * @brief 设置模式名
     * @param schema 模式名
     */
    void setSchema(const QString &schema);

    /**
     * @brief 获取当前模式名
     */
    QString schema() const;

    /**
     * @brief 获取所有表名
     */
    QStringList getTableNames();

    /**
     * @brief 获取表的列信息
     * @param tableName 表名(不带模式前缀)
     * @return 列名列表
     */
    QStringList getTableColumns(const QString &tableName);

    /**
     * @brief 获取表的主键列名
     * @param tableName 表名(不带模式前缀)
     * @return 主键列名列表(复合主键可能有多个)
     */
    QStringList getPrimaryKey(const QString &tableName);

    /**
     * @brief 读取表数据
     * @param tableName 表名(不带模式前缀)
     * @return 数据列表(每行是一个QVariantList)
     */
    QList<QVariantList> readTableData(const QString &tableName);

    QList<QVariantMap> readTableDataForSqlOrder(const QString &sqlOrder);

    /**
     * @brief 获取最后的错误信息
     */
    QString lastError() const;

private:
    /**
     * @brief 获取完整表名(带模式前缀)
     */
    QString fullTableName(const QString &tableName) const;

    QSqlDatabase m_database;
    QString m_connectionName;
    QString m_lastError;
    QString m_schema;
};

#endif // DMDATABASE_H

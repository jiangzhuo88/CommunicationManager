#include "dmdatabase.h"
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DmDatabase::DmDatabase(QObject *parent)
    : QObject(parent)
    , m_connectionName("dm_connection")
{
}

DmDatabase::~DmDatabase()
{
    disconnect();
}

bool DmDatabase::connect(const QString &host, int port, const QString &databaseName,
                         const QString &username, const QString &password,
                         const QString &schema)
{
    // 如果已经连接，先断开
    if (m_database.isOpen()) {
        disconnect();
    }

    // 保存模式名
    m_schema = schema;

    // 添加达梦数据库驱动
    // 注意：需要安装达梦数据库驱动插件
    // 驱动名称可能为 QDM 或 QODBC，取决于具体驱动实现
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    m_database = QSqlDatabase::addDatabase("QDM", m_connectionName);
    
    if (!m_database.isValid()) {
        // 如果QDM驱动不存在，尝试使用ODBC
        m_database = QSqlDatabase::addDatabase("QODBC", m_connectionName);
    }

    m_database.setHostName(host);
    m_database.setPort(port);
    m_database.setDatabaseName(databaseName);
    m_database.setUserName(username);
    m_database.setPassword(password);

    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        qWarning() << "达梦数据库连接失败:" << m_lastError;
        return false;
    }

    // 如果指定了模式，设置当前模式
    if (!m_schema.isEmpty()) {
        QSqlQuery query(m_database);
        QString setSchemaSql = QString("SET SCHEMA %1").arg(m_schema.toUpper());
        if (!query.exec(setSchemaSql)) {
            qWarning() << "设置模式失败:" << query.lastError().text();
            // 不影响连接，继续使用
        } else {
            qDebug() << "当前模式已设置为:" << m_schema;
        }
    }

    qDebug() << "达梦数据库连接成功";
    return true;
}

void DmDatabase::setSchema(const QString &schema)
{
    m_schema = schema;

    if (isConnected() && !m_schema.isEmpty()) {
        QSqlQuery query(m_database);
        QString setSchemaSql = QString("SET SCHEMA %1").arg(m_schema.toUpper());
        query.exec(setSchemaSql);
    }
}

QString DmDatabase::schema() const
{
    return m_schema;
}

QString DmDatabase::fullTableName(const QString &tableName) const
{
    if (!m_schema.isEmpty()) {
        return QString("%1.%2").arg(m_schema.toUpper()).arg(tableName.toUpper());
    }
    return tableName.toUpper();
}

void DmDatabase::disconnect()
{
    if (m_database.isOpen()) {
        m_database.close();
    }

    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
        Q_UNUSED(db)
    }
    
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool DmDatabase::isConnected()
{
    return m_database.isOpen() && m_database.isValid();
}

QStringList DmDatabase::getTableNames()
{
    QStringList tables;
    
    if (!isConnected()) {
        m_lastError = "数据库未连接";
        return tables;
    }

    QSqlQuery query(m_database);
    QString sql;

    if (!m_schema.isEmpty()) {
        // 指定了模式，从ALL_TABLES查询该模式下的表
        sql = QString("SELECT TABLE_NAME FROM ALL_TABLES WHERE OWNER = '%1' ORDER BY TABLE_NAME")
                  .arg(m_schema.toUpper());
    } else {
        // 未指定模式，查询当前用户的表
        sql = "SELECT TABLE_NAME FROM USER_TABLES ORDER BY TABLE_NAME";
    }
    
    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        qWarning() << "查询表名失败:" << m_lastError;
        return tables;
    }

    while (query.next()) {
        tables << query.value(0).toString();
    }

    return tables;
}

QStringList DmDatabase::getTableColumns(const QString &tableName)
{
    QStringList columns;
    
    if (!isConnected()) {
        m_lastError = "数据库未连接";
        return columns;
    }

    QSqlQuery query(m_database);
    QString sql;

    if (!m_schema.isEmpty()) {
        sql = QString("SELECT COLUMN_NAME FROM ALL_TAB_COLUMNS "
                      "WHERE OWNER = '%1' AND TABLE_NAME = '%2' "
                      "ORDER BY COLUMN_ID")
                  .arg(m_schema.toUpper(), tableName.toUpper());
    } else {
        sql = QString("SELECT COLUMN_NAME FROM USER_TAB_COLUMNS "
                      "WHERE TABLE_NAME = '%1' ORDER BY COLUMN_ID")
                  .arg(tableName.toUpper());
    }
    
    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        qWarning() << "查询列信息失败:" << m_lastError;
        return columns;
    }

    while (query.next()) {
        columns << query.value(0).toString();
    }

    return columns;
}

QStringList DmDatabase::getPrimaryKey(const QString &tableName)
{
    QStringList pkColumns;
    
    if (!isConnected()) {
        m_lastError = "数据库未连接";
        return pkColumns;
    }

    QSqlQuery query(m_database);
    QString sql;

    if (!m_schema.isEmpty()) {
        sql = QString("SELECT CU.COLUMN_NAME FROM ALL_CONSTRAINTS C "
                      "JOIN ALL_CONS_COLUMNS CU "
                      "ON C.CONSTRAINT_NAME = CU.CONSTRAINT_NAME "
                      "AND C.OWNER = CU.OWNER "
                      "WHERE C.OWNER = '%1' "
                      "AND C.TABLE_NAME = '%2' "
                      "AND C.CONSTRAINT_TYPE = 'P' "
                      "ORDER BY CU.POSITION")
                  .arg(m_schema.toUpper(), tableName.toUpper());
    } else {
        sql = QString("SELECT CU.COLUMN_NAME FROM USER_CONSTRAINTS C "
                      "JOIN USER_CONS_COLUMNS CU "
                      "ON C.CONSTRAINT_NAME = CU.CONSTRAINT_NAME "
                      "WHERE C.TABLE_NAME = '%1' "
                      "AND C.CONSTRAINT_TYPE = 'P' "
                      "ORDER BY CU.POSITION")
                  .arg(tableName.toUpper());
    }
    
    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        qWarning() << "查询主键失败:" << m_lastError;
        return pkColumns;
    }

    while (query.next()) {
        pkColumns << query.value(0).toString();
    }

    return pkColumns;
}

QList<QVariantList> DmDatabase::readTableData(const QString &tableName)
{
    QList<QVariantList> data;
    
    if (!isConnected()) {
        m_lastError = "数据库未连接";
        return data;
    }

    // 获取列信息
    QStringList columns = getTableColumns(tableName);
    if (columns.isEmpty()) {
        return data;
    }

    // 读取数据，使用完整表名（带模式前缀）
    QSqlQuery query(m_database);
    QString sql = QString("SELECT * FROM %1").arg(fullTableName(tableName));
    
    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        qWarning() << "查询数据失败:" << m_lastError;
        return data;
    }

    while (query.next()) {
        QVariantList row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); ++i) {
            row << record.value(i);
        }
        data << row;
    }

    return data;
}

QList<QVariantMap> DmDatabase::readTableDataForSqlOrder(const QString &sqlOrder)
{
    QList<QVariantMap> data;
    QSqlQuery query(m_database);
    QString sql = sqlOrder;
    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        qWarning() << "查询数据失败:" << m_lastError;
        return data;
    }

    while (query.next()) {
        QVariantMap row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); ++i) {
            row.insert(record.fieldName(i),record.value(i));
        }
        data << row;
    }

    return data;

}

QString DmDatabase::lastError() const
{
    return m_lastError;
}

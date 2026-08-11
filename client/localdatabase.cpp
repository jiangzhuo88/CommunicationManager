#include "localdatabase.h"
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlQuery>
#include <QFile>
#include <QDebug>

LocalDatabase::LocalDatabase(QObject *parent)
    : QObject(parent)
    , m_connectionName("local_connection")
{
}

LocalDatabase::~LocalDatabase()
{
    close();
}

bool LocalDatabase::initialize(const QString &dbPath)
{
    m_dbPath = dbPath;

    if (m_database.isOpen()) {
        close();
    }

    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    m_database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_database.setDatabaseName(m_dbPath);

    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        qWarning() << "本地数据库打开失败:" << m_lastError;
        return false;
    }

    QSqlQuery query(m_database);
    QString sql = "CREATE TABLE IF NOT EXISTS table_mapping_info ("
                  "table_name TEXT PRIMARY KEY, "
                  "primary_keys TEXT, "
                  "last_sync_time TEXT"
                  ")";
    
    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        qWarning() << "创建映射信息表失败:" << m_lastError;
        return false;
    }

    qDebug() << "本地数据库初始化成功:" << m_dbPath;
    return true;
}

void LocalDatabase::close()
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

bool LocalDatabase::createTable(const QString &tableName, const QStringList &columns,
                                const QStringList &primaryKeys)
{
    if (!m_database.isOpen()) {
        m_lastError = "数据库未打开";
        return false;
    }

    if (columns.isEmpty()) {
        m_lastError = "列列表为空";
        return false;
    }

    QSqlQuery query(m_database);
    
//    // 先删旧表(结构可能变了)
//    QString dropSql = QString("DROP TABLE IF EXISTS %1").arg(tableName);
//    query.exec(dropSql);

    // 建表:完全镜像源表列结构,SQLite不支持直接指定主键列名的普通建表,用列声明
    QString sql = QString("CREATE TABLE IF NOT EXISTS %1 (").arg(tableName);
    for (int i = 0; i < columns.size(); ++i) {
        sql += QString("%1 TEXT").arg(columns[i]);
        if (i < columns.size() - 1) {
            sql += ", ";
        }
    }
    if (!primaryKeys.isEmpty()) {
        sql += QString(", PRIMARY KEY (%1)").arg(primaryKeys.join(", "));
    }
    sql += ")";

    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        qWarning() << "创建表失败:" << m_lastError;
        return false;
    }

    // 保存映射信息
    QString pkStr = primaryKeys.join(",");
    QString infoSql = QString("INSERT OR REPLACE INTO table_mapping_info "
                              "(table_name, primary_keys, last_sync_time) "
                              "VALUES ('%1', '%2', datetime('now'))")
            .arg(tableName, pkStr);
    if (!query.exec(infoSql)) {
        qWarning() << "保存映射信息失败:" << query.lastError().text();
    }

    qDebug() << "表创建成功:" << tableName;
    return true;
}

bool LocalDatabase::dropTable(const QString &tableName)
{
    if (!m_database.isOpen()) {
        m_lastError = "数据库未打开";
        return false;
    }

    QSqlQuery query(m_database);
    QString sql = QString("DROP TABLE IF EXISTS %1").arg(tableName);

    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        qWarning() << "删除表失败:" << m_lastError;
        return false;
    }

    // 同时删除映射信息
    QString infoSql = QString("DELETE FROM table_mapping_info WHERE table_name = '%1'").arg(tableName);
    query.exec(infoSql);

    return true;
}

QStringList LocalDatabase::getTableNames()
{
    QStringList tables;
    
    if (!m_database.isOpen()) {
        m_lastError = "数据库未打开";
        return tables;
    }

    tables = m_database.tables(QSql::Tables);
    
    tables.removeAll("table_mapping_info");
    
    return tables;
}

QStringList LocalDatabase::getPrimaryKey(const QString &tableName)
{
    QStringList pkColumns;
    
    if (!m_database.isOpen()) {
        m_lastError = "数据库未打开";
        return pkColumns;
    }

    QSqlQuery query(m_database);
    QString sql = QString("SELECT primary_keys FROM table_mapping_info WHERE table_name = '%1'")
            .arg(tableName);
    
    if (query.exec(sql) && query.next()) {
        QString pkStr = query.value(0).toString();
        if (!pkStr.isEmpty()) {
            pkColumns = pkStr.split(",");
        }
    }

    return pkColumns;
}

bool LocalDatabase::syncData(const QString &tableName, const QList<QVariantList> &data)
{
    if (!m_database.isOpen()) {
        m_lastError = "数据库未打开";
        return false;
    }

    m_database.transaction();

    QSqlQuery query(m_database);
    QString clearSql = QString("DELETE FROM %1").arg(tableName);
    
    if (!query.exec(clearSql)) {
        m_lastError = query.lastError().text();
        m_database.rollback();
        qWarning() << "清空表数据失败:" << m_lastError;
        return false;
    }

    if (data.isEmpty()) {
        m_database.commit();
        updateSyncTime(tableName);
        return true;
    }

    QStringList columns = getTableColumns(tableName);
    if (columns.isEmpty()) {
        m_database.rollback();
        return false;
    }

    QString insertSql = QString("INSERT INTO %1 (").arg(tableName);
    for (int i = 0; i < columns.size(); ++i) {
        insertSql += columns[i];
        if (i < columns.size() - 1) {
            insertSql += ", ";
        }
    }
    insertSql += ") VALUES (";
    for (int i = 0; i < columns.size(); ++i) {
        insertSql += "?";
        if (i < columns.size() - 1) {
            insertSql += ", ";
        }
    }
    insertSql += ")";

    query.prepare(insertSql);
    
    for (const QVariantList &row : data) {
        for (int i = 0; i < row.size() && i < columns.size(); ++i) {
            query.addBindValue(row[i]);
        }
        
        if (!query.exec()) {
            m_lastError = query.lastError().text();
            m_database.rollback();
            qWarning() << "插入数据失败:" << m_lastError;
            return false;
        }
    }

    m_database.commit();
    updateSyncTime(tableName);
    qDebug() << "数据同步成功,共" << data.size() << "行";
    return true;
}

bool LocalDatabase::syncData(const QString &tableName, const QString &primaryKey, const QList<QMap<QString, QString> > &data)
{
    if(!m_database.isOpen())
    {
        m_lastError = "数据库未打开";
    }
    if(data.isEmpty())
    {
        return true;
    }
    m_database.transaction();
    QSqlQuery query(m_database);
    QStringList columns = getTableColumns(tableName);
    if(columns.isEmpty())
    {
#if 1
        m_lastError = "获取表字段失败";
        m_database.rollback();
        return false;
#else
        const QMap<QString,QString> &firstRow = data.first();
        QStringList fieldDef;
        for(auto it = firstRow.begin();it != firstRow.end();++it)
        {
            const QString &col = it.key();
            if(col == primaryKey)
            {
                fieldDef<<QString("%1 TEXT PRIMARY KEY").arg(col);
            }
            else
            {
                fieldDef << QString("%1 TEXT").arg(col);
            }
        }
        QString createSql = QString("CREATE TABLE IF NOT EXIST %1 (%2)").arg(tableName).arg(fieldDef.join(","));
        if(!query.exec(createSql))
        {
            m_lastError = "建表失败："+query.lastError().text();
            m_database.rollback();
            return false;
        }
        columns = getTableColumns(tableName);
        if(columns.isEmpty())
        {
            m_lastError = "建表后，获取字段失败";
            m_database.rollback();
            return false;
        }
#endif
    }
    for(const auto &rowMap : data)
    {
        QString pkValue = rowMap.value(primaryKey);
        if(pkValue.isEmpty())
        {
            qWarning()<<"存在主键为空的数据，跳过";
            continue;
        }
        QString checkSql = QString("SELECT 1 FROM %1 WHERE %2 = ?").arg(tableName,primaryKey);
        query.prepare(checkSql);
        query.addBindValue(pkValue);
        if(!query.exec())
        {
            m_lastError = query.lastError().text();
            qWarning()<<"查询主键失败："<<m_lastError;
            m_database.rollback();
            return false;
        }
        if(query.next())
        {
            QStringList updateSet;
            for(const auto &col : columns)
            {
                if(col == primaryKey)
                {
                    continue;
                }
                updateSet<<QString("%1 = ?").arg(col);
            }
            QString updateSql = QString("UPDATE %1 SET %2 WHERE %3 = ?").arg(tableName).arg(updateSet.join(",")).arg(primaryKey);
            qDebug()<<"updateSql:"<<updateSql;
            query.prepare(updateSql);
            for(const auto &col : columns)
            {
                if(col == primaryKey)
                {
                    continue;
                }
                query.addBindValue(rowMap.value(col));
            }
            query.addBindValue(pkValue);
        }
        else
        {
            QStringList fieldNames;
            QStringList placeholders;
            for(const auto &col : columns)
            {
                fieldNames << col;
                placeholders << "?";
            }
            QString inserSql = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(tableName).arg(fieldNames.join(",")).arg(placeholders.join(","));
            query.prepare(inserSql);
            for(const auto &col : columns)
            {
                query.addBindValue(rowMap.value(col));
            }
        }
        if(!query.exec())
        {
            m_lastError = query.lastError().text();
            qWarning()<<"单行更新/插入失败"<<m_lastError;
            m_database.rollback();
            return false;
        }
    }
    m_database.commit();
    updateSyncTime(tableName);
    qDebug()<<"增量同步完成，共处理"<<data.size()<<"条";
    return true;
}

bool LocalDatabase::dropData(const QString &tableName, const QString& promaryKey,const QStringList& pkvalueList)
{
    if(!m_database.isOpen())
    {
        m_lastError = "数据库未打开";
    }
    if(pkvalueList.isEmpty())
    {
        return true;
    }
    m_database.transaction();
    QSqlQuery query(m_database);
    QStringList placeHolder;
    for(int i = 0;i<pkvalueList.size();++i)
    {
        placeHolder << "?";
    }
    QString sql = QString("DELETE FROM %1 WHERE %2 IN (%3)").arg(tableName).arg(promaryKey).arg(placeHolder.join(","));
    query.prepare(sql);
    for(const QString &val : pkvalueList)
    {
        query.addBindValue(val);
    }
    if(!query.exec())
    {
        m_lastError = "批量删除失败：" + query.lastError().text();
        qWarning()<<m_lastError;
        m_database.rollback();
        return false;
    }
    m_database.commit();
    qDebug()<<"批量删除执行成功，影响行数："<<query.numRowsAffected();
    return true;
}

bool LocalDatabase::dropData(const QString &tableName)
{
    if(!m_database.isOpen())
    {
        m_lastError = "数据库未打开";
    }
    m_database.transaction();
    QSqlQuery query(m_database);

    QString sql = QString("DELETE FROM %1").arg(tableName);
    query.prepare(sql);
    if(!query.exec())
    {
        m_lastError = "清空表失败：" + query.lastError().text();
        qWarning()<<m_lastError;
        m_database.rollback();
        return false;
    }
    m_database.commit();
    qDebug()<<"清空表执行成功，影响行数："<<query.numRowsAffected();
    return true;
}

bool LocalDatabase::updateCell(const QString &tableName, const QString &columnName,
                               const QVariantMap &primaryKeyValues, const QVariant &value)
{
    if (!m_database.isOpen()) {
        m_lastError = "数据库未打开";
        return false;
    }

    if (primaryKeyValues.isEmpty()) {
        m_lastError = "没有主键信息,无法定位行";
        return false;
    }

    QSqlQuery query(m_database);
    QString sql = QString("UPDATE %1 SET %2 = ? WHERE ").arg(tableName).arg(columnName);
    
    QStringList pkColumns = primaryKeyValues.keys();
    for (int i = 0; i < pkColumns.size(); ++i) {
        sql += QString("%1 = ?").arg(pkColumns[i]);
        if (i < pkColumns.size() - 1) {
            sql += " AND ";
        }
    }
    
    query.prepare(sql);
    query.addBindValue(value);
    for (const QString &pk : pkColumns) {
        query.addBindValue(primaryKeyValues[pk]);
    }

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "更新单元格失败:" << m_lastError;
        return false;
    }

    return true;
}

QList<QVariantMap> LocalDatabase::getTableData(const QString &tableName)
{
    QList<QVariantMap> data;
    
    if (!m_database.isOpen()) {
        m_lastError = "数据库未打开";
        return data;
    }

    QSqlQuery query(m_database);
    QString sql = QString("SELECT * FROM %1").arg(tableName);
    
    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        qWarning() << "查询数据失败:" << m_lastError;
        return data;
    }

    while (query.next()) {
        QVariantMap row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); ++i) {
            row .insert(record.fieldName(i),record.value(i));
        }
        data << row;
    }

    return data;
}

QList<QVariantMap> LocalDatabase::queryRows(const QString &sql, const QVariantList &bindValues)
{
    QList<QVariantMap> result;

    if (!m_database.isOpen()) {
        m_lastError = "数据库未打开";
        return result;
    }

    QSqlQuery query(m_database);
    query.prepare(sql);
    for (const QVariant &val : bindValues) {
        query.addBindValue(val);
    }

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "查询失败:" << m_lastError << " SQL:" << sql;
        return result;
    }

    while (query.next()) {
        QVariantMap row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); ++i) {
            row.insert(record.fieldName(i), record.value(i));
        }
        result << row;
    }

    return result;
}

bool LocalDatabase::executeSql(const QString &sql, const QVariantList &bindValues)
{
    if (!m_database.isOpen()) {
        m_lastError = "数据库未打开";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(sql);
    for (const QVariant &val : bindValues) {
        query.addBindValue(val);
    }

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "SQL执行失败:" << m_lastError << " SQL:" << sql;
        return false;
    }

    return true;
}

QStringList LocalDatabase::getTableColumns(const QString &tableName)
{
    QStringList columns;
    
    if (!m_database.isOpen()) {
        m_lastError = "数据库未打开";
        return columns;
    }

    QSqlQuery query(m_database);
    QString sql = QString("PRAGMA table_info(%1)").arg(tableName);
    
    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        qWarning() << "获取列信息失败:" << m_lastError;
        return columns;
    }

    while (query.next()) {
        columns << query.value(1).toString();
    }

    return columns;
}

void LocalDatabase::updateSyncTime(const QString &tableName)
{
    QSqlQuery query(m_database);
    QString sql = QString("UPDATE table_mapping_info SET last_sync_time = datetime('now') "
                          "WHERE table_name = '%1'").arg(tableName);
    query.exec(sql);
}

QSqlDatabase LocalDatabase::database() const
{
    return m_database;
}

QString LocalDatabase::lastError() const
{
    return m_lastError;
}

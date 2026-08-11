#ifndef LOCALDATABASE_H
#define LOCALDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class LocalDatabase : public QObject
{
    Q_OBJECT

public:
    explicit LocalDatabase(QObject *parent = nullptr);
    ~LocalDatabase();

    bool initialize(const QString &dbPath = "local_mapping.db");
    void close();

    bool createTable(const QString &tableName, const QStringList &columns,
                     const QStringList &primaryKeys = QStringList());
    bool dropTable(const QString &tableName);

    QStringList getTableNames();
    QStringList getTableColumns(const QString &tableName);
    QStringList getPrimaryKey(const QString &tableName);

    bool syncData(const QString &tableName, const QList<QVariantList> &data);
    bool syncData(const QString &tableName, const QString &primaryKey, const QList<QMap<QString, QString> > &data);
    bool dropData(const QString &tableName, const QString &promaryKey, const QStringList &pkvalueList);
    bool dropData(const QString &tableName);
    bool updateCell(const QString &tableName, const QString &columnName,
                    const QVariantMap &primaryKeyValues, const QVariant &value);

    QList<QVariantMap> getTableData(const QString &tableName);

    QList<QVariantMap> queryRows(const QString &sql, const QVariantList &bindValues = QVariantList());
    bool executeSql(const QString &sql, const QVariantList &bindValues = QVariantList());

    QSqlDatabase database() const;
    QString lastError() const;

private:
    void updateSyncTime(const QString &tableName);

    QSqlDatabase m_database;
    QString m_connectionName;
    QString m_lastError;
    QString m_dbPath;
};

#endif // LOCALDATABASE_H

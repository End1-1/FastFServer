#ifndef MSQLDATABASE_H
#define MSQLDATABASE_H

#include "databaseresult.h"
#include <QSqlDatabase>

class MSqlDatabase
{
public:
    MSqlDatabase();
    MSqlDatabase(const QString &host, const QString &file, const QString &username, const QString &password);
    ~MSqlDatabase();
    static bool setConnectionParams(const QString &host, const QString &file, const QString &username, const QString &password);
    bool configure();
    bool open();
    bool isOpen();
    bool close();
    bool commit();
    bool select(const QString &sql, QMap<QString, QVariant> &bindValues, DatabaseResult &dr, bool autoopen = true);
    bool update(const QString &table, QMap<QString, QVariant> &bindValues, const QVariant &id);
    bool insert(const QString &table, QMap<QString, QVariant> &bindValues);
    bool deleteRecord(const QString &table, const QVariant &id);
    QString lastError();
    int genId(const QString &generator);

private:
    QSqlDatabase fDb;
    QString fLastError;
    QString fCurrentDbNum;
    static QString fDbNum;
    void init();
    QString lastQuery(QSqlQuery &q);
    void setLastError(QSqlQuery &q);
    void setLastError(QSqlDatabase &db);
    static QString fHost;
    static QString fFile;
    static QString fUser;
    static QString fPass;
};

#endif // MSQLDATABASE_H

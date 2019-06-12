#include "msqldatabase.h"
#include "mdefined.h"
#include "mtfilelog.h"
#include "dbmutexlocker.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QMutexLocker>

QString MSqlDatabase::fDbNum = "0";
QString MSqlDatabase::fHost;
QString MSqlDatabase::fFile;
QString MSqlDatabase::fUser;
QString MSqlDatabase::fPass;


MSqlDatabase::MSqlDatabase()
{
    init();
    configure();
}

MSqlDatabase::MSqlDatabase(const QString &host, const QString &file, const QString &username, const QString &password)
{
    init();
    fDb.setHostName(host);
    fDb.setDatabaseName(file);
    fDb.setUserName(username);
    fDb.setPassword(password);
    if (!fDb.open()) {
        setLastError(fDb);
    }
}

MSqlDatabase::~MSqlDatabase()
{
    if (fLastError.isEmpty()) {
        if (fDb.isOpen()) {
            fDb.commit();
        }
    }
    fDb = QSqlDatabase::addDatabase("QIBASE");
    QSqlDatabase::removeDatabase(fDbNum);
}

bool MSqlDatabase::setConnectionParams(const QString &host, const QString &file, const QString &username, const QString &password)
{
    fHost = host;
    fFile = file;
    fUser = username;
    fPass = password;
    return true;
}

bool MSqlDatabase::configure()
{
    if (fDb.isOpen()) {
        fDb.close();
    }
    if (fFile.isEmpty()) {
        fLastError = "Database name is empty";
        return false;
    }
    fDb.setHostName(fHost);
    fDb.setDatabaseName(fFile);
    fDb.setUserName(fUser);
    fDb.setPassword(fPass);
    if (!fDb.open()) {
        setLastError(fDb);
        MTFileLog::createLog(__LOG_OTHER, fHost + ":" + fFile + ":" + fUser + ":" + fPass + ":" + fLastError);
    }
    return fDb.isOpen();
}

bool MSqlDatabase::open()
{
    if (!fDb.open()) {
        setLastError(fDb);
        return false;
    }
    return true;
}

bool MSqlDatabase::isOpen()
{
    return fDb.isOpen();
}

bool MSqlDatabase::close()
{
    fDb.close();
    return true;
}

bool MSqlDatabase::commit()
{
    return fDb.commit();
}

bool MSqlDatabase::select(const QString &sql, QMap<QString, QVariant> &bindValues, DatabaseResult &dr, bool autoopen)
{
    if (autoopen) {
        if (!fDb.open()) {
            setLastError(fDb);
            return false;
        }
    }
    QSqlQuery q(fDb);
    if (!q.prepare(sql)) {
        setLastError(q);
        bindValues.clear();
        fDb.close();
        return false;
    }
    foreach (QString k, bindValues.keys()) {
        q.bindValue(k, bindValues[k]);
    }
    if (!q.exec()) {
        setLastError(q);
        bindValues.clear();
        return false;
    }
    bindValues.clear();
    dr.fetchResult(q);
    qDebug() << lastQuery(q);
    if (autoopen) {
        fDb.close();
    }
    return true;
}

bool MSqlDatabase::update(const QString &table, QMap<QString, QVariant> &bindValues, const QVariant &id)
{
    QString sql = "update " + table + " set ";
    bool first = true;
    for (QMap<QString, QVariant>::const_iterator it = bindValues.begin(); it != bindValues.end(); it++) {
        if (first) {
            first = false;
        } else {
            sql += ",";
        }
        sql += QString(it.key()).remove(0, 1) + "=" + it.key();
    }
    QString where = "where id=:id";
    sql += " " + where;
    QSqlQuery q(fDb);
    if (!q.prepare(sql)) {
        setLastError(q);
        bindValues.clear();
        return false;
    }
    foreach (QString k, bindValues.keys()) {
        q.bindValue(k, bindValues[k]);
    }
    q.bindValue(":id", id);
    bindValues.clear();
    if (!q.exec()) {
        setLastError(q);
        return false;
    }
    qDebug() << lastQuery(q);
    return true;
}

bool MSqlDatabase::insert(const QString &table, QMap<QString, QVariant> &bindValues)
{
    QString sql = "insert into %1 (%2) values (%3)";
    QString fields = "";
    QString bind = "";
    bool first = true;
    for (QMap<QString, QVariant>::const_iterator it = bindValues.begin(); it != bindValues.end(); it++) {
        if (first) {
            first = false;
        } else {
            fields += ",";
            bind += ",";
        }
        fields += QString(it.key()).remove(0, 1);
        bind += it.key();
    }
    sql = sql.arg(table).arg(fields).arg(bind);

    QSqlQuery q(fDb);
    if (!q.prepare(sql)) {
        setLastError(q);
        qDebug() << lastQuery(q);
        bindValues.clear();
        return false;
    }
    for (QMap<QString, QVariant>::const_iterator it = bindValues.begin(); it != bindValues.end(); it++) {
        q.bindValue(it.key(), it.value());
    }
    bindValues.clear();
    if (!q.exec()) {
        setLastError(q);
        qDebug() << lastQuery(q);
        return false;
    }
    qDebug() << lastQuery(q);
    return true;
}

bool MSqlDatabase::deleteRecord(const QString &table, const QVariant &id)
{
    QString sql = "delete from " + table + " where id=:id";
    QMap<QString, QVariant> v;
    v[":id"] = id;
    DatabaseResult dr;
    return select(sql, v, dr);
}

QString MSqlDatabase::lastQuery(QSqlQuery &q)
{
    QString sql = q.lastQuery();
    QMapIterator<QString, QVariant> it(q.boundValues());
    while (it.hasNext()) {
        it.next();
        QVariant value = it.value();
        switch (it.value().type()) {
        case QVariant::String:
            value = QString("'%1'").arg(value.toString().replace("'", "''"));
            break;
        case QVariant::Date:
            value = QString("'%1'").arg(value.toDate().toString());
            break;
        case QVariant::DateTime:
            value = QString("'%1'").arg(value.toDateTime().toString());
            break;
        case QVariant::Double:
            value = QString("%1").arg(value.toDouble());
            break;
        case QVariant::Int:
            value = QString("%1").arg(value.toInt());
            break;
        default:
            break;
        }
        sql.replace(it.key(), value.toString());
    }
#ifdef QT_DEBUG
    QFile f("./sql.log");
    f.open(QIODevice::Append);
    f.write(sql.toUtf8());
    f.write("\r\n");
    f.close();
#endif
    return sql;
}

void MSqlDatabase::setLastError(QSqlQuery &q)
{
    fLastError = q.lastError().databaseText();
    qDebug() << fLastError;
    qDebug() << q.lastQuery();
}

void MSqlDatabase::setLastError(QSqlDatabase &db)
{
    fLastError = db.lastError().databaseText();
    qDebug() << fLastError;
}

QString MSqlDatabase::lastError()
{
    return fLastError;
}

int MSqlDatabase::genId(const QString &generator)
{
    DatabaseResult dr;
    QString query = QString("select gen_id(%1, 1) as newid from rdb$database").arg(generator);
    QMap<QString, QVariant> bv;
    select(query, bv, dr, false);
    return dr.value("NEWID").toInt();
}

void MSqlDatabase::init()
{
    QMutexLocker ml(&__mxDb);
    fCurrentDbNum = QString::number((fDbNum.toInt() + 1) * -1);
    fDbNum = fCurrentDbNum;
    fDb = QSqlDatabase::addDatabase("QIBASE", fCurrentDbNum);
}

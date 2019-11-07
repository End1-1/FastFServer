#include "sqlthread.h"
#include <QDebug>
#include <QSqlField>
#include <QSqlRecord>
#include <QCoreApplication>
#include <QMutex>

int SqlThread::mDBCounter = 0;
static QMutex mDbMutex;

SqlThread::SqlThread(QObject *parent) :
    QThread(parent)
{
    fCursorPos = -1;
}

SqlThread::SqlThread(const QString &dbHost, const QString &dbName, const QString &dbUser, const QString &dbPass, const QString &dbOption, QObject *parent) :
    QThread(parent)
{
    setDatabase(dbHost, dbName, dbPass, dbUser, dbOption);
}

void SqlThread::setDatabase(const QString &dbHost, const QString &dbName, const QString &dbUser, const QString &dbPass, const QString &dbOption)
{
    QMutexLocker ml(&mDbMutex);
    mDBNumber = QString("%1-%2").arg("SQLTHREAD").arg(mDBCounter++);
    qDebug() << mDBNumber;
    mDB = QSqlDatabase::addDatabase("QIBASE", mDBNumber);
    mDB.setHostName(dbHost);
    mDB.setDatabaseName(dbName);
    mDB.setUserName(dbUser);
    mDB.setPassword(dbPass);
    mDB.setConnectOptions(dbOption);
}

SqlThread::~SqlThread()
{
    mDB = QSqlDatabase::addDatabase("QIBASE");
    QSqlDatabase::removeDatabase(mDBNumber);
}

QVariant &SqlThread::operator[](const QString &name)
{
    return fBindValues[name];
}

void SqlThread::setSqlQuery(const QString &sqlText)
{
    mSqlText = sqlText;
}

void SqlThread::bind(const QString &key, const QVariant &value)
{
    fBindValues[key] = value;
}

bool SqlThread::nextRow()
{
    if (fCursorPos < rowCount() - 1 && rowCount() > 0) {
        fCursorPos++;
        return true;
    }
    return false;
}

int SqlThread::rowCount()
{
    return fDbRows.count();
}

void SqlThread::run()
{
    if (!mDB.open()) {
        qDebug() << mDB.lastError().databaseText();
    }

    QSqlQuery *q = new QSqlQuery(mDB);

    if (!q->prepare(mSqlText)) {
        qDebug() << q->lastError().databaseText();
    }
    for (QMap<QString, QVariant>::const_iterator it = fBindValues.begin(); it != fBindValues.end(); it++) {
        q->bindValue(it.key(), it.value());
    }
    if (!q->exec()) {
        qDebug() << q->lastError().databaseText();
    }
    bool isSelect = q->isSelect();
    if (!isSelect) {
        isSelect = mSqlText.mid(0, 4).compare("call", Qt::CaseInsensitive) == 0;
    }

    if (isSelect) {
        fCursorPos = -1;
        fColumns.clear();
        QSqlRecord r = q->record();
        for (int i = 0; i < r.count(); i++) {
            fColumns[r.field(i).name().toLower()] = i;
        }
        int colCount = r.count();
        fDbRows.clear();
        while (q->next()) {
            QList<QVariant> row;
            for (int i = 0; i < colCount; i++) {
                row << q->value(i);
            }
            fDbRows << row;
        }
    }
    qDebug() << lastQuery(q);
    delete q;
    mDB.close();
}

QString SqlThread::lastQuery(QSqlQuery *q) const
{
    QString sql = q->lastQuery();
    QMapIterator<QString, QVariant> it(q->boundValues());
    while (it.hasNext()) {
        it.next();
        QVariant value = it.value();
        switch (it.value().type()) {
        case QVariant::String:
            value = QString("'%1'").arg(value.toString().replace("'", "''"));
            break;
        case QVariant::Date:
            value = QString("'%1'").arg(value.toDate().toString("yyyy-MM-dd"));
            break;
        case QVariant::DateTime:
            value = QString("'%1'").arg(value.toDateTime().toString("yyyy-MM-dd HH:mm:ss"));
            break;
        case QVariant::Double:
            value = QString("%1").arg(value.toDouble());
            break;
        case QVariant::Int:
            value = QString("%1").arg(value.toInt());
            break;
        case QVariant::Time:
            value = QString("'%1'").arg(value.toTime().toString("HH:mm:ss"));
            break;
        default:
            break;
        }
        sql.replace(QRegExp(it.key() + "\\b"), value.toString());
    }
    return sql;
}

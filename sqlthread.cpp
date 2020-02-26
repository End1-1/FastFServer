#include "sqlthread.h"
#include <QDebug>
#include <QSqlField>
#include <QSqlRecord>
#include <QCoreApplication>

int SqlThread::mDBCounter = 0;

SqlThread::SqlThread() :
    ThreadObject()
{
    mCursorPos = -1;
}

SqlThread::SqlThread(const QString &dbHost, const QString &dbName, const QString &dbUser, const QString &dbPass, const QString &dbOption) :
    SqlThread()
{
    setDatabase(dbHost, dbName, dbUser, dbPass, dbOption);
}

void SqlThread::setDatabase(const QString &dbHost, const QString &dbName, const QString &dbUser, const QString &dbPass, const QString &dbOption)
{
    mDBNumber = QString("%1-%2").arg("CONSTRUCTOR SQLTHREAD").arg(mDBCounter++);
    qDebug() << "Constructor" << mDBNumber;
    mDB = QSqlDatabase::addDatabase("QIBASE", mDBNumber);
    mDB.setHostName(dbHost);
    mDB.setDatabaseName(dbName);
    mDB.setUserName(dbUser);
    mDB.setPassword(dbPass);
    mDB.setConnectOptions(dbOption);
}

SqlThread::~SqlThread()
{
    qDebug() << QString("%1-%2").arg("DESTRUCTOR SQLTHREAD").arg(mDBNumber);
    mDB = QSqlDatabase::addDatabase("QIBASE");
    QSqlDatabase::removeDatabase(mDBNumber);
}

QVariant &SqlThread::operator[](const QString &name)
{
    return mBindValues[name];
}

void SqlThread::setSqlQuery(const QString &sqlText)
{
    mSqlText = sqlText;
}

void SqlThread::bind(const QString &key, const QVariant &value)
{
    mBindValues[key] = value;
}

void SqlThread::insert(const QString &table, const QString returnId)
{
    QString k, v;
    QStringList keys = mBindValues.keys();
    for (const QString &s: keys) {
        if (!k.isEmpty()) {
            k += ",";
            v += ",";
        }
        k += s.mid(1, s.length() - 1);
        v += s;
    }
    setSqlQuery(QString("insert into %1 (%2) values (%3) %4").arg(table).arg(k).arg(v).arg(returnId.isEmpty() ? "" : QString(" returning %1").arg(returnId)));
    start();
}

void SqlThread::update(const QString &table, int id)
{
    QString vals;
    QStringList keys = mBindValues.keys();
    for (const QString &s: keys) {
        if (!vals.isEmpty()) {
            vals += ",";
        }
        vals += s.mid(1, s.length() - 1) + "=" + s;
    }
    setSqlQuery(QString("update %1 set %2 where id=%3").arg(table).arg(vals).arg(id));
    start();
}

bool SqlThread::nextRow()
{
    if (mCursorPos < rowCount() - 1 && rowCount() > 0) {
        mCursorPos++;
        return true;
    }
    return false;
}

int SqlThread::rowCount()
{
    return mDbRows.count();
}

void SqlThread::run()
{
    mLastError.clear();
    if (!mDB.open()) {
        mLastError = mDB.lastError().databaseText();
        emit error(mLastError);
        return;
    }

    QSqlQuery *q = new QSqlQuery(mDB);

    if (!q->prepare(mSqlText)) {
        mLastError = q->lastError().databaseText();
        emit error(mLastError);
        return;
    }
    for (QMap<QString, QVariant>::const_iterator it = mBindValues.begin(); it != mBindValues.end(); it++) {
        q->bindValue(it.key(), it.value());
    }
    if (!q->exec()) {
        mLastError = q->lastError().databaseText();
        emit error(mLastError);
        return;
    }
    bool isSelect = q->isSelect();
    if (!isSelect) {
        isSelect = mSqlText.mid(0, 4).compare("call", Qt::CaseInsensitive) == 0;
    }

    if (isSelect) {
        mCursorPos = -1;
        mColumns.clear();
        QSqlRecord r = q->record();
        for (int i = 0; i < r.count(); i++) {
            mColumns[r.field(i).name().toLower()] = i;
        }
        int colCount = r.count();
        mDbRows.clear();
        while (q->next()) {
            QList<QVariant> row;
            for (int i = 0; i < colCount; i++) {
                row << q->value(i);
            }
            mDbRows << row;
        }
    }
    qDebug() << lastQuery(q);
    delete q;
    mDB.close();
    moveToThread(qApp->thread());
    emit finished();
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

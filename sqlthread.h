#ifndef SQLTHREAD_H
#define SQLTHREAD_H

#include "threadobject.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QMap>

class SqlThread : public ThreadObject
{
    Q_OBJECT

public:
    SqlThread();
    SqlThread(const QString &dbHost, const QString &dbName, const QString &dbUser, const QString &dbPass, const QString &dbOption);
    void setDatabase(const QString &dbHost, const QString &dbName, const QString &dbUser, const QString &dbPass, const QString &dbOption);
    ~SqlThread();
    QVariant &operator[](const QString &name);
    void setSqlQuery(const QString &sqlText);
    void bind(const QString &key, const QVariant &value);
    void insert(const QString &table, const QString returnId = "id");
    void update(const QString &table, int id);
    bool nextRow();
    int rowCount();
    inline QVariant getValue(const QString &columnName) {return mDbRows.at(mCursorPos).at(mColumns[columnName.toLower()]); }
    inline QString getString (const QString &columnName) { return mDbRows.at(mCursorPos).at(mColumns[columnName.toLower()]).toString(); }
    inline int getInteger (const QString &columnName) { return mDbRows.at(mCursorPos).at(mColumns[columnName.toLower()]).toInt(); }
    inline int getDouble (const QString &columnName) { return mDbRows.at(mCursorPos).at(mColumns[columnName.toLower()]).toDouble(); }
    inline QDate getDate (const QString &columnName) { return mDbRows.at(mCursorPos).at(mColumns[columnName.toLower()]).toDate(); }
    QString mLastError;
    QMap<QString, int> mColumns;
    QList<QList<QVariant> > mDbRows;

public slots:
    void run() override;

private:
    static int mDBCounter;
    QString mDBNumber;
    QSqlDatabase mDB;
    QMap<QString, QVariant> mBindValues;
    QString mSqlText;
    QString lastQuery(QSqlQuery *q) const;
    int mCursorPos;
};

#endif // SQLTHREAD_H

#ifndef SQLTHREAD_H
#define SQLTHREAD_H

#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QMap>

class SqlThread : public QThread
{
    Q_OBJECT

public:
    SqlThread(QObject *parent = nullptr);
    SqlThread(const QString &dbHost, const QString &dbName, const QString &dbUser, const QString &dbPass, const QString &dbOption, QObject *parent = nullptr);
    void setDatabase(const QString &dbHost, const QString &dbName, const QString &dbUser, const QString &dbPass, const QString &dbOption);
    ~SqlThread();
    QVariant &operator[](const QString &name);
    void setSqlQuery(const QString &sqlText);
    void bind(const QString &key, const QVariant &value);
    bool nextRow();
    int rowCount();
    inline QString getString (const QString &columnName) { return fDbRows.at(fCursorPos).at(fColumns[columnName.toLower()]).toString(); }
    inline int getInteger (const QString &columnName) { return fDbRows.at(fCursorPos).at(fColumns[columnName.toLower()]).toInt(); }

protected:
    virtual void run() override;

private:
    static int mDBCounter;
    QString mDBNumber;
    QSqlDatabase mDB;
    QMap<QString, QVariant> fBindValues;
    QString mSqlText;
    QString lastQuery(QSqlQuery *q) const;
    int fCursorPos;
    QList<QList<QVariant> > fDbRows;
    QMap<QString, int> fColumns;

};

#endif // SQLTHREAD_H

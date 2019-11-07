#ifndef PHONEJSONBASE_H
#define PHONEJSONBASE_H

#include "cnfmaindb.h"
#include "sqlthread.h"
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

class PhoneJSONBase : public QObject
{
    Q_OBJECT
public:
    explicit PhoneJSONBase(QObject *parent = nullptr);
    ~PhoneJSONBase();
    virtual void run(const QJsonObject &obj) = 0;
    QByteArray result() const;
    SqlThread mSqlThread;

public slots:
    virtual void queryReady();

protected:
    QJsonObject mObject;

signals:
    void finished();
};

#endif // PHONEJSONBASE_H

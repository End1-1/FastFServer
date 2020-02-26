#ifndef EXCHANGEOBJECT_H
#define EXCHANGEOBJECT_H

#include "threadobject.h"
#include "sqlthread.h"

#define EO_DISCOUNT_ACTION 1

class ExchangeObject : public ThreadObject
{
    Q_OBJECT

public:
    ExchangeObject(int op, const QString &data);

public slots:
    virtual void run() override;
    void threadFinished();

private:
    SqlThread fSqlThread;
    int fOperation;
    QString fData;
};

#endif // EXCHANGEOBJECT_H

#include "exchangeobject.h"
#include "cnfmaindb.h"

ExchangeObject::ExchangeObject(int op, const QString &data) :
    ThreadObject()
{
    fOperation = op;
    fData = data;
    connect(&fSqlThread, SIGNAL(finished()), this, SLOT(threadFinished()));
}

void ExchangeObject::run()
{
    fSqlThread.setDatabase(__cnfmaindb.fHost, __cnfmaindb.fDatabase, __cnfmaindb.fUser, __cnfmaindb.fPassword, "lc_ctype=utf8");
    fSqlThread[":fop"] = fOperation;
    fSqlThread[":fdata"] = fData;
    fSqlThread.setSqlQuery("insert into sys_data_exchange (fcreateat, fop, fdata) values (current_timestamp, :fop, :fdata)");
    fSqlThread.start();
}

void ExchangeObject::threadFinished()
{
    emit finished();
    deleteLater();
}

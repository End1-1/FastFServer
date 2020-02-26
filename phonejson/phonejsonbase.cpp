#include "phonejsonbase.h"
#include "cnfmaindb.h"
#include <QJsonDocument>

PhoneJSONBase::PhoneJSONBase(QObject *parent) :
    QObject(parent)
{
    mSqlThread.setDatabase(__cnfmaindb.fHost, __cnfmaindb.fDatabase, __cnfmaindb.fUser, __cnfmaindb.fPassword, "lc_ctype=utf8");
    connect(&mSqlThread, SIGNAL(finished()), this, SLOT(queryReady()));
}

PhoneJSONBase::~PhoneJSONBase() {

}

QByteArray PhoneJSONBase::result() const
{
    QJsonDocument jdoc(mObject);
    return jdoc.toJson(QJsonDocument::Compact);
}

void PhoneJSONBase::queryReady()
{

}

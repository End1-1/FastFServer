#include "phonejsonreminderstate.h"
#include "sqlthread.h"

PhoneJSONReminderState::PhoneJSONReminderState(QObject *parent) :
    PhoneJSONBase(parent)
{
    mState = 0;
}


void PhoneJSONReminderState::run(const QJsonObject &obj)
{
    mState = obj["state"].toInt();
    mRecord = obj["rec"].toString();
    mSqlThread[":record_id"] = mRecord;
    mSqlThread[":state_id"] = mState;
    QString query;
    switch (obj["state"].toInt()) {
    case 0:
        break;
    case 1:
        query = "update o_dishes_reminder set state_id=:state_id where record_id=:record_id";
        break;
    case 2:
        mSqlThread[":date_start"] = QDateTime::fromString(obj["started"].toString(), "dd.MM.yyyy HH:mm:ss");
        query = "update o_dishes_reminder set date_start=:date_start, state_id=:state_id where record_id=:record_id";
        break;
    case 3:
        mSqlThread[":date_ready"] = QDateTime::fromString(obj["ready"].toString(), "dd.MM.yyyy HH:mm:ss");
        query = "update o_dishes_reminder set date_ready=:date_ready, state_id=:state_id where record_id=:record_id";
        break;
    }
    mSqlThread.setSqlQuery(query);
    mSqlThread.start();
}

void PhoneJSONReminderState::queryReady()
{
    mObject["reply"] = "ok";
    mObject["rec"] = mRecord;
    mObject["state"] = mState;
    emit finished();
}

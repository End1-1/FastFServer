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
        query = "update o_dishes_reminder set state_id=:state_id where record_id=:record_id  and state_id<5 ";
        break;
    case 2:
        if (obj["readyonly"].toString().toInt() == 0) {
            mSqlThread[":date_start"] = QDateTime::currentDateTime();
            query = "update o_dishes_reminder set date_start=:date_start, state_id=:state_id where record_id=:record_id and state_id<5 ";
        } else {
            mSqlThread[":date_start"] = QDateTime::currentDateTime();
            query = "update o_dishes_reminder set date_start=:date_start,  state_id=:state_id where record_id=:record_id and state_id<5 ";
        }
        break;
    case 3:
        mSqlThread[":date_ready"] = QDateTime::currentDateTime();
        query = "update o_dishes_reminder set date_ready=:date_ready, state_id=:state_id where record_id=:record_id and state_id<5 ";
        break;
    case 5:
        mSqlThread[":date_removed"] = QDateTime::currentDateTime();
        query = "update o_dishes_reminder set date_removed=:date_removed, state_id=:state_id where record_id=:record_id";
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

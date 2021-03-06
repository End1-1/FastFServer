#include "phonejsondishreminders.h"
#include "sqlthread.h"

PhoneJSONDishReminders::PhoneJSONDishReminders(QObject *parent) :
    PhoneJSONBase(parent)
{

}

void PhoneJSONDishReminders::run(const QJsonObject &obj)
{
    QString state = obj["first"].toInt() == 1 ? "0,1,2" : "0";
    QString reminder;
    if (obj["reminder"].toString().toInt() > 0) {
        reminder = " and r.reminder_id=" + obj["reminder"].toString();
    }
    mSqlThread.setSqlQuery(QString("select r.record_id,"
    "lpad(extract(hour from cast(r.date_register as time)),2,'0') || ':' || lpad(extract(minute from cast(r.date_register as time)),2,'0') as reg_time, "
    "t.name as table_name, r.state_id, "
    "e.lname || ' ' || e.fname as staff_name, d.name as dish_name, r.qty, "
    "od.comments "
    "from o_dishes_reminder r "
    "inner join h_table t on t.id=r.table_id "
    "inner join employes e on e.id=r.staff_id "
    "inner join me_dishes d on d.id=r.dish_id "
    "inner join o_dishes od on od.id=r.record_id "
    "where r.state_id in (%1) %2 "
    "order by r.id ").arg(state).arg(reminder));

    mSqlThread.start();
}


void PhoneJSONDishReminders::queryReady()
{
    QJsonArray ja;
    while (mSqlThread.nextRow()) {
        QJsonObject jo;
        jo["rec"] = mSqlThread.getString("RECORD_ID");
        jo["time"] = mSqlThread.getString("REG_TIME");
        jo["table"] = mSqlThread.getString("TABLE_NAME");
        jo["staff"] = mSqlThread.getString("STAFF_NAME");
        jo["qty"] = mSqlThread.getString("QTY");
        jo["state"] = mSqlThread.getInteger("STATE_ID");
        jo["dish"] = mSqlThread.getString("DISH_NAME");
        jo["comment"] = mSqlThread.getString("COMMENTS");
        ja.append(jo);
    }
    mObject["dishes"] = ja;
    mObject["reply"] = "ok";
    emit finished();
}

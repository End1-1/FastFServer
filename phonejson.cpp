#include "phonejson.h"
#include "phonejsonbase.h"
#include "phonejsondishreminders.h"
#include "phonejsonreminderstate.h"

static const int cmd_get_reminders = 1;
static const int cmd_reminder_state = 2;

PhoneJson::PhoneJson(const QByteArray &data, QObject *parent) :
    QObject(parent)
{
    QJsonDocument jdoc = QJsonDocument::fromJson(data);
    mObject = jdoc.object();
}

void PhoneJson::go()
{
    int cmd = mObject["c"].toInt();
    PhoneJSONBase *pj = nullptr;
    switch (cmd) {
    case cmd_get_reminders:
        pj = new PhoneJSONDishReminders(this);
        break;
    case cmd_reminder_state:
        pj = new PhoneJSONReminderState(this);
    default:
        break;
    }
    if (pj) {
        connect(pj, SIGNAL(finished()), this, SLOT(jsonFinished()));
        pj->run(mObject);
    } else {
        QByteArray ba;
        ba.append("\"reply]\":\"unknown query\"");
        emit reply(ba);
    }
}

void PhoneJson::jsonFinished()
{
    emit reply(static_cast<PhoneJSONBase*>(sender())->result());
    sender()->deleteLater();
}

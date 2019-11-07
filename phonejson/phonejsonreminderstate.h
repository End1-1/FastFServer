#ifndef PHONEJSONREMINDERSTATE_H
#define PHONEJSONREMINDERSTATE_H

#include "phonejsonbase.h"

class PhoneJSONReminderState : public PhoneJSONBase
{
public:
    PhoneJSONReminderState(QObject *parent = nullptr);
    virtual void run(const QJsonObject &obj) override;

public slots:
    virtual void queryReady() override;

private:
    int mState;
    QString mRecord;
};

#endif // PHONEJSONREMINDERSTATE_H

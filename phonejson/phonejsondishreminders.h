#ifndef PHONEJSONDISHREMINDERS_H
#define PHONEJSONDISHREMINDERS_H

#include "phonejsonbase.h"

class PhoneJSONDishReminders : public PhoneJSONBase
{
    Q_OBJECT
public:
    explicit PhoneJSONDishReminders(QObject *parent = nullptr);
    virtual void run(const QJsonObject &obj) override;

public slots:
    virtual void queryReady() override;
};

#endif // PHONEJSONDISHREMINDERS_H

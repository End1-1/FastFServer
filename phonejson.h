#ifndef PHONEJSON_H
#define PHONEJSON_H

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

class PhoneJson : public QObject
{
    Q_OBJECT
public:
    explicit PhoneJson(const QByteArray &data, QObject *parent = nullptr);
    void go();

private:
    QJsonObject mObject;

private slots:
    void jsonFinished();

signals:
    void reply(const QByteArray &data);
};

#endif // PHONEJSON_H

#ifndef EXCHANGEUPLOAD_H
#define EXCHANGEUPLOAD_H

#include <QObject>
#include <QTimer>

class ExchangeUpload : public QObject
{
    Q_OBJECT
public:
    explicit ExchangeUpload(QObject *parent = nullptr);
    QTimer mTimer;

public slots:
    void timeout();
    void sqlFinished();
    void uploadAmounts();
    void getResponse(const QString &data, bool isError);
    void getAmountResponse(const QString &data, bool isError);
};

#endif // EXCHANGEUPLOAD_H

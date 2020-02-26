#ifndef EXCHANGEDOWNLOAD_H
#define EXCHANGEDOWNLOAD_H

#include <QObject>
#include <QTimer>

class ExchangeDownload : public QObject
{
    Q_OBJECT
public:
    explicit ExchangeDownload(QObject *parent = nullptr);

public slots:
    void timeout();
    void sql1Finished();
    void getResponse(const QString &data, bool isError);

private:
    QTimer mTimer;
};

#endif // EXCHANGEDOWNLOAD_H

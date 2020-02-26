#ifndef TABLELOCKER_H
#define TABLELOCKER_H

#include "threadobject.h"

class QSslServer;

class TableLocker : public ThreadObject
{
    Q_OBJECT

public:
    TableLocker();
    ~TableLocker();

protected slots:
    virtual void run() override;

private:
    QSslServer *fServer;

private slots:
    void newConnection();

signals:
    void logMsg(const QString &s);
};

#endif // TABLELOCKER_H

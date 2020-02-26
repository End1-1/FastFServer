#ifndef PHONESERVERTHREAD_H
#define PHONESERVERTHREAD_H

#include "threadobject.h"
#include <QTcpServer>

class PhoneServerThread : public QObject
{
    Q_OBJECT

public:
    PhoneServerThread();
    ~PhoneServerThread();

private:
    QTcpServer fServer;

private slots:
    void newConnection();
};

#endif // PHONESERVERTHREAD_H

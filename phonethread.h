#ifndef PHONESERVERTHREAD_H
#define PHONESERVERTHREAD_H

#include <QThread>
#include <QTcpServer>

class PhoneServerThread : public QThread
{
    Q_OBJECT

public:
    PhoneServerThread();
    ~PhoneServerThread();

protected:
    virtual void run() override;

private:
    QTcpServer fServer;

private slots:
    void newConnection();
};

#endif // PHONESERVERTHREAD_H

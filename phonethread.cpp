#include "phonethread.h"
#include "phoneclientthread.h"
#include <QNetworkProxy>

PhoneServerThread::PhoneServerThread() :
    QThread()
{
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    fServer.setProxy(QNetworkProxy::NoProxy);
    connect(&fServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
    if (!fServer.listen(QHostAddress::Any, 889)) {
        return;
    }
}

PhoneServerThread::~PhoneServerThread()
{
    fServer.close();
}

void PhoneServerThread::run()
{
    exec();
}

void PhoneServerThread::newConnection()
{
    QTcpSocket *s = fServer.nextPendingConnection();
    PhoneClientThread *pt = new PhoneClientThread(s);
    pt->start();
}

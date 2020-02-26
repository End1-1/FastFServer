#include "threadobject.h"
#include <QThread>

ThreadObject::ThreadObject() :
    QObject()
{
}

ThreadObject::~ThreadObject()
{
}

void ThreadObject::start()
{
    QThread *thread = new QThread();
    connect(thread, SIGNAL(started()), this, SLOT(run()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(this, SIGNAL(finished()), thread, SLOT(quit()));
    connect(this, SIGNAL(finishThread()), thread, SLOT(quit()));
    connect(this, SIGNAL(error(QString)), this, SLOT(finishedWithError(QString)));
    moveToThread(thread);
    thread->start();
}

void ThreadObject::finishedWithError(const QString &err)
{
    Q_UNUSED(err);
    emit finishThread();
}

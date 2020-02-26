#ifndef THREADOBJECT_H
#define THREADOBJECT_H

#include <QObject>

class ThreadObject : public QObject
{
    Q_OBJECT
public:
    explicit ThreadObject();
    ~ThreadObject();
    void start();

protected slots:
    virtual void run() = 0;

private slots:
    void finishedWithError(const QString &err);

signals:
    void finished();
    void finishThread();
    void error(const QString &err);
};

#endif // THREADOBJECT_H
